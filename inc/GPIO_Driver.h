#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H
#include "driver/gpio.h"

typedef struct {
    gpio_num_t Pin;
    gpio_mode_t Mode;
    gpio_pullup_t PullUp;
    gpio_pulldown_t PullDown;
    gpio_int_type_t InterruptType;
} GPIO_InitTypeDef;

esp_err_t GPIO_Init(const GPIO_InitTypeDef *config);
esp_err_t GPIO_Write(gpio_num_t pin, uint32_t level);
int GPIO_Read(gpio_num_t pin);
esp_err_t GPIO_Toggle(gpio_num_t pin);

#endif
