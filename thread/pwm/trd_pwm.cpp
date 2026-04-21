/**
 * @file trd_pwm.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "trd_pwm.h"
#include "top_fsi6.hpp"
#include "zephyr/kernel.h"

using namespace thread;

static void Task(void* p1, void* p2, void* p3)
{
    topic::fsi6::Message msg{
        1100,
        1100,
        1100,
        1100,
        0,
        0,
        0,
    };
    auto& ins = *static_cast<Thread<MotorPwm, pwm::N>*>(p1);    // p1 就是 thread_ 自身
    
    for (;;)
    {
        zbus_chan_read(&remote_fsi6_chan, &msg, K_NO_WAIT);

        ins[PWM0].SetPulse(msg.pitch);
        ins[PWM1].SetPulse(msg.pitch);

        k_msleep(1);
    }
}

void pwm::thread_start(uint8_t prio, void* p2, void* p3)
{
    pwm::thread_.Start(prio, Task, p2, p3);
}





