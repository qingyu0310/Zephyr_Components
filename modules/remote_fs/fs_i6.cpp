/**
 * @file fs_i6.cpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "fs_i6.hpp"
#include "top_fsi6.hpp"

ZBUS_CHAN_DEFINE(remote_fsi6_chan, topic::fsi6::Message,
                 nullptr, nullptr,
                 ZBUS_OBSERVERS_EMPTY,
                 ZBUS_MSG_INIT(0));

void FsI6::Init(const device *dev, BspUartRxCallback cb)
{
    bsp_uart_init(obj_, dev);

    if (cb != nullptr) {
        bsp_uart_set_rx_callback(obj_, cb, this);
    } else {
        bsp_uart_set_rx_callback(obj_, FsI6::UartRxCpltCallback, this);
    }
}

void FsI6::Task()
{
    for (;;)
    {
        k_msleep(1);
    }
}

void FsI6::UartRxCpltCallback(const uint8_t* data, uint16_t len, void* arg)
{
    FsI6* self = static_cast<FsI6*>(arg);

    if (self == nullptr) {
        return;
    }

    self->DataProcess(data, len);
}

void FsI6::DataProcess(const uint8_t* data, uint16_t len)
{
    constexpr float K = 0.002f, C = -3.0f;

    if (len < 18) {
        return;
    }

    flag_ += 1; // 滑动窗口+1

    raw_.ch1 = ((data[3] << 8) | data[2]) & 0x07FF;
    raw_.ch2 = ((data[5] << 8) | data[4]) & 0x07FF;
    raw_.ch3 = ((data[7] << 8) | data[6]) & 0x07FF;
    raw_.ch4 = ((data[9] << 8) | data[8]) & 0x07FF;

    raw_.swa = ((data[11] << 8) | data[10]) & 0x07FF;
    raw_.vra = ((data[15] << 8) | data[14]) & 0x07FF;
    raw_.swc = ((data[17] << 8) | data[16]) & 0x07FF;

    output_.x = K * raw_.ch2 + C;
    output_.y = K * raw_.ch4 + C;
    output_.r = K * raw_.ch1 + C;
    output_.pitch = K * raw_.ch3 + C;

    output_.swa = raw_.swa;
    output_.vra = raw_.vra;
    output_.swc = raw_.swc;

    zbus_chan_pub(&remote_fsi6_chan, &output_, K_NO_WAIT);
}



















