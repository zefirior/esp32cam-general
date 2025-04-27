#include <stdio.h>
#include <string.h>
#include "esp_console.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"


static int system_info_cmd(int argc, char **argv)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is an ESP32 chip with %d CPU cores\n", chip_info.cores);
    return 0;
}

void register_system_commands(void)
{
    const esp_console_cmd_t cmd = {
        .command = "info",
        .help = "Print system information",
        .hint = NULL,
        .func = &system_info_cmd,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

void init_console(void)
{
    // Настройка стандартного ввода-вывода (UART)
    setvbuf(stdin, NULL, _IONBF, 0);

    // Настройка esp_console
    esp_console_config_t console_config = {
        .max_cmdline_length = 256,
        .max_cmdline_args = 8
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    // Настройка linenoise (библиотека ввода строк с историей)
    linenoiseSetMultiLine(true);
    linenoiseSetDumbMode(1);
    linenoiseHistorySetMaxLen(100);
    linenoiseAllowEmpty(false);
}

void run_console(void)
{
    printf("\n"
        "ESP32 interactive console.\n"
        "Type 'help' to list commands.\n");

    // Главный цикл
    while (true) {
        char *line = linenoise("> ");
        if (line == NULL) {
            continue;
        }

        if (strlen(line) > 0) {
            linenoiseHistoryAdd(line);
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND) {
                printf("Unrecognized command\n");
            } else if (err == ESP_OK && ret != ESP_OK) {
                printf("Command returned non-zero error code: 0x%x\n", ret);
            } else if (err != ESP_OK) {
                printf("Internal error: %s\n", esp_err_to_name(err));
            }
        }
        linenoiseFree(line);
    }
}