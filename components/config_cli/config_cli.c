#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "config_cli.h"
#include "storage.h"


#define STORAGE_KEY_MAX_LEN 15

static bool key_is_valid(const char *key)
{
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
