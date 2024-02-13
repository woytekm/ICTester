#include "test.h"
#include "rom_dumper.h"
#include "gpio_pin.h"

rom_dumper_settings_t G_rom_dumper_settings;

void init_rom_dumper(uint8_t addr_width, uint8_t data_width, bool ce, bool oe, char *filename, rom_dumper_settings_t *rom_dumper_settings)
{

  char alias[5];
  uint8_t i;

  rom_dumper_settings->address_width = addr_width;
  rom_dumper_settings->data_width = data_width;
  rom_dumper_settings->ce_active_low = ce;
  rom_dumper_settings->oe_active_low = oe;
  strcpy(rom_dumper_settings->filename,filename);

  // review:

 for(i = 0; i < PIN_COUNT; i++)
   rom_dumper_settings->pin_aliases[i][0]=0x0;
 
 if(rom_dumper_settings->address_width < 9)
  {
   for(i = 0; i < rom_dumper_settings->address_width; i++)
     { 
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[i+10], alias, 4);
     }
  }
 else if(rom_dumper_settings->address_width < 17)
  {

   for(i = 0; i < 8; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[i+10], alias, 4);
     }

   for(i = 8; i < rom_dumper_settings->address_width; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[(i-8)+20], alias, 4);
     }
  }
 else if(rom_dumper_settings->address_width == 17)
  {
   for(i = 0; i < 8; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[i+10], alias, 4);
     }
   for(i = 8; i <16 ; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[(i-8)+20], alias, 4);
     }
   strncpy(rom_dumper_settings->pin_aliases[47], "A16", 4);
  }
 else if(rom_dumper_settings->address_width == 18)
  {
   for(i = 0; i < 8; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[i+10], alias, 4);
     }
   for(i = 8; i < 16; i++)
     {
      sprintf(alias,"A%d",i);
      strncpy(rom_dumper_settings->pin_aliases[(i-8)+20], alias, 4);
     }
   strncpy(rom_dumper_settings->pin_aliases[47], "A16", 4);
   strncpy(rom_dumper_settings->pin_aliases[46], "A17", 4);
  }

 for(i = 0; i < rom_dumper_settings->data_width; i++)
  {
   sprintf(alias,"D%d",i);
   strncpy(rom_dumper_settings->pin_aliases[i+30], alias, 4);
  }

 strncpy(rom_dumper_settings->pin_aliases[40], "CE", 4);
 strncpy(rom_dumper_settings->pin_aliases[41], "WE", 4);
 strncpy(rom_dumper_settings->pin_aliases[42], "OE", 4);

}

