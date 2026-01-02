// Firmware/modulos/sensores/Sensores.h

#ifndef SENSORES_H
#define SENSORES_H

#include <Arduino.h>
#include "../../config/MapeamentoPinos.h"
#include "../utils/Utilidades.h"

#define TAG_SENSORES "SENSORES"

/**
 * @brief Inicializa os pinos dos sensores como INPUT.
 */
void inicializar_sensores() {
    log_info(TAG_SENSORES, "Inicializando pinos dos sensores...");
    
    // Sensor PIR (dataPIR do Shematic).
    pinMode(PINO_SENSOR_PIR, INPUT);

    // Sinal do Botão.
    pinMode(PINO_SINAL_BOTAO, INPUT_PULLUP); //Pode remover ja que tem o resistor externo ou manter ja que e um resistor de 4k7 (Eu acho) mas nao tem problema o que vai mudar e a resistencia equivalente.

    log_info(TAG_SENSORES, "Pinos dos sensores configurados.");
}

/**
 * @brief Lê o estado do sensor PIR.
 * 
 * @return true se movimento detectado (HIGH), false caso contrário (LOW).
 */
bool ler_sensor_pir() {
    return digitalRead(PINO_SENSOR_PIR) == HIGH;
}

/**
 * @brief Lê o estado do botão.
 * 
 * @return true se o botão estiver pressionado (LOW, devido ao PULLUP), false caso contrário (HIGH).
 */
bool ler_sinal_botao() {
    return digitalRead(PINO_SINAL_BOTAO) == LOW;
}

#endif // SENSORES_H
