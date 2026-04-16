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

static BspUartObj uart_obj_{};

void System_Bsp_Init()
{
    bsp_uart_init(uart_obj_, DEVICE_DT_GET(DT_NODELABEL(usart1)));
    bsp_uart_set_rx_callback(uart_obj_, uart1_callback_func, &uart_obj_);
}

void System_Modules_Init()
{
    
}

void System_Thread_Start()
{
    
}