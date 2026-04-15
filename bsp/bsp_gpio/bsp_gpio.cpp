/**
 * @file bsp_gpio.c
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "bsp_gpio.hpp"

BspGpioObj bsp_gpio_init(const struct gpio_dt_spec* dt_spec, BspGpioMode mode)
{
    BspGpioObj obj{};
    obj.spec = *dt_spec;

    if (!gpio_is_ready_dt(dt_spec)) {
        return obj;
    }

    gpio_flags_t flags = (mode == BSP_GPIO_MODE_OUTPUT) ? GPIO_OUTPUT_INACTIVE : GPIO_INPUT;
    int ret = gpio_pin_configure_dt(dt_spec, flags);
    if (ret < 0) {
        return obj;
    }

    obj.ready = true;
    return obj;
}

void bsp_gpio_write(const BspGpioObj& obj, int value)
{
    if (!obj.ready) {
        return;
    }
    (void)gpio_pin_set_dt(&obj.spec, value);
}

void bsp_gpio_toggle(const BspGpioObj& obj)
{
    if (!obj.ready) {
        return;
    }
    (void)gpio_pin_toggle_dt(&obj.spec);
}

int bsp_gpio_read(const BspGpioObj& obj)
{
    if (!obj.ready) {
        return -ENODEV;
    }
    return gpio_pin_get_dt(&obj.spec);
}




