/* Door Bell with PIR sensor and OV2640 camera

   This project is licensed under a - insert license here - license.

   Developed by:

   diogo.correia@fe.up.pt
   francisco.pimenta@fe.up.pt
   goncalo.pereira@fe.up.pt

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_event_loop.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_camera.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

#define GPIO_INPUT_PIN_SEL  ((1ULL<<CONFIG_PIR_PIN)  | (1ULL<<CONFIG_BELL_BUT_PIN))
#define ESP_INTR_FLAG_DEFAULT 0

#define MAX_FRAMES_PIR 50
#define FRAMES_INTERVAL_PIR_MS 100

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void IRAM_ATTR gpio_isr_handler(void* arg);
static void gpio_task_example(void* arg);

static void initialise_wifi(void);
httpd_handle_t start_webserver(void);
static void stop_webserver(httpd_handle_t server);

esp_err_t _camera_init();
esp_err_t _camera_reinit();

esp_err_t jpg_httpd_handler(httpd_req_t *req);
esp_err_t jpg_stream_httpd_handler(httpd_req_t *req);
esp_err_t camera_config_httpd_handler(httpd_req_t *req);
static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len);

static const char* TAG = "door_bell";

static xQueueHandle gpio_evt_queue = NULL;
const int CONNECTED_BIT = BIT0;
static EventGroupHandle_t s_wifi_event_group;
static ip4_addr_t s_ip_addr;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static camera_config_t camera_config = {

    .pin_pwdn  = CONFIG_CAM_PIN_PWDN,
    .pin_reset = CONFIG_CAM_PIN_RESET,
    .pin_xclk = CONFIG_CAM_PIN_XCLK,
    .pin_sscb_sda = CONFIG_CAM_PIN_SIOD,
    .pin_sscb_scl = CONFIG_CAM_PIN_SIOC,

    .pin_d7 = CONFIG_CAM_PIN_D7,
    .pin_d6 = CONFIG_CAM_PIN_D6,
    .pin_d5 = CONFIG_CAM_PIN_D5,
    .pin_d4 = CONFIG_CAM_PIN_D4,
    .pin_d3 = CONFIG_CAM_PIN_D3,
    .pin_d2 = CONFIG_CAM_PIN_D2,
    .pin_d1 = CONFIG_CAM_PIN_D1,
    .pin_d0 = CONFIG_CAM_PIN_D0,
    .pin_vsync = CONFIG_CAM_PIN_VSYNC,
    .pin_href = CONFIG_CAM_PIN_HREF,
    .pin_pclk = CONFIG_CAM_PIN_PCLK,

    .xclk_freq_hz = CONFIG_XCLK_FREQ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,//FRAMESIZE_UXGA,FRAMESIZE_QXGA...

    .jpeg_quality = CONFIG_JPEG_QUALITY,
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};


camera_fb_t camera_buff[MAX_FRAMES_PIR];

/**
 * Brief:
 * RING doorbell but better
 *
 */

esp_err_t _camera_init(){
    //power up the camera if PWDN pin is defined
    ESP_LOGI(TAG,"Camera Initialization");

    if(CONFIG_CAM_PIN_PWDN != -1){
        
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = ((1ULL<<CONFIG_CAM_PIN_PWDN));
        gpio_config(&io_conf);
        gpio_set_level(CONFIG_CAM_PIN_PWDN, 0);
        
    }
    // Initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Initialization Failed");
        return err;
    }
    ESP_LOGI(TAG, "Camera Initialization Done!");

    return ESP_OK;
}

esp_err_t _camera_reinit(){
    //power up the camera if PWDN pin is defined
    ESP_LOGI(TAG,"Camera Reinitialization");
    
    // Initialize the camera
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Disable Failed");
        return err;
    }
    err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Initialization Failed");
        return err;
    }
    ESP_LOGI(TAG, "Camera Initialization Done!");

    return ESP_OK;
}


static void IRAM_ATTR gpio_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg){
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

            if( io_num == CONFIG_PIR_PIN ){
                
                int64_t fr_start = esp_timer_get_time();
                int64_t fr_end = 0;
                camera_fb_t * fb;

                ESP_LOGI(TAG,"PIR Sensor Activated");
                uint8_t k = 0;

                while(k < MAX_FRAMES_PIR){
                    if( (uint32_t)((esp_timer_get_time() - fr_end)/1000) > FRAMES_INTERVAL_PIR_MS){
                        fr_start = esp_timer_get_time();

                        fb = esp_camera_fb_get();

                        camera_buff[k].buf = malloc(fb->len);
                        memcpy(camera_buff[k].buf,fb->buf, fb->len);
                        camera_buff[k].len = fb->len;
                        camera_buff[k].width = fb->width;
                        camera_buff[k].height = fb->height;
                        camera_buff[k].format = fb->format;

                        if (!fb) {
                            ESP_LOGE(TAG, "Camera capture failed");
                        }            
                        fr_end = esp_timer_get_time();
                        ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb->len/1024), (uint32_t)((fr_end - fr_start)/1000));
                        k+=1;
                        }
                }
                
            }
            else if ( io_num == CONFIG_BELL_BUT_PIN ){
                ESP_LOGI(TAG, "THE BELL IS RINGING");
            }
            
        }
    }
}

/* Main function */

void app_main(){

    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("gpio", ESP_LOG_WARN);

    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }

    ESP_LOGI(TAG, "Starting Application");

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //enable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    //io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    //io_conf.pin_bit_mask = CONFIG_PIR_PIN;
    //set as input mode    
    //io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    //gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(CONFIG_PIR_PIN,GPIO_PIN_INTR_POSEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreatePinnedToCore(gpio_task_example, "gpio_task_example", 2048, NULL, 5, NULL,1);
    //xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 5, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(CONFIG_PIR_PIN, gpio_isr_handler, (void*) CONFIG_PIR_PIN);
    gpio_isr_handler_add(CONFIG_BELL_BUT_PIN, gpio_isr_handler, (void*) CONFIG_BELL_BUT_PIN);
    //hook isr handler for specific gpio pin
    
    _camera_init();
    initialise_wifi();
    httpd_handle_t  server = start_webserver();


    while(1) {

        vTaskDelay(10 / portTICK_RATE_MS);

    }

    stop_webserver(server);
}

/* Handle WiFi events such as a lost connection */

static esp_err_t event_handler(void *ctx, system_event_t *event){
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
            s_ip_addr = event->event_info.got_ip.ip_info.ip;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

/* Function to initialise the WiFi and connect to an hotspot */

static void initialise_wifi(void){
    tcpip_adapter_init();
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
    ESP_LOGI(TAG, "Connecting to \"%s\"", wifi_config.sta.ssid);
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected");
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

/* Function to capture frame and send it as the HTTP GET response */

esp_err_t jpg_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}

/* Function to handle a JPEG stream */
/* @TODO Add a way to break out of the while(true) */

esp_err_t jpg_stream_httpd_handler(httpd_req_t *req){

    gpio_intr_disable(CONFIG_PIR_PIN);

    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;

    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
            if(fb->format != PIXFORMAT_JPEG){
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if(!jpeg_converted){
                    ESP_LOGE(TAG, "JPEG compression failed");
                    esp_camera_fb_return(fb);
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    gpio_intr_enable(CONFIG_PIR_PIN);
    return res;
}

/* Function to handle a JPEG stream */
/* @TODO Add a way to break out of the while(true) */

esp_err_t pir_jpg_httpd_handler(httpd_req_t *req){
    
    gpio_intr_disable(CONFIG_PIR_PIN);
    
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = 0;
    uint8_t k = 0;
    camera_fb_t * fb = NULL;
    char * part_buf[64];
    static int64_t last_frame = 0;

    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(1){    
        fb = &camera_buff[k];
        
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        } else {
            if(fb->format != PIXFORMAT_JPEG){
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                if(!jpeg_converted){
                    ESP_LOGE(TAG, "JPEG compression failed");
                    esp_camera_fb_return(fb);
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }

        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        frame_time /= 1000;

        if(frame_time > FRAMES_INTERVAL_PIR_MS){
            k = (k + 1) % MAX_FRAMES_PIR;
            last_frame = fr_end;
            ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
                (uint32_t)(_jpg_buf_len/1024),
                    (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
        }
        else{
            vTaskDelay(FRAMES_INTERVAL_PIR_MS/portTICK_RATE_MS);
        }
    }
    
    last_frame = 0;
    
    gpio_intr_enable(CONFIG_PIR_PIN);
    return res;
    
}

/* Function to capture frame and send it as the HTTP GET response */
esp_err_t camera_config_httpd_handler(httpd_req_t *req){

    esp_err_t res = ESP_OK;

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    char*  buf;
    size_t buf_len;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[16];
            char resp[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "quality", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => quality=%s", param);
                camera_config.jpeg_quality = atoi(param);

                res = _camera_reinit(&camera_config);
                if( res != ESP_OK){
                    sprintf(resp, "%d - Couldn't config the camera.", res);
                }
                sprintf(resp, "%d - Camera init done.", res);
            
                httpd_resp_send(req, resp, sizeof(resp));
                
            }
            if (httpd_query_key_value(buf, "size", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => size=%s", param);
                
                if( strcmp(param,"QVGA") == 0)
                    camera_config.frame_size = FRAMESIZE_QVGA;
                else if(strcmp(param,"CIF") == 0)
                    camera_config.frame_size = FRAMESIZE_CIF;
                else if(strcmp(param,"VGA") == 0)
                    camera_config.frame_size = FRAMESIZE_VGA;
                else if(strcmp(param,"SVGA") == 0)
                    camera_config.frame_size = FRAMESIZE_SVGA;
                else if(strcmp(param,"XGA") == 0)
                    camera_config.frame_size = FRAMESIZE_XGA;
                else if(strcmp(param,"SXGA") == 0)
                    camera_config.frame_size = FRAMESIZE_SXGA;
                else if(strcmp(param,"UXGA") == 0)
                    camera_config.frame_size = FRAMESIZE_UXGA;

                res = _camera_reinit(&camera_config);
                if( res != ESP_OK){
                    sprintf(resp, "%d - Couldn't config the camera.", res);
                }
                sprintf(resp, "%d - Camera init done.", res);
            
                httpd_resp_send(req, resp, sizeof(resp));
                
            }
        }
        free(buf);

    }
    
    return res;
}


/* URI handler structure for HTTP GET request */
httpd_uri_t uri_pic = {
    .uri      = "/take_pic",
    .method   = HTTP_GET,
    .handler  = jpg_httpd_handler,
    .user_ctx = NULL
};

/* URI handler structure for HTTP GET request */
httpd_uri_t uri_config = {
    .uri      = "/camera_config",
    .method   = HTTP_GET,
    .handler  = camera_config_httpd_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_stream = {
    .uri      = "/stream_stuff",
    .method   = HTTP_GET,
    .handler  = jpg_stream_httpd_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_pir = {
    .uri      = "/pir",
    .method   = HTTP_GET,
    .handler  = pir_jpg_httpd_handler,
    .user_ctx = NULL
};


/* Function for starting the webserver */
httpd_handle_t start_webserver(void){

    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_pic);
        httpd_register_uri_handler(server, &uri_stream);
        httpd_register_uri_handler(server, &uri_config);
        httpd_register_uri_handler(server, &uri_pir);

    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
static void stop_webserver(httpd_handle_t server){
    
    if (server) {
        /* Stop the httpd server */
        ESP_ERROR_CHECK( httpd_stop(server) );
        return;
    }
    ESP_LOGE(TAG,"No Server to Stop");

}
