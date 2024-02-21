#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <cr_section_macros.h>

#include "ff.h"
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "state_ops.h"
#include "utils.h"
#include "test.h"
#include "rom_dumper.h"


void cli_set_rom_dumper(int argc, char **argv)
{

    if (argc != 5) {
        vcom_printf("usage: set rom-dumper <addr_bits> <data-bits> <ce-active: L|H> <oe-active: L|H> <filename>\r\n");
        return;
    }

    bool ce,oe;
    uint8_t addr_bits = atoi(argv[0]);
    uint8_t data_bits = atoi(argv[1]);
    char *ce_active = argv[2];
    char *oe_active = argv[3];
    char *filename = argv[4];

    if(addr_bits > 20)
     { vcom_printf("ERROR: addr_bits too long (max 20 bits)\r\n");
        return; }

    if(data_bits > 8)
     { vcom_printf("ERROR: data_bits too long (max 8 bits)\r\n");
        return; }

    if( ((ce_active[0] != 'H') && (ce_active[0] != 'L')) ||
        ((oe_active[0] != 'H') && (oe_active[0] != 'L')) )
     { vcom_printf("ERROR: incorrect CE/OE flags: %c %c %c (should be L or H).\r\n",ce_active[0],oe_active[0]);
        return; }

    if(lh_to_int(ce_active[0])) ce = 1; else ce = 0;
    if(lh_to_int(oe_active[0])) oe = 1; else oe = 0;

    init_rom_dumper(addr_bits, data_bits, ce, oe, filename, &G_rom_dumper_settings);
    

}


void cli_show_rom_dumper(int argc, char **argv)
{

 vcom_cprintf("\e[0;36mCurrent ROM dumper parameters:\e[0m\r\n","Current ROM dumper parameters:\r\n");
 vcom_printf("  Address bus width: %d bit \r\n",G_rom_dumper_settings.address_width);
 vcom_printf("  Data bus width: %d bit \r\n",G_rom_dumper_settings.data_width);
 vcom_printf("  CE active: %c \r\n",sh_lvl_set(G_rom_dumper_settings.ce_active));
 vcom_printf("  OE active: %c \r\n",sh_lvl_set(G_rom_dumper_settings.oe_active));
 vcom_printf("  File to save data to: %s \r\n",G_rom_dumper_settings.filename);

 vcom_printf("\n\r--------- pin cfg --------------------\r\n");
 show_test_aliases(G_rom_dumper_settings.pin_aliases);

}


#define ADDR_LOW_BANK 1
#define ADDR_HIGH_BANK 2
#define DATA_BANK 3
#define CTRL_BANK 4



void cli_run_rom_dumper(int argc, char **argv)
{

  uint32_t j,addr_max = pow(2,G_rom_dumper_settings.address_width);
  uint8_t data_bitmask = (pow(2,G_rom_dumper_settings.data_width) - 1);
  uint8_t output,j_low,j_high1,j_high2;
  uint8_t byte_ctr = 0,ctlbank;

  uint8_t read_buffer[128];

  FIL file;        /* File object */
  FRESULT fr;     /* FatFs return code */
  FATFS drive;
  UINT written;

  f_mount(0,&drive);

  fr = f_unlink(G_rom_dumper_settings.filename);
  fr = f_open(&file, G_rom_dumper_settings.filename, FA_CREATE_NEW);
  f_close(&file);
  fr = f_open(&file, G_rom_dumper_settings.filename, FA_WRITE);

  if(fr)
    {
     vcom_printf("ERROR: cannot open file %s for writing.\r\n",G_rom_dumper_settings.filename);
     return;
    }

  vcom_printf("addr width: %d bit, addr_max: 0x%X\r\n",G_rom_dumper_settings.address_width,(addr_max-1));
  vcom_printf("data width: %d bit, data_max: 0x%X\r\n",G_rom_dumper_settings.data_width,data_bitmask);

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


   set_direction_bank(DATA_BANK, PIN_OUTPUT);

   set_level_bank(ADDR_LOW_BANK,0x0);
   set_level_bank(ADDR_HIGH_BANK,0x0);
   set_level_bank(DATA_BANK,0x0);
   set_level_bank(CTRL_BANK,0xFF);

   vcom_printf("reading: ");
   set_direction_bank(DATA_BANK, PIN_INPUT);

   for(j = 0; j < addr_max; j++)
       {
         output = 0x0;
         j_low = (uint8_t)j;
         j_high1 = (uint8_t)(j >> 8);
         j_high2 = (uint8_t)(j >> 16); // last four bits of 20 bit address
         //vcom_printf("read address: %X, %X, %X\r\n",j_low,j_high1,j_high2);
         set_level_bank(ADDR_LOW_BANK,j_low);
         set_level_bank(ADDR_HIGH_BANK,j_high1);
         
         ctlbank = 0b00001110 | (j_high2 << 7); // add last four bits of address to last two unused pins in bank 4
                                                // this will allow to read 20bit addresses (256K ROM's)
         set_level_bank(CTRL_BANK,ctlbank);     // set last four address bits on pins 44,45,46,47 (if used)
         ctlbank = 0b00001010 | (j_high2 << 7); // set control bank to combination of control lines (OE goes L)
                                                // and 17,18,19,20 address bits
         set_level_bank(CTRL_BANK,ctlbank);     // latch bits on D0-D7
         output = read_level_bank(DATA_BANK);   // read
         set_level_bank(CTRL_BANK,0b00111110);  // end read
    
         read_buffer[byte_ctr++] = output;

         if(byte_ctr == 127)
           {
              // write buffer to file
              f_write(&file,read_buffer,128,&written);
              if(written != 128)
                {
                  vcom_printf("ERROR: file write error\r\n");
                  return;
                }
              byte_ctr = 0;
              vcom_printf(".");
           }
 
       }

    // write rest of the data left after leaving the loop
    f_write(&file,read_buffer,byte_ctr+1,&written);
    if(written != (byte_ctr+1))
      vcom_printf("ERROR: file write error\r\n");

   f_close(&file);
   vcom_printf(" finished.\r\n");

   set_io_bank(1,"disable");
   set_io_bank(2,"disable");
   set_io_bank(3,"disable");
   set_io_bank(4,"disable");

   set_dut_power("disable");

}


