#include "i2c.h"

#include <driver/i2c_master.h>

#include "config.h"

// i2c_master_dev_handle_t AS5600_1_handle;
// i2c_master_dev_handle_t AS5600_2_handle;

void InitI2C(i2c_master_dev_handle_t *AS5600_1_handle, i2c_master_dev_handle_t *AS5600_2_handle) {
  i2c_master_bus_config_t i2c_bus_0_cfg = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = AS5600_1_PORT,
    .scl_io_num = AS5600_1_SCL,
    .sda_io_num = AS5600_1_SDA,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true
  };

  i2c_master_bus_config_t i2c_bus_1_cfg = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = AS5600_2_PORT,
    .scl_io_num = AS5600_2_SCL,
    .sda_io_num = AS5600_2_SDA,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true
    };

  i2c_device_config_t as5560_cfg = {
    .dev_addr_length = AS5600_ADDR_LEN,
    .device_address = AS5600_ADDR,
    .scl_speed_hz = AS5600_I2C_SPEED,
  };

  i2c_master_bus_handle_t i2c_bus_0_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_0_cfg, &i2c_bus_0_handle));

  i2c_master_bus_handle_t i2c_bus_1_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_1_cfg, &i2c_bus_1_handle));

  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_0_handle, &as5560_cfg, AS5600_1_handle));

  ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_1_handle, &as5560_cfg, AS5600_2_handle));
}
