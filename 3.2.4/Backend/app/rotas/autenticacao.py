# Backend/app/rotas/autenticacao.py
import os
from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel
from ..servicos.validacao_ra import validar_ra, atualizar_restricao, atualizar_horarios, atualizar_bloqueio_total
from ..servicos.mqtt_service import enviar_comando_abrir

router = APIRouter()

# Tenta pegar do .env, se não achar, usa um valor padrão
SENHA_MESTRA = os.getenv("SENHA_MESTRA", "Tessera_fallback_inseguro")   #Tessera era usada pelos romanos para indicar uma senha militar

# Modelos de Dados (Payloads)
class ItemValidacaoRA(BaseModel):
    ra: str

class ComandoSeguro(BaseModel):
    senha: str  # Obrigatório enviar a senha

class ConfigHorario(BaseModel):
    senha: str
    inicio: int
    fim: int

class ConfigEstado(BaseModel):
    senha: str
    ativo: bool

# Rota Padrão o ESP32 usa esta sem senha pois é automática
@router.post("/validar_ra")
async def rota_validar_ra(item: ItemValidacaoRA):
    resultado = validar_ra(item.ra)
    # Mantém compatibilidade com o ESP32 retornando booleanos e strings simples
    return {"mensagem": resultado["motivo"], "acesso_permitido": resultado["valido"]}



# --- Rotas Administrativas --- (Exigem Senha)

@router.post("/comando/abrir")
async def abrir_remotamente(comando: ComandoSeguro):
    """
    Envia comando MQTT para abrir a catraca. Requer senha mestra.
    """
    if comando.senha != SENHA_MESTRA:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Senha incorreta. Acesso negado."
        )
    
    enviar_comando_abrir()
    return {"mensagem": "Comando de abertura enviado com sucesso via MQTT"}

@router.post("/config/horario/definir")
async def definir_horarios(config: ConfigHorario):
    """
    Define o intervalo de horas permitido.
    """
    if config.senha != SENHA_MESTRA:
        raise HTTPException(status_code=401, detail="Senha incorreta")
    
    if config.inicio >= config.fim:
        raise HTTPException(status_code=400, detail="Horário de inicio deve ser menor que o fim")

    atualizar_horarios(config.inicio, config.fim)
    return {"mensagem": f"Horários atualizados: {config.inicio}h às {config.fim}h"}

@router.post("/config/horario/estado")
async def definir_estado_restricao(config: ConfigEstado):
    """
    Ativa ou Desativa a restrição de horários globalmente.
    """
    if config.senha != SENHA_MESTRA:
        raise HTTPException(status_code=401, detail="Senha incorreta")

    atualizar_restricao(config.ativo)
    status_msg = "ATIVADA" if config.ativo else "DESATIVADA"
    return {"mensagem": f"Restrição de horário {status_msg}"}


@router.post("/config/bloqueio/total")
async def definir_bloqueio_total(config: ConfigEstado):
    """
    Lockdown total da catraca.  (Ideal para manutenção no campus, ou em eleições).
    Se ativo = true, NINGUÉM entra (nem RA válido, nem no horário certo).
    """
    if config.senha != SENHA_MESTRA:
        raise HTTPException(status_code=401, detail="Senha incorreta.")

    atualizar_bloqueio_total(config.ativo)
    
    status_msg = "BLOQUEIO TOTAL ATIVADO (Ninguém entra)" if config.ativo else "Bloqueio Total REMOVIDO (Operação normal)"
    return {"mensagem": status_msg}