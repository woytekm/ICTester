#include "sram_test.h"
#include "gpio_pin.h"

sram_test_settings_t G_sram_test_settings;

void init_sram_test(uint8_t addr_width, uint8_t data_width, bool ce, bool we, bool oe, uint16_t loops, sram_test_settings_t *sram_test_settings)
{

  char alias[5];
  uint8_t i;

  sram_test_settings->address_width = addr_width;
  sram_test_settings->data_width = data_width;
  sram_test_settings->ce_active_low = ce;
  sram_test_settings->we_active_low = we;
  sram_test_settings->oe_active_low = oe;
  sram_test_settings->loops = loops;

  // review:

 for(i = 0; i < PIN_COUNT; i++)
   sram_test_settings->pin_aliases[i][0]=0x0;
 
 if(sram_test_settings->address_width < 9)
 {
   for(i = 0; i < sram_test_settings->address_width; i++)
     { 
      sprintf(alias,"A%d",i);
      strncpy(sram_test_settings->pin_aliases[i+10], alias, 4);
     }
 }
 else
 {

   for(i = 0; i < 8; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(sram_test_settings->pin_aliases[i+10], alias, 4);
     }

   for(i = 8; i < sram_test_settings->address_width; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(sram_test_settings->pin_aliases[(i-8)+20], alias, 4);
     }

 }

 for(i = 0; i < sram_test_settings->data_width; i++)
  {
   sprintf(alias,"D%d",i);
   strncpy(sram_test_settings->pin_aliases[i+30], alias, 4);
  }

 strncpy(sram_test_settings->pin_aliases[40], "CE", 4);
 strncpy(sram_test_settings->pin_aliases[41], "WE", 4);
 strncpy(sram_test_settings->pin_aliases[42], "OE", 4);

}

