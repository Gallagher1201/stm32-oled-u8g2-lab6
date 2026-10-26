#include "oled_port.h"
#include "i2c.h"
#include <string.h>

volatile HAL_StatusTypeDef oled_i2c_status = HAL_OK;
static uint8_t oled_address = 0x3C << 1;

static uint8_t I2C_Byte(u8x8_t *u8x8, uint8_t msg,
                        uint8_t count, void *data)
{
    static uint8_t buffer[128];
    static uint16_t used;
    (void)u8x8;
    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
    case U8X8_MSG_BYTE_SET_DC:
        return 1;
    case U8X8_MSG_BYTE_START_TRANSFER:
        used = 0;
        return 1;
    case U8X8_MSG_BYTE_SEND:
        if (used + count > sizeof(buffer))
        {
            oled_i2c_status = HAL_ERROR;
            return 0;
        }
        memcpy(buffer + used, data, count);
        used += count;
        return 1;
    case U8X8_MSG_BYTE_END_TRANSFER:
        oled_i2c_status = HAL_I2C_Master_Transmit(
            &hi2c2, oled_address, buffer, used, 100);
        return oled_i2c_status == HAL_OK;
    default:
        return 0;
    }
}

static void DelayCycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {}
}

static uint8_t GPIO_Delay(u8x8_t *u8x8, uint8_t msg,
                          uint8_t count, void *data)
{
    (void)u8x8;
    (void)data;
    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        break;
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay(count);
        break;
    case U8X8_MSG_DELAY_10MICRO:
        DelayCycles((SystemCoreClock / 100000U) * count);
        break;
    case U8X8_MSG_DELAY_100NANO:
        DelayCycles((SystemCoreClock / 10000000U + 1U) * count);
        break;
    default:
        break;
    }
    return 1;
}

uint8_t OLED_Init(u8g2_t *display)
{
    oled_address = 0x3C << 1;
    HAL_Delay(100);
    if (HAL_I2C_IsDeviceReady(&hi2c2, oled_address, 2, 20) != HAL_OK)
    {
        oled_address = 0x3D << 1;
        if (HAL_I2C_IsDeviceReady(&hi2c2, oled_address, 2, 20) != HAL_OK)
        {
            oled_i2c_status = HAL_ERROR;
            return 0;
        }
    }
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
        display, U8G2_R0, I2C_Byte, GPIO_Delay);
    u8g2_SetI2CAddress(display, oled_address);
    u8g2_InitDisplay(display);
    u8g2_SetPowerSave(display, 0);
    u8g2_ClearBuffer(display);
    return oled_i2c_status == HAL_OK;
}

void OLED_ScrollStop(void)
{
    uint8_t command[] = {0x00, 0x2E};
    oled_i2c_status = HAL_I2C_Master_Transmit(
        &hi2c2, oled_address, command, sizeof(command), 100);
}

void OLED_ScrollStart(uint8_t left)
{
    /* Scroll pages 4..7; pages 0..3 stay fixed. No framebuffer writes while active. */
    uint8_t command[] = {0x00, 0x2E, 0x27, 0x00, 0x04,
                         0x00, 0x07, 0x00, 0xFF, 0x2F};
    command[2] = left ? 0x27 : 0x26;
    oled_i2c_status = HAL_I2C_Master_Transmit(
        &hi2c2, oled_address, command, sizeof(command), 100);
}
