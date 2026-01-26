// Firmware/modulos/Controle_Leds/EasterEggs.h

#ifndef EASTER_EGGS_H
#define EASTER_EGGS_H

#include <Arduino.h>
#include "ControleLEDs.h"
#include "../utils/Utilidades.h"

#define TAG_EGG "EASTER_EGG"

// --- Configurações ---
const unsigned long TIMEOUT_EASTER_EGG = 50000; // 50 segundos

// --- Variáveis de Estado  ---
unsigned long tempo_inicio_egg = 0;
bool modo_easter_egg_ativo = false;

// Estado da brincadeira (0 a 3 controla quantos segmentos pintar)
int estagio_genshin = 0; 
Cor coresAtuaisGenshin[NUM_SEGMENTOS]; 

/**
 * @brief Verifica se o código lido é um Easter Egg e aplica os efeitos
 * @param codigo O texto lido do código de barras/QR Code ... o sensor reconhece varis tipos
 * @return true se foi um Easter Egg (e não deve ser validado como RA), false caso contrário
 */
bool easter_egg(String codigo) {
    String texto = codigo;
    texto.toLowerCase(); // Facilita comparação
    
    Cor corEscolhida = COR_DESLIGADO;
    bool eGenshin = false;

    // -- Easter Egg Dev kkkk ---
    if (texto == "silas torres rocha") {
        log_info(TAG_EGG, "Modo DEV ?");
        Cor padraoAzul[4] = {COR_AZUL, COR_AZUL, COR_AZUL, COR_AZUL};
        piscarLEDs(padraoAzul, 5, 200);
        aplicarCoresLEDs(padraoUTF); 
        return true; 
    }
    if (texto.indexOf("silas") != -1 || texto.indexOf("torre") != -1 || texto.indexOf("rocha") != -1) {
        log_info(TAG_EGG, "Modo DEV ?");
        Cor padraoAzul[4] = {COR_AZUL, COR_AZUL, COR_AZUL, COR_AZUL};
        piscarLEDs(padraoAzul, 2, 300);
        aplicarCoresLEDs(padraoUTF); 
        return true; 
    }

    // --- Genshin Impact --- Mas é muita falta do que fazer mesmo kkkkk
    
    // Anemo
    if (texto == "venti" || texto == "barbatos" || texto == "anemo" || texto == "mondstadt") {
        corEscolhida = COR_Anemo; eGenshin = true;
    }
    // Geo
    else if (texto == "morax" || texto == "zhongli" || texto == "geo" || texto == "liyue") {
        corEscolhida = COR_Geo; eGenshin = true;
    }
    // Electro
    else if (texto == "shogun" || texto == "raiden" || texto == "ei" || texto == "shogun raiden" || texto == "electro" || texto == "inazuma") {
        corEscolhida = COR_Electro; eGenshin = true;
    }
    // Dendro
    else if (texto == "nahida" || texto == "dendro" || texto == "sumeru") {
        corEscolhida = COR_Dendro; eGenshin = true;
    }
    // Pyro
    else if (texto == "mavuika" || texto == "pyro" || texto == "natlan") {
        corEscolhida = COR_Pyro; eGenshin = true;
    }
    // Hydro
    else if (texto == "furina" || texto == "hydro" || texto == "fontaine") {
        corEscolhida = COR_Hydro; eGenshin = true;
    }
    // Cryo
    else if (texto == "tsaritsa" || texto == "snezhnaya" || texto == "cryo" || texto == "cyro") {
        corEscolhida = COR_Cryo; eGenshin = true;
    }
    // Columbina
    else if (texto == "columbina" || texto == "nodkrai" || texto == "nod krai") {
        corEscolhida = COR_Columbina; eGenshin = true;

    }else if (texto == "genshin" || texto == "impact" || texto == "genshin impact") {
        corEscolhida = COR_branco; eGenshin = true;
    }

    // Lógica Genshin
    if (eGenshin) {
        log_info(TAG_EGG, ("Easter Egg detectado: " + texto).c_str());
        // Se a brincadeira não estava ativa, prepara o array base
        if (!modo_easter_egg_ativo) {
             for(int i=0; i<NUM_SEGMENTOS; i++) coresAtuaisGenshin[i] = padraoUTF[i];
             estagio_genshin = 0;
        }

        // Lógica de Preenchimento Progressivo (4 -> 3 -> 2 -> 1)
        for (int i = estagio_genshin; i < NUM_SEGMENTOS; i++) {
            coresAtuaisGenshin[i] = corEscolhida;
        }

        aplicarCoresLEDs(coresAtuaisGenshin);
        
        // Prepara o próximo estágio
        estagio_genshin++;
        if (estagio_genshin >= NUM_SEGMENTOS) {
            estagio_genshin = 0; // Reinicia o ciclo
        }

        // Ativa timers e flags
        modo_easter_egg_ativo = true;
        tempo_inicio_egg = millis();
        return true;
    }

    return false; // Não é Easter Egg
}

/**
 * @brief Gerencia o timeout da brincadeira, Deve ser chamado no loop principal
 * @return true se o modo Easter Egg AINDA estiver ativo, false se acabou/está desligado
 */
bool loop_easter_eggs() {
    if (modo_easter_egg_ativo) {
        if (millis() - tempo_inicio_egg > TIMEOUT_EASTER_EGG) {
            log_info(TAG_EGG, "Fim do Easter Egg (Timeout).");
            modo_easter_egg_ativo = false;
            estagio_genshin = 0;
            aplicarCoresLEDs(padraoUTF); 
            return false; 
        }
        return true; // Ainda ativo
    }
    return false; // Não ativo
}

/**
 * @brief Força o cancelamento do Easter Egg (tipoquando alguém passa o crachá real)
 * @return Nota: Quem chama essa função deve definir a cor nova dos LEDs depois
 */
void cancelar_easter_eggs() {
    if (modo_easter_egg_ativo) {
        modo_easter_egg_ativo = false;
        estagio_genshin = 0;
    }
}

#endif // EASTER_EGGS_H