#include "wifi.h"

#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

void InitWiFi(void) {
  // NVS flash needs to be initialized befor WiFi. It is used to store WiFi credentials.
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void StartWiFiAP(const char *SSID, const char *PASSWD) {
  esp_netif_create_default_wifi_ap();
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  wifi_config_t config = {
    .ap = {
        .ssid_len = strlen(SSID),
        .channel = 1,
        .authmode = WIFI_AUTH_WPA2_PSK,
        .max_connection = 4,
        .beacon_interval = 100
    }
  };
  memcpy(config.ap.ssid, SSID, strlen(SSID));
  memcpy(config.ap.password, PASSWD, strlen(PASSWD));
  // If the password is not defined, turn off the security features.
  if (strlen(PASSWD) == 0) {
    config.ap.authmode = WIFI_AUTH_OPEN;
  }
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI("InitWiFiAP", "Wi-Fi AP started, SSID: %s", config.ap.ssid);
}