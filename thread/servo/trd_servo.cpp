/**
 * @file trd_servo.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "trd_servo.h"
#include "Servo.hpp"

using namespace thread;

static void Task(void* p1, void* p2, void* p3)
{
    auto& ins = *static_cast<Thread<Servo, servo::N>*>(p1);  // p1 就是 thread_ 自身

    for (;;)
    {
        ins[SERVO0].SetAngle(0);
        k_msleep(1000);
        ins[SERVO0].SetAngle(180);
        k_msleep(1000);
    }
}

void servo::thread_start(uint8_t prio, void* p2, void* p3)
{
    servo::thread_.Start(prio, Task, p2, p3);
}