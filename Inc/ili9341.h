/*
 * ili9341.h
 *
 *  Created on: 6 ���. 2017 �.
 *      Author: Oleg
 */

#ifndef ILI9341_H_
#define ILI9341_H_
#include "stm32f4xx_hal.h"
#include "fonts.h"
#define ITERATION 33

#define LCD_SOFTRESET           0x01
#define LCD_SLEEPOUT            0x11
#define LCD_DISPLAYON           0x29
#define LCD_COLADDRSET          0x2A
#define LCD_PAGEADDRSET         0x2B
#define LCD_MEMORYWRITE         0x2C
#define LCD_MEMORYREAD          0x2E
#define LCD_MADCTL              0x36
#define MADCTL_MY               0x80
#define MADCTL_MX               0x40
#define MADCTL_MV               0x20
#define MADCTL_BGR              0x08
#define MADCTL_RGB              0x00
#define LCD_COLMOD              0x3A
#define COLMOD_PARAMETR           0x55
#define LCD_READ_ID4            0xD3

#define ROTATION                    2
#if ROTATION == 1
  #define MADCTL_PARAMETR           MADCTL_MX | MADCTL_BGR
  #define LCD_TFTWIDTH          240
  #define LCD_TFTHEIGHT         320
#elif ROTATION == 2
  #define MADCTL_PARAMETR           MADCTL_MV | MADCTL_BGR
  #define LCD_TFTWIDTH          320
  #define LCD_TFTHEIGHT         240
#elif ROTATION == 3
  #define MADCTL_PARAMETR           MADCTL_MY | MADCTL_BGR
  #define LCD_TFTWIDTH          240
  #define LCD_TFTHEIGHT         320
#elif ROTATION == 4
  #define MADCTL_PARAMETR           MADCTL_MX | MADCTL_MY| MADCTL_MV | MADCTL_BGR
  #define LCD_TFTWIDTH          320
  #define LCD_TFTHEIGHT         240
#endif

#define RED_MAX       31
#define GREEN_MAX     63
#define BLUE_MAX    31


#define BLACK                       0x0000
#define BLUE                        0x001F
#define RED                         0xF800
#define GREEN                       0x07E0
#define CYAN                        0x07FF
#define MAGENTA                     0xF81F
#define YELLOW                      0xFFE0
#define WHITE                       0xFFFF

#define LCD_DATA 0x60020000
// ��� ������ ������
#define LCD_REG 0x60000000

#define LCD_DEFAULT_FONT         PT_Sans7


/*Function Prototype Begin*/
__inline void LCD_WriteCommand(uint8_t command)
{
	*(__IO uint16_t *) (LCD_REG) = command;
}

__inline void LCD_WriteParametr(uint8_t parametr)
{
	*(__IO uint16_t *) (LCD_DATA) = parametr;
}

__inline void LCD_WriteData(uint16_t data)
{

	*(__IO uint16_t *) (LCD_DATA) = data;
}

__inline uint16_t LCD_ReadData()
{
  return (*(__IO uint16_t *) (LCD_DATA) );
}

static __inline uint16_t LCD_ReadReg(uint8_t LCD_Reg) {
   /* Write 16-bit Index (then Read Reg) */
  *(__IO uint16_t *) (LCD_REG) = LCD_Reg;
   /* Read 16-bit Reg */
   return (*(__IO uint16_t *) (LCD_DATA) );
}



void LCD_GenerateJuliaFpu(float offset_x, float offset_y, float zoom,
    float real_const, float imag_const, uint16_t color);
void LCD_SetColors(__IO uint16_t _TextColor, __IO uint16_t _BackColor);
void LCD_GetColors(__IO uint16_t *_TextColor, __IO uint16_t *_BackColor);
void LCD_SetTextColor(__IO uint16_t Color);
void LCD_SetBackColor(__IO uint16_t Color);
void LCD_SetFont(MyFONT *fonts);
MyFONT *LCD_GetFont(void);
uint8_t LCD_SetRecordingArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const MyGLYPH *c);
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
void LCD_DisplayStringLine(uint16_t Xpos, uint16_t Ypos, char *ptr);
void LCD_Init(void);
void LCD_FillScreen(uint16_t color);
void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawPlot(float *data, uint16_t len,
		       float zoom, uint16_t zero,
		       uint16_t color);
uint16_t xy2rgb(float h, float s);
uint16_t LCD_DrawColorPipet(uint16_t colorX, uint16_t colorY, uint16_t oldColorX, uint16_t oldColorY);
/*Function Prototype End*/


#endif /* ILI9341_H_ */
