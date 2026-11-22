#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#include "config_cli.h"
#include "storage.h"
#include "nvs.h"
#include "esp_console.h"

#define TYPES_COUNT (sizeof(s_types) / sizeof(s_types[0]))

#define CLI_OK  0
#define CLI_ERR 1

static bool key_is_valid(const char *key) {
    size_t n = strlen(key);

    if (n == 0 || n > STORAGE_KEY_MAX_LEN) {
        return false;
    }

    for (size_t i = 0; i < n; i++) {
        if (!isalnum((unsigned char)key[i]) && key[i] != '_') {
            return false;
        }
    }
    return true;
}

// Parsers char to valid type 
static bool parse_i32(const char *s, int32_t *out) {
    char *end;
    errno = 0;
    long long v = strtoll(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0' || v < INT32_MIN || v > INT32_MAX) {
        return false;
    }
    *out = (int32_t)v;
    return true;
}

static bool parse_float(const char *s, float *out) {
    char *end;
    errno = 0;
    float v = strtof(s, &end);

    if (errno != 0 || end == s || *end != '\0' || !isfinite(v)) {
        return false;
    }
    *out = v;
    return true;
}

static bool parse_bool(const char *s, bool *out) {
    if (!strcasecmp(s, "true") || !strcmp(s, "1") || !strcasecmp(s, "on")) {
        *out = true;
        return true;
    }

    if (!strcasecmp(s, "false") || !strcmp(s, "0") || !strcasecmp(s, "off")) {
        *out = false;
        return true;
    }
    return false;
}

typedef esp_err_t (*set_fn_t)(const char *key, const char *val, bool *parsed);    

// Setters definitions
static esp_err_t set_int(const char *key, const char *s, bool *parsed)
{
    int32_t v;
    *parsed = parse_i32(s, &v);
    return *parsed ? storage_set_i32(key, v) : ESP_OK;
}

static esp_err_t set_float(const char *key, const char *s, bool *parsed)
{
    float v;
    *parsed = parse_float(s, &v);
    return *parsed ? storage_set_float(key, v) : ESP_OK;
}

static esp_err_t set_bool(const char *key, const char *s, bool *parsed)
{
    bool v;
    *parsed = parse_bool(s, &v);
    return *parsed ? storage_set_bool(key, v) : ESP_OK;
}

static esp_err_t set_string(const char *key, const char *s, bool *parsed)
{
    *parsed = (strlen(s) <= STORAGE_STR_MAX_LEN);
    return *parsed ? storage_set_str(key, s) : ESP_OK;
}

static const struct {
    const char *name;
    set_fn_t    set;
} s_types[] = {
    { "int",    set_int    },
    { "float",  set_float  },
    { "bool",   set_bool   },
    { "str", set_string },
};

static const char *type_name(storage_type_t type) {
    switch(type) {
        case STORAGE_INT: return "int";
        case STORAGE_FLOAT: return "float";
        case STORAGE_BOOL: return "bool";
        case STORAGE_STRING: return "str";
    }
    return "?";
}

/*
-------------- CLI commands ---------------
*/

// 'set int key = value'
static int cmd_set(int argc, char **argv) {
    bool with_eq = (argc == 5 && strcmp(argv[3], "=") == 0);
    
    if (argc != 4 && !with_eq) {
        printf("WRONG COMMAND! Command structure: \n");
        printf("set <int|float|bool|str> <key_name> [=] <value>\n");
        return CLI_ERR;
    }

    const char *type_s = argv[1];
    const char *key    = argv[2];
    const char *val    = argv[argc -1]; 

    if (!key_is_valid(key)) {
        printf("Error: key can only have letters, digits and '_' , length cannot exceed %d signs\n", STORAGE_KEY_MAX_LEN);
        return CLI_ERR;
    }

    for (size_t i = 0; i < TYPES_COUNT; i++) {
        if (strcmp(type_s, s_types[i].name) != 0) {
            continue;
        }

        bool parsed = false;
        esp_err_t err = s_types[i].set(key,val, &parsed);

        if (!parsed) {
            printf("Error: '%s' is not valid value of type %s\n", val, type_s);
            return CLI_ERR;
        }

        if (err == ESP_ERR_INVALID_STATE) {
            printf("Error: '%s' exists with different type. Firstly delete '%s'\n", key, key);
            return CLI_ERR;
        }

        if (err != ESP_OK) {
            printf("Error: error writing to storage: %s\n", esp_err_to_name(err));
            return CLI_ERR;
        }

        printf("Written: %s %s = %s\n", type_s, key, val);
        printf("Noice :D\n");
        return CLI_OK;
    }

    printf("Error: unidentified type '%s'. Use one from avaliable: int, float, bool, str\n", type_s);
    return CLI_ERR;
}

// info of/for wanted data
typedef struct {
    const char    *key;
    bool           found;
    bool           is_default;
    storage_type_t type;
    char           value[STORAGE_STR_MAX_LEN + 1];
} find_ctx_t; 

static void find_cb(const char *key, storage_type_t type, const char *value, bool is_default, void *ctx) {
    find_ctx_t *f = (find_ctx_t *)ctx;

    if (f->found || strcmp(key, f->key) != 0) {
        return;
    }
    
    f->found      = true;
    f->type       = type;
    f->is_default = is_default;
    snprintf(f->value, sizeof(f->value), "%s", value);
}

// 'get key_name'
static int cmd_get(int argc, char **argv) {
    if (argc != 2) {
        printf("Hint: use get <key>\n");
        printf("Hint: learn to write stupid...\n");
        return CLI_ERR;
    }

    find_ctx_t f = { .key = argv[1] };
    esp_err_t err = storage_list_all(find_cb, &f);

    if (err != ESP_OK) {
        printf("Error: %s\n", esp_err_to_name(err));
        return CLI_ERR;
    }
    if (!f.found) {
        printf("Error: key '%s' cannot be found\n", argv[1]);
        printf("Maybe ask yo mama...\n");
        return CLI_ERR;
    }

    printf("Read: %s %s = %s %s\n", type_name(f.type), f.key, f.value, f.is_default ? "(default)" : "");
    return CLI_OK;
}

// 'delate key'
static int cmd_del(int argc, char **argv) {
    if (argc != 2) {
        printf("Hint: del <key>\n");
        printf("Hint: learn to write stupid...\n");
        return CLI_ERR;
    }

    esp_err_t err = storage_delete_element(argv[1]);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        printf("Error: '%s' is not written to flash (does not exists or is default that cannot be erased)\n", argv[1]);
        return CLI_ERR;
    }
    if (err != ESP_OK) {
        printf("Error: %s\n", esp_err_to_name(err));
        return CLI_ERR;
    }

    printf("%s BURNED IN THE LAVA!!!\n", argv[1]);
    return CLI_OK;
}

// list all - helper
static void print_cb(const char *key, storage_type_t type, const char *value, bool is_default, void *ctx) {
    int *count = (int *)ctx;

    printf("%-7s %-15s = %s %s\n", type_name(type), key, value, is_default ? "  (default)" : "");
    (*count)++;
}

// list all
static int cmd_list(int argc, char **argv) {
    (void)argc;
    (void)argv;

    int count = 0;
    
    printf("############################################################\n");
    printf("#               MAGNIFICENT LIST OF STORAGE                #\n");
    printf("############################################################\n");

    printf("%-7s %-15s %s\n", "TYPE", "KEY", "VALUE");

    esp_err_t err = storage_list_all(print_cb, &count);
    if (err != ESP_OK) {
        printf("Error: %s\n", esp_err_to_name(err));
        return CLI_ERR;
    }

    printf("SUM: %d\n", count);
    return CLI_OK;
}

// erases all data and lefts only defaults
static int cmd_reset(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    esp_err_t err = storage_reset();
    if (err != ESP_OK) {
        printf("Blad: %s\n", esp_err_to_name(err));
        return CLI_ERR;
    }

    printf("ALL DATA EATEN BY YO MAMA! only defaults left\n");
    return CLI_OK;
}

// Finaly main boss 

void cli_start(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_cfg = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_cfg.prompt = "rocket> ";
    repl_cfg.task_stack_size = 6144;

    ESP_ERROR_CHECK(esp_console_new_repl_stdio(&repl_cfg, &repl));

    static const esp_console_cmd_t cmds[] = {
        { .command = "set",   .help = "Write key to flash",
          .hint = "<int|float|bool|str> <key> [=] <value>", .func = cmd_set },
        { .command = "get",   .help = "Read key and it's type and value from flash",
          .hint = "<key>", .func = cmd_get },
        { .command = "del",   .help = "Delete variable from flash - searched by key",
          .hint = "<key>", .func = cmd_del },
        { .command = "list",  .help = "List all data in flash",
          .func = cmd_list },
        { .command = "reset", .help = "Erase all data - only defaults left",
          .func = cmd_reset },
    };

    for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&cmds[i]));
    }

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}