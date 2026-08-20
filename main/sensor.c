#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include "DHT22.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi.h"

char ip[] = "0.0.0.0";
int port = 9999;

#define THIS_ROOM 1

struct message {
  float humidity;
  float temperature;
  uint8_t room;
};

void app_main(void) {
  char *ourTaskName = pcTaskGetName(NULL);
  ESP_LOGI(ourTaskName, "Hello, starting up!\n");

  // connect to the wifi to use the posix api
  wifi_connect();

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    printf("socket failed\n");
  }
  struct sockaddr_in addrDest;
  addrDest.sin_family = AF_INET;
  addrDest.sin_port = htons(port);
  addrDest.sin_addr.s_addr = inet_addr(ip);
  
  vTaskDelay(3000 / portTICK_PERIOD_MS);

  setDHTgpio(4);
  printf("Starting...\n");

  while (1) {
    int ret = readDHT();
    errorHandler(ret);

    printf("%.1f, %.1f\n", getHumidity(), getTemperature());
    
    if (ret >= 0) {
      struct message msg;
      msg.room = THIS_ROOM;
      msg.humidity = getHumidity();
      msg.temperature = getTemperature();

      sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&addrDest,
             sizeof(addrDest));
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}
