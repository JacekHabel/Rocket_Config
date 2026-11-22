# Rocket_config
 
Persistent configuration storage for an ESP32 flight computer, with a small terminal CLI.
 
Configuration variables survive a reboot. They are stored in the ESP32 flash (NVS) and can be created, read, modified and deleted from a serial terminal. When nothing is stored for a variable, a built-in default value is used.
 
## Requirements
 
- ESP32 board
- **ESP-IDF 6.1 or newer** (the module uses the native NVS `float` type, which is not available in older versions)
- A serial connection to the board (UART or USB)

## CLI
Avaliable commands:

| Command | Description |
|---|---|
| `set <type> <key> [=] <value>` | Create or modify a variable |
| `get <key>` | Show type and value of a variable |
| `del <key>` | Delete a variable from flash |
| `list` | List all variables (stored ones and defaults) |
| `reset` | Delete all stored variables, defaults apply again |
| `help` | List the commands |

Supported types:
 
| Type | Accepted values |
|---|---|
| `int` | 32-bit signed integer, decimal |
| `float` | finite decimal number |
| `bool` | `true`, `false`, `1`, `0`, `on`, `off` (case-insensitive)|
| `str` | text up to 64 characters |

The module is a thin typed layer over the ESP-IDF NVS library.
 
| Module type | NVS type |
|---|---|
| `int` | `NVS_TYPE_I32` |
| `float` | `NVS_TYPE_FLOAT` (native, ESP-IDF 6.1+) |
| `bool` | `NVS_TYPE_U8` (0 or 1) |
| `str` | `NVS_TYPE_STR` |
 
The type of a variable can be recovered from NVS alone, so `get` and `list` do not need the type from the user.

### NVS initialization
 
If `nvs_flash_init()` reports `ESP_ERR_NVS_NO_FREE_PAGES` or `ESP_ERR_NVS_NEW_VERSION_FOUND`, the partition is erased and initialized again. **This removes all stored variables.**

### Input validation
 
- Key: 1-15 characters (NVS limit), letters, digits and `_` only.
- Integers: whole text must be a number, range-checked against `int32_t`, overflow detected.
- Floats: whole text must be a number, `nan` and `inf` rejected.
- Booleans: only the listed words, so a typo like `ture` is an error instead of silently becoming `false`.
- Strings: maximum 64 characters.
- Unknown command, unknown type, wrong number of arguments and missing keys produce a message instead of a crash.

## Limitations
 
- One namespace (`storage`).
- `list` is not sorted, the order comes from NVS.
- Unit tests are not included 

 
