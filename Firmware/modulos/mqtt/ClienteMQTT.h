// Firmware/modulos/mqtt/ClienteMQTT.h

#ifndef CLIENTE_MQTT_H
#define CLIENTE_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h> 
//include <EthernetClient.h>
#include "../../config/Configuracoes.h"
#include "../utils/Utilidades.h"
#include "../ethernet/EthernetW5500.h" // Para verificar a conexão Ethernet
#include "../Controle_Leds/ControleLEDs.h"

#define TAG_MQTT "MQTT"

// Instâncias globais
EthernetClient eth_cliente;
PubSubClient mqtt_cliente(eth_cliente);

// Variável para armazenar a função de callback de mensagens
typedef void (*CallbackMensagem)(char* topico, byte* payload, unsigned int length);
CallbackMensagem callback_mensagem_global = nullptr;

bool publicar_mqtt(const char* topico, const char* payload);

/**
 * @brief Função de callback para processar mensagens MQTT recebidas
 * 
 * @param topico Tópico da mensagem
 * @param payload Conteúdo da mensagem
 * @param length Tamanho do payload
 */
void callback_mqtt(char* topico, byte* payload, unsigned int length) {
    log_info(TAG_MQTT, "Mensagem recebida.");
    Serial.print(F("  Tópico: "));
    Serial.println(topico);
    Serial.print(F("  Payload: "));
    
    // Cria uma string para o payload (não nula)
    char payload_str[length + 1];
    memcpy(payload_str, payload, length);
    payload_str[length] = '\0';
    Serial.println(payload_str);

    if (callback_mensagem_global) {
        callback_mensagem_global(topico, payload, length);
    }
}

/**
 * @brief Tenta reconectar ao broker MQTT
 */
void reconectar_mqtt() {
    // Loop até estar conectado
    while (!mqtt_cliente.connected()) {
        log_info(TAG_MQTT, "Tentando conexão MQTT...");
        
        // Tenta conectar
        if (mqtt_cliente.connect(MQTT_CLIENTE_ID, MQTT_USUARIO, MQTT_SENHA)) {
            log_info(TAG_MQTT, "Conectado ao broker MQTT.");
            
            // Subscreve aos tópicos
            mqtt_cliente.subscribe(TOPICO_SUBSCRIBE_COMANDO);
            log_info(TAG_MQTT, ("Subscrito em: " + String(TOPICO_SUBSCRIBE_COMANDO)).c_str());
            
            // Publica o status inicial (AGORA ESTA LINHA FUNCIONA)
            publicar_mqtt(TOPICO_PUBLISH_STATUS, "Online");
        } else {
            log_erro(TAG_MQTT, "Falha na conexão MQTT. Código: ");
            Serial.println(mqtt_cliente.state());
            log_info(TAG_MQTT, "Tentando novamente em 5 segundos...");
            delay(5000);
        }
    }
}

/**
 * @brief Inicializa o cliente MQTT
 * 
 * @param callback_func Função a ser chamada quando uma mensagem for recebida
 */
void inicializar_mqtt(CallbackMensagem callback_func) {
    mqtt_cliente.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORTA);
    mqtt_cliente.setCallback(callback_mqtt);
    callback_mensagem_global = callback_func;
    log_info(TAG_MQTT, "Cliente MQTT configurado.");
}

/**
 * @brief Publica uma mensagem em um tópico MQTT
 * 
 * @param topico Tópico para publicar
 * @param payload Conteúdo da mensagem
 * @return true se a publicação foi bem-sucedida, false caso contrário
 */
bool publicar_mqtt(const char* topico, const char* payload) {
    if (mqtt_cliente.connected()) {
        bool sucesso = mqtt_cliente.publish(topico, payload);
        if (sucesso) {
            log_info(TAG_MQTT, ("Publicado em " + String(topico) + ": " + String(payload)).c_str());
        } else {
            log_erro(TAG_MQTT, "Falha na publicação MQTT.");
        }
        return sucesso;
    } else {
        log_erro(TAG_MQTT, "Cliente MQTT não conectado para publicar.");
        return false;
    }
}

/**
 * @brief Mantém a conexão MQTT e processa mensagens
 * 
 * Deve ser chamada no loop principal
 */
void loop_mqtt() {
    if (!ethernet_conectado()) {
        log_erro(TAG_MQTT, "Ethernet desconectada. Não é possível manter MQTT.");
        return;
    }
    
    if (!mqtt_cliente.connected()) {
        aplicarCoresLEDs(padraoSemMQTT);
        reconectar_mqtt();
        if(mqtt_cliente.connected()){
            aplicarCoresLEDs(padraoUTF);
            publicar_mqtt(TOPICO_PUBLISH_STATUS, "Reconectado e Online");
        }
    }
    
    mqtt_cliente.loop();
}

#endif // CLIENTE_MQTT_H
