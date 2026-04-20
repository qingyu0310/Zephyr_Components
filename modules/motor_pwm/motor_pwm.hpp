/**
 * @file motor_pwm.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "bsp_pwm.hpp"

class MotorPwm
{
public:
    MotorPwm() = default;
    ~MotorPwm() = default;
    
    void Init(const pwm_dt_spec *dt_spec, uint32_t max_value) {
        obj_ = bsp_pwm_init(dt_spec, max_value);
    };

    void SetPulse(uint32_t value) {
        bsp_pwm_set(obj_, value);
    }

private:
    BspPwmObj obj_{};
};





























