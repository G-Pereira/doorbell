#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "nvs_flash.h"

#define SSID "Doorbell"
#define PASS "knockknock"

static EventGroupHandle_t wifi_egroup;
static const char wifitag[] = "WiFi";
const int WIFI_CONNECTED_BIT = BIT0;

static esp_err_t startAP_handler(void *ctx, system_event_t *event){
	switch(event->event_id){
	case SYSTEM_EVENT_AP_STACONNECTED:
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		break;
	default:
		break;
	}
	return ESP_OK;
}

void startAP(){
	wifi_egroup = xEventGroupCreate();

	tcpip_adapter_init();

	/*SET IP ADRESS OF AP*/
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    tcpip_adapter_ip_info_t info;
    memset(&info, 0, sizeof(info));
    IP4_ADDR(&info.ip, 10, 0, 0, 1);
    IP4_ADDR(&info.gw, 10, 0, 0, 1);
    IP4_ADDR(&info.netmask, 255, 0, 0, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

	ESP_ERROR_CHECK(esp_event_loop_init(startAP_handler, NULL));

    wifi_init_config_t wifiInitializationConfig = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitializationConfig));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t ap_config = {
          .ap = {
            .ssid = SSID,
			.password = PASS,
            .channel = 0,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 1,
            .beacon_interval = 100
          }
        };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    ESP_ERROR_CHECK(esp_wifi_start());

}

static esp_err_t connectWiFi_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(wifitag, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_egroup, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(wifitag, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(wifitag, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_egroup, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void connectWiFi(){
    wifi_egroup = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA));
    tcpip_adapter_ip_info_t ipInfo;
    IP4_ADDR(&ipInfo.ip, 10,0,0,2);
    IP4_ADDR(&ipInfo.gw, 10,0,0,1);
    IP4_ADDR(&ipInfo.netmask, 255,0,0,0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo));


    ESP_ERROR_CHECK(esp_event_loop_init(connectWiFi_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(wifitag, "wifi_init_sta finished.");
    ESP_LOGI(wifitag, "connect to ap SSID:%s password:%s", SSID, PASS);
}
