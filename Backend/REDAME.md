
docker-compose up --build

docker-compose down




docker rm -f fastapi_controle_acesso


docker-compose up --build --remove-orphans



# 1. Derruba tudo e remove volumes órfãos
docker-compose down --remove-orphans

# 2. Garante que o container antigo morreu mesmo
docker rm -f fastapi_controle_acesso

# 3. Reconstrói e sobe com os logs liberados
docker-compose up --build







# 0. Sem

docker logs -f fastapi_controle_acesso

####



No kubenet isso deve funcinar :

# Isso deve o log de qualquer que tenha o apelido 
kubectl logs -f -l app=fastapi-backend



kubectl get pods --show-labels





