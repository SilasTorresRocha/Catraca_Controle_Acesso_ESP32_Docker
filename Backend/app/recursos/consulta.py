#Backend/app/recursos/consultar.py

#Simula a implementação de consulta de RA no LDAP
import sys
import time
import random
import os

#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!
#ESTE ARQUIVO É APENAS PARA TESTES. NÃO USAR EM PRODUÇÃO!

def main():
    if len(sys.argv) < 2:
        print(0)
        return

    ra_consultado = sys.argv[1].strip()
    #Simula tempo de consulta de uns 5 segundos
    time.sleep(5)

    diretorio_atual = os.path.dirname(os.path.abspath(__file__))
    caminho_arquivo = os.path.join(diretorio_atual, "ra_validos.txt")

    ras_validos = set()
    if os.path.exists(caminho_arquivo):
        try:
            with open(caminho_arquivo, 'r') as f:
                ras_validos = {linha.strip() for linha in f if linha.strip()}
        except Exception:
            print(0)
            return
    else:
        print(0)
        return
    if ra_consultado in ras_validos:
        codigo_retorno = random.randint(1, 999)
        print(codigo_retorno)
    else:
        print(0)







#---------------------Funcao para integrar com o codigo---------------------#]
def verificar_ra(ra: str) -> bool:
    """
    Entrada: RA (str)
    Saída: True se válido, False se inválido
    """
    if not ra or not ra.strip():
        print("[ERRO] RA vazio ou nulo.")
        return False
    return random.randint(0, 1)

if __name__ == "__main__":
    main()