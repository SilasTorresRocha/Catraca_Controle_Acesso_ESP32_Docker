// Firmware_Controle_Acesso/modulos/utils/Utilidades.h

#ifndef UTILIDADES_H
#define UTILIDADES_H

#include <Arduino.h>

/**
 * @brief Inicializa a comunicação serial para logs, Timeouts de 2 segundos para conexão
 * 
 * @param baud_rate Taxa de transmissão.
 */
void inicializar_serial(long baud_rate = 115200) {
    Serial.begin(baud_rate);
    //Para nao travar o boot
    unsigned long tempo_limite = millis() + 2000;
    while (!Serial && millis() < tempo_limite) {
        delay(10); // delay para aliviae a CPU. (Isso de esperaer conectar e Necessário apenas para porta USB nativa).
    }
    Serial.println(F("\n[UTIL] Serial inicializada."));
}

/**
 * @brief Imprime uma mensagem de log formatada.
 * 
 * @param tag Tag do módulo (ex: "MQTT", "ETH", "GM81S")...
 * @param mensagem Mensagem a ser exibida
 */
void log_info(const char* tag, const char* mensagem) {
    Serial.print(F("["));
    Serial.print(tag);
    Serial.print(F("] "));
    Serial.println(mensagem);
}

/**
 * @brief Imprime uma mensagem de erro formatada.
 * 
 * @param tag Tag do módulo.
 * @param mensagem Mensagem de erro.
 */
void log_erro(const char* tag, const char* mensagem) {
    Serial.print(F("[ERRO-"));
    Serial.print(tag);
    Serial.print(F("] "));
    Serial.println(mensagem);
}

/**
 * @brief Converte um array de bytes para uma string hexadecimal.
 * 
 * @param buffer Array de bytes.
 * @param tamanho Tamanho do array.
 * @return String com a representação hexadecimal.
 */
String bytes_para_hex(const byte* buffer, size_t tamanho) {
    String resultado = "";
    for (size_t i = 0; i < tamanho; i++) {
        if (buffer[i] < 0x10) {
            resultado += "0";
        }
        resultado += String(buffer[i], HEX);
    }
    return resultado;
}

#endif // UTILIDADES_H
