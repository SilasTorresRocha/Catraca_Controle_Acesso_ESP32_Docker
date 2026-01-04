// Firmware_Controle_Acesso/modulos/leitor_GM81S/LeitorGM81S.h
/*
   LeitorGM81S.h - Biblioteca para comunicação com o leitor de cartão GM81S via UART.
   Criado por: Silas Torres Rocha, 24/10/2025
   Última modificação: 13/12/2025 (Mudei depois porém não lembro quando kkkk)
   Versão: 1.0.2
   Licença: MIT
   Conofig: UART 9600 bps, delay 5000 ms(padrao) , Induction Modde (pg 23 pdf). 
*/

#ifndef LEITOR_GM81S_H
#define LEITOR_GM81S_H

#include <Arduino.h>
#include "../../config/MapeamentoPinos.h"
#include "../../config/Configuracoes.h"
#include "../utils/Utilidades.h"

#define TAG_GM81S "GM81S"

HardwareSerial SerialLeitor(1); // Usando UART 1

/**
 * @brief Inicializa a comunicação serial com o leitor GM81S
 */
void inicializar_leitor_gm81s() {
    SerialLeitor.begin(GM81S_BAUD_RATE, SERIAL_8N1, PINO_UART_RX, PINO_UART_TX);
    log_info(TAG_GM81S, ("Serial do leitor inicializada em " + String(GM81S_BAUD_RATE) + " bps.").c_str());
}

/**
 * @brief Verifica se há dados disponíveis para leitura do leitor.
 * 
 * @return true se houver dados, false caso contrário.
 */
bool dados_disponiveis_gm81s() {
    return SerialLeitor.available() > 0;
}

/**
 * @brief Lê uma linha de dados do leitor.
 * 
 * @return String contendo a linha lida.
 */
String ler_linha_gm81s() {
    if (dados_disponiveis_gm81s()) {
        String linha = SerialLeitor.readStringUntil('\n');
        linha.trim(); // Remove espaços em branco e quebras de linha
        log_info(TAG_GM81S, ("Dado lido: " + linha).c_str());
        return linha;
    }
    return "";
}

/**
 * @brief Envia comando para o leitor
 * 
 * @param comando O comando a ser enviado.
 */
void enviar_comando_gm81s(const String& comando) {
    SerialLeitor.println(comando);
    log_info(TAG_GM81S, ("Comando enviado: " + comando).c_str());
}

/**
 * @brief Esvazia o buffer de recepção serial.
 * Útil para descartar leituras acumuladas durante processamentos longos
 */
void limpar_buffer_gm81s() {
    // Enquanto houver dados na fila, vai para o lixo
    while (SerialLeitor.available() > 0) {
        SerialLeitor.read();
    }
}

/**
 * @brief Envia um comando HEX para o GM81S e verifica se ele responde corretamente.
 * Comando enviado: 7E 00 0A 01 00 00 00 30 1A
 * Resposta esperada: 03 00 00 01 00 33 31
 * @return true se resposta correta, false se timeout ou resposta inválida.
 */
bool diagnostico_gm81s() {
    limpar_buffer_gm81s();
    // Segundo o manual na paguina 16 do PDF ou 10 do doc 
    byte comando[] = {0x7E, 0x00, 0x0A, 0x01, 0x00, 0x00, 0x00, 0x30, 0x1A};
    SerialLeitor.write(comando, sizeof(comando));
    delay(100); 
    byte respostaEsperada[] = {0x03, 0x00, 0x00, 0x01, 0x00, 0x33, 0x31};
    int tamanhoEsperado = sizeof(respostaEsperada);

    if (SerialLeitor.available() >= tamanhoEsperado) {
        byte resposta[tamanhoEsperado];
        SerialLeitor.readBytes(resposta, tamanhoEsperado);

        // Comparar byte a byte
        for (int i = 0; i < tamanhoEsperado; i++) {
            if (resposta[i] != respostaEsperada[i]) {
                return false; // Deu ruim resposta incorreta
            }
        }
        return true; // Tudo certo
    }
    return false; // timeout
}


#endif // LEITOR_GM81S_H
