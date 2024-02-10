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
#include "dram_test.h"

void cli_set_dram_test(int argc, char **argv)
{

    if (argc != 8) {
        vcom_printf("usage: set dram-test <addr_bits> <data-bits> <ce-active: L|H> <we-active: L|H> <oe-active: L|H> <cas-active: L|H> <ras-active: L|H> <loops>\r\n");
        return;
    }

    bool ce_low,oe_low,we_low,cas_low,ras_low;
    uint8_t addr_bits = atoi(argv[0]);
    uint8_t data_bits = atoi(argv[1]);
    char *ce_active = argv[2];
    char *oe_active = argv[3];
    char *we_active = argv[4];
    char *cas_active = argv[5];
    char *ras_active = argv[6];
    uint16_t loops = atoi(argv[7]);
    

    if(addr_bits > 16)
     { vcom_printf("ERROR: addr_bits too long (max 16 bits)\r\n");
        return; }

    if(data_bits > 8)
     { vcom_printf("ERROR: data_bits too long (max 8 bits)\r\n");
        return; }

    if( ((ce_active[0] != 'H') && (ce_active[0] != 'L')) ||
        ((oe_active[0] != 'H') && (oe_active[0] != 'L')) ||
        ((we_active[0] != 'H') && (we_active[0] != 'L')) ||
        ((cas_active[0] != 'H') && (cas_active[0] != 'L')) ||
        ((ras_active[0] != 'H') && (ras_active[0] != 'L')) )
     { vcom_printf("ERROR: incorrect CE/WE/OE/CAS/RAS flags: %c %c %c (should be L or H).\r\n",ce_active[0],oe_active[0],we_active[0]);
        return; }

    if(lh_to_bool(ce_active[0])) ce_low = false; else ce_low = true;
    if(lh_to_bool(oe_active[0])) oe_low = false; else oe_low = true;
    if(lh_to_bool(we_active[0])) we_low = false; else we_low = true;
    if(lh_to_bool(cas_active[0])) cas_low = false; else cas_low = true;
    if(lh_to_bool(ras_active[0])) ras_low = false; else ras_low = true;

    init_dram_test(addr_bits, data_bits, ce_low, we_low, oe_low, cas_low, ras_low, loops, &G_dram_test_settings);

}


void cli_show_dram_test(int argc, char **argv)
{

 vcom_printf("Current DRAM test parameters:\r\n");
 vcom_printf("  Address bus width: %d bit \r\n",G_dram_test_settings.address_width);
 vcom_printf("  Data bus width: %d bit \r\n",G_dram_test_settings.data_width);
 vcom_printf("  CE active low: %d \r\n",G_dram_test_settings.ce_active_low);
 vcom_printf("  OE active low: %d \r\n",G_dram_test_settings.we_active_low);
 vcom_printf("  WE active low: %d \r\n",G_dram_test_settings.oe_active_low);
 vcom_printf("  RAS active low: %d \r\n",G_dram_test_settings.cas_active_low);
 vcom_printf("  CAS active low: %d \r\n",G_dram_test_settings.ras_active_low);
 vcom_printf("  Test loops: %d \r\n",G_dram_test_settings.loops);

 vcom_printf("\n\r--------- pin cfg --------------------\r\n");
 show_test_aliases(G_dram_test_settings.pin_aliases);

}

void cli_run_dram_test(int argc, char **argv)
{

  uint16_t i,addr_max = pow(2,G_dram_test_settings.address_width);
  uint8_t data_bitmask = (pow(2,G_dram_test_settings.data_width) - 1);
  uint8_t data_pattern;
  uint16_t errors = 0;

  vcom_printf("addr width: %d bit, addr_max: 0x%X\r\n",G_dram_test_settings.address_width,(addr_max-1));
  vcom_printf("data width: %d bit, data_max: 0x%X\r\n",G_dram_test_settings.data_width,data_bitmask);
  vcom_printf("w(e)     - early write\r\n");
  vcom_printf("w(oe)    - OE write\r\n");
  vcom_printf("w(P)     - page mode write\r\n");
  vcom_printf("r        - read\r\n");
  vcom_printf("r(P)     - page mode read\r\n");
  vcom_printf("r-m-w    - read-modify-write\r\n");
  vcom_printf("r-m-w(P) - page mode read-modify-write\r\n");
  vcom_printf("RR       - RAS refresh\r\n");
  vcom_printf("C-B-R    - CAS before RAS refresh\r\n");  
  vcom_printf("HR       - hidden refresh\r\n");
  vcom_printf("1 - data pattern: alternate 01010101, 2 - data pattern: alternate 10101010\r\n");
  vcom_printf("---------------------------------------\r\n");

  set_direction_bank(DR_ADDR_LOW_BANK, PIN_OUTPUT);
  set_direction_bank(DR_ADDR_HIGH_BANK, PIN_OUTPUT);
  set_direction_bank(DR_DATA_BANK, PIN_OUTPUT);
  set_direction_bank(DR_CTRL_BANK, PIN_OUTPUT);

  set_level_bank(DR_ADDR_LOW_BANK,0x0);
  set_level_bank(DR_ADDR_HIGH_BANK,0x0);
  set_level_bank(DR_DATA_BANK,0x0);
  set_level_bank(DR_CTRL_BANK,0xFF);

  set_dut_power("enable");

  set_io_bank(1,"enable");
  set_io_bank(2,"enable");
  set_io_bank(3,"enable");
  set_io_bank(4,"enable");

  for(i = 0; i < G_dram_test_settings.loops; i++)
   {
    set_level_bank(DR_ADDR_LOW_BANK,0x0);
    set_level_bank(DR_ADDR_HIGH_BANK,0x0);
    set_level_bank(DR_DATA_BANK,0x0);
    set_level_bank(DR_CTRL_BANK,0xFF);

    data_pattern = 0b01010101;

    vcom_printf("%d: ",i);

    vcom_printf("w(e)1,");
    test_dram_early_write(addr_max,data_pattern,true);
    errors = test_dram_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r1:");
    if(errors)
     vcom_printf("fail ");
    else
     vcom_printf("ok ");
 

    data_pattern = 0b10101010;

    vcom_printf("w(e)2,");
    test_dram_early_write(addr_max,data_pattern,true);
    errors = test_dram_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r2:");
    if(errors)
     vcom_printf("fail ");
    else
     vcom_printf("ok ");


    vcom_printf("w(oe)1,");
    test_dram_oe_write(addr_max,data_pattern,true);
    errors = test_dram_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r1:");
    if(errors)
     vcom_printf("fail ");
    else
     vcom_printf("ok ");


    data_pattern = 0b10101010;

    vcom_printf("w(oe)2,");
    test_dram_oe_write(addr_max,data_pattern,true);
    errors = test_dram_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r2:");
    if(errors)
     vcom_printf("fail ");
    else
     vcom_printf("ok ");

    // rest of the tests goes here
 
    vcom_printf("\r\n"); 
   }

   set_io_bank(1,"disable");
   set_io_bank(2,"disable");
   set_io_bank(3,"disable");
   set_io_bank(4,"disable");

   set_dut_power("disable");

}


