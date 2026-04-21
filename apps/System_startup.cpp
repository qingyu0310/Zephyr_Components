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
#include "led.hpp"

void System_Bsp_Init()
{
    
}

void System_Modules_Init()
{
    {
        static const gpio_dt_spec spec[] {
            GPIO_DT_SPEC_GET(DT_NODELABEL(key0), gpios),
        };

        for (uint8_t i = 0; i < sizeof(spec) / sizeof(spec[0]); i++) {
            Key ins{};
            ins.Init(&spec[i]);
            thread::key::thread_.Join(ins);
        }
    }
}

void System_Thread_Start()
{
    {
        static Led ins{};
        gpio_dt_spec spec = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
        ins.Init(&spec);
        thread::key::thread_start(5, &ins);
    }
}