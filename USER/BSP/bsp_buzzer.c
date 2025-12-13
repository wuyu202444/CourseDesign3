/*
* bsp_buzzer.c
 */

#include "bsp_buzzer.h"
#include "tim.h"  // 引用 CubeMX 生成的 tim 头文件

/**
 * @brief  设置蜂鸣器音调和音量
 * @note   原理:
 *         1. 频率 = 定时器时钟 / (ARR + 1)
 *            假设 Prescaler 设为 83 (即 84MHz/84 = 1MHz时钟)
 *            则 ARR = 1,000,000 / freq - 1
 *         2. 音量 = 占空比
 *            无源蜂鸣器在 50% 占空比时最响。
 *            输入 volume (0-100):
 *            - 100 -> 50% 占空比
 *            - 50  -> 25% 占空比
 *            - 10  -> 5%  占空比
 */
void BSP_Buzzer_SetTone(uint32_t freq_hz, uint8_t volume)
{
    // 1. 安全检查
    if (freq_hz == 0 || volume == 0)
    {
        BSP_Buzzer_Off();
        return;
    }

    if (volume > 100) volume = 100;

    // 2. 计算 ARR (自动重装载值) - 控制频率
    // 假设 Timer 时钟为 1MHz (需要在 CubeMX 设置 Prescaler = 84-1)
    uint32_t arr_val = (1000000 / freq_hz) - 1;

    // 3. 计算 CCR (捕获比较值) - 控制音量(占空比)
    // 逻辑：Volume=100 时，CCR = ARR/2 (50% 占空比，最响)
    //       Volume=50  时，CCR = ARR/4 (25% 占空比)
    uint32_t ccr_val = (arr_val + 1) * volume / 200;

    // 4. 应用参数
    // 修改频率 (ARR)
    __HAL_TIM_SET_AUTORELOAD(&htim2, arr_val);

    // 修改占空比 (CCR)
    __HAL_TIM_SET_COMPARE(&htim2, BUZZER_TIM_CHANNEL, ccr_val);

    // 5. 启动 PWM
    HAL_TIM_PWM_Start(&htim2, BUZZER_TIM_CHANNEL);
}

/**
 * @brief  关闭蜂鸣器
 */
void BSP_Buzzer_Off(void)
{
    // 停止 PWM 输出
    HAL_TIM_PWM_Stop(&htim2, BUZZER_TIM_CHANNEL);

    // 清除比较值，确保引脚低电平
    __HAL_TIM_SET_COMPARE(&htim2, BUZZER_TIM_CHANNEL, 0);
}