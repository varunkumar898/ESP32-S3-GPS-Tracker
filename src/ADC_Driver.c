#include "ADC_Driver.h"

esp_err_t ADC_Init(const ADC_InitTypeDef *c, adc_oneshot_unit_handle_t *handle)
{
    if (!c || !handle) return ESP_ERR_INVALID_ARG;
    adc_oneshot_unit_init_cfg_t unit = {
        .unit_id = c->Unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t err = adc_oneshot_new_unit(&unit, handle);
    if (err != ESP_OK) return err;
    adc_oneshot_chan_cfg_t ch = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = c->Attenuation,
    };
    return adc_oneshot_config_channel(*handle, c->Channel, &ch);
}

esp_err_t ADC_Read(adc_oneshot_unit_handle_t handle, adc_channel_t channel, int *raw)
{
    if (!handle || !raw) return ESP_ERR_INVALID_ARG;
    return adc_oneshot_read(handle, channel, raw);
}
