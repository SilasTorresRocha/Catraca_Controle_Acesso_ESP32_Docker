
# 1. Criar uma rede networks: controle_acesso_net
echo "--- Criando Rede Docker ---"
docker network create controle_acesso_net || true

# 2. Rodar o Mosquitto (Broker MQTT)
echo "--- Iniciando Mosquitto ---"
docker stop mosquitto_broker || true
docker rm mosquitto_broker || true

docker run -d \
  --name mosquitto_broker \
  --network controle_acesso_net \
  --restart always \
  -p 1883:1883 \
  -p 9001:9001 \
  -v "$(pwd)/mosquitto/config:/mosquitto/config" \
  -v "$(pwd)/mosquitto/data:/mosquitto/data" \
  -v "$(pwd)/mosquitto/log:/mosquitto/log" \
  eclipse-mosquitto:2.0

# 3. Construir a Imagem do Backend
echo "--- Construindo Backend ---"
docker build -t imagem_backend_catraca .

# 4. Rodar o Backend
# env_file, environment, volumes e portas
echo "--- Iniciando Backend ---"
docker stop fastapi_controle_acesso || true
docker rm fastapi_controle_acesso || true

docker run -d \
  --name fastapi_controle_acesso \
  --network controle_acesso_net \
  --restart always \
  -p 8000:8000 \
  --env-file .env \
  -e PYTHONUNBUFFERED=1 \
  -e TZ="America/Sao_Paulo" \
  -v "$(pwd)/app/recursos:/app/app/recursos" \
  imagem_backend_catraca

echo "=== Sistema Rodando ==="
echo "Backend: http://localhost:8000"
echo "MQTT: localhost:1883"