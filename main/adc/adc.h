#ifndef WIND_TURBINE_ADC_H_
#define WIND_TURBINE_ADC_H_

#include <esp_adc/adc_oneshot.h>

void InitADC();

int ReadADC(int adc_channel);

#endif  // WIND_TURBINE_ADC_H_