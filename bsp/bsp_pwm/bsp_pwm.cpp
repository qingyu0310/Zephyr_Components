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

constexpr uint32_t BSP_PWM_NS_PER_US = 1000;

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

int32_t bsp_pwm_set(const BspPwmObj& obj, uint32_t value)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    if (value > obj.max_value) {
        value = obj.max_value;
    }

    // 线性映射公式：
    // pulse_ns / period_ns = value / max_value
    // 推导得：pulse_ns = period_ns * value / max_value
    // 其中 obj.spec.period 的单位是 ns。
    uint32_t pulse_ns = (uint64_t)obj.spec.period / obj.max_value * value;

    return pwm_set_dt(&obj.spec, obj.spec.period, pulse_ns);
}

int32_t bsp_pwm_set_us(const BspPwmObj& obj, uint32_t pulse_us)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    // 单位换算公式：pulse_ns = pulse_us * 1000。
    uint32_t period_us = obj.spec.period / BSP_PWM_NS_PER_US;
    if (pulse_us > period_us) {
        pulse_us = period_us;
    }

    return pwm_set_dt(&obj.spec, obj.spec.period, pulse_us * BSP_PWM_NS_PER_US);
}

int32_t bsp_pwm_set_ns(const BspPwmObj& obj, uint32_t pulse_ns)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    if (pulse_ns > obj.spec.period) {
        pulse_ns = obj.spec.period;
    }

    return pwm_set_dt(&obj.spec, obj.spec.period, pulse_ns);
}










