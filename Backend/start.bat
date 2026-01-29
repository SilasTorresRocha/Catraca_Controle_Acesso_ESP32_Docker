@echo off
echo --- 1. Criando Rede ---
docker network create controle_acesso_net 2>NUL

echo --- 2. Limpando versao antiga ---
docker stop mosquitto_broker 2>NUL
docker rm mosquitto_broker 2>NUL
docker stop fastapi_controle_acesso 2>NUL
docker rm fastapi_controle_acesso 2>NUL

echo --- 3. Subindo Mosquitto ---
docker run -d --name mosquitto_broker --network controle_acesso_net --restart always -p 1883:1883 -p 9001:9001 -v "%cd%\mosquitto\config":/mosquitto/config -v "%cd%\mosquitto\data":/mosquitto/data -v "%cd%\mosquitto\log":/mosquitto/log eclipse-mosquitto:2.0

echo --- 4. Construindo Backend ---
docker build -t imagem_backend_catraca .

echo --- 5. Subindo Backend ---
docker run -d --name fastapi_controle_acesso --network controle_acesso_net --restart always -p 8000:8000 --env-file .env -e PYTHONUNBUFFERED=1 -e TZ="America/Sao_Paulo" -v "%cd%\app\recursos":/app/app/recursos imagem_backend_catraca

echo.
echo === TUDO RODANDO ===
echo Logs: docker logs -f fastapi_controle_acesso
pause