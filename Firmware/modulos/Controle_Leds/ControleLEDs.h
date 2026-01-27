#ifndef CONTROLE_LEDS_H
#define CONTROLE_LEDS_H

#include <Arduino.h>



// Número de segmentos RGB
//#define NUM_SEGMENTOS 4

// Frequência e resolução PWM
#define LEDC_FREQ 5000
#define LEDC_RESOLUCAO 8  // 8 bits = 0 a 255

#include "../../config/MapeamentoPinos.h" 

// Estrutura para representar uma cor RGB
struct Cor {
  bool r;      // Vermelho (0/1)
  uint8_t g;   // Verde (PWM)
  uint8_t b;   // Azul (PWM)
};

//====================Cores Pré-definidas====================//
const Cor COR_AMARELO = { true, 200, 0 };
const Cor COR_VERDE = { false, 255, 0 }; 
const Cor COR_BRANCO = { true, 255, 255 }; 
const Cor COR_DESLIGADO = { false, 0, 0 };
const Cor COR_VERMELHO = { true, 0, 0 };
const Cor COR_AZUL = { false, 0, 255 };  

//====================Cores EasterEgg====================//
const Cor COR_Anemo= { false, 255, 200 };
const Cor COR_Geo= { true, 190, 0 };
const Cor COR_Electro= { true, 0, 255 };
const Cor COR_Hydro= { false, 10, 255 };
const Cor COR_Pyro= { true, 0, 0 };
const Cor COR_Cryo= { false, 200, 255 };
const Cor COR_Dendro= { false, 255, 0 };
const Cor COR_Columbina= { true, 200, 210 }; //especial dela, não sei que cor eu coloco para ela

//====================Padrões de Cores====================//
Cor padraoUTF[NUM_SEGMENTOS] = {
  COR_AMARELO,
  COR_BRANCO,
  COR_AMARELO,
  COR_BRANCO
};

Cor padraoVermelho[NUM_SEGMENTOS] = {
  { true, 0, 0 },
  { true, 0, 0 },
  { true, 0, 0 },
  { true, 0, 0 }
};

Cor padraoVerde[NUM_SEGMENTOS] = {
  COR_VERDE,
  COR_VERDE,
  COR_VERDE,
  COR_VERDE
};

Cor padraoDesligado[NUM_SEGMENTOS] = {
  COR_DESLIGADO,
  COR_DESLIGADO,
  COR_DESLIGADO,
  COR_DESLIGADO
};

Cor padraoAmarelo[NUM_SEGMENTOS] = {
  COR_AMARELO,
  COR_AMARELO,
  COR_AMARELO,
  COR_AMARELO
};

Cor padraoAzul[NUM_SEGMENTOS] = {
  COR_AZUL,
  COR_AZUL,
  COR_AZUL,
  COR_AZUL
};

Cor padraoSetup[NUM_SEGMENTOS] = {
  COR_AZUL,
  COR_VERDE,
  COR_AZUL,
  COR_VERDE
};

Cor padraoBranco[NUM_SEGMENTOS] = {
  COR_BRANCO,
  COR_BRANCO,
  COR_BRANCO,
  COR_BRANCO
};
Cor padraoErro1[NUM_SEGMENTOS] = {
  COR_VERMELHO,
  COR_DESLIGADO,
  COR_DESLIGADO,
  COR_VERMELHO
};
Cor padraoErro2[NUM_SEGMENTOS] = {
  COR_DESLIGADO,
  COR_VERMELHO,
  COR_VERMELHO,
  COR_DESLIGADO
};
Cor padraoSemIP[NUM_SEGMENTOS] = {
  COR_VERMELHO,
  COR_BRANCO,
  COR_BRANCO,
  COR_BRANCO
};
Cor padraoSemMQTT[NUM_SEGMENTOS] = {
  COR_BRANCO,
  COR_VERMELHO,
  COR_BRANCO,
  COR_BRANCO
};




//====================IMPLEMENTAÇÃO DAS FUNÇÕES ====================//

/**
 * @brief Função de Setup do Módulo de Controle de LEDs 
 * Configura os pinos digitais e anexa os pinos PWM ao controlador LEDC.
 */
inline void setupControleLEDs() {
  for (int i = 0; i < NUM_SEGMENTOS; i++) {
    // Vermelho (digital)
    pinMode(pinosR[i], OUTPUT);
    digitalWrite(pinosR[i], LOW);
    //PWM - usando a API nova do ledc
    // Verde 
    ledcAttach(pinosG[i], LEDC_FREQ, LEDC_RESOLUCAO);
    // Azul 
    ledcAttach(pinosB[i], LEDC_FREQ, LEDC_RESOLUCAO);
  }
}

/**
 * @brief Aplica as cores aos segmentos de LED
 * @param cores Array de structs Cor
 * Exemplo:
 * Cor cores[NUM_SEGMENTOS] = {
 *   { true, 255, 0 },   // Segmento 1: Verm
 *  { false, 0, 255 },  // Segmento 2: Azul
 *  { true, 0, 0 },     // Segmento 3: Vermelho
 *  { false, 0, 0 }     // Segmento 4: Desligado
 * };
 * aplicarCoresLEDs(cores);
 */
inline void aplicarCoresLEDs(Cor cores[NUM_SEGMENTOS]) {
  for (int i = 0; i < NUM_SEGMENTOS; i++) {
    digitalWrite(pinosR[i], cores[i].r ? HIGH : LOW);
    ledcWrite(pinosG[i], cores[i].g);
    ledcWrite(pinosB[i], cores[i].b);
  }
}


/**
 * @brief Pisca os LEDs com o padrão especificado
 * @param padrao Array de structs Cor representando o padrão de cores
 * @param vezes Número de vezes que os LEDs devem piscar
 * @param intervalo_ms Intervalo em milissegundos entre ligar e desligar
 */
inline void piscarLEDs(Cor padrao[NUM_SEGMENTOS], uint8_t vezes, uint16_t intervalo_ms) {
  for (uint8_t i = 0; i < vezes; i++) {
    aplicarCoresLEDs(padrao);       
    delay(intervalo_ms);
    aplicarCoresLEDs(padraoDesligado); 
    delay(intervalo_ms);
  }
}


#endif // CONTROLE_LEDS_H