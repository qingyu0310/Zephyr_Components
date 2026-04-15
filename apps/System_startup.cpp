/**
 * @file System_startup.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "System_startup.h"
#include "trd_led.h"
#include <zephyr/drivers/gpio.h>

void System_Bsp_Init()
{
    
}

void System_Modules_Init()
{
    {
        static const struct gpio_dt_spec spec[] {
            GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios),
            GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios),
        };

        for (uint8_t i = 0; i < thread::led::N; i++) {
            Led ins{};
            ins.Init(&spec[i]);
            thread::led::thread_.Join(ins);
        }
    }
}

void System_Thread_Start()
{
    thread::led::thread_start(5);
}