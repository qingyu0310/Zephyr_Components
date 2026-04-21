/**
 * @file trd_led.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "trd_led.h"
#include "zephyr/kernel.h"
#include "timer.hpp"

using namespace thread;

static void Task(void* p1, void* p2, void* p3)
{
    auto& ins = *static_cast<Thread<Led, led::N>*>(p1);  // p1 就是 thread_ 自身
    static Timer timer(1000);

    for (;;)
    {
        timer.Update();

        timer.Clock([&](){
            ins[LED0].Toggle();
        });

        uint16_t step = timer.GetCounter();
        if (step == timer.GetPeriod() / 2) {
            ins[LED1].Toggle();
        }

        k_msleep(1);
    }
}

void led::thread_start(uint8_t prio, void* p2, void* p3)
{
    led::thread_.Start(prio, Task, p2, p3);
}





