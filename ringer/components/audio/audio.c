#include "esp_system.h"
#include <errno.h>
#include "esp_log.h"
#include <string.h>
#include "driver/dac.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#define INTERCOM_IP "10.0.0.1"
#define RINGER_IP "10.0.0.2"
#define AUDIO_PORT1 8001
#define AUDIO_PORT2 8002

void ringer_audiorx(void *pvParameters){

	char tag[] = "ringer_audiorx";
	char rx_buffer[255];
	char addr_str[128];
	struct sockaddr_in dest_addr;
	int addr_family;
	int ip_protocol;
	int port = 2015;

	while(1){
		dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(AUDIO_PORT1);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(tag, "Socket created");
	    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	    if (err < 0) {
	    	ESP_LOGE(tag, "Socket unable to bind: errno %d", errno);
	    }
	    ESP_LOGI(tag, "Socket bound, port %d", port);
	    while(1){
	      	ESP_LOGI(tag, "Waiting for data");
	       	struct sockaddr_in source_addr;
	       	socklen_t socklen = sizeof(source_addr);
	       	int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
	        if (len < 0) {
	        	ESP_LOGE(tag, "recvfrom failed: errno %d", errno);
	            break;
	        }
	        else{
	        	inet_ntoa_r((&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
	            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
	            ESP_LOGI(tag, "Received %d bytes from %s:", len, addr_str);
	            ESP_LOGI(tag, "%s", rx_buffer);
	        }
	    }
	    if (sock != -1) {
	    	ESP_LOGE(tag, "Shutting down socket and restarting...");
	        shutdown(sock, 0);
	        close(sock);
	    }
	}
	vTaskDelete(NULL);
}

void ringer_audiotx(uint32_t data){
	char tag[] = "ringer_audiotx";
	char addr_str[128];
	int addr_family;
	int ip_protocol;

		struct sockaddr_in dest_addr;
		dest_addr.sin_addr.s_addr = inet_addr(INTERCOM_IP);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(8001);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
			return;
		}
		ESP_LOGI(tag, "Socket created, sending to %s:%d", INTERCOM_IP, 8001);
		char dataa = 0x8f;
			int err = sendto(sock, &dataa, 2, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if(err < 0){
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
	            return;
			}
			ESP_LOGI(tag, "Message sent: %zu", data);
	    	shutdown(sock, 0);
	        close(sock);
}

void ringer_audiotx2(uint32_t data){
	int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8001); 
    servaddr.sin_addr.s_addr = inet_addr("10.0.0.1");

	uint8_t convertedData = data/4096*256;

	sendto(sockfd, &convertedData, 1, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
    close(sockfd);
}

void intercom_audiorx(void *pvParameters){

}

void intercom_audiotx(void *pvParameters){

}
