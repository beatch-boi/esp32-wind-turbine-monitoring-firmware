#ifndef WIND_TURBINE_I2C_H_
#define WIND_TURBINE_I2C_H_

#include <driver/i2c_master.h>

void InitI2C(i2c_master_dev_handle_t *AS5600_1_handle, i2c_master_dev_handle_t *AS5600_2_handle);

#endif  // WIND_TURBINE_I2C_H_