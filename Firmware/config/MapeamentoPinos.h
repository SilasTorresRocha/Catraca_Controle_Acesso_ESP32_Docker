// Firmware_Controle_Acesso/config/MapeamentoPinos.h

#ifndef MAPEAMENTO_PINOS_H
#define MAPEAMENTO_PINOS_H

#include "Configuracoes.h"

// --- Pinos de Saída (Atuadores) ---

//Solenoide Drive 
#define PINO_SOLENOIDE_DRIVE 9



// --- Pinos de Entrada (Sensores/Botões) ---

// Indutores
#define PINO_INDUTOR_1 16
#define PINO_INDUTOR_2 18

// Sensor PIR (dataPIR)
#define PINO_SENSOR_PIR 6

// Sinal do Botão
#define PINO_SINAL_BOTAO 15

// --- Pinos de Comunicação Serial (UART) ---
#define PINO_TX_LEITOR 35 // TX_ESP 
#define PINO_RX_LEITOR 33 // RX_ESP 


#define PINO_UART_TX 35 // TX_ESP (Transmissão do ESP32) 
#define PINO_UART_RX 33 // RX_ESP (Recepção no ESP32)

// --- Pinos de Comunicação SPI ---

// O W5500 precisa de MOSI, MISO, CLK e CS.
#define PINO_SPI_MOSI 40 // SPI_MOSI(40) 
#define PINO_SPI_MISO 39 // SPI_MISO(39) 
#define PINO_SPI_CLK 38  // SPI_CLK(38) 
//----------------------------------------------------------------------------------
#define PINO_SPI_CS 36   // SPI_CS(36) - Chip Select (Pino de controle)
#define PINO_SPI_INT 21  // SPI_INT(21) - Pino de Interrupção (Opcional)
#define PINO_W5500_RST 37 // W5500_RST(37) - Pino de Reset do W5500

// --- Leds RGB ---

// Pinos Digitais (Vermelho)
const uint8_t pinosR[NUM_SEGMENTOS] = {5,2,10,12};

// Pinos PWM (Verde) 
const uint8_t pinosG[NUM_SEGMENTOS] = {3,4,13,11};

// Pinos PWM (Azul) 
const uint8_t pinosB[NUM_SEGMENTOS] = {7,1,8,14};

#endif // MAPEAMENTO_PINOS_H
