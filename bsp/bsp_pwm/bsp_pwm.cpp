/**
 * @file bsp_pwm.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "bsp_pwm.hpp"

BspPwmObj bsp_pwm_init(const pwm_dt_spec* dt_spec, uint32_t max_value)
{
    BspPwmObj obj{};
    obj.spec = *dt_spec;
    obj.max_value = max_value;

    if (!pwm_is_ready_dt(dt_spec)) {
        return obj;
    }

    obj.ready = true;
    return obj;
}

int bsp_pwm_set(const BspPwmObj& obj, uint32_t value)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    if (value > obj.max_value) {
        value = obj.max_value;
    }

    // value 按 max_value 线性映射到一个 PWM 周期内的脉宽(ns)。
    uint32_t pulse_ns = (uint64_t)obj.spec.period * value / obj.max_value;

    return pwm_set_dt(&obj.spec, obj.spec.period, pulse_ns);
}












