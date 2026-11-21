#pragma once

#include <stddef.h>


#include "esp_err.h"

#define STORAGE_STR_MAX_LEN 64 

typedef enum {
    STORAGE_INT,
    STORAGE_FLOAT,
    STORAGE_BOOL,
    STORAGE_STRING,
} storage_type_t;

typedef void (*storage_list_cb_t)(
    const char *key,
    storage_type_t type,
    const char *value_str,
    bool is_default,
    void *ctx
);

esp_err_t storage_init(void);
void storage_deinit(void); //for tests

esp_err_t storage_set_i32(const char *key, int32_t value);
esp_err_t storage_set_float(const char *key, float value);
esp_err_t storage_set_bool(const char *key, bool value);
esp_err_t storage_set_str(const char *key, const char *value);

esp_err_t storage_get_i32(const char *key, int32_t *value);
esp_err_t storage_get_float(const char *key, float *value);
esp_err_t storage_get_bool(const char *key, bool *value);
esp_err_t storage_get_str(const char *key, char *buf, size_t buf_len);

esp_err_t storage_delete_element(const char *key);

esp_err_t storage_list_all(storage_list_cb_t cb, void *ctx);



