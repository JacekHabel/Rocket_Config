#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "STORAGE";

static nvs_handle_t handle;


esp_err_t storage_init(void) {
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs recovery, erasing partition");

        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "NVS erase failed: %s",
                     esp_err_to_name(err));
            return err;
        }

        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS initialization failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Opening NVS handle...");
    err = nvs_open("storage", NVS_READWRITE, &handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s",
                 esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Storage initialized");

    return ESP_OK;    
}

esp_err_t storage_set_i32(const char *key, int32_t value) {

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