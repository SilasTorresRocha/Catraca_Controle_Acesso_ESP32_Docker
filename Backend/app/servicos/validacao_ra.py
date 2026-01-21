# Backend/app/servicos/validacao_ra.py

import os
import json
import subprocess
import sys
from datetime import datetime

# Caminhos de arquivos
DIRETORIO_ATUAL = os.path.dirname(__file__)
ARQUIVO_CONFIG = os.path.join(DIRETORIO_ATUAL, "../recursos/config_acesso.json")
CONSULTA_SCRIPT = os.path.abspath(os.path.join(DIRETORIO_ATUAL, "../recursos/consulta.py"))


# Configuração Padrão (caso o arquivo não exista)
configuracao_atual = {
    "bloqueio_total": False,  # Controle de lockdown (Aplica apenas para entrada, a saída sempre permitida)
    "restricao_ativa": False, # Inicialmente desativada
    "horario_inicio": 7,
    "horario_fim": 22,
    # Config padrão de Reinício 
    "reinicio_agendado": {
        "ativo": True,          # Padrão: Ativo
        "hora_base": 3,         # 3 da manhã
        "ruido_minutos": 90     # +/- 90 minutos de aleatoriedade
    }
}

def carregar_configuracao():
    """Carrega configurações de horário do JSON (Persistência)"""
    global configuracao_atual
    try:
        if os.path.exists(ARQUIVO_CONFIG):
            with open(ARQUIVO_CONFIG, 'r') as f:
                dados = json.load(f)
                # Atualiza chaves existentes
                configuracao_atual.update(dados)
            print(f"[SERVICO] Configurações carregadas: {configuracao_atual}")
        else:
            print("[SERVICO] Arquivo de config não encontrado. Usando padrão.")
            salvar_configuracao() # cria o arquivo pela primeira vez
    except Exception as e:
        print(f"[ERRO] Falha ao carregar config: {e}")

def salvar_configuracao():
    """Salva a configuração atual no disco."""
    try:
        with open(ARQUIVO_CONFIG, 'w') as f:
            json.dump(configuracao_atual, f, indent=4)
        print("[SERVICO] Configurações salvas no disco.")
    except Exception as e:
        print(f"[ERRO] Não foi possível salvar config: {e}")


# ==== Integração com o script externo de consulta ====

def consultar_ra_externo(ra: str, timeout: int = 10) -> bool:
    """
    Executa o script Python externo para validar o RA.
    Retorna True se o RA for válido (código > 0).
    """
    ra = str(int(ra)) # Garante que é um número válido sem zeros à esquerda (assim no cracha tem um 0 sei la pq motivo mas no banco nao tem entao ... conveter para inteiro e volta para string)
    try:
        cmd = [sys.executable, CONSULTA_SCRIPT, ra]
        resultado = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        saida = resultado.stdout.strip()
        if not saida:
            return False
        # o script retorna um número >0 quando encontra o RA
        try:
            codigo = int(saida.split()[0])
            return codigo != 0
        except Exception:
            return False
    except subprocess.TimeoutExpired:
        print(f"[ERRO] Timeout na consulta do RA {ra}")
        return False
    except Exception as e:
        print(f"[ERRO] Falha na consulta externa: {e}")
        return False

# Inicialização ( Apenas carrega os dados) (Acho que esse codigo tem muito coisa que nao e validar ra kkkkkk)
carregar_configuracao()

def validar_ra(ra: str) -> dict:
    # lockdown
    if configuracao_atual.get("bloqueio_total", False):
        print(f"[BLOQUEIO TOTAL] Tentativa de acesso rejeitada: {ra} (Motivo: Bloqueado pelo adminstrador)")
        return {"valido": False, "motivo": "Acesso BLOQUEADO TEMPORARIAMENTE "}
    
    #Verifica RA
    if not consultar_ra_externo(ra):
        return {"valido": False, "motivo": "RA nao encontrado"}

    # Verifica Horário, usando a config carregada
    if configuracao_atual["restricao_ativa"]:
        hora_agora = datetime.now().hour       #Se algum momento estiver dando inconsistencia de horario, olha o docker, ajsutei o fuso horario la
        inicio = configuracao_atual["horario_inicio"]
        fim = configuracao_atual["horario_fim"]
        
        if hora_agora < inicio or hora_agora >= fim:
            print(f"[BLOQUEIO] RA {ra} fora do horário ({hora_agora}h).")
            return {"valido": False, "motivo": "Fora do horario permitido"}

    return {"valido": True, "motivo": "Acesso permitido"}

# funções de Controle para a API

def atualizar_bloqueio_total(ativo: bool):
    configuracao_atual["bloqueio_total"] = ativo
    salvar_configuracao()

def atualizar_restricao(ativa: bool):
    configuracao_atual["restricao_ativa"] = ativa
    salvar_configuracao() 

def atualizar_horarios(inicio: int, fim: int):
    # Validação simples
    if not (0 <= inicio <= 23):
        raise ValueError("Hora de início inválida (0–23).")
    if not (0 <= fim <= 23):
        raise ValueError("Hora de fim inválida (0–23).")
    
    configuracao_atual["horario_inicio"] = inicio
    configuracao_atual["horario_fim"] = fim
    salvar_configuracao()