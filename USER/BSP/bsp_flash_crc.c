/*
 * bsp_flash_crc.c
 */

#include "bsp_flash_crc.h"
#include <string.h>

// 声明 CRC 句柄，假设 CubeMX 生成的代码中叫 hcrc
// 如果没有自动生成，这里定义一个外部引用或自己初始化
extern CRC_HandleTypeDef hcrc;

// 如果你的 CubeMX 没有生成 hcrc，请取消下面这段注释手动定义：
/*
CRC_HandleTypeDef hcrc;
void BSP_CRC_Init(void) {
    hcrc.Instance = CRC;
    HAL_CRC_Init(&hcrc);
}
*/

// 计算结构体数据的 CRC (不包含结构体最后的 crc32 字段)
static uint32_t Calculate_Settings_CRC(SystemSettings_t *pSettings) {
    // 计算长度：总大小 - CRC字段本身的大小 (4字节)
    // 转换为 32位 字的数量
    uint32_t length_words = (sizeof(SystemSettings_t) - 4) / 4;

    // 复位 CRC 计算单元
    __HAL_CRC_DR_RESET(&hcrc);

    // 计算
    return HAL_CRC_Accumulate(&hcrc, (uint32_t *)pSettings, length_words);
}

void BSP_CRC_Init(void) {
    // 确保 CRC 时钟开启 (通常在 main.c MX_CRC_Init 中完成)
    // 这里是一个钩子，如果需要手动初始化可以在此添加
}

uint8_t BSP_Settings_Read(SystemSettings_t *pSettings) {
    // 1. 直接从 Flash 地址拷贝数据到 RAM
    SystemSettings_t *pFlashData = (SystemSettings_t *)FLASH_USER_START_ADDR;
    memcpy(pSettings, pFlashData, sizeof(SystemSettings_t));

    // 2. 检查 Flash 是否为空 (擦除后全是 0xFF)
    // 如果温度是 NaN 或者 0xFFFFFFFF，说明没存过
    uint32_t *pCheck = (uint32_t *)pSettings;
    if (pCheck[0] == 0xFFFFFFFF) {
        return 1; // 空数据
    }

    // 3. 计算 CRC 校验
    uint32_t calc_crc = Calculate_Settings_CRC(pSettings);

    // 4. 对比读取到的 CRC
    if (calc_crc == pSettings->crc32) {
        return 0; // 校验通过
    } else {
        return 1; // 校验失败
    }
}

uint8_t BSP_Settings_Save(SystemSettings_t *pSettings) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;
    HAL_StatusTypeDef status;

    // 1. 计算新的 CRC 并填入结构体
    pSettings->crc32 = Calculate_Settings_CRC(pSettings);

    // 2. 解锁 Flash
    HAL_FLASH_Unlock();

    // 3. 擦除扇区 (Sector 7)
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7V - 3.6V
    EraseInitStruct.Sector = FLASH_SECTOR_7;
    EraseInitStruct.NbSectors = 1;

    status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return 1; // 擦除失败
    }

    // 4. 写入数据
    // 因为是按字 (32-bit) 写入，循环写入结构体内容
    uint32_t *pSource = (uint32_t *)pSettings;
    uint32_t write_addr = FLASH_USER_START_ADDR;
    uint32_t num_words = sizeof(SystemSettings_t) / 4;

    for (uint32_t i = 0; i < num_words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr, pSource[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return 2; // 写入失败
        }
        write_addr += 4;
    }

    // 5. 锁定 Flash
    HAL_FLASH_Lock();
    return 0; // 成功
}