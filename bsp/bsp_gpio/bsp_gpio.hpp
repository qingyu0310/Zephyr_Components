/**
 * @file bsp_gpio.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <zephyr/drivers/gpio.h>

enum BspGpioMode
{
    BSP_GPIO_MODE_INPUT = 0,
    BSP_GPIO_MODE_OUTPUT,
};

struct BspGpioObj
{
    gpio_dt_spec spec;
    bool ready = false;
};

BspGpioObj bsp_gpio_init(const gpio_dt_spec* dt_spec, BspGpioMode mode);
void bsp_gpio_write(const BspGpioObj& obj, int value);
void bsp_gpio_toggle(const BspGpioObj& obj);
int bsp_gpio_read(const BspGpioObj& obj);


