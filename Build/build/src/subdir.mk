################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../code/src/cdc_desc.c \
../code/src/cdc_main.c \
../code/src/cdc_vcom.c \
../code/src/cr_startup_lpc175x_6x.c \
../code/src/embedded_cli.c \
../code/src/cli_io.c \
../code/src/commands_main.c \
../code/src/commands_direction.c \
../code/src/commands_level.c \
../code/src/commands_io.c \
../code/src/commands_clock.c \
../code/src/commands_test.c \
../code/src/commands_sram_test.c \
../code/src/commands_dram_test.c \
../code/src/commands_rom_dumper.c \
../code/src/commands_test_criteria.c \
../code/src/sram_test.c \
../code/src/dram_test.c \
../code/src/rom_dumper.c \
../code/src/state_ops.c \
../code/src/gpio.c \
../code/src/timers.c \
../code/src/utils.c \
../code/src/test.c \
../code/src/ff.c \
../code/src/sdcard.c \
../code/src/sysinit.c \
../code/src/ccsbcs.c 


OBJS += \
./build/src/cdc_desc.o \
./build/src/cdc_main.o \
./build/src/cdc_vcom.o \
./build/src/cr_startup_lpc175x_6x.o \
./build/src/sysinit.o \
./build/src/embedded_cli.o \
./build/src/commands_main.o \
./build/src/commands_direction.o \
./build/src/commands_level.o \
./build/src/commands_io.o \
./build/src/commands_clock.o \
./build/src/commands_test.o \
./build/src/commands_sram_test.o \
./build/src/commands_dram_test.o \
./build/src/commands_rom_dumper.o \
./build/src/commands_test_criteria.o \
./build/src/sram_test.o \
./build/src/dram_test.o \
./build/src/rom_dumper.o \
./build/src/state_ops.o \
./build/src/gpio.o \
./build/src/timers.o \
./build/src/utils.o \
./build/src/test.o \
./build/src/ff.o \
./build/src/sdcard.o \
./build/src/cli_io.o \
./build/src/ccsbcs.o 

# Each subdirectory must supply rules for building sources it contributes
build/src/%.o: ../code/src/%.c build/src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
#	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M3 -I "../RTT/inc" -I"../lpc_chip_175x_6x/inc" -I"../lpc_board_nxp_lpcxpresso_1769/inc" -I"../code/inc" -I"../lpc_chip_175x_6x/inc/usbd" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -Wno-implicit-function-declaration -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_LPCOPEN -DCORE_M3 -I "../RTT/inc" -I"../lpc_chip_175x_6x/inc" -I"../lpc_board_nxp_lpcxpresso_1769/inc" -I"../code/inc" -I"../lpc_chip_175x_6x/inc/usbd" -O3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -Wno-implicit-function-declaration -Wno-array-bounds -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-build-2f-src

clean-build-2f-src:
	-$(RM) ./build/src/cdc_desc.d ./build/src/cdc_desc.o ./build/src/cdc_main.d ./build/src/cdc_main.o ./build/src/cdc_vcom.d ./build/src/cdc_vcom.o ./build/src/cr_startup_lpc175x_6x.d ./build/src/cr_startup_lpc175x_6x.o ./build/src/sysinit.d ./build/src/sysinit.o ./build/src/embedded_cli.o ./build/src/cli_io.o ./build/src/commands_main.o ./build/src/commands_io.o ./build/src/commands_direction.o ./build/src/commands_level.o ./build/src/gpio.o ./build/src/timers.o ./build/src/commands_test.o ./build/src/test.o ./build/src/commands_test_criteria.o ./build/src/state_ops.o ./build/src/ff.o ./build/src/ccsbcs.o ./build/src/sdcard.o ./build/src/sram_test.o ./build/src/commands_sram_test.o ./build/src/rom_dumper.o ./build/src/commands_rom_dumper.o ./build/src/dram_test.o ./build/src/commands_dram_test.o

.PHONY: clean-build-2f-src

