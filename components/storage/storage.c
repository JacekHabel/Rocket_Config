#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "STORAGE";

static nvhandl_t handle;

static esp_err_t pre_write_check(const char *key, nvs_type_t type, bool *exists) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }    

    if (!key || strlen(key) == 0 || strlen(key) > NVS_KEY_NAME_MAX_SIZE - 1) {
        return ESP_ERR_INVALID_ARG;
    }
        
    nvs_type_t existing;
    esp_err_t err = nvs_find_key(handle, key, &existing);

    if (err == ESP_ERR_NVS_NOT_FOUND) { 
        *exists = false;
        return ESP_OK;
    }

    if (err != ESP_OK) {
        return err;
    }

    *exists = true;
    return (existing == type) ? ESP_OK : ESP_ERR_INVALID_STATE;  // zmiana typu zabroniona
}

static esp_err_t commit_if_ok(esp_err_t err)
{
    if (err != ESP_OK) {
        return err;    
    } 

    err = nvs_commit(handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t storage_init(void) {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs recovery, erasing partition");

        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS erase failed: %s", esp_err_to_name(err));
            return err;
        }

        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Opening NVS handle...");
    err = nvs_open("storage", NVS_READWRITE, &handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Storage initialized");

    return ESP_OK;    
}

esp_err_t storage_set_i32(const char *key, int32_t value) {
    bool exists;
    
    esp_err_t err = pre_write_check(key, NVS_TYPE_I32, &exists);
    
    if (err != ESP_OK) {
        return err;
    }

    if (exists) {
        int32_t cur;
        if (nvs_get_i32(handle, key, &cur) == ESP_OK && cur == value)
            return ESP_OK;                      
    }

    return commit_if_ok(nvs_set_i32(handle, key, value));
}

esp_err_t storage_set_f32(const char *key, float value) {
    
}

esp_err_t storage_set_bool(const char *key, bool value) {

}

esp_err_t storage_set_str(const char *key, const char *value) {
  
}

esp_err_t storage_get_i32(const char *key, int32_t *out) {

}
esp_err_t storage_get_f32(const char *key, float *out) {

}
esp_err_t storage_get_bool(const char *key, bool *out) {

}
esp_err_t storage_get_str(const char *key, char *buf, size_t buf_len) {

}

esp_err_t storage_delete(const char *key) {

}

esp_err_t storage_list_all(void) {
    
}