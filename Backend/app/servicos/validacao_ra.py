# Backend/app/servicos/validacao_ra.py

import os
import json
import subprocess
import sys
from datetime import datetime
import importlib.util

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
    print(f"\n--- [DEBUG INICIO] Iniciando validação via Subprocesso para RA: {ra} ---")
    
    # 1. Validação de Caminho e Arquivo
    if not os.path.exists(CONSULTA_SCRIPT):
        print(f"[ERRO FATAL] O arquivo do script NÃO EXISTE no caminho: {CONSULTA_SCRIPT}")
        print(f" -> Diretório atual de execução (getcwd): {os.getcwd()}")
        print(f" -> Conteúdo da pasta recursos esperado em: {os.path.dirname(CONSULTA_SCRIPT)}")
        try:
            print(f" -> Arquivos encontrados lá: {os.listdir(os.path.dirname(CONSULTA_SCRIPT))}")
        except Exception as e:
            print(f" -> Não foi possível listar diretório: {e}")
        return False

    # 2. Validação de Permissões
    if not os.access(CONSULTA_SCRIPT, os.R_OK):
        print(f"[ERRO PERMISSÃO] O script existe, mas o Python NÃO TEM permissão de LEITURA.")
        return False

    # 3.Comando
    cmd = [sys.executable, CONSULTA_SCRIPT, ra]
    print(f"[DEBUG] Comando montado: {cmd}")
    print(f"[DEBUG] Executável Python: {sys.executable}")

    try:
        inicio = datetime.now()
        resultado = subprocess.run(
            cmd, 
            capture_output=True, 
            text=True, 
            timeout=timeout
        )
        tempo_gasto = (datetime.now() - inicio).total_seconds()
        print(f"[DEBUG] Tempo de execução: {tempo_gasto} segundos")

        # 4. Análise do Código de Saída 
        if resultado.returncode != 0:
            print(f"[ERRO CRÍTICO] O script rodou mas falhou (Exit Code {resultado.returncode})!")
            print(f" -> STDERR (Erros): {resultado.stderr}")
            print(f" -> STDOUT (Saída): {resultado.stdout}")
            return False

        # 4. Análise da Saída (STDOUT)
        saida = resultado.stdout.strip()
        print(f"[DEBUG] STDOUT Bruto: '{saida}'")
        
        if not saida:
            print("[ALERTA] O script terminou com sucesso (code 0) mas NÃO RETORNOU NADA.")
            if resultado.stderr:
                print(f" -> Havia algo no STDERR (Warnings?): {resultado.stderr}")
            return False

        # 5. Conversão da Resposta
        try:
            # Pega apenas a primeira palavra/número
            codigo = int(saida.split()[0])
            print(f"[DEBUG] Código extraído com sucesso: {codigo}")
            
            valido = (codigo > 0)
            print(f"--- [DEBUG FIM] Resultado final: {'PERMITIDO' if valido else 'NEGADO'} ---\n")
            return valido

        except ValueError:
            print(f"[ERRO DADOS] O script retornou algo que não é um número inteiro: '{saida}'")
            return False

    except subprocess.TimeoutExpired:
        print(f"[ERRO TIMEOUT] O script demorou mais que {timeout}s e foi abortado.")
        return False
    except Exception as e:
        print(f"[ERRO GERAL] Exceção não tratada ao tentar rodar o script: {e}")
        return False
    









# ==== Tentativa de caregar todo o módulo ====

# ==============================================================================
# Requer que adicione a função `consultar_ponte(ra)` no consulta.py
# ==============================================================================

def consultar_ra_modulo(ra: str) -> bool:
    print(f"\n--- [MODULO] Tentando validar RA {ra} via importação direta ---")
    
    try:
        # Tenta importar o arquivo consulta.py dinamicamente
        # Isso deve funcionar mesmo que não esteja no diretório padrão
        spec = importlib.util.spec_from_file_location("modulo_consulta", CONSULTA_SCRIPT)
        if spec is None:
            print(f"[ERRO] Não foi possível encontrar o módulo em: {CONSULTA_SCRIPT}")
            return False
            
        modulo_consulta = importlib.util.module_from_spec(spec)
        sys.modules["modulo_consulta"] = modulo_consulta
        spec.loader.exec_module(modulo_consulta)
        
        # Verifica se a função 'ponte' existe no arquivo
        #  def verificar_ra(ra): ...
        if hasattr(modulo_consulta, "verificar_ra"):
            resultado = modulo_consulta.verificar_ra(ra)
            print(f"[MODULO] Função 'verificar_ra' retornou: {resultado}")
            return resultado
            
        else:
            print("[ERRO] A função 'verificar_ra' não existe no script consulta.py.")
            print(" -> Criar: def verificar_ra(ra): return codigo 0/1")
            return False

    except Exception as e:
        print(f"[ERRO MODULO] Falha ao importar ou executar módulo: {e}")
        return False




#=============================================================================================




# Inicialização ( Apenas carrega os dados) (Acho que esse codigo tem muito coisa que nao e validar ra kkkkkk)
carregar_configuracao()

def validar_ra(ra: str) -> dict:
    # lockdown
    if configuracao_atual.get("bloqueio_total", False):
        print(f"[BLOQUEIO TOTAL] Tentativa de acesso rejeitada: {ra} (Motivo: Bloqueado pelo adminstrador)")
        return {"valido": False, "motivo": "Acesso BLOQUEADO TEMPORARIAMENTE", "RA": ra}
    
    #Verifica RA

    #1.  Metodo usando subprocess para chamar o script externo
    
    #if not consultar_ra_externo(ra):
    #    return {"valido": False, "motivo": "RA nao encontrado", "RA": ra}

    #2.  Metodo direto
    if not consultar_ra_modulo(ra):
         return {"valido": False, "motivo": "RA nao encontrado", "RA": ra}



    # Verifica Horário, usando a config carregada
    if configuracao_atual["restricao_ativa"]:
        hora_agora = datetime.now().hour       #Se algum momento estiver dando inconsistencia de horario, olha o docker, ajsutei o fuso horario la
        inicio = configuracao_atual["horario_inicio"]
        fim = configuracao_atual["horario_fim"]
        
        if hora_agora < inicio or hora_agora >= fim:
            print(f"[BLOQUEIO] RA {ra} fora do horário ({hora_agora}h).")
            return {"valido": False, "motivo": "Fora do horario permitido", "RA": ra}

    return {"valido": True, "motivo": "Acesso permitido", "RA": ra}

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