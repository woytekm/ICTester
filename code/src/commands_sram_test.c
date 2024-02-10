#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <cr_section_macros.h>

#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "state_ops.h"
#include "utils.h"
#include "sram_test.h"

bool lh_to_bool(char c)
 {

   if(c == 'L')
    return false;
   else if(c == 'H')
    return true;

   return false;
 }

void cli_set_sram_test(int argc, char **argv)
{

    if (argc != 6) {
        vcom_printf("usage: set sram-test <addr_bits> <data-bits> <ce-active: L|H> <we-active: L|H> <oe-active: L|H> <loops>\r\n");
        return;
    }

    bool ce_low,oe_low,we_low;
    uint8_t addr_bits = atoi(argv[0]);
    uint8_t data_bits = atoi(argv[1]);
    char *ce_active = argv[2];
    char *oe_active = argv[3];
    char *we_active = argv[4];
    uint16_t loops = atoi(argv[5]);
    

    if(addr_bits > 16)
     { vcom_printf("ERROR: addr_bits too long (max 16 bits)\r\n");
        return; }

    if(data_bits > 8)
     { vcom_printf("ERROR: data_bits too long (max 8 bits)\r\n");
        return; }

    if( ((ce_active[0] != 'H') && (ce_active[0] != 'L')) ||
        ((oe_active[0] != 'H') && (oe_active[0] != 'L')) ||
        ((we_active[0] != 'H') && (we_active[0] != 'L')) )
     { vcom_printf("ERROR: incorrect CE/WE/OE flags: %c %c %c (should be L or H).\r\n",ce_active[0],oe_active[0],we_active[0]);
        return; }

    if(lh_to_bool(ce_active[0])) ce_low = false; else ce_low = true;
    if(lh_to_bool(oe_active[0])) oe_low = false; else oe_low = true;
    if(lh_to_bool(we_active[0])) we_low = false; else we_low = true;

    init_sram_test(addr_bits, data_bits, ce_low, we_low, oe_low, loops, &G_sram_test_settings);
    

}


void cli_show_sram_test(int argc, char **argv)
{

 vcom_printf("Current SRAM test parameters:\r\n");
 vcom_printf("  Address bus width: %d bit \r\n",G_sram_test_settings.address_width);
 vcom_printf("  Data bus width: %d bit \r\n",G_sram_test_settings.data_width);
 vcom_printf("  CE active low: %d \r\n",G_sram_test_settings.ce_active_low);
 vcom_printf("  OE active low: %d \r\n",G_sram_test_settings.we_active_low);
 vcom_printf("  WE active low: %d \r\n",G_sram_test_settings.oe_active_low);
 vcom_printf("  Test loops: %d \r\n",G_sram_test_settings.loops);

 vcom_printf("\n\r--------- pin cfg --------------------\r\n");
 show_test_aliases(G_sram_test_settings.pin_aliases);

}


#define ADDR_LOW_BANK 1
#define ADDR_HIGH_BANK 2
#define DATA_BANK 3
#define CTRL_BANK 4



void cli_run_sram_test(int argc, char **argv)
{

  uint16_t i,j,addr_max = pow(2,G_sram_test_settings.address_width);
  uint8_t data_bitmask = (pow(2,G_sram_test_settings.data_width) - 1);
  uint8_t output,j_low,j_high, data_pattern;
  uint16_t errors = 0;

  vcom_printf("addr width: %d bit, addr_max: 0x%X\r\n",G_sram_test_settings.address_width,(addr_max-1));
  vcom_printf("data width: %d bit, data_max: 0x%X\r\n",G_sram_test_settings.data_width,data_bitmask);

  set_direction_bank(ADDR_LOW_BANK, PIN_OUTPUT);
  set_direction_bank(ADDR_HIGH_BANK, PIN_OUTPUT);
  set_direction_bank(DATA_BANK, PIN_OUTPUT);
  set_direction_bank(CTRL_BANK, PIN_OUTPUT);


   set_level_bank(ADDR_LOW_BANK,0x0);
   set_level_bank(ADDR_HIGH_BANK,0x0);
   set_level_bank(DATA_BANK,0x0);
   set_level_bank(CTRL_BANK,0xFF);

   set_dut_power("enable");

   set_io_bank(1,"enable");
   set_io_bank(2,"enable");
   set_io_bank(3,"enable");
   set_io_bank(4,"enable");


  for(i = 0; i < G_sram_test_settings.loops; i++)
   {
      set_direction_bank(DATA_BANK, PIN_OUTPUT);

      set_level_bank(ADDR_LOW_BANK,0x0);
      set_level_bank(ADDR_HIGH_BANK,0x0);
      set_level_bank(DATA_BANK,0x0);
      set_level_bank(CTRL_BANK,0xFF);

      data_pattern = 0b01010101;

      set_level_bank(CTRL_BANK,0b11111110);  // CE = LOW

      vcom_printf("w1");
      for(j = 0; j < addr_max; j++)
       {
         j_low = (uint8_t)j;
         j_high = j >> 8;
         set_level_bank(ADDR_LOW_BANK,j_low);
         set_level_bank(ADDR_HIGH_BANK,j_high);
         set_level_bank(DATA_BANK,data_pattern);
         set_level_bank(CTRL_BANK,0b11111100);  // write
         set_level_bank(CTRL_BANK,0b11111110);  // end write
      
         data_pattern = ~data_pattern; // alternate pattern
       }

      vcom_printf(" r1");
      data_pattern = 0b01010101;
      set_direction_bank(DATA_BANK, PIN_INPUT);

      for(j = 0; j < addr_max; j++)
       {
         output = 0x0;
         j_low = (uint8_t)j;
         j_high = j >> 8;
         set_level_bank(ADDR_LOW_BANK,j_low);
         set_level_bank(ADDR_HIGH_BANK,j_high);
         set_level_bank(CTRL_BANK,0b11111010);  // latch bits on D0-D7
         output = read_level_bank(DATA_BANK);   // read
         set_level_bank(CTRL_BANK,0b11111110);  // end read
     
         if((output&data_bitmask) != (data_pattern&data_bitmask))
          {
           vcom_printf("!");
           errors++;
          }

         data_pattern = ~data_pattern;
       }

      vcom_printf(" w2");
      data_pattern = 0b10101010;
      set_direction_bank(DATA_BANK, PIN_OUTPUT);

      for(j = 0; j < addr_max; j++)
       {
         j_low = (uint8_t)j;
         j_high = j >> 8;
         set_level_bank(ADDR_LOW_BANK,j_low);
         set_level_bank(ADDR_HIGH_BANK,j_high);
         set_level_bank(DATA_BANK,data_pattern);
         set_level_bank(CTRL_BANK,0b11111100);  // write
         set_level_bank(CTRL_BANK,0b11111110);  // end write

         data_pattern = ~data_pattern; // alternate pattern
       }

      vcom_printf(" r2");
      data_pattern = 0b10101010;
      set_direction_bank(DATA_BANK, PIN_INPUT);

      for(j = 0; j < addr_max; j++)
       {
         output = 0x0;
         j_low = (uint8_t)j;
         j_high = j >> 8;
         set_level_bank(ADDR_LOW_BANK,j_low);
         set_level_bank(ADDR_HIGH_BANK,j_high);
         set_level_bank(CTRL_BANK,0b11111010);  // latch bits on D0-D7
         output = read_level_bank(DATA_BANK);   // read
         set_level_bank(CTRL_BANK,0b11111110);  // end read

         if((output&data_bitmask) != (data_pattern&data_bitmask))
          {
           vcom_printf("!");
           errors++;
          }

         data_pattern = ~data_pattern;
       }


    if(errors)
     {
      vcom_printf("!!! %d errors found on pass %d\r\n",errors,i);
      break;
     }
   
    vcom_printf(" %d ok\r\n",i);
   
   }


   set_io_bank(1,"disable");
   set_io_bank(2,"disable");
   set_io_bank(3,"disable");
   set_io_bank(4,"disable");

   set_dut_power("disable");

}


