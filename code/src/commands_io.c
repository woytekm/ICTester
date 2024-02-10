#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"


void cli_set_io(int argc, char **argv) {

  if(argc == 0)
    {
     vcom_printf( "usage: set io bank <bank id> <enable|disable>, set io all <enable|disable>\n\r");
     return;
    }

  char *subcommand_set_io = argv[0];
 
  if (strcmp(subcommand_set_io, "bank") == 0) {
     if(argc != 3)
      {
       vcom_printf( "usage: set io bank <bank id> <enable|disable>, set io all <enable|disable>\n\r");
       return;
      }
     uint8_t bank_id = atoi(argv[1]);   
     if((bank_id < 1) || (bank_id > 4))
       {
         vcom_printf( "ERROR: invalid bank ID (should be 1|2|3|4)\n\r");
         return;
       }
     if(strcmp(argv[2], "enable") == 0)
      set_pin_low_simple(G_pin_array[bank_id].gpio_id);
     else if(strcmp(argv[2], "disable") == 0)  
      set_pin_high_simple(G_pin_array[bank_id].gpio_id); 
     else
      vcom_printf( "ERROR: invalid operation (should be enable|disable)\n\r"); 
     
   } else if (strcmp(subcommand_set_io, "all") == 0) {
     if(argc != 2)
      {
       vcom_printf( "usage: set io bank <bank id> <enable|disable>, set io all <enable|disable>\n\r");
       return;
      }
     if(strcmp(argv[1], "enable") == 0)
      {
        set_pin_low_simple(G_pin_array[1].gpio_id);
        set_pin_low_simple(G_pin_array[2].gpio_id);
        set_pin_low_simple(G_pin_array[3].gpio_id);
        set_pin_low_simple(G_pin_array[4].gpio_id);
      }
     else if(strcmp(argv[1], "disable") == 0)
      {
        set_pin_high_simple(G_pin_array[1].gpio_id);
        set_pin_high_simple(G_pin_array[2].gpio_id);
        set_pin_high_simple(G_pin_array[3].gpio_id);
        set_pin_high_simple(G_pin_array[4].gpio_id);
      }
     else
      vcom_printf( "ERROR: invalid operation (should be enable|disable)\n\r");

   }

}


void set_io_bank(uint8_t bank_id,char *state) {

     if(strcmp(state, "enable") == 0)
      set_pin_low_simple(G_pin_array[bank_id].gpio_id);
     else if(strcmp(state, "disable") == 0)
      set_pin_high_simple(G_pin_array[bank_id].gpio_id);
}



void cli_show_io(void)
 {
   uint8_t i,pin_level;
   for(i=1;i<5;i++)
    {
      pin_level = get_pin(G_pin_array[i].gpio_id);
      if(pin_level == 1)
        vcom_printf("Bank %d I/O status: disabled\r\n",i);
      else
        vcom_printf("Bank %d I/O status: enabled\r\n",i);
    }
 }


void cli_set_dut_power(int argc, char **argv) {

  if(argc == 0)
    {
     vcom_printf( "usage: set dut-power <enable|disable>, power DUT up/down. \n\r");
     return;
    }

  char *subcommand_set_dut = argv[0];

  if (strcmp(subcommand_set_dut, "enable") == 0) {
        set_pin_high_simple(PIN_DUT_POWER);
        vcom_printf( "DUT power enabled.\n\r");
      }
  else if(strcmp(subcommand_set_dut, "disable") == 0)
      {
        set_pin_low_simple(PIN_DUT_POWER);
        vcom_printf( "DUT power disabled.\n\r");
      }
  else
      vcom_printf( "ERROR: invalid parameter. Should be: <enable|disable>\n\r");

}

void set_dut_power(char *state) {

  if (strcmp(state, "enable") == 0) {
        set_pin_high_simple(PIN_DUT_POWER);
      }
  else if(strcmp(state, "disable") == 0)
      {
        set_pin_low_simple(PIN_DUT_POWER);
      }
}

