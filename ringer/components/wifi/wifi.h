#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define ESP_WIFI_SSID      "doorbell"
#define ESP_WIFI_PASS      "knockknock"
#define ESP_WIFI_MAXIMUM_RETRY  5

void wifi_event_handler();
void connectWifi();
