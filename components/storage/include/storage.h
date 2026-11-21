#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#define STORAGE_STR_MAX_LEN 64 

typedef enum {
    STORAGE_INT,
    STORAGE_FLOAT,
    STORAGE_BOOL,
    STORAGE_STRING,
} storage_type_t;

esp_err_t storage_init(void);

esp_err_t storage_set_i32(const char *key, int32_t value);
esp_err_t storage_set_f32(const char *key, float value);
esp_err_t storage_set_bool(const char *key, bool value);
esp_err_t storage_set_str(const char *key, const char *value);

esp_err_t storage_get_i32(const char *key, int32_t *value);
esp_err_t storage_get_f32(const char *key, float *value);
esp_err_t storage_get_bool(const char *key, bool *value);
esp_err_t storage_get_str(const char *key, char *buf, size_t buf_len);

esp_err_t storage_delete_element(const char *key);

esp_err_t storage_list_all(void);



