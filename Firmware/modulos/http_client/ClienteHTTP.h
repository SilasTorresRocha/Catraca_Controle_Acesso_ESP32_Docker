// Firmware/modulos/http_client/ClienteHTTP.h

#ifndef CLIENTE_HTTP_H
#define CLIENTE_HTTP_H

#include <Arduino.h>
#include <EthernetClient.h>
#include <HTTPClient.h> //  HTTPClient para ESP32
#include "../../config/Configuracoes.h"
#include "../utils/Utilidades.h"

#define TAG_HTTP "HTTP_CLIENTE"

// O HTTPClient do ESP32 é projetado para Wi-Fi
// Para usar HTTP sobre Ethernet,percisa de uma gambiarra
// O HTTPClient nativo é o mais comum, porém no entanto, ele usa a camada TCP/IP do ESP32 (Wi-Fi/LwIP)
// Para Ethernet, a maneira mais simples é usar a classe EthernetClient.


EthernetClient cliente_http;

/**
 * @brief Envia uma requisição POST para validar um RA no backend
 * 
 * @param ra O Registro Acadêmico a ser validado
 * @return true se a validação for bem-sucedida (código 200), false caso contrário
 */
bool validar_ra_http(const String& ra) {
    log_info(TAG_HTTP, "Iniciando validação de RA via HTTP...");

    if (!cliente_http.connect(BACKEND_HOST, BACKEND_PORTA)) {
        log_erro(TAG_HTTP, "Falha na conexão com o servidor backend.");
        return false;
    }

    String corpo_requisicao = "{\"ra\": \"" + ra + "\"}";
    
    // Envia a requisição HTTP POST
    cliente_http.print("POST ");
    cliente_http.print(URL_VALIDACAO_RA);
    cliente_http.println(" HTTP/1.1");
    cliente_http.print("Host: ");
    cliente_http.println(BACKEND_HOST);
    cliente_http.println("Content-Type: application/json");
    cliente_http.print("Content-Length: ");
    cliente_http.println(corpo_requisicao.length());
    cliente_http.println("Connection: close");
    cliente_http.println(); // Linha em branco para separar cabeçalhos do corpo
    cliente_http.println(corpo_requisicao);

    // Espera a resposta do servidor
    unsigned long inicio = millis();
    while (cliente_http.connected() && !cliente_http.available() && (millis() - inicio < TEMPO_TIMEOUT_HTTP)) {
        delay(10);
    }

    if (!cliente_http.available()) {
        log_erro(TAG_HTTP, "Timeout ou conexão perdida antes da resposta.");
        cliente_http.stop();
        return false;
    }
    // Lê a linha de status HTTP
    String status_line = cliente_http.readStringUntil('\n');
    log_info(TAG_HTTP, ("Status: " + status_line).c_str());

    if (status_line.indexOf("200 OK") == -1) {
        log_erro(TAG_HTTP, "Validação de RA falhou (Status diferente de 200).");
        cliente_http.stop();
        return false;
    }
    // A função find() procura pela sequência de fim de cabeçalho (\r\n\r\n)
    if (!cliente_http.find("\r\n\r\n")) {
        log_erro(TAG_HTTP, "Erro: Cabeçalhos inválidos ou resposta vazia.");
        cliente_http.stop();
        return false;
    }

    // Ler e Validar JSON
    String corpo_resposta = cliente_http.readString();
    log_info(TAG_HTTP, ("JSON Recebido: " + corpo_resposta).c_str());
    
    cliente_http.stop();

    // Verifica se o backend retornou acesso permitido ou negado, Dependendo do backend A resposta pode mudar então vai os 2 modelo 
    
    if (corpo_resposta.indexOf("\"acesso_permitido\":true") != -1 || 
        corpo_resposta.indexOf("\"acesso_permitido\": true") != -1) {
        
        log_info(TAG_HTTP, "Validação POSITIVA no JSON.");
        return true;
    } 
    else if (corpo_resposta.indexOf("\"acesso_permitido\":false") != -1 || 
             corpo_resposta.indexOf("\"acesso_permitido\": false") != -1) {
        
        log_erro(TAG_HTTP, "Validação NEGATIVA no JSON (Acesso Negado).");
        return false;
    }
    else {
        log_erro(TAG_HTTP, "Erro: JSON malformado ou campo não encontrado.");
        return false;
    }
}

#endif // CLIENTE_HTTP_H
