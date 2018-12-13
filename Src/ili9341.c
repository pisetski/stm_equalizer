/*
 * ili9341.c
 *
 *  Created on: 6 ���. 2017 �.
 *      Author: Oleg
 */
#include "ili9341.h"
#include "math.h"

static MyFONT *LCD_Currentfonts;

__IO uint16_t TextColor = 0xFFFF, BackColor = 0x0000;

void LCD_SetColors(__IO uint16_t _TextColor, __IO uint16_t _BackColor)
{
  TextColor = _TextColor;
  BackColor = _BackColor;
}

void LCD_GetColors(__IO uint16_t *_TextColor, __IO uint16_t *_BackColor)
{
  *_TextColor = TextColor;
  *_BackColor = BackColor;
}

void LCD_SetTextColor(__IO uint16_t Color)
{
  TextColor = Color;
}

void LCD_SetBackColor(__IO uint16_t Color)
{
  BackColor = Color;
}

void LCD_SetFont(MyFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

MyFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

uint8_t LCD_SetRecordingArea(uint16_t x, uint16_t y, uint16_t width,
    uint16_t height)
{
  uint16_t x2 = x + width - 1;
  uint16_t y2 = y + height - 1;

  if (x > LCD_TFTWIDTH - 1 || y > LCD_TFTHEIGHT - 1 || x2 > LCD_TFTWIDTH - 1
      || y2 > LCD_TFTHEIGHT - 1) return 1;

  LCD_WriteCommand(LCD_COLADDRSET);
  LCD_WriteParametr(x >> 8);
  LCD_WriteParametr(x);
  LCD_WriteParametr(x2 >> 8);
  LCD_WriteParametr(x2);
  LCD_WriteCommand(LCD_PAGEADDRSET);
  LCD_WriteParametr(y >> 8);
  LCD_WriteParametr(y);
  LCD_WriteParametr(y2 >> 8);
  LCD_WriteParametr(y2);

  return 0;
}

void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const MyGLYPH *glyph)
{
  uint8_t full_width = glyph->Width + LCD_Currentfonts->Letter_spacing * 2;
  uint32_t j = 0, i = 0;
  if (LCD_SetRecordingArea(Xpos, Ypos, full_width, LCD_Currentfonts->Height)
      == 1) return;

  LCD_WriteCommand(LCD_MEMORYWRITE);

  uint8_t glyph_height = glyph->Y1 - glyph->Y0 + 1;

  for (i = 0; i < glyph->Y0; i++)
  {
    for (j = 0; j < full_width; j++)
      LCD_WriteData(BackColor);
  }

  for (i = 0; i < glyph_height; i++)
  {
    /* Prepare to write GRAM */

    for (j = 0; j < LCD_Currentfonts->Letter_spacing; j++)
      LCD_WriteData(BackColor);

    for (j = 0; j < glyph->Width; j++)
    {
      uint32_t indexBit = j + i * glyph->Width + glyph->Index;
      uint32_t index = indexBit >> 5;
      uint8_t bit = indexBit % 32;
      uint32_t mask = 1 << bit;
      uint32_t data = LCD_Currentfonts->table[index];
      if ((data & mask) > 0) LCD_WriteData(TextColor);
      else LCD_WriteData(BackColor);
    }

    for (j = 0; j < LCD_Currentfonts->Letter_spacing; j++)
      LCD_WriteData(BackColor);
  }

  for (i = glyph->Y1 + 1; i < LCD_Currentfonts->Height; i++)
  {
    for (j = 0; j < full_width; j++)
      LCD_WriteData(BackColor);
  }

}

void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  if (Ascii < 32) return;
  LCD_DrawChar(Xpos, Ypos, &LCD_Currentfonts->glyphTable[Ascii - 32]);
}

void LCD_DisplayStringLine(uint16_t Xpos, uint16_t Ypos, char *ptr)
{
  static char cp1251[100];
  convert_utf8_to_windows1251(ptr, cp1251, strlen(ptr));
  uint16_t len = strlen(cp1251);
  ptr = cp1251;
  uint16_t x_delta = 0;
  uint16_t index = 0;
  /* Send the string character by character on lCD */
  while (*ptr != 0)
  {

      if(((*ptr) & 0b11100000)  == 0b11000000)
      {

      }
    /* Display one character on LCD */
    LCD_DisplayChar(Xpos + x_delta, Ypos, *ptr);
    /* Decrement the column position by 16 */
    uint8_t width = (&LCD_Currentfonts->glyphTable[*ptr - 32])->Width;

    x_delta += width + LCD_Currentfonts->Letter_spacing * 2;
    if (x_delta + Xpos >= LCD_TFTWIDTH) return;
    /* Point on the next character */
    index++;
    if (index == len) return;
    ptr++;
  }
}

void LCD_Init(void)
{
  LCD_WriteCommand(LCD_SOFTRESET);
  HAL_Delay(5);



 // magic?
     LCD_WriteCommand(0xcf);
     LCD_WriteParametr(0x00);
     LCD_WriteParametr(0x83);
     LCD_WriteParametr(0x30);

      LCD_WriteCommand(0xed);
      LCD_WriteParametr(0x64);
      LCD_WriteParametr(0x03);
      LCD_WriteParametr(0x12);
      LCD_WriteParametr(0x81);
      LCD_WriteCommand(0xe8);
      LCD_WriteParametr(0x85);
      LCD_WriteParametr(0x01);
      LCD_WriteParametr(0x79);
      LCD_WriteCommand(0xcb);
      LCD_WriteParametr(0x39);
      LCD_WriteParametr(0x2c);
      LCD_WriteParametr(0x00);
      LCD_WriteParametr(0x34);
      LCD_WriteParametr(0x02);
      LCD_WriteCommand(0xf7);
      LCD_WriteParametr(0x20);
      LCD_WriteCommand(0xea);
      LCD_WriteParametr(0x00);
      LCD_WriteParametr(0x00);



  //------------power control------------------------------
  LCD_WriteCommand(0xc0); //power control
    LCD_WriteParametr(0x26);
    LCD_WriteCommand(0xc1); //power control
    LCD_WriteParametr(0x11);
    //--------------VCOM
    LCD_WriteCommand(0xc5); //vcom control
    LCD_WriteParametr(0x35);//35
    LCD_WriteParametr(0x3e);//3E
    LCD_WriteCommand(0xc7); //vcom control
    LCD_WriteParametr(0xbe); // 0x94

 LCD_WriteCommand( 0x36);
    // memory access control
  LCD_WriteParametr(0x28); //0048 my,mx,mv,ml,BGR,mh,0.0
    LCD_WriteCommand(0x3a); // pixel format set
    LCD_WriteParametr( 0x55);//16bit /pixel

  //Frame Rate Control
  LCD_WriteCommand(0xB1);
  LCD_WriteParametr(0x00);
  LCD_WriteParametr(0x1B);	 //Frame Rate 70Hz



  //Display Function Control
  LCD_WriteCommand(0xB6);
  LCD_WriteParametr(0x02);
  LCD_WriteParametr(0x82);

  LCD_WriteCommand(0xf2); // 3Gamma Function Disable
  LCD_WriteParametr(0x08);
    LCD_WriteCommand(0x26);
    LCD_WriteParametr(0x01); // gamma set 4 gamma curve 01/02/04/08

  LCD_WriteCommand(0xE0); //positive gamma correction
    LCD_WriteParametr(0x1f);
    LCD_WriteParametr(0x1a);
    LCD_WriteParametr(0x18);
    LCD_WriteParametr(0x0a);
    LCD_WriteParametr(0x0f);
    LCD_WriteParametr(0x06);
    LCD_WriteParametr(0x45);
    LCD_WriteParametr(0x87);
    LCD_WriteParametr(0x32);
    LCD_WriteParametr(0x0a);
    LCD_WriteParametr(0x07);
    LCD_WriteParametr(0x02);
    LCD_WriteParametr(0x07);
    LCD_WriteParametr(0x05);
    LCD_WriteParametr(0x00);
    LCD_WriteCommand(0xE1); //negamma correction
    LCD_WriteParametr(0x00);
    LCD_WriteParametr(0x25);
    LCD_WriteParametr(0x27);
    LCD_WriteParametr(0x05);
    LCD_WriteParametr(0x10);
    LCD_WriteParametr(0x09);
    LCD_WriteParametr(0x3a);
    LCD_WriteParametr(0x78);
    LCD_WriteParametr(0x4d);
    LCD_WriteParametr(0x05);
    LCD_WriteParametr(0x18);
    LCD_WriteParametr(0x0d);
    LCD_WriteParametr(0x38);
    LCD_WriteParametr(0x3a);
    LCD_WriteParametr(0x1f);

    LCD_WriteCommand(0xb7); //entry mode set
    LCD_WriteParametr(0x07);

    LCD_WriteCommand( 0xb6);
        // display function control
    LCD_WriteParametr(0x0a);
    LCD_WriteParametr(0x82);
    LCD_WriteParametr( 0x27);
    LCD_WriteParametr( 0x00);

  LCD_WriteCommand(LCD_SLEEPOUT);
  HAL_Delay(100);
  LCD_WriteCommand(LCD_DISPLAYON);
  HAL_Delay(100);

  LCD_SetFont(&LCD_DEFAULT_FONT);
}

void LCD_GenerateJuliaFpu(float offset_x, float offset_y, float zoom,
    float real_const, float imag_const, uint16_t color)
{

  LCD_SetRecordingArea(0, 24, LCD_TFTWIDTH, LCD_TFTHEIGHT - 24);

  LCD_WriteCommand(LCD_MEMORYWRITE);
  float tmp1, tmp2;
  float num_real, num_img;
  float radius;
  uint8_t i;
  uint16_t x, y;
  uint16_t r = color >> 11;
  uint16_t g = (color >> 5) & 0x3F;
  uint16_t b = color & 0x1F;
  for (y = 24; y < LCD_TFTHEIGHT; y++)
  {
    for (x = 0; x < LCD_TFTWIDTH; x++)
    {
      num_real = y - 20 - (LCD_TFTHEIGHT - 24)/2;
      num_real /= zoom;
      num_real += offset_y;
      num_img = x - LCD_TFTWIDTH/2;
      num_img /= zoom;
      num_img += offset_x;
      i = 0;

      radius = 0;
      while ((i < ITERATION - 1) && (radius < 3))
      {
        tmp1 = num_real * num_real;
        tmp2 = num_img * num_img;
        num_img = 2 * num_real * num_img + imag_const;
        num_real = tmp1 - tmp2 + real_const;
        radius = tmp1 + tmp2;
        i++;
      }
      /* Store the value in the buffer */
      uint16_t ur = (r*i) >> 5;
      uint16_t ug = (g*i) >> 5;
      uint16_t ub = (b*i) >> 5;
      LCD_WriteData((ur<<11) | (ug << 5) | (ub));
    }
  }
}

void LCD_FillScreen(uint16_t color)
{
  LCD_SetRecordingArea(0, 0, LCD_TFTWIDTH, LCD_TFTHEIGHT);

  LCD_WriteCommand(LCD_MEMORYWRITE);
  for (uint32_t counter = 0;
      counter < ((uint32_t) LCD_TFTWIDTH * (uint32_t) LCD_TFTHEIGHT); counter++)
  {
    LCD_WriteData(color);
  }
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
    uint16_t color)
{

  if (LCD_SetRecordingArea(x, y, width, height) == 1) return;

  LCD_WriteCommand(LCD_MEMORYWRITE);
  for (uint32_t counter = 0; counter < ((uint32_t) width * (uint32_t) height);
      counter++)
  {
    LCD_WriteData(color);
  }
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{

  if (LCD_SetRecordingArea(x, y, 1, 1) == 1) return;
  LCD_WriteCommand(LCD_MEMORYWRITE);

  LCD_WriteData(color);
}

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
    uint16_t color)
{
  int16_t dx, dy, sx, sy, err, e2;
  uint16_t tmp;

  /* Check for overflow */
  if (x0 > LCD_TFTWIDTH - 1)
  {
    x0 = LCD_TFTWIDTH - 1;
  }
  if (x1 > LCD_TFTWIDTH - 1)
  {
    x1 = LCD_TFTWIDTH - 1;
  }
  if (y0 > LCD_TFTHEIGHT)
  {
    y0 = LCD_TFTHEIGHT - 1;
  }
  if (y1 > LCD_TFTHEIGHT)
  {
    y1 = LCD_TFTHEIGHT - 1;
  }

  uint8_t step = 0;

  if (x0 > x1)
  {
    tmp = x0;
    x0 = x1;
    x1 = tmp;
    step = 1 - step;
  }
  if (y0 > y1)
  {
    tmp = y0;
    y0 = y1;
    y1 = tmp;
    step = 1 - step;
  }

  dx = x1 - x0;
  dy = y1 - y0;

  /* Vertical or horizontal line */
  if (dx == 0 || dy == 0)
  {
    LCD_DrawRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1, color);
    return;
  }

  sx = (x0 < x1) ? 1 : -1;
  sy = (y0 < y1) ? 1 : -1;
  err = ((dx > dy) ? dx : -dy) / 2;
  if (step) while (1)
  {
    LCD_DrawPixel(x1, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = err;
    if (e2 > -dx)
    {
      err -= dy;
      x1 -= sx;
    }
    if (e2 < dy)
    {
      err += dx;
      y0 += sy;
    }
  }
  else while (1)
  {
    LCD_DrawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    e2 = err;
    if (e2 > -dx)
    {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dy)
    {
      err += dx;
      y0 += sy;
    }
  }

}







void LCD_DrawPlot(float *data, uint16_t len, float zoom, uint16_t zero,
    uint16_t color)
{
  float delta = ((float) LCD_TFTWIDTH) / len;
  uint16_t y0 = (uint16_t) (data[0] * zoom + zero);
  uint16_t x0 = 0;
  for (uint16_t i = 1; i < len; i++)
  {
    uint16_t y1 = (uint16_t) (data[i] * zoom + zero);
    uint16_t x1 = (uint16_t) (delta * i);
    LCD_DrawLine(x1, y1, x0, y0, color);
    x0 = x1;
    y0 = y1;
  }
}



uint16_t xy2rgb(float x, float y)
{

    float s = sqrtf(x*x+y*y);
    float h = atan2f(y,x)/((float)M_PI)*180.1f + 179.0f;
    float r,g,b;
    float v;
    v = 0.99f;



    float p, q, t, ff;



    if(s <= 0.0) {       // < is bogus, just shuts up warnings

        return 0xFFFF;
    }


    h /= 60.0f;
    uint16_t i = (uint16_t)  floorf(h);



    ff = h - i;
    p = v*(1.0f - s);
    q = v*(1.0f - (s * ff));
    t = v*(1.0f - (s * (1.0f - ff)));

    switch(i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;

    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
    default:
        r = v;
        g = p;
        b = q;
        break;
    }
    volatile uint16_t ur = (uint16_t)(r*32);
    volatile uint16_t ug = (uint16_t)(g*64);
    volatile uint16_t ub = (uint16_t)(b*32);

    return (ur<<11) | (ug<<5) | ub;
}

const uint32_t cursor[] =
{
    0x00015555,
    0x0041AAA9,
    0x0101AAA9,
    0x0401AAA9,
    0x1555AAA9,
    0x0401AAA9,
    0x0101AAA9,
    0x0041AAA9,
    0x00015555
};

uint16_t LCD_DrawColorPipet(uint16_t colorX, uint16_t colorY, uint16_t oldColorX, uint16_t oldColorY)
{

  if(LCD_SetRecordingArea(oldColorX,oldColorY-4, 16, 9) == 1) return WHITE;
  LCD_WriteCommand(LCD_MEMORYWRITE);
  for (uint16_t y = 0; y < 9; y++)
  {
    for (uint16_t x = 0; x < 16; x++)
    {
      uint16_t background = xy2rgb(((float)oldColorX + x          -  LCD_TFTWIDTH     /2.0f)/(LCD_TFTWIDTH)    *2.0f,
                                   ((float)oldColorY + y - 4 - 24 - (LCD_TFTHEIGHT-24)/2.0f)/(LCD_TFTHEIGHT-24)*2.0f);
      LCD_WriteData(background);
    }
  }


  if(LCD_SetRecordingArea(colorX,colorY-4, 16, 9) == 1) return WHITE;
  LCD_WriteCommand(LCD_MEMORYWRITE);
  uint16_t color = xy2rgb(((float)colorX - LCD_TFTWIDTH/2.0f)/(LCD_TFTWIDTH)*2.0f, ((float)colorY - 24 - (LCD_TFTHEIGHT-24)/2.0f)/(LCD_TFTHEIGHT-24)*2.0f);


  for (uint8_t y = 0; y < 9; y++)
  {
    uint32_t curs_v_line = cursor[y];
    for (uint8_t x = 0; x < 16; x++)
    {
      uint8_t pixel = (curs_v_line >> (30-x*2)) & 0x3;
      if(pixel == 0)
      {
        uint16_t background = xy2rgb(((float)colorX + x          -  LCD_TFTWIDTH     /2.0f)/(LCD_TFTWIDTH)    *2.0f,
                                     ((float)colorY + y - 4 - 24 - (LCD_TFTHEIGHT-24)/2.0f)/(LCD_TFTHEIGHT-24)*2.0f);
        LCD_WriteData(background);
      }
      else if(pixel == 1) LCD_WriteData(BLACK);
      else if(pixel == 2) LCD_WriteData(color);

    }
  }



  return color;

}

