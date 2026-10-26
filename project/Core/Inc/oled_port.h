#ifndef OLED_PORT_H
#define OLED_PORT_H

#include "main.h"
#include "u8g2.h"

extern volatile HAL_StatusTypeDef oled_i2c_status;
uint8_t OLED_Init(u8g2_t *display);
void OLED_ScrollStart(uint8_t left);
void OLED_ScrollStop(void);

#endif
