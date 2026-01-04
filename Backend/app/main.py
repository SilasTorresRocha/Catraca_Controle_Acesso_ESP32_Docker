# Backend_Controle_Acesso/app/main.py

from fastapi import FastAPI, Request
#from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles 
from starlette.exceptions import HTTPException as StarletteHTTPException 
from .rotas import autenticacao
from .servicos.mqtt_service import iniciar_mqtt


app = FastAPI(
    title="Servidor de Controle de Acesso Catraca",
    description="Backend para validação de RAs e controle de acesso. Com integração com MQTT.",
    version="3.2.1"
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

# Se não iniciar fica difícil
@app.on_event("startup")
async def startup_event():
    print("[SYSTEM] Inicializando serviços...")
    iniciar_mqtt()

#Rotas de autenticação
app.include_router(autenticacao.router, prefix="/api/v1", tags=["Autenticação"])

@app.get("/")
async def root():
    return {"mensagem": "Servidor de Controle de Acesso está rodando."}


