// Firmware/config/Configuracoes.h

#ifndef CONFIGURACOES_H
#define CONFIGURACOES_H

// --- Configurações de Rede Ethernet W5500 ---
// Endereço MAC do ESP32 (pode ser qualquer um, mas deve ser único na rede)
byte MAC_ENDERECO[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };                  //============== Aqui ====================+

// Configuração de IP Estático (se DHCP não for usado)
// IPAddress IP_ESTATICO(192, 168, 1, 177);
// IPAddress GATEWAY(192, 168, 1, 1);
// IPAddress SUBNET(255, 255, 255, 0);
// IPAddress DNS_SERVER(8, 8, 8, 8);


#define ETHERNET_HOSTNAME "Catraca_STR"

// --- Configurações do Servidor Backend (HTTP) ---

#define BACKEND_HOST "192.168.0.1" // Endereço IPdo servidor                     //============== Aqui ====================+
#define BACKEND_PORTA 8000           // Porta do servidor FastAPI
#define URL_VALIDACAO_RA "/api/v1/validar_ra" // Rota para validação de RA

// --- Configurações do Broker MQTT ---

#define MQTT_BROKER_HOST "192.168.0.1" // Endereço IPdo Broker                      //============== Aqui ====================+
#define MQTT_BROKER_PORTA 1883           // Porta padrão MQTT
#define MQTT_CLIENTE_ID "ESP32" // ID único do cliente MQTT                         //============== Aqui ====================+

// Tópicos MQTT
#define TOPICO_PUBLISH_STATUS "controle_acesso/status"
#define TOPICO_SUBSCRIBE_COMANDO "controle_acesso/comando"
#define TOPICO_PUBLISH_EVENTO "controle_acesso/evento"

// Credenciais MQTT
#define MQTT_USUARIO ""                                                           //============== Aqui ====================+
#define MQTT_SENHA ""                                                             //============== Aqui ====================+









// --- Configurações do Leitor GM81S --- //============== Esses aqui não mexe  ====================+

// Baud Rate para comunicação serial com o leitor
#define GM81S_BAUD_RATE 9600   // Velocidade configurada no GM81S

// --- Outras Configurações ---
#define TEMPO_TIMEOUT_HTTP 15000 // Timeout para requisições HTTP em milissegundos

// Configurações da quantidade de segmentos LED RGB
#define NUM_SEGMENTOS 4  

#endif // CONFIGURACOES_H
