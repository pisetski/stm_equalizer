
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "usb_host.h"



/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ili9341.h"


#define  NUMBER_IS_2_POW_K(x)   ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...
#define  FT_DIRECT        -1    // Direct transform.
#define  FT_INVERSE        1    // Inverse transform.
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern ApplicationTypeDef Appli_state;
FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */

extern USBH_HandleTypeDef hUsbHostFS;

FIL MyFile;
int positionX = 0;

typedef struct
{
  uint32_t   ChunkID;
  uint32_t   FileSize;
  uint32_t   FileFormat;
  uint32_t   SubChunk1ID;
  uint32_t   SubChunk1Size;
  uint16_t   AudioFormat;
  uint16_t   NbrChannels;
  uint32_t   SampleRate;
  uint32_t   ByteRate;
  uint16_t   BlockAlign;
  uint16_t   BitPerSample;
  uint32_t   SubChunk2ID;
  uint32_t   SubChunk2Size;

}WAVE_FormatTypeDef;
WAVE_FormatTypeDef *waveformat;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void  FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag)
{
	// parameters error check:
	if ((Rdat == NULL) || (Idat == NULL))                  return;
	if ((N > 16384) || (N < 1))                            return;
	if (!NUMBER_IS_2_POW_K(N))                             return;
	if ((LogN < 2) || (LogN > 14))                         return;
	if ((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE)) return;

	register int  i, j, n, k, io, ie, in, nn;
	float         ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

	static const float Rcoef[14] =
	{ -1.0000000000000000F,  0.0000000000000000F,  0.7071067811865475F,
		0.9238795325112867F,  0.9807852804032304F,  0.9951847266721969F,
		0.9987954562051724F,  0.9996988186962042F,  0.9999247018391445F,
		0.9999811752826011F,  0.9999952938095761F,  0.9999988234517018F,
		0.9999997058628822F,  0.9999999264657178F
	};
	static const float Icoef[14] =
	{ 0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
		-0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
		-0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
		-0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
		-0.0007669903187427F, -0.0003834951875714F
	};

	nn = N >> 1;
	ie = N;
	for (n = 1; n <= LogN; n++)
	{
		rw = Rcoef[LogN - n];
		iw = Icoef[LogN - n];
		if (Ft_Flag == FT_INVERSE) iw = -iw;
		in = ie >> 1;
		ru = 1.0F;
		iu = 0.0F;
		for (j = 0; j<in; j++)
		{
			for (i = j; i<N; i += ie)
			{
				io = i + in;
				rtp = Rdat[i] + Rdat[io];
				itp = Idat[i] + Idat[io];
				rtq = Rdat[i] - Rdat[io];
				itq = Idat[i] - Idat[io];
				Rdat[io] = rtq * ru - itq * iu;
				Idat[io] = itq * ru + rtq * iu;
				Rdat[i] = rtp;
				Idat[i] = itp;
			}

			sr = ru;
			ru = ru * rw - iu * iw;
			iu = iu * rw + sr * iw;
		}

		ie >>= 1;
	}

	for (j = i = 1; i<N; i++)
	{
		if (i < j)
		{
			io = i - 1;
			in = j - 1;
			rtp = Rdat[in];
			itp = Idat[in];
			Rdat[in] = Rdat[io];
			Idat[in] = Idat[io];
			Rdat[io] = rtp;
			Idat[io] = itp;
		}

		k = nn;

		while (k < j)
		{
			j = j - k;
			k >>= 1;
		}

		j = j + k;
	}

	if (Ft_Flag == FT_DIRECT) return;

	rw = 1.0F / N;

	for (i = 0; i<N; i++)
	{
		Rdat[i] *= rw;
		Idat[i] *= rw;
	}

	return;
}


int getFileSize(FILE* inFile)
{
	int fileSize = 0;
	fseek(inFile, 0, SEEK_END);

	fileSize = ftell(inFile);

	fseek(inFile, 0, SEEK_SET);
	return fileSize;
}


void DrawAudioChart(uint8_t data[4096])
{
	 float value = 0;

	float Re[4096];
	float Im[4096];

	//waveformat->SampleRate * (waveformat->FileSize / waveformat->ByteRate)  - count = 4096

	 for (int i = 0; i < 4096; i++)
	 {
		int16_t c = (data[i] << 8) | data[i + 1];
		value = c * 30 / 32768; //for chart
		Re[i] = value;
		Im[i] = 0;

	 }

	 FFT(Re, Im, 4096, 12, FT_DIRECT);

	for(int i = 0; i < 4096; i++)
	{
		positionX++;

		if (positionX >= LCD_TFTWIDTH)
		{
			positionX = 0;

			for(int i = 0 ; i < LCD_TFTWIDTH; i++)
			{
				LCD_DrawLine(i, LCD_TFTHEIGHT + 1, i, LCD_TFTHEIGHT/2 , BLACK);
			}
		}

		LCD_DrawLine(positionX, LCD_TFTHEIGHT/1.3f, positionX, LCD_TFTHEIGHT/1.3f + Re[i], YELLOW);
	}
}


void DrawWavHeader(WAVE_FormatTypeDef *waveformat)
{
	LCD_SetTextColor(GREEN);

    char temp_string[100];
	sprintf(temp_string, "FileSize: %lu", (unsigned long)waveformat->FileSize);
	LCD_DisplayStringLine(0, 30, temp_string);

	sprintf(temp_string, "ByteRate: %lu", (unsigned long)waveformat->ByteRate);
	LCD_DisplayStringLine(0, 40, temp_string);

	sprintf(temp_string, "Duration: %lu", (unsigned long) (waveformat->FileSize / waveformat->ByteRate));
	LCD_DisplayStringLine(0, 50, temp_string);

	sprintf(temp_string, "BitPerSample: %lu", (unsigned long)waveformat->BitPerSample);
	LCD_DisplayStringLine(0, 60, temp_string);

	sprintf(temp_string, "SampleRate: %lu", (unsigned long)waveformat->SampleRate);
	LCD_DisplayStringLine(0, 70, temp_string);

	sprintf(temp_string, "NumberOfChannels: %lu", (unsigned long)waveformat->NbrChannels);
	LCD_DisplayStringLine(0, 80, temp_string);

	sprintf(temp_string, "ChunkID: %lu", (unsigned long)waveformat->ChunkID);
	LCD_DisplayStringLine(0, 90, temp_string);
}


void DrawClef(int positionX, int positionY, int number)
{
	if( number == 0)
	{
		LCD_DisplayStringLine(positionX, positionY     , "        ,     ");
		LCD_DisplayStringLine(positionX, positionY + 10, "        |\\    ");
		LCD_DisplayStringLine(positionX, positionY + 20, "        | |   ");
		LCD_DisplayStringLine(positionX, positionY + 30, "        |/    ");
		LCD_DisplayStringLine(positionX, positionY + 40, "      /|_    ");
		LCD_DisplayStringLine(positionX, positionY + 50, "     / |\\   ");
		LCD_DisplayStringLine(positionX, positionY + 60, "    | \\|\_|  ");
		LCD_DisplayStringLine(positionX, positionY + 70, "    \\\-|-/   ");
		LCD_DisplayStringLine(positionX, positionY + 80, "        |     ");
		LCD_DisplayStringLine(positionX, positionY + 90, "    \\-/     ");
	}
	else
	{
		LCD_DisplayStringLine(positionX, positionY     , "         .                __.          ");
		LCD_DisplayStringLine(positionX, positionY + 10, "        / \'-.          ,-','/         ");
		LCD_DisplayStringLine(positionX, positionY + 20, "       (   \ ,'--.__.--'.,' /          ");
		LCD_DisplayStringLine(positionX, positionY + 30, "       =---Y(_i.-'  |-.i_)-=           ");
		LCD_DisplayStringLine(positionX, positionY + 40, "      f ,  '..'/\\v/|/|/\  ,l          ");
		LCD_DisplayStringLine(positionX, positionY + 50, "      l//  ,'|/   V / /||  \\j         ");
		LCD_DisplayStringLine(positionX, positionY + 60, "       '--; / db     db|/---'          ");
		LCD_DisplayStringLine(positionX, positionY + 70, "          | \ YY   , YY//              ");
		LCD_DisplayStringLine(positionX, positionY + 80, "         '.\>_   (_),'' __             ");
		LCD_DisplayStringLine(positionX, positionY + 90, "  LOOK AT ME I'M AN ANIME GIRL !!!!!!!!");
	}

}


void FileRead(void)
{
	FRESULT res;
	uint32_t bytesread;
	uint8_t data[4096];

	LCD_SetTextColor(YELLOW);

	if(f_mount(&USBDISKFatFs, (TCHAR const*)USBHPath, 0) != FR_OK)
	{
		LCD_DisplayStringLine(0, 0, "USB_NOT_FOUND!!!");
	}
	else
	{
		LCD_DisplayStringLine(0, 0, "USB_FOUND");

		if(f_open(&MyFile, "sample3.wav", FA_READ) != FR_OK)
		{
			 LCD_DisplayStringLine(0, 10, "WAV_NOT_FOUND");
		}
		else
		{
			 res = f_read(&MyFile, data, sizeof(data), (void *)&bytesread);

			 if((bytesread == 0) || (res != FR_OK))
			 {
				 LCD_DisplayStringLine(0, 20, "BYTES_NOT_READ");
			 }
			 else
			 {
				 LCD_DisplayStringLine(0, 10, "WAV_FOUND");

				 data[bytesread] = 0;
				 waveformat = (WAVE_FormatTypeDef*)data;

				 DrawWavHeader(waveformat);
				 DrawClef(200, 10, 0);

				 DrawAudioChart(data);


//				 uint32_t BUFFER_SIZE = waveformat->SampleRate * (waveformat->FileSize / waveformat->ByteRate);
//				 uint8_t buffer[BUFFER_SIZE];
//
//				 res = f_read(&MyFile, buffer, sizeof(buffer), (void *)&bytesread);
//
//				 if((bytesread == 0) || (res != FR_OK))
//				 {
//					 Error_Handler();
//				 }
//				 else
//				 {
//					 buffer[bytesread] = 0;
//					 for (int i = 0; i < BUFFER_SIZE; i++)
//					 					{
//					 						int16_t c = (buffer[i] << 8) | buffer[i + 1];
//					 						DrawAudioChart(c*120/32768 , waveformat->ByteRate);
//					 					}
//				 }
//
//				 f_close(&MyFile);
			 }
		}
	}
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_HOST_Init();
  MX_FATFS_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  LCD_FillScreen(BLACK);

  while (1)
  {
  /* USER CODE END WHILE */
    MX_USB_HOST_Process();

  /* USER CODE BEGIN 3 */

    if(Appli_state==APPLICATION_START)
    {
        FileRead();
        Appli_state = APPLICATION_IDLE;
    }
    else if(Appli_state==APPLICATION_IDLE)
    {
    }
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD6 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing;

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
