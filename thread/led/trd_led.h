/**
 * @file trd_led.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-04-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "thread.hpp"
#include "led.hpp"

enum LedIndex
{
    LED0 = 0,
    LED1,
    LED2,
    LED3,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED9,
};

namespace thread::led
{
    constexpr uint8_t N = 3;
    inline Thread<Led, N> thread_{};
    void thread_start(uint8_t prio, void* p2 = nullptr, void* p3 = nullptr);
}





