#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#include "config_cli.h"
#include "storage.h"


#define STORAGE_KEY_MAX_LEN 15

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
