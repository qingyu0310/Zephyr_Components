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

int  bsp_uart_init(BspUartObj& obj, const device* dev, uint32_t rx_timeout = 10000);
void bsp_uart_set_rx_callback(BspUartObj& obj, BspUartRxCallback cb, void* arg = nullptr);
int  bsp_uart_send(const BspUartObj& obj, const uint8_t* data, uint16_t len);
int  bsp_uart_send_poll(const BspUartObj& obj, const uint8_t* data, uint16_t len);
int  bsp_uart_read(BspUartObj& obj, uint8_t* data, uint16_t len);
uint16_t bsp_uart_available(const BspUartObj& obj);















