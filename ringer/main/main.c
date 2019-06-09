/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "wifi.h"
#include "audio.h"
void app_main()
{
	startAP();
	xTaskCreate(ringer_audiorx, "audio_out", 4096, NULL, 5, NULL);
	printf("SDK version:%s\n", esp_get_idf_version());
}
