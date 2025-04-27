#include "http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "servo.h"
#include <string.h>

static const char *TAG = "http_server";
static httpd_handle_t server = NULL;

// Handler for getting current servo angle
static esp_err_t get_angle_handler(httpd_req_t *req)
{
    char response[32];
    snprintf(response, sizeof(response), "{\"angle\":%d}", current_angle);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Handler for setting servo angle
static esp_err_t set_angle_handler(httpd_req_t *req)
{
    char buf[32];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive data");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    int angle;
    if (sscanf(buf, "angle=%d", &angle) == 1) {
        if (angle >= 0 && angle <= 180) {
            set_servo_angle(angle);
            httpd_resp_sendstr(req, "OK");
            return ESP_OK;
        }
    }
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid angle");
    return ESP_FAIL;
}

static const char html_template[] =
             "<!DOCTYPE html><html><head><title>ESP32 Servo Control</title>"
             "<meta name='viewport' content='width=device-width, initial-scale=1'>"
             "<style>"
             "body { font-family: Arial, sans-serif; margin: 20px; text-align: center; }"
             ".slider-container { margin: 20px auto; max-width: 300px; }"
             ".slider { width: 100%%; height: 25px; }"
             ".angle-display { font-size: 24px; margin: 10px; }"
             "</style>"
             "</head><body>"
             "<h1>ESP32 Servo Control</h1>"
             "<div class='slider-container'>"
             "<input type='range' class='slider' id='angleSlider' min='0' max='180' value='0'>"
             "<div class='angle-display'>Angle: <span id='angleValue'>0</span>°</div>"
             "</div>"
             "<script>"
             "const slider = document.getElementById('angleSlider');"
             "const angleDisplay = document.getElementById('angleValue');"
             "let updateTimeout;"
             ""
             "function updateAngle(angle) {"
             "  angleDisplay.textContent = angle;"
             "  fetch('/set_angle', {"
             "    method: 'POST',"
             "    body: 'angle=' + angle"
             "  });"
             "}"
             ""
             "slider.addEventListener('input', function() {"
             "  const angle = this.value;"
             "  angleDisplay.textContent = angle;"
             "  clearTimeout(updateTimeout);"
             "  updateTimeout = setTimeout(() => updateAngle(angle), 30);"
             "});"
             ""
             "// Initial angle update"
             "fetch('/get_angle')"
             "  .then(response => response.json())"
             "  .then(data => {"
             "    slider.value = data.angle;"
             "    angleDisplay.textContent = data.angle;"
             "  });"
             "</script>"
             "</body></html>";


// Handler for the root path "/"
static esp_err_t root_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Sending HTML template");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Type", "text/html; charset=utf-8");
    httpd_resp_send(req, html_template, strlen(html_template));
    return ESP_OK;
}

// URI handler structures
static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t get_angle = {
    .uri       = "/get_angle",
    .method    = HTTP_GET,
    .handler   = get_angle_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t set_angle = {
    .uri       = "/set_angle",
    .method    = HTTP_POST,
    .handler   = set_angle_handler,
    .user_ctx  = NULL
};

// Start the HTTP server
void http_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 8;

    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &get_angle);
        httpd_register_uri_handler(server, &set_angle);
        ESP_LOGI(TAG, "HTTP server started");
    } else {
        ESP_LOGE(TAG, "Error starting server!");
    }
}

// Stop the HTTP server
void http_server_stop(void)
{
    if (server) {
        httpd_stop(server);
        ESP_LOGI(TAG, "HTTP server stopped");
    }
} 