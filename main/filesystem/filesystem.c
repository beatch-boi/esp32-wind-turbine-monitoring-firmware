#include "filesystem.h"

#include <esp_spiffs.h>

void MountFileSystem(void) {
  esp_vfs_spiffs_conf_t spiffs_config = {
    .base_path = "/fs",
    .partition_label = NULL,
    .max_files = 10,
    .format_if_mount_failed = true
  };
  ESP_ERROR_CHECK(esp_vfs_spiffs_register(&spiffs_config));
}