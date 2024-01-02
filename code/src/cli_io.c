#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "cli_io.h"
#include "cdc_vcom.h"
#include "SEGGER_RTT.h"
#include "timers.h"



void vcom_putch(void *data, unsigned char ch, bool is_last)
{
  delayMS(1);
  vcom_write(&ch, 1);
}

void vcom_message(char *message)
 {
   uint8_t i;

   for(i=0; i<strlen(message); i++)
     vcom_putch(NULL, message[i], false);
 }

