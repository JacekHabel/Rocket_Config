#include "esp_err.h"
#include "esp_log.h"

#include "config_cli.h"
#include "storage.h"

void app_main(void)
{
    ESP_ERROR_CHECK(storage_init());

    /* Keep INFO logs from mixing with the console prompt */
    esp_log_level_set("*", ESP_LOG_WARN);

    cli_start();
}
