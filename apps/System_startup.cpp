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
#include "Servo.hpp"

void System_Bsp_Init()
{
    
}

void System_Modules_Init()
{
    const pwm_dt_spec spec = PWM_DT_SPEC_GET(DT_NODELABEL(servo0));
    Servo ins{};
    ins.Init(&spec, 20000);
    thread::servo::thread_.Join(ins);
}

void System_Thread_Start()
{
    thread::servo::thread_start(5);
}