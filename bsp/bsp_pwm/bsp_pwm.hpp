/**
 * @file bsp_pwm.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <zephyr/drivers/pwm.h>

struct BspPwmObj
{
    pwm_dt_spec spec;
    uint32_t max_value;
    bool ready = false;
};

BspPwmObj bsp_pwm_init(const pwm_dt_spec* dt_spec, uint32_t max_value);
int bsp_pwm_set(const BspPwmObj& obj, uint32_t value);


