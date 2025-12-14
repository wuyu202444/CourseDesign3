/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : bsp_bt.c
  * Description        : 精简版蓝牙透传驱动 (Production Ready)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "bsp_bt.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ================= 缓冲区配置 =================
// 即使是发送方，也需要保留RX缓冲用于初始化时的AT指令回显检查
#define BT_RX_BUF_SIZE   128
static uint8_t rx_byte;
static uint8_t rx_buffer[BT_RX_BUF_SIZE];
static uint16_t rx_index = 0;

// ================= 内部工具函数 =================

static void BT_ClearRxBuf(void) {
    memset(rx_buffer, 0, BT_RX_BUF_SIZE);
    rx_index = 0;
}

static void BT_RawSend(const char *str) {
    HAL_UART_Transmit(BT_UART_HANDLE, (uint8_t*)str, strlen(str), 100);
}

// 简单的AT指令检查函数
static uint8_t BT_SendCmd_CheckACK(const char *cmd, const char *ack, uint32_t timeout_ms) {
    BT_ClearRxBuf();
    BT_RawSend(cmd);

    uint32_t start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < timeout_ms) {
        osDelay(10);
        if (strstr((char*)rx_buffer, ack) != NULL) {
            return 1; // 成功
        }
    }
    return 0; // 超时失败
}

// ================= 外部接口 =================

// 中断回调：主要用于初始化阶段接收 "OK"
void BSP_BT_RxCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART6) {
        if (rx_index < BT_RX_BUF_SIZE - 1) {
            rx_buffer[rx_index++] = rx_byte;
            rx_buffer[rx_index] = '\0';
        }
        // 开启下一次接收
        HAL_UART_Receive_IT(BT_UART_HANDLE, &rx_byte, 1);
    }
}

void BSP_BT_Init(void) {
    // 启动接收中断
    if(HAL_UART_Receive_IT(BT_UART_HANDLE, &rx_byte, 1) != HAL_OK) return;

    printf("[BT] Init Start...\r\n");
    osDelay(500); // 等待模块上电稳定

    // --- 自动配置流程 ---
    // 1. 尝试握手 (防止模块还在睡眠)
    BT_RawSend("AT\r\n");
    osDelay(200);

    // 2. 这里的策略是：不管之前什么状态，尝试配置一遍，最后进入透传
    // 如果模块已经配好了，发这些指令也无所谓

    BT_SendCmd_CheckACK("ATE0\r\n", "OK", 200);        // 关闭回显
    BT_SendCmd_CheckACK("AT+BLEMODE=0\r\n", "OK", 500); // 设为从机
    BT_SendCmd_CheckACK("AT+BLEADVEN=1\r\n", "OK", 500); // 开启广播

    // 3. 关键：进入透传模式
    // 发送后，模块将变身为透明串口线
    printf("[BT] Entering Transparent Mode...\r\n");
    BT_RawSend("AT+TRANSENTER\r\n");
    osDelay(200);

    printf("[BT] Init Done. Ready.\r\n");
}

void BSP_BT_ProcessTask(SensorData_t *pData) {
    static uint32_t last_send_time = 0;
    char data_buf[64];

    // 1秒发送一次
    if (HAL_GetTick() - last_send_time >= 1000) {

        // 1. 格式化数据
        // 格式: $Temp,Voltage,Freq
        sprintf(data_buf, "$%.1f, %.2f, %lu\r\n",
                pData->temp_celsius,
                (float)(pData->adc_raw * 3.3f / 4095.0f),
                pData->freq_hz);

        // 2. 直接发送 (因为已经在透传模式了)
        BT_RawSend(data_buf);

        // 3. 简单的日志，证明活著
        // printf("[BT] Sent: %s", data_buf); // 如果觉得串口刷屏太烦，可以注释掉这行

        last_send_time = HAL_GetTick();
    }

    // 这里的延时可以稍微短一点，保证系统调度流畅
    osDelay(100);
}