/*
 * 
 * ICTester (c) woytekm 2023
 * Code based on usbd_ilib_cdc example from LPCOpen Library
 * 
 *
 */

#include "board.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"
#include "stopwatch.h"
#include "cli_io.h"
#include "embedded_cli.h"
#include "commands.h"
#include "SEGGER_RTT.h"
#include "sram_test.h"
#include "dram_test.h"
#include "rom_dumper.h"

#include "integer.h"
#include "diskio.h"
#include "ff.h"
#include "rtc.h"
#include "gpio_pin.h"


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

static struct embedded_cli cli;

USBD_HANDLE_T g_hUsb;
uint8_t g_rxBuff[256];

extern const  USBD_HW_API_T hw_api;
extern const  USBD_CORE_API_T core_api;
extern const  USBD_CDC_API_T cdc_api;
/* Since this example only uses CDC class link functions for that clas only */
static const  USBD_API_T g_usbApi = {
	&hw_api,
	&core_api,
	0,
	0,
	0,
	&cdc_api,
	0,
	0x02221101,
};

const  USBD_API_T *g_pUsbApi = &g_usbApi;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initialize pin and clocks for USB0/USB1 port */
static void usb_pin_clk_init(void)
{
	/* enable USB PLL and clocks */
	Chip_USB_Init();
	/* enable USB 1 port on the board */
	Board_USBD_Init(1);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from USB0
 * @return	Nothing
 */
void USB_IRQHandler(void)
{
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}



DWORD get_fattime (void)
{
        RTCTime rtc;

        rtc.RTC_Sec = 0;
        rtc.RTC_Min = 0;
        rtc.RTC_Hour = 0;
        rtc.RTC_Mday = 1;
        rtc.RTC_Wday = 0;
        rtc.RTC_Yday = 0;              /* current date 01/01/2010 */
        rtc.RTC_Mon = 1;
        rtc.RTC_Year = 2010;
        
        // Pack date and time into a DWORD variable
        return    ((DWORD)(rtc.RTC_Year - 1980) << 25)
                        | ((DWORD)rtc.RTC_Mon << 21)
                        | ((DWORD)rtc.RTC_Mday << 16)
                        | ((DWORD)rtc.RTC_Hour << 11)
                        | ((DWORD)rtc.RTC_Min << 5)
                        | ((DWORD)rtc.RTC_Sec >> 1);

 }



/**
 * @brief	main routine for example
 * @return	Function should not exit.
 */
int main(void)
{

        DSTATUS status;
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;

        VCOM_DATA_T *pVcom = &g_vCOM;

	/* Initialize board and chip */
	SystemCoreClockUpdate();
	Board_Init();

        //gpio_testfunc();

	StopWatch_Init();

        SEGGER_RTT_printf(0,"SystemCoreClock: %d \n",SystemCoreClock);

	/* enable clocks and pinmux */
	usb_pin_clk_init();

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE + 0x200;
	usb_param.max_num_ep = 3;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) &USB_DeviceDescriptor[0];
	desc.string_desc = (uint8_t *) &USB_StringDescriptor[0];
	/* Note, to pass USBCV test full-speed only devices should have both
	   descriptor arrays point to same location and device_qualifier set to 0.
	 */
	desc.high_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
	desc.full_speed_desc = (uint8_t *) &USB_FsConfigDescriptor[0];
	desc.device_qualifier = 0;

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {

		/* Init VCOM interface */
		ret = vcom_init(g_hUsb, &desc, &usb_param);
		if (ret == LPC_OK) {
			/*  enable USB interrupts */
			NVIC_EnableIRQ(USB_IRQn);
			/* now connect */
			USBD_API->hw->Connect(g_hUsb, 1);
		}
	}


        init_pin_array(); // initialize analyzer pin data structures
        init_pins();      // initialize all pins to default state
        init_timers();
        init_leds();
        init_sram_test(SRAM_DFT_ADDR_WIDTH,SRAM_DFT_DATA_WIDTH,SRAM_CE_LOW,SRAM_WE_LOW,SRAM_OE_LOW,SRAM_LOOPS,&G_sram_test_settings);
        init_dram_test(DRAM_DFT_ADDR_WIDTH,DRAM_DFT_DATA_WIDTH,DRAM_CE_LOW,DRAM_WE_LOW,DRAM_OE_LOW,DRAM_RAS_LOW,DRAM_CAS_LOW,DRAM_LOOPS,&G_dram_test_settings);
        init_rom_dumper(ROM_DFT_ADDR_WIDTH,ROM_DFT_DATA_WIDTH,ROM_CE_LOW,ROM_OE_LOW,"rom.bin",&G_rom_dumper_settings);

        bool done = false;

        LPC17xx_SPI_Init();

        status = disk_initialize(0);
        SEGGER_RTT_printf(0,"disk_initialize(): %d\n",status);

        uint8_t i,bytes;

        embedded_cli_init(&cli, "ICTester#", vcom_putch, stdout);
        embedded_cli_prompt(&cli);

        set_pin_high_simple(PIN_YELLOW_LED);  // just tur it on as a "system ready" indicator for now

        while (!done) {

           delayMS(5);

           if(pVcom->rx_count == 0)
              __WFI();
           bytes = vcom_bread(&g_hUsb,&g_rxBuff[0], 256);

           set_pin_high_simple(PIN_GREEN_LED); 

           if(bytes > 0)
            {
             for(i=0; i<bytes; i++)
              if (embedded_cli_insert_char(&cli, g_rxBuff[i])) {
               int cli_argc;
               char **cli_argv;
               cli_argc = embedded_cli_argc(&cli, &cli_argv);
               //for (int i = 0; i < cli_argc; i++) {
               //   SEGGER_RTT_printf(0,"arg %d/%d: '%s'\n", i, cli_argc, cli_argv[i]);
               //}
             dispatch_cli_command(cli_argc, cli_argv);
             embedded_cli_prompt(&cli);
            }
           }

           set_pin_low_simple(PIN_GREEN_LED);

         }

      }
