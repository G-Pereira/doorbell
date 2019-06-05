void ringer_audiorx(void *pvParameters){

	char tag[] = "ringer_audiorx";
	char rx_buffer[128];
	char addr_str[128];
	struct sockaddr_in dest_addr;
	int addr_family;
	int ip_protocol;
	int port = 2015;

	while(1){
		dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(SOCKET_PORT);
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
};

void intercom_audiorx(void *pvParameters){

	char tag[] = "intercom_audiorx";
	char rx_buffer[128];
	char addr_str[128];
	struct sockaddr_in dest_addr;
	int addr_family;
	int ip_protocol;
	int port = 2020;

	while(1){
		dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(SOCKET_PORT);
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
};

void ringer_audiotx(){
	char tag[] = "ringer_audiotx";
	char rx_buffer[128];
	char addr_str[128];
	char serverip[] = "192.168.4.2";
	int addr_family;
	int ip_protocol;
	int port = 2020;
	char mic_in[] = "Hi Alice!";

	while(1){
		struct sockaddr_in dest_addr;
		dest_addr.sin_addr.s_addr = inet_addr(serverip);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(port);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(tag, "Socket created, sending to %s:%d", serverip, port);
		while(1){
			int err = sendto(sock, mic_in, strlen(mic_in), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if(err < 0){
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
	            break;
			}
			ESP_LOGI(tag, "Message sent");

			struct sockaddr_in source_addr;
			socklen_t socklen = sizeof(source_addr);
			int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

	        // Error occurred during receiving
	        if (len < 0) {
	        	ESP_LOGE(tag, "recvfrom failed: errno %d", errno);
	            break;
	        }
	        // Data received
	        else {
	        	rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
	            ESP_LOGI(tag, "Received %d bytes from %s:", len, addr_str);
	            ESP_LOGI(tag, "%s", rx_buffer);
	        }

	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
	    if (sock != -1) {
	    	ESP_LOGE(tag, "Shutting down socket and restarting...");
	    	shutdown(sock, 0);
	        close(sock);
	    }
	}
	vTaskDelete(NULL);
}

void intercom_audiotx(){
	char tag[] = "ringer_audiotx";
	char rx_buffer[128];
	char addr_str[128];
	char serverip[] = "192.168.4.2";
	int addr_family;
	int ip_protocol;
	int port = 2015;
	char mic_in[] = "Hi Bob!";

	while(1){
		struct sockaddr_in dest_addr;
		dest_addr.sin_addr.s_addr = inet_addr(serverip);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(port);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
		inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0) {
			ESP_LOGE(tag, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(tag, "Socket created, sending to %s:%d", serverip, port);
		while(1){
			int err = sendto(sock, mic_in, strlen(mic_in), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			if(err < 0){
				ESP_LOGE(tag, "Error occurred during sending: errno %d", errno);
	            break;
			}
			ESP_LOGI(tag, "Message sent");

			struct sockaddr_in source_addr;
			socklen_t socklen = sizeof(source_addr);
			int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

	        // Error occurred during receiving
	        if (len < 0) {
	        	ESP_LOGE(tag, "recvfrom failed: errno %d", errno);
	            break;
	        }
	        // Data received
	        else {
	        	rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
	            ESP_LOGI(tag, "Received %d bytes from %s:", len, addr_str);
	            ESP_LOGI(tag, "%s", rx_buffer);
	        }

	        vTaskDelay(2000 / portTICK_PERIOD_MS);
		}
	    if (sock != -1) {
	    	ESP_LOGE(tag, "Shutting down socket and restarting...");
	    	shutdown(sock, 0);
	        close(sock);
	    }
	}
	vTaskDelete(NULL);
}
