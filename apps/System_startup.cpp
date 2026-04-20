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
#include "fs_i6.hpp"
#include "zephyr/device.h"
#include "zephyr/drivers/pwm.h"

void System_Bsp_Init()
{
    
}

void System_Modules_Init()
{
    {
        const device* spec = DEVICE_DT_GET(DT_NODELABEL(usart1));
        thread::fsi6::fsi6_.Init(spec);
    }
    
    {
        const pwm_dt_spec spec[] {
            PWM_DT_SPEC_GET(DT_NODELABEL(motor)),
        };

        for (uint8_t i = 0; i < sizeof(spec) / sizeof(spec[0]); i++)
        {
            MotorPwm ins{};
            ins.Init(&spec[i], 2000);
            thread::pwm::thread_.Join(ins);
        } 
    }
}

void System_Thread_Start()
{
    thread::fsi6::fsi6_.Start();
    thread::pwm::thread_start();
}


