/**
 * @file trd_servo.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "thread.hpp"
#include "Servo.hpp"

enum ServoIndex
{
    SERVO0 = 0,
    SERVO1,
    SERVO2,
    SERVO3,
    SERVO4,
    SERVO5,
    SERVO6,
    SERVO7,
    SERVO8,
    SERVO9,
};

namespace thread::servo
{
    constexpr uint8_t N = 3;
    inline Thread<Servo, N, 1024> thread_{};
    void thread_start(uint8_t prio, void* p2 = nullptr, void* p3 = nullptr);
}