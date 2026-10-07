#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H
#include "esp_adc/adc_oneshot.h"

typedef struct {
    adc_unit_t Unit;
    adc_channel_t Channel;
    adc_atten_t Attenuation;
} ADC_InitTypeDef;

esp_err_t ADC_Init(const ADC_InitTypeDef *config, adc_oneshot_unit_handle_t *handle);
esp_err_t ADC_Read(adc_oneshot_unit_handle_t handle, adc_channel_t channel, int *raw);

#endif
