//Firmware/modulos/utils/AutoDiagnostico.h

#ifndef AUTO_DIAGNOSTICO_H
#define AUTO_DIAGNOSTICO_H

#include <Arduino.h>
#include "../../config/MapeamentoPinos.h"
#include "../leitor_GM81S/LeitorGM81S.h"
#include "../mqtt/ClienteMQTT.h"
#include "../Controle_Leds/ControleLEDs.h"

#define TAG_DIAG "DIAGNOSTICO"

// ================== CONFIGURAÇÕES DE TEMPO (EM MS) ==================

// Intervalo para enviar o relatório completo via MQTT  
const unsigned long INTERVALO_RELATORIO_MQTT = 60000; // 10 minutos // vou mudar para 1 minuto para testes

// Tempo máximo que o BOTÃO pode ficar pressionado antes de ser erro 
const unsigned long TIMEOUT_BOTAO_TRAVADO = 60000; // 60s

// Tempo máximo que o INDUTOR pode indicar metal antes de ser erro 
const unsigned long TIMEOUT_INDUTOR_TRAVADO = 60000; // 60s

// Tempo máximo do PIR
// Se colocar 0, desativa a verificação do PIR
const unsigned long TIMEOUT_PIR_TRAVADO = 8UL * 60UL * 60UL * 1000UL; // 8 horas não tem muito problema esse sensor ele é mais um módulo de perfumaria

// Tentativas o GM81S tem antes de decretar falha
const int TENTATIVAS_GM81S = 3;

// Flag para ativar/desativar o módulo
bool diagnostico_ativo = true;

// ================== VARIÁVEIS DE CONTROLE ==================

unsigned long timer_relatorio = 0;
unsigned long inicio_travamento_botao = 0;
unsigned long inicio_travamento_indutor = 0;
unsigned long inicio_travamento_indutor2 = 0;
unsigned long inicio_travamento_pir = 0;
bool erro_critico = false;

// ================== FUNÇÕES AUXILIARES ==================

/**
 * @brief Verifica se um sensor específico está travado além do tempo permitido.
 * Retorna true se estiver travado (ERRO).
 */
bool verificar_travamento(int pino, bool nivel_ativacao, unsigned long &timer_inicio, unsigned long timeout_limite, const char* nome) {
    if (timeout_limite == 0) return false; // Verificação desativada para este sensor

    bool leitura = digitalRead(pino);

    // Se o sensor está no estado de ativação
    if (leitura == nivel_ativacao) {
        if (timer_inicio == 0) {
            timer_inicio = millis();
        } 
        // Se já passou do tempo limite
        else if (millis() - timer_inicio > timeout_limite) {
            log_erro(TAG_DIAG, (String(nome) + " TRAVADO HA MAIS DE " + String(timeout_limite/1000) + "s").c_str());
            return true;
        }
    } else {
        // Sensor voltou, reseta o timer
        timer_inicio = 0;
    }
    return false;
}

/**
 * @brief Tenta comunicar com o GM81S 3 vezes
 */
bool teste_ativo_gm81s() {
    log_info(TAG_DIAG, "Iniciando teste ativo do GM81S");
    for (int i = 1; i <= TENTATIVAS_GM81S; i++) {
        if (diagnostico_gm81s()) {
            return true; // Sucesso
        }
        log_info(TAG_DIAG, ("Tentativa " + String(i) + " falhou. Retentando...").c_str());
        delay(1000); 
    }
    log_erro(TAG_DIAG, "GM81S FALHOU após 3 tentativas.");
    return false;
}

// ================== DIAGNÓSTICO ==================

/**
 * @brief Executa as verificações. Deve ser chamado no loop() principal
 * @param sistema_ocupado Passar true se a catraca estiver em algum uso
 * @return true se houver erro crítico, false caso contrário
 */
bool loop_diagnostico(bool sistema_ocupado) {
    if (!diagnostico_ativo) return false;

    // Roda sempre mas só acusa erro se passar do tempo
    // Botão de Saída, tem pull up
    bool botao_erro = verificar_travamento(PINO_SINAL_BOTAO, LOW, inicio_travamento_botao, TIMEOUT_BOTAO_TRAVADO, "BOTAO_SAIDA");
    // Indutores, ja que quando está detectando metal fica em LOW, pela logica do meu conversor de nivel logico
    bool ind1_erro = verificar_travamento(PINO_INDUTOR_1, LOW, inicio_travamento_indutor, TIMEOUT_INDUTOR_TRAVADO, "INDUTOR_1");
    bool ind2_erro = verificar_travamento(PINO_INDUTOR_2, LOW, inicio_travamento_indutor2, TIMEOUT_INDUTOR_TRAVADO, "INDUTOR_2");
    // PIR 
    bool pir_erro = verificar_travamento(PINO_SENSOR_PIR, HIGH, inicio_travamento_pir, TIMEOUT_PIR_TRAVADO, "PIR");

    // Erro CRÍTICO (Botão ou Indutor travados impedem uso)
    if (botao_erro || ind1_erro || ind2_erro) {
        if (!erro_critico) {
            log_erro(TAG_DIAG, "Falha critica.");
            erro_critico = true;
        }
        // Pisca vermelho se o sistema não estiver ocupado com usuário
        if (!sistema_ocupado) {
         //piscarLEDs(padraoVermelho, 3, 300); tem delay ali dentro 
         aplicarCoresLEDs(padraoErro1);
        }
    } else {
        erro_critico = false;
    }

    // Relatório Periódico MQTT + Teste Ativo GM81S
    if (!sistema_ocupado && (millis() - timer_relatorio > INTERVALO_RELATORIO_MQTT)) {
        
        bool gm81s_ok = teste_ativo_gm81s();
        
        String payload = "{";
        payload += "\"tipo\":\"diagnostico\",";
        payload += "\"status\":" + String(erro_critico ? "\"ERRO\"" : "\"OK\"") + ",";
        payload += "\"gm81s\":" + String(gm81s_ok ? "\"OK\"" : "\"FALHA\"") + ",";
        payload += "\"botao_saida\":" + String(botao_erro ? "\"TRAVADO\"" : "\"OK\"") + ",";
        payload += "\"indutor_1\":" + String(ind1_erro ? "\"TRAVADO\"" : "\"OK\"") + ",";
        payload += "\"indutor_2\":" + String(ind2_erro ? "\"TRAVADO\"" : "\"OK\"") + ",";
        payload += "\"pir\":" + String(pir_erro ? "\"TRAVADO\"" : "\"OK\"") + ",";
        payload += "\"uptime_h\":" + String(millis() / 3600000);
        payload += "}";

        publicar_mqtt("controle_acesso/diagnostico", payload.c_str());
        timer_relatorio = millis();
    }

    return erro_critico;
}

// Funções para controle externo via MQTT
void set_diagnostico_ativo(bool estado) {
    diagnostico_ativo = estado;
    log_info(TAG_DIAG, estado ? "Diagnostico ATIVADO" : "Diagnostico DESATIVADO");
}

#endif // AUTO_DIAGNOSTICO_H