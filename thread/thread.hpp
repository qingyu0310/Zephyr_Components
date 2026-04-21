/**
 * @file thread.hpp
 * @author qingyu
 * @brief Zephyr 线程模板，封装线程创建、对象池、启动等样板代码
 * @version 0.1
 * @date 2026-04-12
 * 
 * @copyright Copyright (c) 2026
 * 
 * @example
 * 
 *   // ========== 定义 ==========
 *   static ZThread<Led, 5, 1024> leds;
 * 
 *   static void task(void *p1, void *p2, void *p3) {
 *       auto& self = *static_cast<ZThread<Led, 5, 1024>*>(p1);
 *       for (;;) {
 *           self[0].Toggle();
 *           k_msleep(500);
 *       }
 *   }
 * 
 *   // ========== 外部调用 ==========
 *   leds.Join(led0);
 *   leds.Join(led1);
 *   leds.Start(5, task);
 * 
 */
#pragma once

#include <zephyr/kernel.h>

template<typename T, uint8_t N, size_t StackSize = 256>
class Thread
{
public:
    bool Join(const T& item)
    {
        if (used_ >= N) return false;
        items_[used_++] = item;
        return true;
    }

    void Start(int prio, k_thread_entry_t entry, void* p2, void* p3)
    {
        k_thread_create(&thread_, stack_, K_THREAD_STACK_SIZEOF(stack_),
                        entry, this, p2, p3, prio, 0, K_NO_WAIT);
    }

    T& operator[](uint8_t i) { return items_[i < used_ ? i : 0]; }
    const T& operator[](uint8_t i) const { return items_[i < used_ ? i : 0]; }
    uint8_t Count() const { return used_; }

private:
    k_thread thread_{};
    K_KERNEL_STACK_MEMBER(stack_, StackSize);
    T items_[N]{};
    uint8_t used_ = 0;
};
