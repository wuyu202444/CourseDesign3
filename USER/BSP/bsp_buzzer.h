/*
* bsp_buzzer.h
 * 描述: 无源蜂鸣器驱动 (PWM模式)
 * 支持: 设置频率(音调) 和 模拟音量
 */

#ifndef BSP_BUZZER_H
#define BSP_BUZZER_H

#include "main.h"

// 定义使用的定时器句柄 (需与 main.c/tim.c 中一致)
extern TIM_HandleTypeDef htim2;

// 定义使用的通道
#define BUZZER_TIM_CHANNEL TIM_CHANNEL_2

// ==============================
// 接口函数
// ==============================

/**
 * @brief  开启蜂鸣器并设置参数
 * @param  freq_hz : 频率 (Hz), 例如 2000 代表 2kHz
 * @param  volume  : 音量 (0-100), 100为最大音量(50%占空比), 0为静音
 */
void BSP_Buzzer_SetTone(uint32_t freq_hz, uint8_t volume);

/**
 * @brief  关闭蜂鸣器
 */
void BSP_Buzzer_Off(void);

#endif // BSP_BUZZER_H