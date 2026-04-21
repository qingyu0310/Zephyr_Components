/**
 * @file System_startup.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "bsp_uart.hpp"

void System_Bsp_Init();
void System_Modules_Init();
void System_Thread_Start();

void uart1_callback_func(const uint8_t* data, uint16_t len, void* arg);