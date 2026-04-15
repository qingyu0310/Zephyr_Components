/**
 * @file main.c
 * @author qingyu
 * @brief 
 * @version 0.1
 * @date 2026-04-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <zephyr/kernel.h>
#include "Init.h"

int main(void)
{
	Init();

	while (1) 
	{
		k_msleep(1000);
	}
}
