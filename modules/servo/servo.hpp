/**
 * @file servo.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-03-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "bsp_pwm.hpp"

/**
 * @brief 
 * 
 */
class Servo
{
public:
    Servo() = default;
    ~Servo() = default;

    void Init(const struct pwm_dt_spec* dt_spec, uint32_t max_value, uint32_t max_angle = 180) {
        max_angle_ = max_angle;
        obj_ = bsp_pwm_init(dt_spec, max_value);
    }

    inline void SetPulse(uint32_t ccrx) {
        bsp_pwm_set(obj_, ccrx);
    }

    inline void SetAngle(float angle) {
        if (angle < 0) angle = 0;
        if (angle > max_angle_) angle = max_angle_;
        uint32_t ccrx = 500 + static_cast<uint32_t>(angle / max_angle_ * 2000);
        bsp_pwm_set(obj_, ccrx);
    }

private:
    BspPwmObj obj_{};

    uint16_t max_angle_ = 180;
};