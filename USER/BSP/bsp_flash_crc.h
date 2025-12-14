/*
* bsp_flash_crc.h
 * 描述: 内部 Flash 读写与 CRC 校验驱动
 * 功能: 用于保存系统设置(如报警阈值)
 */

#ifndef __BSP_FLASH_CRC_H__
#define __BSP_FLASH_CRC_H__

#include "main.h"

// ================= 配置参数 =================

// STM32F411RE Sector 7 起始地址 (128KB)
// 范围: 0x0806 0000 - 0x0807 FFFF
// 注意: 请确保你的代码大小没有超过 384KB (即没有占用 Sector 7)
#define FLASH_USER_START_ADDR   0x08060000

// ================= 数据结构 =================

// 系统设置结构体 (必须 4 字节对齐)
typedef struct {
    float temp_threshold;     // 温度报警阈值
    uint32_t padding;         // 保留位(对齐用)
    uint32_t crc32;           // CRC校验码 (必须在最后)
} SystemSettings_t;

// ================= 接口声明 =================

/**
 * @brief  初始化 CRC 外设
 */
void BSP_CRC_Init(void);

/**
 * @brief  读取系统设置
 * @param  pSettings: 接收数据的结构体指针
 * @retval 0:读取成功且校验通过, 1:校验失败或为空(需加载默认值)
 */
uint8_t BSP_Settings_Read(SystemSettings_t *pSettings);

/**
 * @brief  保存系统设置到 Flash
 * @param  pSettings: 要保存的结构体指针
 * @retval 0:成功, 其他:失败
 */
uint8_t BSP_Settings_Save(SystemSettings_t *pSettings);

#endif // __BSP_FLASH_CRC_H__