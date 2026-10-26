#ifndef OLED_DEMO_H
#define OLED_DEMO_H

#include "u8g2.h"

void Demo_Identity(u8g2_t *display);
void Demo_Graphics(u8g2_t *display, uint16_t frame);
void Demo_Horizontal(u8g2_t *display, int16_t x);
void Demo_Vertical(u8g2_t *display, int16_t y);
void Demo_Avatar(u8g2_t *display, uint16_t frame);
void Demo_HardwarePage(u8g2_t *display);
void Demo_Run(u8g2_t *display);

#endif
