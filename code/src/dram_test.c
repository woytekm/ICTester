#include "dram_test.h"
#include "gpio_pin.h"
#include "commands.h"
#include "cli_io.h"
#include "utils.h"

dram_test_settings_t G_dram_test_settings;

void init_dram_test(uint8_t addr_width, uint8_t data_width, bool ce, bool we, bool oe, bool ras, bool cas, uint16_t loops, dram_test_settings_t *dram_test_settings)
{

  char alias[5];
  uint8_t i;

  dram_test_settings->address_width = addr_width;
  dram_test_settings->data_width = data_width;
  dram_test_settings->ce_active_low = ce;
  dram_test_settings->we_active_low = we;
  dram_test_settings->oe_active_low = oe;
  dram_test_settings->ras_active_low = ras;
  dram_test_settings->cas_active_low = cas;
  dram_test_settings->loops = loops;

  // review:

 for(i = 0; i < PIN_COUNT; i++)
   dram_test_settings->pin_aliases[i][0]=0x0;
 
 if(dram_test_settings->address_width < 9)
 {
   for(i = 0; i < dram_test_settings->address_width; i++)
     { 
      sprintf(alias,"A%d",i);
      strncpy(dram_test_settings->pin_aliases[i+10], alias, 4);
     }
 }
 else
 {

   for(i = 0; i < 8; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(dram_test_settings->pin_aliases[i+10], alias, 4);
     }

   for(i = 8; i < dram_test_settings->address_width; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(dram_test_settings->pin_aliases[(i-8)+20], alias, 4);
     }

 }

 for(i = 0; i < dram_test_settings->data_width; i++)
  {
   sprintf(alias,"D%d",i);
   strncpy(dram_test_settings->pin_aliases[i+30], alias, 4);
  }

 strncpy(dram_test_settings->pin_aliases[40], "CE", 4);
 strncpy(dram_test_settings->pin_aliases[41], "WE", 4);
 strncpy(dram_test_settings->pin_aliases[42], "OE", 4);
 strncpy(dram_test_settings->pin_aliases[43], "CAS", 4);
 strncpy(dram_test_settings->pin_aliases[45], "RAS", 4); // there is some problem with pin 44 on my prototype

}

//
// DRAM test procedures:
// These functions do not have any timing. I assume that this code is slow enough without any wait states,
// to comply with minimum timing requirements of most DRAM IC's from 80's and 90's (in 100 - 200 nsec range). 
// This code will execute on the C level with usec interval, not nsec, so it's like 10 times slower than access time of most DRAM chips.
//


void test_dram_early_write(uint16_t addr_max, uint8_t pattern, bool alternate_pattern)
 {


  uint16_t caddr,raddr;
  uint8_t data_pattern = pattern,caddr_low,caddr_high,raddr_low,raddr_high;

  set_direction_bank(DR_DATA_BANK, PIN_OUTPUT);

  for(raddr = 0; raddr < addr_max; raddr++)
   {
      raddr_low = (uint8_t)raddr;
      raddr_high = raddr >> 8;

      for(caddr = 0; caddr < addr_max; caddr++)
       {
         caddr_low = (uint8_t)caddr;
         caddr_high = caddr >> 8;
  
         set_level_bank(DR_CTRL_BANK,0b11111110);
         set_level_bank(DR_ADDR_LOW_BANK,raddr_low);   // enter row address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,raddr_high); // enter row address on address lines
         set_level_bank(DR_DATA_BANK,data_pattern);    // enter data on data lines
         set_level_bank(DR_CTRL_BANK,0b11001110);   // CE=L, WE=H, OE=H, CAS=H, RAS=L  - latch row address
         set_level_bank(DR_ADDR_LOW_BANK,caddr_low);   // enter column address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,caddr_high); // enter column address on address lines
         set_level_bank(DR_CTRL_BANK,0b11001100);   // CE=L, WE=L, OE=H, CAS=H, RAS=L  - activate WE (active low)
         set_level_bank(DR_CTRL_BANK,0b11000100);   // CE=L, WE=H, OE=H, CAS=L, RAS=L  - latch column address
         set_level_bank(DR_CTRL_BANK,0b11110100);   // RAS goes H 
         set_level_bank(DR_CTRL_BANK,0b11111110);   // CAS goes H - end of early write cycle

         if(alternate_pattern)
           data_pattern = ~data_pattern; // alternate pattern
       }
   }

 }


void test_dram_oe_write(uint16_t addr_max, uint8_t pattern, bool alternate_pattern)
 {


  uint16_t caddr,raddr;
  uint8_t data_pattern = pattern,caddr_low,caddr_high,raddr_low,raddr_high;

  set_direction_bank(DR_DATA_BANK, PIN_OUTPUT);

  for(raddr = 0; raddr < addr_max; raddr++)
   {
      raddr_low = (uint8_t)raddr;
      raddr_high = raddr >> 8;

      for(caddr = 0; caddr < addr_max; caddr++)
       {
         caddr_low = (uint8_t)caddr;
         caddr_high = caddr >> 8;

         set_level_bank(DR_CTRL_BANK,0b11111110);
         set_level_bank(DR_ADDR_LOW_BANK,raddr_low);   // enter row address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,raddr_high); // enter row address on address lines
         set_level_bank(DR_DATA_BANK,data_pattern);    // enter data on data lines
         set_level_bank(DR_CTRL_BANK,0b11001110);   // CE=L, WE=H, OE=H, CAS=H, RAS=L  - latch row address
         set_level_bank(DR_ADDR_LOW_BANK,caddr_low);   // enter column address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,caddr_high); // enter column address on address lines
         set_level_bank(DR_CTRL_BANK,0b11000110);   // CE=L, WE=H, OE=H, CAS=L, RAS=L  - latch column address
         set_level_bank(DR_CTRL_BANK,0b11001110);   // CE=L, WE=L, OE=H, CAS=H, RAS=L  - keep OE high 
         set_level_bank(DR_CTRL_BANK,0b11001100);   // CE=L, WE=L, OE=H, CAS=H, RAS=L  - activate WE (active low)
         set_level_bank(DR_CTRL_BANK,0b11110100);   // RAS goes H
         set_level_bank(DR_CTRL_BANK,0b11111110);   // CAS goes H - end of OE write cycle

         if(alternate_pattern)
           data_pattern = ~data_pattern; // alternate pattern
       }
   }

 }




uint16_t test_dram_read(uint16_t addr_max, uint8_t data_bitmask, uint8_t pattern, bool alternate_pattern)
 {

  uint16_t caddr, raddr, errors = 0;
  uint8_t data_pattern = pattern,caddr_low,caddr_high,raddr_low,raddr_high,input;

  set_direction_bank(DR_DATA_BANK, PIN_INPUT);

  for(raddr = 0; raddr < addr_max; raddr++)
   {
      raddr_low = (uint8_t)raddr;
      raddr_high = raddr >> 8;

      for(caddr = 0; caddr < addr_max; caddr++)
       {
         caddr_low = (uint8_t)caddr;
         caddr_high = caddr >> 8;

         set_level_bank(DR_CTRL_BANK,0b11111110);
         set_level_bank(DR_ADDR_LOW_BANK,raddr_low);   // enter row address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,raddr_high); // enter row address on address lines
         set_level_bank(DR_CTRL_BANK,0b11001110);   // CE=L, WE=H, OE=H, CAS=H, RAS=L  - latch row address
         set_level_bank(DR_ADDR_LOW_BANK,caddr_low);   // enter column address on address lines
         set_level_bank(DR_ADDR_HIGH_BANK,caddr_high); // enter column address on address lines
         set_level_bank(DR_CTRL_BANK,0b11000110);   // CE=L, WE=H, OE=H, CAS=L, RAS=L  - latch column address
         set_level_bank(DR_CTRL_BANK,0b11000010);   // CE=L, WE=H, OE=L, CAS=L, RAS=L  - activate OE (active low)
         input = read_level_bank(DR_DATA_BANK);     // read in the data
         set_level_bank(DR_CTRL_BANK,0b11110110);   // CAS goes H, OE goes H
         set_level_bank(DR_CTRL_BANK,0b11111110);   // RAS goes H - end of read cycle

         if((input&data_bitmask) != (data_pattern&data_bitmask))
          {
           errors++;
          }

         if(alternate_pattern)
           data_pattern = ~data_pattern; // alternate pattern
       }
   }
 
  return errors;

 }




