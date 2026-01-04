# Backend_Controle_Acesso/Dockerfile

# Imagem base
FROM python:3.11-slim

# Define o diretório de trabalho
WORKDIR /app

# Copia o arquivo de dependências e instala
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copia o código da aplicação
COPY app/ app/

# Expõe a porta que o Uvicorn irá rodar
EXPOSE 8000

# Comando para rodar a aplicação
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]

