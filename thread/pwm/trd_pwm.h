/**
 * @file trd_pwm.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "thread.hpp"
#include "motor_pwm.hpp"

enum PwmIndex
{
    PWM0 = 0,
    PWM1,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM6,
    PWM7,
    PWM8,
    PWM9,
};

namespace thread::pwm
{
    constexpr uint8_t N = 6;
    inline Thread<MotorPwm, N> thread_{};
    void thread_start(uint8_t prio = 5, void* p2 = nullptr, void* p3 = nullptr);
}




