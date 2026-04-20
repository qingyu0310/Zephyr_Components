/**
 * @file bsp_uart.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

constexpr uint16_t BSP_UART_RX_BUF_SIZE = 128;

using BspUartRxCallback = void (*)(const uint8_t* data, uint16_t len, void* arg);

/**
 * UART 用法说明：
 * 1. 先调用 bsp_uart_init() 完成设备绑定、驱动回调注册和 RX 使能。
 * 2. 如需在收到新数据时得到通知，可调用 bsp_uart_set_rx_callback() 注册回调。
 *    回调中的 data 和 len 表示本次刚收到的那一段数据。
 * 3. 无论是否注册回调，接收到的数据都会先写入 obj.rx_buf 环形缓冲区。
 * 4. 上层可通过 bsp_uart_available() 查询当前可读字节数，再通过
 *    bsp_uart_read() 从环形缓冲区取走数据。
 * 5. 如果回调使用类成员函数，请使用 static 包装函数，并通过 arg 传入 this。
 * 6. 不要对同一批数据重复处理：要么直接使用回调参数 data/len，要么统一从
 *    obj.rx_buf 中读取。
 */

struct BspUartObj
{
    const device* dev = nullptr;
    uint8_t dma_buf[2][BSP_UART_RX_BUF_SIZE]{};
    uint8_t rx_buf[BSP_UART_RX_BUF_SIZE * 2];
    uint16_t head = 0;
    uint16_t tail = 0;
    uint8_t cur_buf = 0;
    BspUartRxCallback rx_cb = nullptr;
    void* rx_cb_arg = nullptr;
    uint32_t rx_timeout = 10000;
    k_sem tx_sem;
    bool ready = false;
};

int32_t  bsp_uart_init(BspUartObj& obj, const device* dev, uint32_t rx_timeout = 10000);    // 10ms
void bsp_uart_set_rx_callback(BspUartObj& obj, BspUartRxCallback cb, void* arg = nullptr);
int32_t  bsp_uart_send(const BspUartObj& obj, const uint8_t* data, uint16_t len);
int32_t  bsp_uart_read(BspUartObj& obj, uint8_t* data, uint16_t len);
uint16_t bsp_uart_available(const BspUartObj& obj);















