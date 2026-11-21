#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define STORAGE_NAMESPACE "storage"

static const char *TAG = "STORAGE";

static nvs_handle_t s_handle;

static bool s_initialized = false;

static bool key_is_valid(const char *key) {
    return key != NULL && key[0] != '\0' && strlen(key) <= NVS_KEY_NAME_MAX_SIZE - 1;
}

// Default configuration
typedef struct {
    const char *key;
    storage_type_t type;
    union {
        int32_t     i;
        float       f;
        bool        b;
        const char *s;
    } def;
} storage_default_t;

static const storage_default_t s_defaults[] = {
    { "thrust",   STORAGE_INT,    .def.i = 1000 },
    { "mass",     STORAGE_FLOAT,  .def.f = 25.5f },
    { "armed",    STORAGE_BOOL,   .def.b = false },
    { "name",     STORAGE_STRING, .def.s = "rocket" },
};

#define DEFAULTS_COUNT (sizeof(s_defaults) / sizeof(s_defaults[0]))

static const storage_default_t *find_default(const char *key, storage_type_t type)
{
    for (size_t i = 0; i < DEFAULTS_COUNT; i++) {
        if (s_defaults[i].type == type && strcmp(s_defaults[i].key, key) == 0) {
            return &s_defaults[i];
        }
    }
    return NULL;
}

static esp_err_t pre_write_check(const char *key, nvs_type_t type, bool *exists) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }    

    if (!key_is_valid(key)) {
        return ESP_ERR_INVALID_ARG;
    }

    nvs_type_t existing;
    esp_err_t err = nvs_find_key(s_handle, key, &existing);

    if (err == ESP_ERR_NVS_NOT_FOUND) { 
        *exists = false;
        return ESP_OK;
    }

    if (err != ESP_OK) {
        return err;
    }

    *exists = true;
    return (existing == type) ? ESP_OK : ESP_ERR_INVALID_STATE;  
}

static esp_err_t commit_if_ok(esp_err_t err)
{
    if (err != ESP_OK) {
        return err;    
    } 

    err = nvs_commit(s_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t storage_init(void) {
    if (s_initialized) {
        return ESP_OK;
    }

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
    err = nvs_open("STORAGE_NAMESPACE", NVS_READWRITE, &s_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        return err;
    }

    s_initialized = true;
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
        if (nvs_get_i32(s_handle, key, &cur) == ESP_OK && cur == value) {
            return ESP_OK;        
        }
    }

    return commit_if_ok(nvs_set_i32(s_handle, key, value));
}


esp_err_t storage_set_float(const char *key, float value) {
    bool exists;

    esp_err_t err = pre_write_check(key, NVS_TYPE_FLOAT, &exists);
    
    if (err != ESP_OK) {
        return err;
    } 

    if (exists) {
        float cur;
        if (nvs_get_float(s_handle, key, &cur) == ESP_OK && cur == value) {
            return ESP_OK;
        }
    }
    return commit_if_ok(nvs_set_float(s_handle, key, value));
}

esp_err_t storage_set_bool(const char *key, bool value) {
    bool exists;

    esp_err_t err = pre_write_check(key, NVS_TYPE_U8, &exists);
    if (err != ESP_OK) {
        return err;
    }

    uint8_t v = value ? 1 : 0;
    if (exists) {
        uint8_t cur;
        if (nvs_get_u8(s_handle, key, &cur) == ESP_OK && cur == v) {
            return ESP_OK;
        }
    }
    return commit_if_ok(nvs_set_u8(s_handle, key, v));
}

esp_err_t storage_set_str(const char *key, const char *value) {
    if (value == NULL || strlen(value) > STORAGE_STR_MAX_LEN) {
        return ESP_ERR_INVALID_ARG;
    }

    bool exists;
    esp_err_t err = pre_write_check(key, NVS_TYPE_STR, &exists);

    if (err != ESP_OK) {
        return err;
    }

    if (exists) {
        char cur[STORAGE_STR_MAX_LEN + 1];
        size_t len = sizeof(cur);

        if (nvs_get_str(s_handle, key, cur, &len) == ESP_OK && strcmp(cur, value) == 0) {
            return ESP_OK;
        }
    }
    return commit_if_ok(nvs_set_str(s_handle, key, value));
}

esp_err_t storage_get_i32(const char *key, int32_t *value) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if(!key_is_valid(key) || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = nvs_get_i32(s_handle, key, value);

    if (err == ESP_OK) {
        return err;
    }

    // If no value found, get default.  
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        const storage_default_t *d = find_default(key, STORAGE_INT);
        if (d) {
            *value = d->def.i; 
            return ESP_OK; 
        }
    }
    return err;
}

esp_err_t storage_get_float(const char *key, float *value) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!key_is_valid(key) || value == NULL) { 
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = nvs_get_float(s_handle, key, value);

     if (err == ESP_OK) {
        return err;
    }

    // If no value found, get default.  
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        const storage_default_t *d = find_default(key, STORAGE_FLOAT);
        if (d) {
            *value = d->def.f;
            return ESP_OK; 
        }
    }
    return err;
}

esp_err_t storage_get_bool(const char *key, bool *value) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!key_is_valid(key) || value == NULL) { 
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t v;
    esp_err_t err = nvs_get_u8(s_handle, key, &v);
    
    if (err == ESP_OK) {
        *value = (v != 0);
        return ESP_OK;
    }

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        const storage_default_t *d = find_default(key, STORAGE_BOOL);
        if (d) {
            *value = d->def.b;
            return ESP_OK; 
        }
    }
    return err;
}

esp_err_t storage_get_str(const char *key, char *buf, size_t buf_len) {
    if (!s_initialized) { 
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!key_is_valid(key) || buf == NULL || buf_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t len = buf_len;
    esp_err_t err = nvs_get_str(s_handle, key, buf, &len);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        const storage_default_t *d = find_default(key, STORAGE_STRING);
        
        if (d) {
            if (strlen(d->def.s) + 1 > buf_len) {
                return ESP_ERR_INVALID_SIZE;
            }
            
            strcpy(buf, d->def.s);
            return ESP_OK;
        }
    }
    return err;   
}

esp_err_t storage_delete_element(const char *key) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!key_is_valid(key)) {
        return ESP_ERR_INVALID_ARG;
    }

    return commit_if_ok(nvs_erase_key(s_handle, key)); 
}

esp_err_t storage_list_all(storage_list_cb_t cb, void *ctx) {
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (cb == NULL) { 
        return ESP_ERR_INVALID_ARG;
    }

    char text[STORAGE_STR_MAX_LEN + 1];

    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find(NVS_DEFAULT_PART_NAME, STORAGE_NAMESPACE, NVS_TYPE_ANY, &it);

    while (err == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        switch (info.type) {
            case NVS_TYPE_I32: {
                int32_t v;
                if (nvs_get_i32(s_handle, info.key, &v) == ESP_OK) {
                    snprintf(text, sizeof(text), "%ld", (long)v);
                    cb(info.key, STORAGE_INT, text, false, ctx);
                }
                break;
            }
            case NVS_TYPE_FLOAT: {
                float v;
                if (nvs_get_float(s_handle, info.key, &v) == ESP_OK) {
                    snprintf(text, sizeof(text), "%.6g", (double)v);
                    cb(info.key, STORAGE_FLOAT, text, false, ctx);
                }
                break;
            }
            case NVS_TYPE_U8: {
                uint8_t v;
                if (nvs_get_u8(s_handle, info.key, &v) == ESP_OK) {
                    cb(info.key, STORAGE_BOOL, v ? "true" : "false", false, ctx);
                }
                break;
            }
            case NVS_TYPE_STR: {
                size_t len = sizeof(text);
                if (nvs_get_str(s_handle, info.key, text, &len) == ESP_OK) {
                    cb(info.key, STORAGE_STRING, text, false, ctx);
                }
                break;
            }
            default:
                break;   
        }       
        err = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);

    //Defaults which are not stored yet
    for (size_t i = 0; i < DEFAULTS_COUNT; i++) {
        const storage_default_t *d = &s_defaults[i];
        nvs_type_t existing;
        if (nvs_find_key(s_handle, d->key, &existing) != ESP_ERR_NVS_NOT_FOUND) {
            continue;
        }
        switch (d->type) {
            case STORAGE_INT:
                snprintf(text, sizeof(text), "%ld", (long)d->def.i);
                break;
            case STORAGE_FLOAT:
                snprintf(text, sizeof(text), "%.6g", (double)d->def.f);
                break;
            case STORAGE_BOOL:
                snprintf(text, sizeof(text), "%s", d->def.b ? "true" : "false");
                break;
            case STORAGE_STRING:
                snprintf(text, sizeof(text), "%s", d->def.s);
                break;
        }
        cb(d->key, d->type, text, true, ctx);
    }

    return ESP_OK;
}
