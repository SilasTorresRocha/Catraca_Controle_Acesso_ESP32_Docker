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
 * @brief Implementação do Hash DJB2 para o easater egg
 */
unsigned long calcular_hash_djb2(String str) {
    unsigned long hash = 5381;
    for (unsigned int i = 0; i < str.length(); i++) {
        hash = ((hash << 5) + hash) + str.charAt(i);    // hash * 33 + c 
    }
    return hash;
}

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

    unsigned long hash=calcular_hash_djb2(texto);

/*--- INICIO DOS HASHES DJB2 ---

0xe71ab04dUL // silas torres rocha
0x105b6e41UL // silas
0x1070ecb1UL // torre
0x104c7a92UL // rocha
0x108f912bUL // venti
0x28c87353UL // barbatos
0xf1856d5UL // anemo
0xd03753UL // mondstadt
0xff23f0cUL // morax
0xadafc8e0UL // zhongli
0xb887680UL // geo
0xfdcfc8dUL // liyue
0x1bb8dd39UL // shogun
0x18e1aff8UL // raiden
0x5977b3UL // ei
0xdc70e66cUL // shogun raiden
0x65c2aa73UL // electro
0x9e1a2cfaUL // inazuma
0xf8c962aUL // nahida
0xf88498c1UL // dendro
0x1ca2fa26UL // sumeru
0xb5292133UL // mavuika
0x7c9c8bcfUL // pyro
0xf933723UL // natlan
0xfe52b84aUL // furina
0xf9d06ebUL // hydro
0x77000f9UL // fontaine
0x87fe990UL // tsaritsa
0x4d340e56UL // snezhnaya
0x7c954e02UL // cryo
0x7c956ae2UL // cyro
0x130b7dfUL // columbina
0x218086edUL // nodkrai
0x4c40a7cdUL // nod krai
0xf00e75f1UL // genshin
0x4c03203UL // impact
0xb0bb022fUL // genshin impact

--- FIM ---*/

    switch (hash){
    case 0xe71ab04dUL: //silas torres rocha
        log_info(TAG_EGG, "Modo Completo");
        piscarLEDs(padraoAzul, 5, 200);
        aplicarCoresLEDs(padraoUTF); 
        break;
    case 0x105b6e41UL: //silas
    case 0x1070ecb1UL: //torre
    case 0x104c7a92UL: //rocha
        log_info(TAG_EGG, "Modo Parcial");
        piscarLEDs(padraoAzul, 2, 300);
        aplicarCoresLEDs(padraoUTF); 
        break;
    //Mas é muita falta do que fazer mesmo kkkkk
    //Anemo
    case 0x108f912bUL: //venti
    case 0x28c87353UL: //barbatos
    case 0xf1856d5UL: //anemo
    case 0xd03753UL: //mondstadt
        corEscolhida = COR_Anemo; eGenshin = true;
        break;
    //Geo
    case 0xff23f0cUL: //morax
    case 0xadafc8e0UL: //zhongli
    case 0xb887680UL: //geo
    case 0xfdcfc8dUL: //liyue
        corEscolhida = COR_Geo; eGenshin = true;
        break;
    //Electro
    case 0x1bb8dd39UL: //shogun
    case 0x18e1aff8UL: //raiden
    case 0x5977b3UL: //ei
    case 0xdc70e66cUL: //shogun raiden
    case 0x65c2aa73UL: //electro
    case 0x9e1a2cfaUL: //inazuma
        corEscolhida = COR_Electro; eGenshin = true;
        break;
    //Dendro
    case 0xf8c962aUL: //nahida
    case 0xf88498c1UL: //dendro
    case 0x1ca2fa26UL: //sumeru
        corEscolhida = COR_Dendro; eGenshin = true;
        break;
    //Pyro
    case 0xb5292133UL: //mavuika
    case 0x7c9c8bcfUL: //pyro
    case 0xf933723UL: //natlan
        corEscolhida = COR_Pyro; eGenshin = true;
        break;
    //Hydro
    case 0xfe52b84aUL: //furina
    case 0xf9d06ebUL: //hydro
    case 0x77000f9UL: //fontaine
        corEscolhida = COR_Hydro; eGenshin = true;
        break;
    //Cryo
    case 0x87fe990UL: //tsaritsa
    case 0x4d340e56UL: //snezhnaya
    case 0x7c954e02UL: //cryo
    case 0x7c956ae2UL: //cyro
        corEscolhida = COR_Cryo; eGenshin = true;
        break;
    //Columbina
    case 0x130b7dfUL: //columbina
    case 0x218086edUL: //nodkrai
    case 0x4c40a7cdUL: //nod krai
        corEscolhida = COR_Columbina; eGenshin = true;
        break;
    case 0xf00e75f1UL: //genshin
    case 0x4c03203UL: //impact
    case 0xb0bb022fUL: //genshin impact
        corEscolhida = COR_BRANCO; eGenshin = true;
        break;

    default:
        eGenshin = false;
        break;
    }

    // Lógica Genshin
    if (eGenshin) {
        log_info(TAG_EGG, ("Easter Egg Hash: 0x" + String(hash, HEX)).c_str());
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