// Firmware_Controle_Acesso/modulos/nvs_config/NVSConfig.h

#ifndef NVS_CONFIG_H
#define NVS_CONFIG_H

#include <Arduino.h>
#include <nvs_flash.h>
#include <nvs.h>
#include "../utils/Utilidades.h"

#define TAG_NVS "NVS_CONFIG"
#define NVS_NAMESPACE "config_app"

/**
 * @brief Inicializa o sistema NVS (Non-Volatile Storage).
 * 
 * @return true se a inicialização foi bem-sucedida, false caso contrário.
 */
bool inicializar_nvs() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        log_info(TAG_NVS, "NVS corrompido ou versão nova. Apagando e reformatando...");
        // Apaga e tenta inicializar novamente
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret == ESP_OK) {
        log_info(TAG_NVS, "NVS inicializado com sucesso.");
        return true;
    } else {
        log_erro(TAG_NVS, "Falha ao inicializar NVS.");
        return false;
    }
}

/**
 * @brief Salva um valor inteiro no NVS.
 * 
 * @param chave A chave para o valor.
 * @param valor O valor a ser salvo.
 * @return true se a operação foi bem-sucedida, false caso contrário.
 */
bool salvar_int_nvs(const char* chave, int32_t valor) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        log_erro(TAG_NVS, "Falha ao abrir NVS para escrita.");
        return false;
    }

    err = nvs_set_i32(handle, chave, valor);
    if (err != ESP_OK) {
        log_erro(TAG_NVS, "Falha ao salvar valor inteiro.");
    } else {
        nvs_commit(handle);
        log_info(TAG_NVS, ("Valor salvo: " + String(chave) + " = " + String(valor)).c_str());
    }
    nvs_close(handle);
    return err == ESP_OK;
}

/**
 * @brief Lê um valor inteiro do NVS.
 * 
 * @param chave A chave para o valor.
 * @param valor_padrao O valor a ser retornado se a chave não for encontrada.
 * @return O valor lido ou o valor padrão.
 */
int32_t ler_int_nvs(const char* chave, int32_t valor_padrao) {
    nvs_handle_t handle;
    int32_t valor = valor_padrao;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        log_erro(TAG_NVS, "Falha ao abrir NVS para leitura.");
        return valor_padrao;
    }

    err = nvs_get_i32(handle, chave, &valor);
    nvs_close(handle);

    switch (err) {
        case ESP_OK:
            log_info(TAG_NVS, ("Valor lido: " + String(chave) + " = " + String(valor)).c_str());
            return valor;
        case ESP_ERR_NVS_NOT_FOUND:
            log_info(TAG_NVS, ("Chave não encontrada, usando valor padrão: " + String(valor_padrao)).c_str());
            return valor_padrao;
        default:
            log_erro(TAG_NVS, "Erro ao ler valor inteiro.");
            return valor_padrao;
    }
}

// Funções para outros tipos (string, blob) podem ser adicionadas conforme a necessidade.

#endif // NVS_CONFIG_H
