# Backend_Controle_Acesso/app/main.py

from fastapi import FastAPI, Request
#from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles 
from starlette.exceptions import HTTPException as StarletteHTTPException 
from .rotas import autenticacao
from .servicos.mqtt_service import iniciar_mqtt, enviar_comando_reinicio
import asyncio
from datetime import datetime, timedelta
import json
import random

app = FastAPI(
    title="Servidor de Controle de Acesso Catraca",
    description="Backend para validação de RAs e controle de acesso. Com integração com MQTT.",
    version="3.4.0"
)

app.mount("/static", StaticFiles(directory="app/static"), name="static")
templates = Jinja2Templates(directory="app/templates")

@app.exception_handler(404)
async def custom_404_handler(request: Request, exc: StarletteHTTPException):
    return templates.TemplateResponse(
        "404.html", 
        {"request": request}, 
        status_code=404
    )


# ==== AGENDADOR DE REINÍCIO ====
ARQUIVO_CONFIG = "app/recursos/config_acesso.json"
proximo_reinicio = None

def calcular_proximo_reinicio():
    """Lê a config e define uma data/hora aleatória para o reinício"""
    try:
        cfg_reinicio = {"ativo": False} 
        try:
            with open(ARQUIVO_CONFIG, 'r') as f:
                config = json.load(f)
                cfg_reinicio = config.get("reinicio_agendado", {"ativo": False})
        except FileNotFoundError:
            pass 

        if not cfg_reinicio.get("ativo", False):
            print("[AGENDADOR] Reinício agendado está DESATIVADO.")
            return None

        hora_base = cfg_reinicio.get("hora_base", 3)
        ruido = cfg_reinicio.get("ruido_minutos", 60)
        agora = datetime.now()
        data_base = agora.replace(hour=hora_base, minute=0, second=0, microsecond=0)
        
        # Aplica o ruído aleatório
        minutos_random = random.randint(-ruido, ruido)
        data_alvo = data_base + timedelta(minutes=minutos_random)

        if data_alvo <= agora:
            data_base_amanha = data_base + timedelta(days=1)
            # Sorteia um novo ruído para amanhã para ser imprevisível
            minutos_random = random.randint(-ruido, ruido)
            data_alvo = data_base_amanha + timedelta(minutes=minutos_random)

        print(f"[AGENDADOR] Próximo reinício agendado para: {data_alvo.strftime('%d/%m %H:%M')}")
        return data_alvo

    except Exception as e:
        print(f"[AGENDADOR] Erro ao calcular data: {e}")
        return None

async def loop_agendador():
    """Processo em background que verifica o horário"""
    global proximo_reinicio
    # Espera 30s iniciais para garantir que MQTT e arquivos carregaram (não remover)
    await asyncio.sleep(30) 
    proximo_reinicio = calcular_proximo_reinicio()

    while True:
        try:
            if proximo_reinicio:
                agora = datetime.now()
                if agora >= proximo_reinicio:
                    print(f"[AGENDADOR] Executando Reinício Programado... ({agora})")
                    sucesso = enviar_comando_reinicio()
                    if sucesso:
                        # Espera 2 minutos para garantir que não envia comando duplicado
                        await asyncio.sleep(120) 
                    proximo_reinicio = calcular_proximo_reinicio()
            await asyncio.sleep(60)
            
        except Exception as e:
            print(f"[AGENDADOR] Erro no loop: {e}")
            await asyncio.sleep(60)

# Se não iniciar fica difícil
@app.on_event("startup")
async def startup_event():
    print("[SYSTEM] Inicializando serviços...")
    iniciar_mqtt()
    # Inicia a tarefa do agendador em paralelo 
    asyncio.create_task(loop_agendador())

#Rotas de autenticação
app.include_router(autenticacao.router, prefix="/api/v1", tags=["Autenticação"])

@app.get("/")
async def root():
    return {"mensagem": "Servidor de Controle de Acesso está rodando."}


