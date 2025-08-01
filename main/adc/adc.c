#include "adc.h"

#include <esp_adc/adc_oneshot.h>

#include "config.h"

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_handle;

void InitADC() {
  adc_oneshot_unit_init_cfg_t adc1_init_config = {
    .unit_id = ACS712_ADC_UNIT,
    .ulp_mode = ACS712_ADC_ULP_MODE,
  };

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc1_init_config, &adc_handle));

  adc_oneshot_chan_cfg_t adc_config = {
    .bitwidth = ACS712_ADC_BITWIDTH,
    .atten = ACS712_ADC_ATTEN
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ACS712_1_ADC_CHANNEL, &adc_config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ACS712_2_ADC_CHANNEL, &adc_config));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ACS712_3_ADC_CHANNEL, &adc_config));

  // ADC calibration scheme
  adc_cali_line_fitting_config_t adc_cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc_cali_config, &adc_cali_handle));
}

int ReadADC(int adc_channel) {
  int raw, voltage;
  ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, &raw));
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage));
  return voltage;
}