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


/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

static uint8_t g_rxBuff[256];

static struct embedded_cli cli;

static USBD_HANDLE_T g_hUsb;
//static uint8_t g_rxBuff[256];

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

/**
 * @brief	main routine for example
 * @return	Function should not exit.
 */
int main(void)
{
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;

        VCOM_DATA_T *pVcom = &g_vCOM;

	/* Initialize board and chip */
	SystemCoreClockUpdate();
	Board_Init();

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

        bool done = false;

        embedded_cli_init(&cli, "ICTester#", vcom_putch, stdout);
        embedded_cli_prompt(&cli);

        uint8_t i,bytes;

        while (!done) {

           if(pVcom->rx_count == 0)
              __WFI();
           bytes = vcom_bread(&g_hUsb,&g_rxBuff[0], 256);

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
         }

      }
