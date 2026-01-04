// Firmware_Controle_Acesso/modulos/ethernet/EthernetW5500.h

#ifndef ETHERNET_W5500_H
#define ETHERNET_W5500_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet2.h> // Requer a biblioteca Ethernet (compatível com W5500)
#include "../../config/MapeamentoPinos.h"
#include "../../config/Configuracoes.h"
#include "../utils/Utilidades.h"

#define TAG_ETH "ETHERNET"

// Variável global para o endereço IP
IPAddress endereco_ip;

/**
 * @brief Inicializa o módulo Ethernet W5500
 * 
 * Tenta obter um endereço IP via DHCP
 */
void inicializar_ethernet() {
    log_info(TAG_ETH, "Inicializando Ethernet W5500...");
    //======================reset do W5500======================//
    pinMode(PINO_W5500_RST, OUTPUT);
    digitalWrite(PINO_W5500_RST, LOW);
    delay(50);
    digitalWrite(PINO_W5500_RST, HIGH);
    delay(50);
    //=========================================================// 

    // Configura o pino CS do W5500
    Ethernet.init(PINO_SPI_CS);

    // Comunicação SPI
    SPI.begin(PINO_SPI_CLK, PINO_SPI_MISO, PINO_SPI_MOSI);

    // Tenta configurar a rede via DHCP
    if (Ethernet.begin(MAC_ENDERECO) == 0) {
        log_erro(TAG_ETH, "Falha ao configurar Ethernet usando DHCP.");
        
    }

    endereco_ip = Ethernet.localIP();
    String ip_str = "IP Obtido: " + String(endereco_ip[0]) + "." + String(endereco_ip[1]) + "." + String(endereco_ip[2]) + "." + String(endereco_ip[3]);
    log_info(TAG_ETH, ip_str.c_str());
}

/**
 * @brief Verifica se a conexão Ethernet está ativa
 * 
 * @return true se conectado, false caso contrário
 */
bool ethernet_conectado() {
    // Uma forma confiável de verificar a conexão seria vendo se tem um IP válido
    // Um IP 0.0.0.0 indica que a conexão foi de F
    IPAddress ip = Ethernet.localIP();
    
    // O operador '==' para IPAddress verifica se todos os octetos são iguais
    // (0,0,0,0) é o endereço padrão para "sem IP"
    if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
        log_erro(TAG_ETH, "Conexão perdida (IP: 0.0.0.0).");
        return false;
    }
    
    return true;
}

/**
 * @brief Retorna o endereço IP local
 * 
 * @return IPAddress do Sistema
 */
IPAddress obter_ip_local() {
    return endereco_ip;
}

#endif // ETHERNET_W5500_H
