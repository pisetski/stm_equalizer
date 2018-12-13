################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/fatfs.c \
../Src/fonts.c \
../Src/ili9341.c \
../Src/main.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/usb_host.c \
../Src/usbh_conf.c \
../Src/usbh_diskio.c 

OBJS += \
./Src/fatfs.o \
./Src/fonts.o \
./Src/ili9341.o \
./Src/main.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/usb_host.o \
./Src/usbh_conf.o \
./Src/usbh_diskio.o 

C_DEPS += \
./Src/fatfs.d \
./Src/fonts.d \
./Src/ili9341.d \
./Src/main.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/usb_host.d \
./Src/usbh_conf.d \
./Src/usbh_diskio.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F407xx -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Inc" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Drivers/STM32F4xx_HAL_Driver/Inc" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Middlewares/ST/STM32_USB_Host_Library/Core/Inc" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Middlewares/Third_Party/FatFs/src" -I"E:/PYMY/USB_LCD_Last/USB_LCD_Norm/USB_LCD/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


