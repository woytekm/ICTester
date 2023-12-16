#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "cli_io.h"
#include "cdc_vcom.h"
#include "SEGGER_RTT.h"



void vcom_putch(void *data, unsigned char ch, bool is_last)
{
  uint16_t i;

  for(i = 0; i < 16384; i++)
    {}

  vcom_write(&ch, 1);
}

void vcom_message(char *message)
 {
   char out_msg[255];
   sprintf(out_msg,"info: %s\r",message);
   vcom_write(out_msg, strlen(out_msg));
 }

