// Firmware/modulos/sensores/ControleCatraca.h

#ifndef CONTROLE_CATRACA_H
#define CONTROLE_CATRACA_H

#include <Arduino.h>
#include "../../config/MapeamentoPinos.h"
#include "../../config/Configuracoes.h"
#include "../utils/Utilidades.h"

// -------------------- CONFIGURAÇÕES --------------------
#define STEPS_POR_PASSAGEM  8    
#define TIMEOUT_MS          10000 
#define COOL_DOWN_MS        800  
#define DEBOUNCE_MS         30   

// Define se vai imprimir mensagens detalhadas de sensores 
#define DEBUG_CATRACA_DETALHADO true 

// -------------------- ENUMERAÇÕES --------------------
typedef enum {
    CATRACA_AGUARDANDO,
    CATRACA_LIBERADA,   
    CATRACA_EM_GIRO,    
    CATRACA_COMPLETO,
    CATRACA_TIMEOUT,
    CATRACA_BLOQUEADA,
    CATRACA_ERRO_SENTIDO // Estado para tentativa de fraude/giro invertido
} EstadoCatraca_t;

// -------------------- VARIÁVEIS GLOBAIS --------------------
volatile bool houveMudancaSensor = false;
volatile uint32_t ultimoISRTime = 0;

EstadoCatraca_t estadoCatraca = CATRACA_AGUARDANDO;

int passosAcumulados = 0;
uint8_t ultimoAB = 0;
unsigned long tempoInicioGiro = 0;
unsigned long tempoAutorizacao = 0; 
unsigned long tempoUltimaPassagem = 0;

// Variável para forçar o sentido: 1 = Positivo (Entrada), -1 = Negativo (Saída), 0 = Qualquer
int sentidoAutorizado = 0; 

// Tabela de transição (Sequência 0 -> 1 -> 3 -> 2 -> 0)
const int8_t tabelaEncoder[4][4] = {
    { 0,  1, -1,  0}, 
    {-1,  0,  0,  1}, 
    { 1,  0,  0, -1}, 
    { 0, -1,  1,  0}  
};

// -------------------- FUNÇÕES AUXILIARES --------------------
void solenoide_travar() {
    pinMode(PINO_SOLENOIDE_DRIVE, OUTPUT);
    digitalWrite(PINO_SOLENOIDE_DRIVE, LOW);  
    if(DEBUG_CATRACA_DETALHADO) Serial.println("[DEBUG] Solenoide TRAVADO");
}

void solenoide_liberar() {
    pinMode(PINO_SOLENOIDE_DRIVE, OUTPUT);
    digitalWrite(PINO_SOLENOIDE_DRIVE, HIGH); 
    if(DEBUG_CATRACA_DETALHADO) Serial.println("[DEBUG] Solenoide LIBERADO");
}

uint8_t lerAB() {
    int a = digitalRead(PINO_INDUTOR_1);
    int b = digitalRead(PINO_INDUTOR_2);
    return ( (a << 1) | b ); 
}

// -------------------- ISR --------------------
void IRAM_ATTR isr_indutores() {
    uint32_t agora = (uint32_t)(esp_timer_get_time() / 1000ULL);
    if ((agora - ultimoISRTime) < DEBOUNCE_MS) return;
    ultimoISRTime = agora;
    houveMudancaSensor = true;
}

// -------------------- FUNÇÕES PRINCIPAIS --------------------
void setup_catraca() {
    pinMode(PINO_INDUTOR_1, INPUT);
    pinMode(PINO_INDUTOR_2, INPUT);

    solenoide_travar();

    attachInterrupt(digitalPinToInterrupt(PINO_INDUTOR_1), isr_indutores, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PINO_INDUTOR_2), isr_indutores, CHANGE);

    ultimoAB = lerAB();
    estadoCatraca = CATRACA_AGUARDANDO;
    sentidoAutorizado = 0;

    log_info("Catraca", "Modulo inicializado com DEBUG e CONTROLE DE SENTIDO.");
}

void autorizar_entrada() {
    if (estadoCatraca == CATRACA_AGUARDANDO || estadoCatraca == CATRACA_BLOQUEADA || estadoCatraca == CATRACA_ERRO_SENTIDO) {
        estadoCatraca = CATRACA_LIBERADA;
        tempoAutorizacao = millis(); 
        
        sentidoAutorizado = 1; // Usar -1 ou 1 para definir o sentido ou inverter o sentido dos indutores
        
        solenoide_liberar();
        log_info("Catraca", "Entrada AUTORIZADA (Esperando Giro Positivo +).");
    } else {
        log_info("Catraca", "Ignorado: Ja existe uma operacao em andamento.");
    }
}

void autorizar_saida() {
    if (estadoCatraca == CATRACA_AGUARDANDO || estadoCatraca == CATRACA_BLOQUEADA || estadoCatraca == CATRACA_ERRO_SENTIDO) {
        estadoCatraca = CATRACA_LIBERADA;
        tempoAutorizacao = millis(); 
        
        //SENTIDO DA SAÍDA (Oposto da entrada)
        sentidoAutorizado = -1; 
        
        solenoide_liberar();
        log_info("Catraca", "Saida AUTORIZADA (Esperando Giro Negativo -).");
    } else {
        log_info("Catraca", "Ignorado: Ja existe uma operacao em andamento.");
    }
}

void resetar_catraca() {
    estadoCatraca = CATRACA_AGUARDANDO;
    passosAcumulados = 0;
    sentidoAutorizado = 0;
    ultimoAB = lerAB();
    solenoide_travar();
    log_info("Catraca", "Reset forçado.");
}

void atualizar_catraca() {
    // 1. Timeout de Espera (Usuário não girou nada)
    if (estadoCatraca == CATRACA_LIBERADA) {
        if (millis() - tempoAutorizacao > TIMEOUT_MS) {
            estadoCatraca = CATRACA_TIMEOUT;
            tempoInicioGiro = millis(); 
            solenoide_travar();
            log_erro("Catraca", "Timeout: Nenhuma acao do usuario.");
        }
    }

    // 2. Processamento do Movimento
    if (houveMudancaSensor) {
        uint8_t atual = lerAB();
        int8_t passo = tabelaEncoder[ultimoAB][atual];
        
        // --- DEBUG VISUAL ---
        if (DEBUG_CATRACA_DETALHADO) {
            Serial.printf("[DEBUG] A:%d B:%d (Dec:%d) | Passo:%d | Acumulado:%d | Estado:%d\n", 
                          (atual >> 1) & 1, atual & 1, atual, passo, passosAcumulados, estadoCatraca);
        }

        // Anti-Carona
        if (estadoCatraca == CATRACA_COMPLETO && (millis() - tempoUltimaPassagem) < COOL_DOWN_MS) {
            estadoCatraca = CATRACA_BLOQUEADA;
            solenoide_travar();
            log_erro("Catraca", "BLOQUEIO: Carona detectada!");
            houveMudancaSensor = false;
            return;
        }

        if (passo != 0) {
            // --- VERIFICAÇÃO DE SENTIDO ---
            
            bool sentidoInvalido = false;
            if (estadoCatraca == CATRACA_LIBERADA || estadoCatraca == CATRACA_EM_GIRO) {
                 if (sentidoAutorizado == 1 && passo < 0) sentidoInvalido = true;
                 if (sentidoAutorizado == -1 && passo > 0) sentidoInvalido = true;
            }

            if (sentidoInvalido) {
                log_erro("Catraca", "SENTIDO INVALIDO! Tentativa de giro contrario.");
                               
                // Trava o sistema e acusa erro 
                estadoCatraca = CATRACA_ERRO_SENTIDO;
                solenoide_travar();
                tempoInicioGiro = millis(); // Usa para timer de reset do erro
            } 
            else {
                // Sentido Correto
                passosAcumulados += passo;
                ultimoAB = atual;

                if (estadoCatraca == CATRACA_LIBERADA) {
                    estadoCatraca = CATRACA_EM_GIRO;
                    tempoInicioGiro = millis();
                    log_info("Catraca", "Giro iniciado corretamente.");
                }

                // Verifica conclusão pelo valor ABSOLUTO, mas sabendo que o sentido foi respeitado
                if (abs(passosAcumulados) >= STEPS_POR_PASSAGEM) {
                    if (estadoCatraca == CATRACA_EM_GIRO) {
                        estadoCatraca = CATRACA_COMPLETO;
                        tempoUltimaPassagem = millis();
                        solenoide_travar(); 
                        log_info("Catraca", "Passagem CONCLUIDA com sucesso.");
                    }
                    passosAcumulados = 0; 
                }
            }
        }
        houveMudancaSensor = false;
    }

    // 3. Timeout DURANTE o giro (Travou no meio)
    if (estadoCatraca == CATRACA_EM_GIRO && (millis() - tempoInicioGiro > TIMEOUT_MS)) {
        estadoCatraca = CATRACA_TIMEOUT;
        passosAcumulados = 0;
        solenoide_travar();
        log_erro("Catraca", "Timeout: Usuario parou no meio do giro.");
    }

    // 4. Reset Automático de Erros (Timeout ou Sentido Errado)
    if (estadoCatraca == CATRACA_TIMEOUT || estadoCatraca == CATRACA_ERRO_SENTIDO) {
        unsigned long referenciaTempo = (tempoInicioGiro > 0) ? tempoInicioGiro : tempoAutorizacao; 
        
        // Dá 2 segundos para o usuário entender o erro/timeout antes de liberar
        if (millis() - referenciaTempo > 2000) { 
            estadoCatraca = CATRACA_AGUARDANDO;
            passosAcumulados = 0;
            sentidoAutorizado = 0;
            ultimoAB = lerAB(); 
            log_info("Catraca", "Sistema pronto (Reset automatico de erro).");
        }
    }
}

#endif // CONTROLE_CATRACA_H