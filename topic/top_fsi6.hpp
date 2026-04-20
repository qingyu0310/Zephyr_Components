/**
 * @file top_fsi6.hpp
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "fs_i6.hpp"
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

namespace topic::fsi6 {

using Message = FsI6::OutputData;

}

ZBUS_CHAN_DECLARE(remote_fsi6_chan);