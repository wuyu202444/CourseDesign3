/*
* bsp_lm75.c
 * 描述: LM75 温度传感器驱动实现
 */

#include "bsp_lm75.h"
#include "i2c.h"      // 引用 CubeMX 生成的 I2C 句柄 (hi2c1)
#include <stdio.h>    // 用于调试打印 (可选)

// I2C 超时时间 (ms)
#define I2C_TIMEOUT   100

/**
 * @brief  初始化 LM75
 * @note   实际上是检查设备是否在线
 */
uint8_t BSP_LM75_Init(void)
{
    // 检查设备是否在 I2C 总线上响应
    // 尝试 3 次，每次等待 100ms
    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, LM75_ADDR_WRITE, 3, 100);

    if (status == HAL_OK)
    {
        // 可以在这里配置 LM75 的配置寄存器 (如需要)
        // 目前保持默认即可
        return 0; // 成功
    }
    else
    {
        return 1; // 失败
    }
}

/**
 * @brief  读取温度并转换为浮点数
 * @note   LM75 温度寄存器为 16 位，高 8 位为整数，低 8 位的高位包含小数部分
 *         不同厂家 (NXP, TI, ST) 的 LM75 精度略有不同 (9-bit 到 11-bit)
 *         通用算法：(HighByte << 8 | LowByte) >> 5，然后 * 0.125
 */
float BSP_LM75_ReadTemp(void)
{
    uint8_t raw_data[2] = {0};
    int16_t temp_raw = 0;
    float temp_celsius = 0.0f;

    // 使用 Mem_Read 自动处理 Start -> Send Reg -> Repeated Start -> Read Data 时序
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1,
                                                LM75_ADDR_WRITE,
                                                LM75_REG_TEMP,
                                                I2C_MEMADD_SIZE_8BIT,
                                                raw_data,
                                                2,
                                                I2C_TIMEOUT);

    if (status != HAL_OK)
    {
        // 读取失败，返回错误标识值
        return -999.0f;
    }

    // 数据拼接
    // raw_data[0]: 高 8 位 (整数部分 + 符号)
    // raw_data[1]: 低 8 位 (包含小数部分)

    // 组合成 16 位有符号整数
    temp_raw = (int16_t)((raw_data[0] << 8) | raw_data[1]);

    // LM75 标准精度通常有效位是高 11 位 (即右移 5 位)
    // 分辨率为 0.125 度
    temp_raw >>= 5;

    // 计算实际温度
    temp_celsius = temp_raw * 0.125f;

    return temp_celsius;
}