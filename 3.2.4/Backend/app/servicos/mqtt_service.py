# Backend/app/servicos/mqtt_service.py
import paho.mqtt.client as mqtt
import json
import os

# Configurações do Broker, mesmos nomes do docker-compose
# Configurações do Broker via Variáveis de Ambiente
BROKER = os.getenv("MQTT_BROKER_HOST", "mosquitto_broker")
PORT = int(os.getenv("MQTT_PORT", 1883)) # Converte pois env vem como string

TOPICO_EVENTOS = "controle_acesso/evento"
TOPICO_COMANDO = "controle_acesso/comando"

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Conectado ao Broker!")
        client.subscribe(TOPICO_EVENTOS)
    else:
        print(f"[MQTT] Falha na conexão. Código: {rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"[LOG ESP32] Tópico: {msg.topic} | Mensagem: {payload}")
    # Logs em tempo real no terminal do Docker

def iniciar_mqtt():
    try:
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(BROKER, PORT, 60)
        client.loop_start() # Roda em uma thread separada
    except Exception as e:
        print(f"[ERRO MQTT] Não foi possível conectar: {e}")

def enviar_comando_abrir(): 
    client.publish(TOPICO_COMANDO, "ABRIR_SOLENOIDE")
    print("[MQTT] Comando de abertura enviado")