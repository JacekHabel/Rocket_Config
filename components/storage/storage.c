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
    nvs_flash_init(void);
    
}

esp_err_t storage_write(const char *key, storage_type_t type, const void *value) {
   esp_err_t err;

    switch (type) {

        case STORAGE_INT:
            err = nvs_set_i32(
                handle,
                key,
                *(const int32_t *)value
            );
            break;

        case STORAGE_FLOAT:
            err = nvs_set_blob(
                handle,
                key,
                value,
                sizeof(float)
            );
            break;

        case STORAGE_BOOL:
            err = nvs_set_u8(
                handle,
                key,
                *(const bool *)value ? 1 : 0
            );
            break;

        case STORAGE_STRING:
            err = nvs_set_str(
                handle,
                key,
                (const char *)value
            );
            break;

        default:
            return ESP_ERR_INVALID_ARG;
    }

    if (err != ESP_OK) {
        return err;
    }

    return nvs_commit(handle);
}

esp_err_t storage_read(const char *key, int32_t *value) {
   
}

esp_err_t storage_delete(const char *key) {

}

esp_err_t storage_list(void) {
    
}