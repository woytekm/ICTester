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


void cli_set_dram_type(int argc, char **argv)
{
  if(argc != 1){
       vcom_printf("usage: set dram-type <single-port|dual-port|single-port-2ras>\r\n");
       return;
    }

  char *parameter = argv[0];

  if (strcmp(parameter, "single-port") == 0) {
    G_dram_test_settings.dram_type = SINGLE_PORT;
    clear_aliases(&G_dram_test_settings);
    set_aliases_single_port(&G_dram_test_settings);
  } else if (strcmp(parameter, "dual-port") == 0) {
    if(G_dram_test_settings.address_width>8)
     {
       vcom_printf("auto setting address bus width to 8\r\n");
       G_dram_test_settings.address_width = 8;
     }
    G_dram_test_settings.dram_type = DUAL_PORT;
    clear_aliases(&G_dram_test_settings);
    set_aliases_dual_port(&G_dram_test_settings);
  }
  else
   vcom_printf("unknown parameter: %s\r\n",parameter);

}

void cli_set_dram_test(int argc, char **argv)
{

    if (argc != 8) {
        vcom_printf("usage: set dram-test <addr_bits> <data-bits> <ce-active: L|H> <we-active: L|H> <oe-active: L|H> <cas-active: L|H> <ras-active: L|H> <loops>\r\n");
        return;
    }

    bool ce,oe,we,cas,ras;
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

    if(lh_to_int(ce_active[0])) ce = 1; else ce = 0;
    if(lh_to_int(oe_active[0])) oe = 1; else oe = 0;
    if(lh_to_int(we_active[0])) we = 1; else we = 0;
    if(lh_to_int(cas_active[0])) cas = 1; else cas = 0;
    if(lh_to_int(ras_active[0])) ras = 1; else ras = 0;

    init_dram_test(addr_bits, data_bits, ce, we, oe, cas, ras, loops, &G_dram_test_settings);

}


void cli_show_dram_test(int argc, char **argv)
{
 uint16_t addr_max = pow(2,G_dram_test_settings.address_width);

 vcom_cprintf("\e[0;36mCurrent DRAM test parameters:\e[0m\r\n","Current DRAM test parameters:\r\n");
 vcom_printf("  DRAM type: ");
 if(G_dram_test_settings.dram_type == SINGLE_PORT)
   vcom_printf("single-port (multiplexed input/output data bus) \r\n");
 else if(G_dram_test_settings.dram_type == DUAL_PORT)
   vcom_printf("dual-port (separate input and output data buses) \r\n");
 vcom_printf("  Address bus width: %d bit, addr_max: %d (0x%X)\r\n",G_dram_test_settings.address_width,addr_max,addr_max);
 vcom_printf("  Data bus width: %d bit \r\n",G_dram_test_settings.data_width);
 vcom_printf("  CE active: %c \r\n",sh_lvl_set(G_dram_test_settings.ce_active));
 vcom_printf("  OE active: %c \r\n",sh_lvl_set(G_dram_test_settings.we_active));
 vcom_printf("  WE active: %c \r\n",sh_lvl_set(G_dram_test_settings.oe_active));
 vcom_printf("  RAS active: %c \r\n",sh_lvl_set(G_dram_test_settings.cas_active));
 vcom_printf("  CAS active: %c \r\n",sh_lvl_set(G_dram_test_settings.ras_active));
 vcom_printf("  Test loops: %d \r\n",G_dram_test_settings.loops);

 vcom_printf("\n\r--------- pin cfg --------------------\r\n");
 show_test_aliases(G_dram_test_settings.pin_aliases);

}

void cli_run_dram_test(int argc, char **argv)
{

  uint16_t i,addr_max = pow(2,G_dram_test_settings.address_width);
  uint8_t data_bitmask = (pow(2,G_dram_test_settings.data_width) - 1);
  uint8_t data_pattern;
  uint32_t errors = 0;
  uint8_t dram_type = G_dram_test_settings.dram_type;

  if(G_dram_test_settings.address_width == 16)
   addr_max = 0xFFFF;

 vcom_printf("DRAM type: ");
 if(G_dram_test_settings.dram_type == SINGLE_PORT)
   vcom_printf("single-port\r\n");
 else if(G_dram_test_settings.dram_type == DUAL_PORT)
   vcom_printf("dual-port\r\n");
  vcom_printf("addr width: %d bit, addr_max: 0x%X\r\n",G_dram_test_settings.address_width,(addr_max-1));
  vcom_printf("data width: %d bit, data_max: 0x%X\r\n",G_dram_test_settings.data_width,data_bitmask);
  vcom_printf("loops: %d \r\n",G_dram_test_settings.loops);
  vcom_printf("---------------------------------------\r\n");
  vcom_printf("w(e)     - early write\r\n");
  vcom_printf("w(oe)    - OE write\r\n");
  vcom_printf("w(P)     - page mode write\r\n");
  vcom_printf("r        - read\r\n");
  vcom_printf("r(P)     - page mode read\r\n");
  vcom_printf("r-m-w    - read-modify-write\r\n");
  vcom_printf("RR       - RAS refresh\r\n");
  vcom_printf("CBR      - CAS before RAS refresh\r\n");  
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

    vcom_printf("%d (R/W): ",i+1);

    data_pattern = 0b01010101;
    vcom_printf("w(e)1,");
    test_dram_early_write(dram_type,addr_max,data_pattern,true);
    errors = test_dram_read(dram_type,addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r1:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }
 

    data_pattern = 0b10101010;
    vcom_printf("w(e)2,");
    test_dram_early_write(dram_type,addr_max,data_pattern,true);
    errors = test_dram_read(dram_type,addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r2:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    vcom_printf("w(oe)1,");
    test_dram_oe_write(addr_max,data_pattern,true);
    errors = test_dram_read(dram_type,addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r1:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    data_pattern = 0b10101010;
    vcom_printf("w(oe)2,");
    test_dram_oe_write(addr_max,data_pattern,true);
    errors = test_dram_read(dram_type,addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r2:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    data_pattern = 0b10101010;
    vcom_printf("w(e)1,");
    test_dram_early_write(dram_type,addr_max,data_pattern,true);
    vcom_printf("r-m-w,");
    test_dram_read_mod_write(dram_type,addr_max); // this test will read, bitwise reverse, and write bytes back to the DRAM
    errors = test_dram_read(dram_type,addr_max,data_bitmask,(~data_pattern),true); // data read should be reversed after r-m-w test
    vcom_printf("r2:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    data_pattern = 0b10101010;
    vcom_printf("w(P)1,");
    test_dram_page_write(dram_type,addr_max,data_pattern,true);
    errors = test_dram_page_read(addr_max,data_bitmask,data_pattern,true); 
    vcom_printf("r(P)1:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    data_pattern = 0b01010101;
    errors = 0;
    vcom_printf("w(P)2,");
    test_dram_page_write(dram_type,addr_max,data_pattern,true);
    errors = test_dram_page_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r(P)2:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    vcom_printf("\r\n"); 

    vcom_printf("%d (refresh): ",i+1);

    data_pattern = 0b10101010;
    vcom_printf("w(P)1,");
    test_dram_page_write(dram_type,addr_max,data_pattern,true);
    vcom_printf("RR,");
    test_dram_rr_cycle(addr_max,5000,1);  // RAS only refresh - 1000 cycles, 1 msec interval
    errors = test_dram_page_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r(P)1:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    data_pattern = 0b10101010;
    vcom_printf("w(P)1,");
    test_dram_page_write(dram_type,addr_max,data_pattern,true);
    vcom_printf("CBR,");
    test_dram_cbr_cycle(10000,1);  // CAS before RAS refresh - 1000 cycles, 1 msec interval
    errors = test_dram_page_read(addr_max,data_bitmask,data_pattern,true);
    vcom_printf("r(P)1:");
    if(errors)
     {
       vcom_cprintf("\e[0;31mfail \e[0m"," fail");
       led_signal_test_fail();
     }
    else
     {
      vcom_cprintf("\e[0;32mok \e[0m","ok ");
      led_signal_test_ok();
     }

    vcom_printf("\r\n");

   }

   set_io_bank(1,"disable");
   set_io_bank(2,"disable");
   set_io_bank(3,"disable");
   set_io_bank(4,"disable");

   set_dut_power("disable");

}


