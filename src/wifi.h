#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "esp_console.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASS

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("Получен IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
    }
}

void wifi_init_sta(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}


static int get_ip_cmd(int argc, char **argv)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif == NULL) {
        printf("Не удалось найти интерфейс\n");
        return 1;
    }

    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        printf("IP-адрес: " IPSTR "\n", IP2STR(&ip_info.ip));
        printf("Шлюз: " IPSTR "\n", IP2STR(&ip_info.gw));
        printf("Маска: " IPSTR "\n", IP2STR(&ip_info.netmask));
    } else {
        printf("Нет IP-адреса\n");
    }
    return 0;
}

void register_ip_command(void)
{
    const esp_console_cmd_t cmd = {
        .command = "get_ip",
        .help = "Показать текущий IP-адрес",
        .hint = NULL,
        .func = &get_ip_cmd,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}