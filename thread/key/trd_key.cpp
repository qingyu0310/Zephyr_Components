/**
 * @file trd_key.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "trd_key.h"
#include "led.hpp"

using namespace thread;

static void Task(void* p1, void* p2, void* p3)
{
    auto& ins = *static_cast<Thread<Key, key::N>*>(p1);  // p1 就是 thread_ 自身
    auto& led = *static_cast<Led*>(p2);

    for (;;)
    {
        if (ins[KEY0].IsPressed()) {
            led.On();
        } else {
            led.Off();
        }

        k_msleep(1);
    }
}

void key::thread_start(uint8_t prio, void* p2, void* p3)
{
    key::thread_.Start(prio, Task, p2, p3);
}










