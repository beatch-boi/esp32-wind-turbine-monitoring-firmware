#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config.h"
#include "wifi/wifi.h"
#include "http-server/http-server.h"
#include "filesystem/filesystem.h"
#include "blade-speed-sensor/blade-speed-sensor.h"
#include "motor-speed-sensor/motor-speed-sensor.h"
#include "adc/adc.h"

// TODO: Use queues to send data between tasks!

static double I1, I2, I3;
static double blade_rpm, motor_rpm;

static double blade_start_angle;
static double motor_start_angle;

#define TICKS_TO_MS portTICK_PERIOD_MS

static void ReadCurrentSensors(void*) {
  for (;;) {
    int voltage_values[3] = {0};
    int channels[] = {ACS712_1_ADC_CHANNEL, ACS712_2_ADC_CHANNEL, ACS712_3_ADC_CHANNEL};
    for (int i = 0; i < 3; ++i) {
      voltage_values[i] = ReadADC(channels[i]);
    }

    const double Reff_V = 1650.0; // Reference voltage in mV
    I1 = ((((double)(voltage_values[0]) - Reff_V) / 66.0) * 1000.0);
    I2 = ((((double)(voltage_values[1]) - Reff_V) / 66.0) * 1000.0);
    I3 = ((((double)(voltage_values[2]) - Reff_V) / 66.0) * 1000.0);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static double CorrectAngle(double a, double start_a, int ex_q, int n_turns) {
  a -= start_a;
  if (a < 0) a += 360;

  int q = (a / 90) + 1;
  if (q != ex_q) {
    n_turns += (q == 1 && ex_q == 4) ? 1: (q == 4 && ex_q == 1) ? -1 : 0;
    ex_q = q;
  }

  a += (n_turns * 360.0);

  return a;
}

static void CalculateBladeRPM(void*) {
  /* Static variables are automatically initialized to 0. */
  static int ex_q;
  static int n_turns;
  static double ex_a;
  static uint32_t ex_n_ticks;

  for (;;) {
    double a = BladeSpeedSensorReadAngleDeg();
    a = CorrectAngle(a, blade_start_angle, ex_q, n_turns);

    uint32_t n_ticks = xTaskGetTickCount();
    uint32_t delta_ms = (n_ticks - ex_n_ticks) * TICKS_TO_MS;

    /* 200 ms is an interval, measured empirically. */ 
    if (delta_ms >= 200) {
      blade_rpm = (60000.0 / delta_ms) * (a - ex_a) / 360.0;
      ex_a = a;
      ex_n_ticks = n_ticks;
    }
    vTaskDelay(1);
  }
}

static void CalculateMotorRPM(void*) {
  /* Static variables are automatically initialized to 0. */
  static int ex_q;
  static int n_turns;
  static double ex_a;
  static uint32_t ex_n_ticks;

  for (;;) {
    double a = MotorSpeedSensorReadAngleDeg();
    a = CorrectAngle(a, motor_start_angle, ex_q, n_turns);

    uint32_t n_ticks = xTaskGetTickCount();
    uint32_t delta_ms = (n_ticks - ex_n_ticks) * TICKS_TO_MS;

    /* 200 ms is an interval, measured empirically. */ 
    if (delta_ms >= 200) {
      motor_rpm = (60000.0 / delta_ms) * (a - ex_a) / 360.0;
      ex_a = a;
      ex_n_ticks = n_ticks;
    }
    vTaskDelay(1);
  }
}

static void UpdateWebPageValues(void*) {
  char message[256];
  size_t len = sizeof(message);
  for (;;) {
    snprintf(message, len, "RPM_0: %.2f, RPM_1: %.2f, I0: %.2f mA, I1: %.2f mA, I2: %.2f mA", blade_rpm, motor_rpm, I1, I2, I3);
    SendWsMessage(message);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void app_main(void) {
  InitWiFi();

  StartWiFiAP(WIFI_AP_SSID, WIFI_AP_PASSWD);

  StartHttpServer();

  MountFileSystem();

  InitADC();

  InitBladeSpeedSensor(&blade_start_angle);

  InitMotorSpeedSensor(&motor_start_angle);

  if (xTaskCreate(CalculateBladeRPM, "CalculateBladeRPM", 2 * 1024, NULL, 4, NULL) != pdPASS) {
    ESP_LOGE("app_main", "Failed to create task CalculateBladeRPM.");
    goto halt;
  }

  if (xTaskCreate(CalculateMotorRPM, "CalculateMotorRPM", 2 * 1024, NULL, 4, NULL) != pdPASS) {
    ESP_LOGE("app_main", "Failed to create task CalculateMotorRPM.");
    goto halt;
  }

  if (xTaskCreate(ReadCurrentSensors, "ReadCurrentSensors", 1024, NULL, 3, NULL) != pdPASS) {
    ESP_LOGE("app_main", "Failed to create  task ReadCurrentSensors.");
    goto halt;
  }

  if (xTaskCreate(UpdateWebPageValues, "UpdateWebPageValues", 3 * 1024, NULL, 3, NULL) != pdPASS) {
    ESP_LOGE("app_main", "Failed to create task UpdateWebPageValues.");
  }

halt:
  for (;;) {
    vTaskDelay(portMAX_DELAY);
  }
}
