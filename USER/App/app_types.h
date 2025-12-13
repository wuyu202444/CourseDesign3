#ifndef __APP_TYPES_H
#define __APP_TYPES_H

typedef struct {
    float temp_celsius;   // 温度 (来自 LM75)
    uint16_t adc_raw;     // 电位器电压值 (来自 ADC DMA)
    uint32_t freq_hz;     // 信号频率 (来自 TIM IC，后续任务)
} SensorData_t;

#endif
