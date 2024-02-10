################################################################################
# Automatically-generated file. Do not edit!
################################################################################

RM := rm -rf

# All of the sources participating in the build are defined here
include sources.mk
include build/src/subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
include $(C_DEPS)
endif
endif

OPTIONAL_TOOL_DEPS := 

# All Target
all:
	@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: ictester.axf

# Tool invocations
ictester.axf: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L "../RTT" -L"../SPI_SD" -L"../lpc_chip_175x_6x/Debug" -L"../lpc_board_nxp_lpcxpresso_1769/Debug" -L"../lpc_chip_175x_6x/libs" -Xlinker -Map="ictester.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T ictester.ld -o "ictester.axf" $(OBJS) $(USER_OBJS) $(LIBS) -llpc_board_nxp_lpcxpresso_1769 -llpc_chip_175x_6x -lusbd_175x_6x_lib -lRTT -lSPI-SD
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	$(RM) ictester.axf ictester.hex
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "ictester.axf"; arm-none-eabi-objcopy -v -O ihex "ictester.axf" "ictester.hex" ; 
	-@echo ' '

.PHONY: all clean dependents main-build post-build

