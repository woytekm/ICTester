################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./src/lpc17xx_sd.c \
./src/lpc17xx_spi.c

C_DEPS += \
./src/lpc17xx_sd.d \
./src/lpc17xx_spi.d 

OBJS += \
./src/lpc17xx_sd.o \
./src/lpc17xx_spi.o 

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ./src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M3  -I "../lpc_chip_175x_6x/inc/" -I "../code/inc" -I"./inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/lpc17xx_sd.d ./src/lpc17xx_sd.o ./src/lpc17xx_spi.d ./src/lpc17xx_spi.o 

.PHONY: clean-src

