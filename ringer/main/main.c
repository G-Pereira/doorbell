#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <esp_log.h>

void app_main()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);

    while(1) {
        uint32_t val = adc1_get_raw(ADC1_CHANNEL_6);
        ESP_LOGI("sample_mic" , "MIC: %d", val);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
