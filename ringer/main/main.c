#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <esp_log.h>
#include "wifi.h"
#include "audio.h"
#include "nvs_flash.h"

void sampleMic(){
    while(1) {
        uint32_t val = adc1_get_raw(ADC1_CHANNEL_6);
        ESP_LOGI("sample_mic" , "MIC: %d", val);
        ringer_audiotx2(val);
        //vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    adc1_config_width(ADC_WIDTH_BIT_9);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
    nvs_flash_init();
    nvs_flash_erase();
    connectWiFi();
    xTaskCreate(sampleMic, "sample_mic", 4096, NULL, 5, NULL);
}
