#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "servo.h"
#include "setup_console.h"
#include "wifi.h"
#include "http_server.h"

void register_system_commands(void);

void app_main(void)
{
    init_servo();
    wifi_init_sta();
    esp_task_wdt_deinit();

    init_console();
    
    // Зарегистрировать команды
    register_system_commands();
    register_servo_command();
    register_servo_smooth_command();
    register_ip_command();
    esp_console_register_help_command();

    // Initialize HTTP server
    http_server_init();

    run_console();
}
