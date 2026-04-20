/**
 * @file fs_i6.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "bsp_uart.hpp"
#include <cstdint>

class FsI6
{
public:
    FsI6() = default;
    ~FsI6() = default;

    struct OutputData
    {
        float x, y, r;
        float pitch;
        uint16_t swa, swc, vra;
    };

    void Init(const device *dev, BspUartRxCallback cb = nullptr);

    void Start(int prio = 5, void* p2 = nullptr, void* p3 = nullptr)
    {
        k_thread_create(&thread_, stack_, K_THREAD_STACK_SIZEOF(stack_),
                        TaskEntry, this, p2, p3, prio, 0, K_NO_WAIT);
    }

    static void UartRxCpltCallback(const uint8_t* data, uint16_t len, void* arg);

private:

    BspUartObj obj_{};

    uint32_t flag_ = 0;
    uint32_t pre_flag_ = 0;

    struct 
    {
        uint16_t ch1, ch2, ch3, ch4;
        uint16_t swa, swc, vra;
    } raw_{};

    OutputData output_{};

    void DataProcess(const uint8_t* data, uint16_t len);

    k_thread thread_{};
    K_KERNEL_STACK_MEMBER(stack_, 1024);

    void Task();

    static void TaskEntry(void *p1, void *p2, void *p3)
    {
        FsI6 *self = static_cast<FsI6*>(p1);
        self->Task();
    }
};

namespace thread::fsi6 {
    inline FsI6 fsi6_{};
}
















































