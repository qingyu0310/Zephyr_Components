/**
 * @file key.hpp
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

class Key
{
public:
    Key() = default;
    ~Key() = default;

    void Init(const struct gpio_dt_spec* dt_spec) {
        obj_ = bsp_gpio_init(dt_spec, BSP_GPIO_MODE_INPUT);
    }

    bool IsPressed() {
        bool pressed = (bsp_gpio_read(obj_) == 1);
        press_bit_ = (press_bit_ << 1) | pressed;

        return (press_bit_ & 0xFFFF) == 0xFFFF;
    }

private:
    BspGpioObj obj_{};
    
    uint16_t press_bit_ = 0x0000;
};