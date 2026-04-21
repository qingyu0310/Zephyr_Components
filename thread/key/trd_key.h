/**
 * @file trd_key.h
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "thread.hpp"
#include "key.hpp"

enum KeyIndex
{
    KEY0 = 0,
    KEY1,
    KEY2,
    KEY3,
    KEY4,
    KEY5,
    KEY6,
    KEY7,
    KEY8,
    KEY9,
};

namespace thread::key
{
    constexpr uint8_t N = 3;
    inline Thread<Key, N> thread_{};
    void thread_start(uint8_t prio, void* p2 = nullptr, void* p3 = nullptr);
}



