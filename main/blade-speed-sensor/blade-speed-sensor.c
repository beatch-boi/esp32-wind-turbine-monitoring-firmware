#include "blade-speed-sensor.h"

#include <driver/i2c_master.h>

#include "config.h"

static i2c_master_dev_handle_t I2C_handle;

void InitBladeSpeedSensor(double *start_angle) {
  i2c_master_bus_config_t i2c_bus_cfg = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = AS5600_1_PORT,
    .scl_io_num = AS5600_1_SCL,
    .sda_io_num = AS5600_1_SDA,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true
  };

  i2c_device_config_t as5560_cfg = {
    .dev_addr_length = AS5600_ADDR_LEN,
    .device_address = AS5600_ADDR,
    .scl_speed_hz = AS5600_I2C_SPEED,
  };

  i2c_master_bus_handle_t i2c_bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handle));

  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &as5560_cfg, &I2C_handle));
}

double BladeSpeedSensorReadAngleDeg(void) {
  uint8_t buff[2] = {0};
  uint8_t cmd[] = {0x0E};
  ESP_ERROR_CHECK(i2c_master_transmit(I2C_handle, cmd, 1, 10));
  ESP_ERROR_CHECK(i2c_master_receive(I2C_handle, buff, 2, 10));
  return (buff[0] << 8 | buff[1]) * 360.0 / 4096.0;
}