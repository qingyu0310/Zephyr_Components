/**
 * @file led.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "bsp_gpio.hpp"

/**
 * @brief 
 * 
 */
class Led final
{
public:
    Led() = default;
    ~Led() = default;

    void Init(const struct gpio_dt_spec* dt_spec) {
        obj_ = bsp_gpio_init(dt_spec, BSP_GPIO_MODE_OUTPUT);
    }

    void On() {
        bsp_gpio_write(obj_, 1);
    }

    void Off() {
        bsp_gpio_write(obj_, 0);
    }

    void Toggle() {
        bsp_gpio_toggle(obj_);
    }

private:
    BspGpioObj obj_{};
};
















