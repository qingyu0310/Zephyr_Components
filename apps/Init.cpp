/**
 * @file Init.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "Init.h"
#include "System_startup.h"

void uart1_callback_func(const uint8_t* data, uint16_t len, void* arg)
{
   auto& obj = *static_cast<BspUartObj*>(arg);
   bsp_uart_send(obj, data, len);
}

void Init()
{
   System_Bsp_Init();
   System_Modules_Init();
   System_Thread_Start();
}














