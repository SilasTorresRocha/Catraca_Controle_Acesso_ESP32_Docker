// Firmware/Firmware.ino
// Versão 3.3.0

#include "config/MapeamentoPinos.h"
#include "config/Configuracoes.h"
#include "modulos/utils/Utilidades.h"
#include "modulos/utils/AutoDiagnostico.h"
#include "modulos/ethernet/EthernetW5500.h"
#include "modulos/mqtt/ClienteMQTT.h"
#include "modulos/http_client/ClienteHTTP.h"
#include "modulos/sensores/Sensores.h"
#include "modulos/sensores/ControleCatraca.h"
#include "modulos/nvs_config/NVSConfig.h"
#include "modulos/leitor_GM81S/LeitorGM81S.h"
#include "modulos/Controle_Leds/ControleLEDs.h"
#include "modulos/Controle_Leds/EasterEggs.h" 

#define TAG_MAIN "PRINCIPAL"

unsigned long ultimo_check_ethernet = 0;
const long intervalo_check_ethernet = 5000; // 5 segundos

// --- CONTROLE DO PIR ---
unsigned long tempo_ultimo_movimento_pir = 0;
bool modo_pir_ativo = false;
const unsigned long TIMEOUT_PIR_LEDS = 30000;
// Tempo em milissegundos para voltar ao normal (30000 = 30s)

// --- Funções de Callback ---

/**
 * @brief Função de callback para mensagens MQTT recebidas
 */
void callback_comando_mqtt(char* topico, byte* payload, unsigned int length) {
    // Cria uma string para o payload (não nula)
    char payload_str[length + 1];
    memcpy(payload_str, payload, length);
    payload_str[length] = '\0';

    log_info(TAG_MAIN, ("Comando MQTT recebido no tópico: " + String(topico)).c_str());
    
    // Processamento de comandos
    if (strcmp(topico, TOPICO_SUBSCRIBE_COMANDO) == 0) {
        if (strcmp(payload_str, "ABRIR_SOLENOIDE") == 0) {
            aplicarCoresLEDs(padraoVerde);
            autorizar_entrada();
            publicar_mqtt(TOPICO_PUBLISH_EVENTO, "SOLENOIDE_ABERTA_COMANDO");
        } else if (strcmp(payload_str, "STATUS") == 0) {
            publicar_mqtt(TOPICO_PUBLISH_STATUS, "Online e OK");
        }else if (strcmp(payload_str, "REINICIAR") == 0) {
            log_info(TAG_MAIN, "Comando de REINICIO recebido via MQTT.");
            publicar_mqtt(TOPICO_PUBLISH_EVENTO, "SISTEMA_REINICIANDO_AGENDADO");
            aplicarCoresLEDs(padraoBranco); 
            delay(1000); // Dá tempo do MQTT enviar a mensagem
            ESP.restart(); 
        }

    }
}

void evento_giro() {
    // Atualiza a lógica física (passos, sensores, timeouts)
    atualizar_catraca();
    
    // Flag para garantir que o erro/sucesso seja processado apenas uma vez
    static bool eventoProcessado = false;
    
    // SUCESSO
    if (estadoCatraca == CATRACA_COMPLETO) {
        // Só processa se ainda não tiver processado este evento específico
        if (!eventoProcessado) {
            log_info(TAG_MAIN, "Giro concluido com SUCESSO!");
            
            piscarLEDs(padraoVerde, 2, 300); 
            publicar_mqtt(TOPICO_PUBLISH_EVENTO, "GIRO COMPLETO");
            
            // Cancela Easter Egg se alguém passar
            cancelar_easter_eggs();

            // Reseta imediatamente para aguardar o próximo
            estadoCatraca = CATRACA_AGUARDANDO;
            eventoProcessado = false;
            
            // Volta para a cor padrão de espera
            aplicarCoresLEDs(padraoVermelho);
            
            tempo_ultimo_movimento_pir = millis();
            modo_pir_ativo = true;
        }
    }
    
    // ERROS (Timeout, Sentido Errado ou Carona)
    else if (estadoCatraca == CATRACA_TIMEOUT || 
             estadoCatraca == CATRACA_ERRO_SENTIDO || 
             estadoCatraca == CATRACA_BLOQUEADA) {
        
        if (!eventoProcessado) {
            String msgErro = "ERRO GENERICO";
            if (estadoCatraca == CATRACA_TIMEOUT)      msgErro = "GIRO TIMEOUT";
            if (estadoCatraca == CATRACA_ERRO_SENTIDO) msgErro = "SENTIDO INVALIDO";
            if (estadoCatraca == CATRACA_BLOQUEADA)    msgErro = "CARONA BLOQUEADA";
            
            log_erro(TAG_MAIN, ("Erro detectado: " + msgErro).c_str());
            
            piscarLEDs(padraoVermelho, 3, 300);
            publicar_mqtt(TOPICO_PUBLISH_EVENTO, msgErro.c_str());
            
            // Deixa os LEDs vermelhos fixos enquanto o erro persistir
            aplicarCoresLEDs(padraoVermelho);
            eventoProcessado = true; // Marca como tratado
        }
    }
    
    // VOLTOU AO NORMAL (Reset Automático ocorreu) 
    else if (estadoCatraca == CATRACA_AGUARDANDO || estadoCatraca == CATRACA_LIBERADA) {
        // Se estava processando um erro e o sistema resetou
        if (eventoProcessado) {
            eventoProcessado = false; // Limpa a flag para o próximo evento
            
            if (estadoCatraca == CATRACA_AGUARDANDO) {
                // Se um Easter Egg estiver rodando, não impõe o vermelho
                if (!loop_easter_eggs()) { 
                    aplicarCoresLEDs(padraoVermelho);
                    tempo_ultimo_movimento_pir = millis();
                    modo_pir_ativo = true;
                }
            }
        }
    }
}

// --- Setup ---

void setup() {
    // 0. Inicializa o Controle de LEDs
    setupControleLEDs();
    aplicarCoresLEDs(padraoSetup);

    // 1. Inicializa a comunicação serial para logs
    delay(5000);
    inicializar_serial();
    log_info(TAG_MAIN, "Iniciando Firmware de Controle de Acesso...");

    // 2. Inicializa NVS (Preferi deixar o controle no Backend, mas pode ser util no futuro). Como dependo do backend, e mais facil salvar por la 
    inicializar_nvs();
    
    // 3. Inicializa Sensores
    inicializar_sensores();

    // 4. Inicializa Leitor GM81S
    inicializar_leitor_gm81s();
    
    // 5. Inicializa Ethernet W5500
    inicializar_ethernet();

    // 6. Inicializa Cliente MQTT
    inicializar_mqtt(callback_comando_mqtt);
    
    // 7. Inicializa Controle da Catraca
    setup_catraca();
    
    log_info(TAG_MAIN, "Setup concluído.");
    aplicarCoresLEDs(padraoUTF);
    //
    if(dados_disponiveis_gm81s()) limpar_buffer_gm81s();

}

// --- Loop ---

void loop() {
    // 1. Manutenção da Conexão Ethernet
    if (millis() - ultimo_check_ethernet > intervalo_check_ethernet) {
        if (!ethernet_conectado()) {
            log_erro(TAG_MAIN, "Ethernet desconectada. Tentando reconectar...");
            inicializar_ethernet(); 
        }
        ultimo_check_ethernet = millis();
    }

    // 2. Manutenção da Conexão MQTT
    loop_mqtt();
    
    //Gerencia Timeout do Easter Egg
    bool easterEggAtivo = loop_easter_eggs();

    // 3. Leitura do Leitor GM81S
    if (dados_disponiveis_gm81s()) {
        
        // Só processa leitura se a catraca estiver pronta para receber alguém
        if (estadoCatraca == CATRACA_AGUARDANDO) {
            
            String ra_lido = ler_linha_gm81s();
            log_info(TAG_MAIN, ("Dado Bruto lido: " + ra_lido).c_str());

            // --- LÓGICA DE FILTRO DE LIXO ---
            bool AlfaNumerico = true;
            bool Numeros = true;
            
            // Varre string verificando caractere por caractere
            for (unsigned int i = 0; i < ra_lido.length(); i++) {
                char c = ra_lido.charAt(i);
                // Permite letras e números
                // Também permite '-' ou '_' ou ' ' caso venha no nome
                if (!isalnum(c) && c != ' ' && c != '-' && c != '_') { 
                    AlfaNumerico = false;
                }
                // Verifica se é puramente numérico (para RA)
                if (!isdigit(c)) {
                    Numeros = false;
                }
            }

            // Se tiver lixo (caracteres estranhos), Lixo
            if (!AlfaNumerico) {
                log_info(TAG_MAIN, "Lixo detectado (caracteres inválidos). Ignorando leitura.");
                limpar_buffer_gm81s();
            } 
            else {
                // --- VERIFICAÇÃO DE EASTER EGG --- (Módulo Externo) 
                bool ativouEasterEgg = easter_egg(ra_lido);

                // Se ativou easter egg, desativa PIR temporariamente
                if (ativouEasterEgg) {
                    modo_pir_ativo = false;
                    tempo_ultimo_movimento_pir = millis();
                }
                // --- SE NÃO FOR EASTER EGG, TENTA VALIDAR RA ---
                else {
                    // Só valida se for NUMÉRICO evita mandar 'batata' pro servidor
                    if (Numeros && ra_lido.length() > 0) {
                        
                        // Cancela qualquer brincadeira visual que esteja ocorrendo
                        cancelar_easter_eggs();
                        
                        log_info(TAG_MAIN, "RA Numérico válido detectado! Iniciando validação...");
                        
                        modo_pir_ativo = false;
                        aplicarCoresLEDs(padraoBranco); // Indica processamento

                        // Validação HTTP
                        if (validar_ra_http(ra_lido)) {
                            log_info(TAG_MAIN, "RA VALIDADO! Acesso concedido.");
                            publicar_mqtt(TOPICO_PUBLISH_EVENTO, ("ACESSO_CONCEDIDO:" + ra_lido).c_str());
                            
                            aplicarCoresLEDs(padraoVerde);
                            autorizar_entrada();
                            piscarLEDs(padraoVerde, 2, 300);
                            aplicarCoresLEDs(padraoVerde);
                        } else {
                            log_erro(TAG_MAIN, "RA INVÁLIDO! Acesso negado.");
                            publicar_mqtt(TOPICO_PUBLISH_EVENTO, ("ACESSO_NEGADO:" + ra_lido).c_str());
                            
                            piscarLEDs(padraoVermelho, 3, 300);
                            aplicarCoresLEDs(padraoVermelho);
                            
                            tempo_ultimo_movimento_pir = millis();
                            modo_pir_ativo = true;
                        }
                    } 
                    else {
                        // Era alfanumérico mas não era Easter Egg (digitou "TESTE")
                        log_info(TAG_MAIN, "Código alfanumérico ignorado (Não é RA )");
                    }
                }
                
                // Limpa o buffer após processar
                limpar_buffer_gm81s();
            }

        } else {
            // Se a catraca NÃO está aguardando, limpa buffer para não acumular
            limpar_buffer_gm81s();
        }
    }


    // 4. LÓGICA DO SENSOR PIR 
    // Só atua se a catraca estiver AGUARDANDO
    if (estadoCatraca == CATRACA_AGUARDANDO && !easterEggAtivo) {
        
        if (ler_sensor_pir()) {
            tempo_ultimo_movimento_pir = millis();
            
            if (!modo_pir_ativo) {
                log_info(TAG_MAIN, "Movimento detectado. LEDs Vermelhos (Bloqueado).");
                publicar_mqtt(TOPICO_PUBLISH_EVENTO, "MOVIMENTO_DETECTADO");
                aplicarCoresLEDs(padraoVermelho);
                modo_pir_ativo = true;
            }
        }
    } else if (estadoCatraca != CATRACA_AGUARDANDO) {
        // Se alguém está passando, desliga flag do PIR
        modo_pir_ativo = false;
    }

    // Verifica se deve voltar ao padrão UTF (Timeout do PIR)
    // Só volta se não estiver rolando um Easter Egg
    if (modo_pir_ativo && !easterEggAtivo) {
        if (millis() - tempo_ultimo_movimento_pir > TIMEOUT_PIR_LEDS) {
            log_info(TAG_MAIN, "Sem movimento. Voltando LEDs para PadraoUTF.");
            aplicarCoresLEDs(padraoUTF);
            modo_pir_ativo = false;
        }
    }

    // 5. Leitura de Botão (Botão de Saída)
    if (ler_sinal_botao()) {
        log_info(TAG_MAIN, "Botão de saída pressionado.");
        
        modo_pir_ativo = false; 
        cancelar_easter_eggs(); // Garante a parada
        
        publicar_mqtt(TOPICO_PUBLISH_EVENTO, "BOTAO_SAIDA_PRESSIONADO");
        aplicarCoresLEDs(padraoVerde);
        autorizar_saida();
        delay(500); 
    }

    // 6. Atualização do giro da Catraca
    evento_giro();

    // 7. Loop de Auto-Diagnóstico
    bool sistema_ocupado = (estadoCatraca != CATRACA_AGUARDANDO) || easterEggAtivo;
    bool erro_critico = loop_diagnostico(sistema_ocupado);

    if(erro_critico){
        //Variável de contole 
        unsigned long ultimo_check_erro=0;
        const long intervalo_check_erro = 1000; // 1 segundo
        bool usandoPadrao1 = true;
        unsigned long ultimoTrocaCor = 0;
        log_erro(TAG_MAIN, "Erro crítico detectado! Loop travado.");
        while (erro_critico){
            // 1. Manutenção da Conexão Ethernet
            if (millis() - ultimo_check_ethernet > intervalo_check_ethernet) {
                if (!ethernet_conectado()) {
                    log_erro(TAG_MAIN, "Ethernet desconectada. Tentando reconectar...");
                    inicializar_ethernet(); 
                }
                ultimo_check_ethernet = millis();
            }

            // 2. Manutenção da Conexão MQTT
            loop_mqtt();

            //Verifica se o erro presiste
            if (millis() - ultimo_check_erro > intervalo_check_erro) {
                erro_critico = loop_diagnostico(false);
                ultimo_check_erro = millis();
            }

            if (millis() - ultimoTrocaCor >= 500) {
                ultimoTrocaCor = millis();
                if (usandoPadrao1) { 
                    aplicarCoresLEDs(padraoErro2);
                } else { 
                    aplicarCoresLEDs(padraoErro1); 
                }
                usandoPadrao1 = !usandoPadrao1;
            }
            delay(10); // Delay para evitar travamento do while
        }
        if(!erro_critico){
            log_info(TAG_MAIN, "Erro crítico resolvido! Loop liberado.");
            aplicarCoresLEDs(padraoUTF);
            // Não estou confiando nem em mim mesmo quem dirá em usuário, que que alguem passa alguma coisa no leitor
            if(dados_disponiveis_gm81s()) limpar_buffer_gm81s();
        }
    }
    // Se o uptime passar de 24 horas, reinicia, como uso bastate string acaba desfragmentando a memoria, para ganratir , reinicia(e bem rapido) 
    if (millis() > 93600000UL) { 
        log_info(TAG_MAIN, "Reinício preventivo de 24h...");// Se bem que tenho ruido de 90 minutos então tem que ser 26h(eu sei que seria 25,5h mas vai 26) em ms fica 93600000
        delay(1000);
        ESP.restart();
    }

    delay(10); // Delay para evitar travamento do loop
}