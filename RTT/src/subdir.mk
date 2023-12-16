################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./src/SEGGER_RTT.c \
./src/SEGGER_RTT_printf.c

C_DEPS += \
./src/SEGGER_RTT.d \
./src/SEGGER_RTT_printf.d 

OBJS += \
./src/SEGGER_RTT.o \
./src/SEGGER_RTT_printf.o 

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ./src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M3  -I"./inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/SEGGER_RTT.d ./src/SEGGER_RTT.o ./src/SEGGER_RTT_printf.d ./src/SEGGER_RTT_printf.o 

.PHONY: clean-src

