#include "oled_demo.h"
#include "oled_port.h"

extern const uint8_t lab6_font[];

static const char long_text[] = "Hello，欢迎来到重庆交通大学物联网205实训室！";
static const char *const lines[] = {
    "Hello，欢迎来到",
    "重庆交通大学",
    "物联网205实训室",
    "周新城",
    "632407030424"
};

/* 32x32 XBM, one bit per pixel, least significant bit first in each byte. */
static const uint8_t avatar[] = {
    0x00,0xE0,0x07,0x00, 0x00,0x1C,0x38,0x00,
    0x00,0x02,0x40,0x00, 0x80,0x01,0x80,0x01,
    0x40,0x00,0x00,0x02, 0x20,0x00,0x00,0x04,
    0x10,0x00,0x00,0x08, 0x10,0x00,0x00,0x08,
    0x08,0x00,0x00,0x10, 0x08,0x00,0x00,0x10,
    0x04,0x1C,0x38,0x20, 0x04,0x1C,0x38,0x20,
    0x04,0x1C,0x38,0x20, 0x04,0x00,0x00,0x20,
    0x04,0x00,0x00,0x20, 0x04,0x00,0x00,0x20,
    0x04,0x00,0x00,0x20, 0x04,0x00,0x00,0x20,
    0x04,0x20,0x04,0x20, 0x04,0x40,0x02,0x20,
    0x08,0x80,0x01,0x10, 0x08,0x00,0x00,0x10,
    0x10,0x00,0x00,0x08, 0x10,0x00,0x00,0x08,
    0x20,0x00,0x00,0x04, 0x40,0x00,0x00,0x02,
    0x80,0x01,0x80,0x01, 0x00,0x02,0x40,0x00,
    0x00,0x1C,0x38,0x00, 0x00,0xE0,0x07,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

static void Begin(u8g2_t *display, const char *title)
{
    u8g2_ClearBuffer(display);
    u8g2_SetDrawColor(display, 1);
    u8g2_SetFont(display, u8g2_font_6x12_tf);
    u8g2_DrawStr(display, 0, 11, title);
    u8g2_DrawHLine(display, 0, 14, 128);
}

void Demo_Identity(u8g2_t *display)
{
    Begin(display, "STM32 OLED / I2C");
    u8g2_SetFont(display, lab6_font);
    u8g2_DrawUTF8(display, 0, 34, "周新城");
    u8g2_SetFont(display, u8g2_font_6x12_tf);
    u8g2_DrawStr(display, 0, 50, "ID: 632407030424");
    u8g2_DrawStr(display, 0, 63, "STM32F103C8T6 + U8g2");
}

void Demo_Graphics(u8g2_t *display, uint16_t frame)
{
    Begin(display, "U8g2 Demo");
    u8g2_DrawFrame(display, 1, 19, 42, 23);
    u8g2_DrawBox(display, 5, 23, 12, 15);
    u8g2_DrawCircle(display, 64, 30, 11, U8G2_DRAW_ALL);
    u8g2_DrawDisc(display, 100, 30, 3 + frame % 9, U8G2_DRAW_ALL);
    u8g2_DrawLine(display, 81, 41, 119, 19);
    u8g2_DrawFrame(display, 1, 48, 126, 12);
    u8g2_DrawBox(display, 3, 50, frame % 123, 8);
}

void Demo_Horizontal(u8g2_t *display, int16_t x)
{
    Begin(display, "Text: LEFT / RIGHT");
    u8g2_SetClipWindow(display, 0, 17, 128, 64);
    u8g2_SetFont(display, lab6_font);
    u8g2_DrawUTF8(display, x, 43, long_text);
    u8g2_SetMaxClipWindow(display);
}

void Demo_Vertical(u8g2_t *display, int16_t y)
{
    uint8_t i;
    Begin(display, "Text: UP / DOWN");
    u8g2_SetClipWindow(display, 0, 17, 128, 64);
    u8g2_SetFont(display, lab6_font);
    for (i = 0; i < sizeof(lines) / sizeof(lines[0]); ++i)
        u8g2_DrawUTF8(display, 0, y + i * 20, lines[i]);
    u8g2_SetMaxClipWindow(display);
}

void Demo_Avatar(u8g2_t *display, uint16_t frame)
{
    int16_t x = frame % 144;
    int16_t y;
    if (x > 72) x = 144 - x;
    y = 22 + (frame % 20 < 10 ? frame % 10 : 10 - frame % 10);
    Begin(display, "XBM: animated avatar");
    u8g2_DrawXBM(display, x + 12, y, 32, 32, avatar);
    if (frame % 30 < 5)
    {
        u8g2_SetDrawColor(display, 0);
        u8g2_DrawBox(display, x + 20, y + 10, 16, 3);
        u8g2_SetDrawColor(display, 1);
        u8g2_DrawHLine(display, x + 21, y + 11, 3);
        u8g2_DrawHLine(display, x + 33, y + 11, 3);
    }
}

void Demo_HardwarePage(u8g2_t *display)
{
    Begin(display, "SSD1306 HW SCROLL");
    u8g2_DrawStr(display, 0, 28, "Top: fixed");
    u8g2_SetFont(display, lab6_font);
    u8g2_DrawUTF8(display, 0, 49, "周新城");
    u8g2_SetFont(display, u8g2_font_6x12_tf);
    u8g2_DrawStr(display, 0, 63, "632407030424");
}

void Demo_Run(u8g2_t *display)
{
    int16_t x, y, limit;
    uint16_t frame;
    OLED_ScrollStop();
    Demo_Identity(display);
    u8g2_SendBuffer(display);
    HAL_Delay(3000);

    for (frame = 0; frame < 123; ++frame)
    {
        Demo_Graphics(display, frame);
        u8g2_SendBuffer(display);
        HAL_Delay(20);
    }

    u8g2_SetFont(display, lab6_font);
    limit = 128 - (int16_t)u8g2_GetUTF8Width(display, long_text);
    /* Stop at both ends so that the full long sentence is readable. */
    for (x = 0; x >= limit; --x)
    {
        Demo_Horizontal(display, x);
        u8g2_SendBuffer(display);
        HAL_Delay(15);
    }
    HAL_Delay(500);
    for (x = limit; x <= 0; ++x)
    {
        Demo_Horizontal(display, x);
        u8g2_SendBuffer(display);
        HAL_Delay(15);
    }

    for (y = 63; y >= -64; --y)
    {
        Demo_Vertical(display, y);
        u8g2_SendBuffer(display);
        HAL_Delay(20);
    }
    for (y = -64; y <= 63; ++y)
    {
        Demo_Vertical(display, y);
        u8g2_SendBuffer(display);
        HAL_Delay(20);
    }

    for (frame = 0; frame < 144; ++frame)
    {
        Demo_Avatar(display, frame);
        u8g2_SendBuffer(display);
        HAL_Delay(25);
    }

    Demo_HardwarePage(display);
    u8g2_SendBuffer(display);
    OLED_ScrollStart(1);
    HAL_Delay(4000);
    OLED_ScrollStop();
    /* SSD1306 requires rewriting display RAM after scroll deactivation. */
    u8g2_SendBuffer(display);
    OLED_ScrollStart(0);
    HAL_Delay(4000);
    OLED_ScrollStop();
}
