/**
 * @file bsp_uart.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "bsp_uart.hpp"

/* --------------------------------- private -------------------------------- */

static void rx_store(BspUartObj& obj, const uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (obj.head + 1) % (BSP_UART_RX_BUF_SIZE * 2);
        if (next != obj.tail) {
            obj.rx_buf[obj.head] = data[i];
            obj.head = next;
        }
    }
}

static void uart_async_cb(const device* dev, uart_event* evt, void* user_data)
{
    auto& obj = *static_cast<BspUartObj*>(user_data);

    switch (evt->type) 
    {
        case UART_RX_RDY:
        {
            const uint8_t* data = evt->data.rx.buf + evt->data.rx.offset;
            uint16_t len = evt->data.rx.len;
            rx_store(obj, data, len);
            if (obj.rx_cb) {
                obj.rx_cb(data, len, obj.rx_cb_arg);
            }
            break;
        }
        case UART_RX_BUF_REQUEST:
        {
            obj.cur_buf = 1 - obj.cur_buf;
            uart_rx_buf_rsp(dev, obj.dma_buf[obj.cur_buf], BSP_UART_RX_BUF_SIZE);
            break;
        }
        case UART_RX_DISABLED:
        {
            obj.cur_buf = 1 - obj.cur_buf;
            uart_rx_enable(dev, obj.dma_buf[obj.cur_buf], BSP_UART_RX_BUF_SIZE, obj.rx_timeout);
            break;
        }
        case UART_TX_DONE:
        {
            k_sem_give(&obj.tx_sem);
            break;
        }
        default:
        {
            break;
        }
    }
}

/* --------------------------------- public --------------------------------- */

int bsp_uart_init(BspUartObj& obj, const device* dev, uint32_t rx_timeout)
{
    obj.dev        = dev;
    obj.head       = 0;
    obj.tail       = 0;
    obj.cur_buf    = 0;
    obj.rx_timeout = rx_timeout;
    k_sem_init(&obj.tx_sem, 1, 1);

    if (!device_is_ready(dev)) {
        return -ENODEV;
    }

    int ret = uart_callback_set(dev, uart_async_cb, &obj);
    if (ret < 0) {
        return ret;
    }

    ret = uart_rx_enable(dev, obj.dma_buf[0], BSP_UART_RX_BUF_SIZE, obj.rx_timeout);
    if (ret < 0) {
        return ret;
    }

    obj.ready = true;
    return 0;
}

void bsp_uart_set_rx_callback(BspUartObj& obj, BspUartRxCallback cb, void* arg)
{
    obj.rx_cb     = cb;
    obj.rx_cb_arg = arg;
}

int bsp_uart_send(const BspUartObj& obj, const uint8_t* data, uint16_t len)
{
    if (!obj.ready) {
        return -ENODEV;
    }
    if (k_sem_take(const_cast<k_sem*>(&obj.tx_sem), K_FOREVER) != 0) {
        return -EBUSY;
    }
    return uart_tx(obj.dev, data, len, SYS_FOREVER_US);
}

int bsp_uart_send_poll(const BspUartObj& obj, const uint8_t* data, uint16_t len)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    for (uint16_t i = 0; i < len; i++) {
        uart_poll_out(obj.dev, data[i]);
    }

    return len;
}

int bsp_uart_read(BspUartObj& obj, uint8_t* data, uint16_t len)
{
    if (!obj.ready) {
        return -ENODEV;
    }

    uint16_t count    = 0;
    uint16_t buf_size = BSP_UART_RX_BUF_SIZE * 2;

    while (count < len && obj.tail != obj.head) {
        data[count++] = obj.rx_buf[obj.tail];
        obj.tail = (obj.tail + 1) % buf_size;
    }
    return count;
}

uint16_t bsp_uart_available(const BspUartObj& obj)
{
    uint16_t buf_size = BSP_UART_RX_BUF_SIZE * 2;
    return (obj.head - obj.tail + buf_size) % buf_size;
}
