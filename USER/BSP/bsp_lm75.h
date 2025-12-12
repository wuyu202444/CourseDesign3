/*
* bsp_lm75.h
 * 描述: LM75 温度传感器驱动头文件
 */

#ifndef BSP_LM75_H
#define BSP_LM75_H

#include "main.h" // 包含 HAL 库定义

// ==============================
// 1. 硬件地址定义
// ==============================
// LM75 基础地址 0x48 (A0,A1,A2 接地)
// HAL库使用 8-bit 地址 (左移1位): 0x48 << 1 = 0x90
#define LM75_ADDR_WRITE   0x90
#define LM75_ADDR_READ    0x91

// ==============================
// 2. 寄存器映射
// ==============================
#define LM75_REG_TEMP     0x00  // 温度寄存器 (只读)
#define LM75_REG_CONF     0x01  // 配置寄存器
#define LM75_REG_THYST    0x02  // 滞后寄存器
#define LM75_REG_TOS      0x03  // 过温关断寄存器

// ==============================
// 3. 接口函数声明
// ==============================

/**
 * @brief  初始化 LM75 传感器
 * @retval 0: 成功, 1: 失败 (设备未就绪)
 */
uint8_t BSP_LM75_Init(void);

/**
 * @brief  读取当前温度值
 * @retval 温度值 (单位: 摄氏度 float), 如失败返回 -999.0f
 */
float BSP_LM75_ReadTemp(void);

#endif // BSP_LM75_H