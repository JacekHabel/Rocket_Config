#pragma once

typedef enum {
    STORAGE_INT,
    STORAGE_FLOAT,
    STORAGE_BOOL,
    STORAGE_STRING
} storage_type_t;

esp_err_t storage_init(void);

esp_err_t storage_write_i32(const char *key, storage_type_t type, const void *value);

esp_err_t storage_read_i32(const char *key, storage_type_t type, void *value);

esp_err_t storage_delete(const char *key);

esp_err_t storage_list(void);



