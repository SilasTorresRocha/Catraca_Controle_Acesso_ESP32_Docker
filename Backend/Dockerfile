FROM python:3.11-slim

# Define variáveis de ambiente PADRÃO (podem ser sobrescritas no run)
ENV PYTHONUNBUFFERED=1
ENV TZ="America/Sao_Paulo"

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY app/ app/

EXPOSE 8000

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
