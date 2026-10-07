#include "GPIO_Driver.h"

esp_err_t GPIO_Init(const GPIO_InitTypeDef *c)
{
    if (!c) return ESP_ERR_INVALID_ARG;
    esp_err_t err = gpio_reset_pin(c->Pin);
    if (err != ESP_OK) return err;
    err = gpio_set_direction(c->Pin, c->Mode);
    if (err != ESP_OK) return err;

    gpio_pull_mode_t pull = GPIO_FLOATING;
    if (c->PullUp == GPIO_PULLUP_ENABLE && c->PullDown == GPIO_PULLDOWN_ENABLE)
        pull = GPIO_PULLUP_PULLDOWN;
    else if (c->PullUp == GPIO_PULLUP_ENABLE)
        pull = GPIO_PULLUP_ONLY;
    else if (c->PullDown == GPIO_PULLDOWN_ENABLE)
        pull = GPIO_PULLDOWN_ONLY;

    err = gpio_set_pull_mode(c->Pin, pull);
    if (err != ESP_OK) return err;
    return gpio_set_intr_type(c->Pin, c->InterruptType);
}

esp_err_t GPIO_Write(gpio_num_t pin, uint32_t level)
{
    return gpio_set_level(pin, level ? 1 : 0);
}

int GPIO_Read(gpio_num_t pin)
{
    return gpio_get_level(pin);
}

esp_err_t GPIO_Toggle(gpio_num_t pin)
{
    return gpio_set_level(pin, !gpio_get_level(pin));
}
