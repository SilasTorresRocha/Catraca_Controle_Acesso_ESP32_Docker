# Backend/app/servicos/mqtt_service.py
import paho.mqtt.client as mqtt
import json
import os

# Configurações do Broker via Variáveis de Ambiente
BROKER = os.getenv("MQTT_BROKER_HOST", "mosquitto_broker")
PORT = int(os.getenv("MQTT_PORT", 1883))

# Tópicos
TOPICO_EVENTOS = "controle_acesso/evento"
TOPICO_COMANDO = "controle_acesso/comando"
TOPICO_DIAGNOSTICO = "controle_acesso/diagnostico" 
TOPICO_STATUS = "controle_acesso/status"

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Conectado ao Broker!")
        client.subscribe([(TOPICO_EVENTOS, 0), (TOPICO_DIAGNOSTICO, 0), (TOPICO_STATUS, 0)])
        print(f"[MQTT] Inscrito em: {TOPICO_EVENTOS} e {TOPICO_DIAGNOSTICO}")
    else:
        print(f"[MQTT] Falha na conexão. Código: {rc}")

def on_message(client, userdata, msg):
    payload_str = msg.payload.decode()
    
    # Auto Diagnóstico
    if msg.topic == TOPICO_DIAGNOSTICO:
        try:
            dados = json.loads(payload_str)
            status_geral = dados.get("status", "DESCONHECIDO")

            if status_geral == "ERRO":
                print("\n" + "="*40)
                print(f"[ALERTA CRÍTICO] FALHA NA CATRACA!")
                print(f" -> Status: {status_geral}")
                print(f" -> Leitor GM81S: {dados.get('gm81s')}")
                print(f" -> Botão Saída: {dados.get('botao_saida')}")
                print(f" -> Sensor PIR: {dados.get('pir')}")
                ind1 = dados.get('indutor_1', 'ND')
                ind2 = dados.get('indutor_2', 'ND')
                print(f" -> Indutores: 1={ind1} | 2={ind2}")
                print(f" -> Uptime: {dados.get('uptime_h')} horas")
                print("="*40 + "\n")
            else:
                # Log de rotina apenas para saber que está vivo
                print(f"[DIAGNOSTICO] Sistema OK | Uptime: {dados.get('uptime_h')}h | Leitor: {dados.get('gm81s')}")
                
        except json.JSONDecodeError:
            print(f"[ERRO JSON] Falha ao ler diagnóstico: {payload_str}")

    #Eventos Gerais
    else:
        print(f"[LOG ESP32] Tópico: {msg.topic} | Mensagem: {payload_str}")

def iniciar_mqtt():
    try:
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(BROKER, PORT, 60)
        client.loop_start() 
    except Exception as e:
        print(f"[ERRO MQTT] Não foi possível conectar: {e}")

def enviar_comando_abrir(): 
    client.publish(TOPICO_COMANDO, "ABRIR_SOLENOIDE")
    print("[MQTT] Comando de abertura enviado")

def enviar_comando_reinicio():
    """
    Envia comando para o ESP32 reiniciar
    """
    if client.is_connected():
        client.publish(TOPICO_COMANDO, "REINICIAR")
        print("[MQTT] Comando REINICIAR enviado para a catraca.")
        return True
    else:
        print("[MQTT] FALHA: Não conectado. Comando REINICIAR não enviado.")
        return False