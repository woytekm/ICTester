#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "LPC17xx.h"
#include "core_cm3.h"
#include "test.h"

// Top-level function to dispatch commands
void dispatch_cli_command(int cli_argc, char **cli_argv) {
  
    if(cli_argc == 0)
      return;

    char *command = cli_argv[0];
    
    if(command[0] == '#') // comment
      return;

    // Array of known top-level commands
    const char *top_level_commands[] = {"set", "clear", "show","run","help","hwinfo","reset"};

    int i;
    int num_top_level_commands = sizeof(top_level_commands) / sizeof(top_level_commands[0]);

    for (i = 0; i < num_top_level_commands; ++i) {
        if (strcmp(command, top_level_commands[i]) == 0) {
            break;
        }
    }

    switch (i) {
        case 0: // set
            if (cli_argc < 2) {
                vcom_printf("usage: set %s <subcommand>\n\r", cli_argv[1]);
                return;
            }

            char *subcommand_set = cli_argv[1];

            if (strcmp(subcommand_set, "level") == 0) {
                cli_set_level(cli_argc - 2, cli_argv + 2);
            } else if (strcmp(subcommand_set, "direction") == 0) {
                cli_set_direction(cli_argc - 2, cli_argv + 2);
            } 
            else if (strcmp(subcommand_set, "io") == 0) {
                cli_set_io(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "clock") == 0) {
                cli_set_clock(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "dut-power") == 0) {
                cli_set_dut_power(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "test") == 0) {
                cli_set_test(cli_argc - 2, cli_argv + 2);
            }
            else {
                vcom_printf( "unknown subcommand for 'set': %s\n\r", subcommand_set);
            }
            break;

        case 1: // clear
            // Handle 'clear' command if needed
            vcom_printf( "clear command not implemented\n\r");
            break;

        case 2: // show
            if (cli_argc < 2) {
                vcom_printf( "usage: %s show <subcommand>\n\r", cli_argv[0]);
                return;
            }

            char *subcommand_show = cli_argv[1];

            if (strcmp(subcommand_show, "level") == 0) {
                cli_show_level(cli_argc - 2, cli_argv + 2);
            } else if (strcmp(subcommand_show, "direction") == 0) {
                cli_show_direction(cli_argc - 2, cli_argv + 2);
            } else if (strcmp(subcommand_show, "io") == 0) {
                cli_show_io();
            } else if (strcmp(subcommand_show, "clock") == 0) {
                cli_show_clock();
            } else if (strcmp(subcommand_show, "test") == 0) {
                cli_show_test(cli_argc - 2, cli_argv + 2);
            }
            else {
                vcom_printf( "unknown subcommand for 'show': %s\n\r", subcommand_show);
            }
            break;

        case 3: // run
            if (cli_argc < 2) {
                vcom_printf( "usage: %s run <subcommand>\n\r", cli_argv[0]);
                return;
            }

            char *subcommand_run = cli_argv[1];

            if (strcmp(subcommand_run, "test") == 0) {
                cli_run_test(cli_argc - 2, cli_argv + 2);
            } 
            else {
                vcom_printf( "unknown subcommand for 'run': %s\n\r", subcommand_run);
            }
            break;

        case 4: // help
            display_help();
            break;

        case 5: // hwinfo
            display_hwinfo();
            break;

        case 6: // reset
            vcom_printf( "resetting the system... %s\n\r");
            NVIC_SystemReset();
            break;

        default:
            // Unknown command
            vcom_printf( "unknown command: %s\n\r", command);
            break;
    }
}



void display_help() {
    vcom_printf("available commands:\n\r");
    vcom_printf("set level <pin|bank|all> <L|H>: set pin/bank/all level to a value\n\r");
    vcom_printf("set level bitmap <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>: set all pins in all banks to specified bitmaps\n\r");
    vcom_printf("show level <pin|bank|all>: show level for pin/bank/all\n\r");
    vcom_printf("show direction <pin|bank|all>: show pin/bank/all direction: input or ouptut\n\r");
    vcom_printf("show io: show I/O status\n\r");
    vcom_printf("show clock: show clock status and settings\n\r");
    vcom_printf("set direction <pin|bank|all> <I|O>: set pin/bank/all direction to input/output\n\r");
    vcom_printf("set io <bank|all> <enable|disable>: enable/disable I/O on bank/all\n\r");
    vcom_printf("set clock pin <ID>: set pin for clock output\n\r");
    vcom_printf("set clock period <kHz>: set clock frequency in Khz\n\r");
    vcom_printf("set clock state <enable|disable>: enable or disable clock output\n\r");
    vcom_printf("set dut-power <enable|disable>: enable/disable power to DUT\n\r");

    vcom_printf("set test <name>: create new test\r\n");
    vcom_printf("set test frame-interval <test_name> <ms>: set test frame interval to ms miliseconds\r\n");
    vcom_printf("set test io-settings <test_name> <bank 1: I|O>  <bank 2: I|O>  <bank 3: I|O>  <bank 4: I|O>\r\n");
    vcom_printf("set test frame <test_name> <number> <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>\r\n");
    vcom_printf("show test <name>\r\n");
    vcom_printf("show test frame <test_name> <number>\r\n");
    vcom_printf("run test <name>\n\r");

    vcom_printf("hwinfo: show system information\n\r");
    vcom_printf("reset: reset the system\n\r");
    vcom_printf("help: display available commands\n\r");
}


long check_freemem(void)  // alocate 1 byte, check returned heap add, 32768 - heap add = free mem
{

    // 
    // LPC1769 has 32KB of SRAM for program data: 0x1000 0000 - 0x1000 7FFF
    // Heap is allocated from the bottom of this area
    // malloc(1) will return last taken address on heap
    //

    #define SRAM_SIZE 32768

    void *ptr;
    uint16_t freemem;

    ptr = malloc(1);
    freemem = SRAM_SIZE - ((uint32_t)ptr - 0x10000000); 
    free(ptr);
  
    return freemem;
}


void display_hwinfo() {
    vcom_printf("Hardware information:\n\r");
    vcom_printf("CPU ID: 0x%X\n\r",SCB->CPUID);
    vcom_printf("SystemCoreClock: %d\n\r",SystemCoreClock);
    vcom_printf("Available SRAM in first 32KB of LPC1769: %dK\n\r",check_freemem());
    vcom_printf("PINCON registers:\n\r");
    vcom_printf(" PINSEL0:      0x%X\n\r",LPC_PINCON->PINSEL0);
    vcom_printf(" PINSEL1:      0x%X\n\r",LPC_PINCON->PINSEL1);
    vcom_printf(" PINSEL2:      0x%X\n\r",LPC_PINCON->PINSEL2);
    vcom_printf(" PINSEL3:      0x%X\n\r",LPC_PINCON->PINSEL3);
    vcom_printf(" PINSEL4:      0x%X\n\r",LPC_PINCON->PINSEL4);
    vcom_printf(" PINSEL5:      0x%X\n\r",LPC_PINCON->PINSEL5);
    vcom_printf(" PINSEL6:      0x%X\n\r",LPC_PINCON->PINSEL6);
    vcom_printf(" PINSEL7:      0x%X\n\r",LPC_PINCON->PINSEL7);
    vcom_printf(" PINSEL8:      0x%X\n\r",LPC_PINCON->PINSEL8);
    vcom_printf(" PINSEL9:      0x%X\n\r",LPC_PINCON->PINSEL9);
    vcom_printf(" PINMODE0:     0x%X\n\r",LPC_PINCON->PINMODE0);
    vcom_printf(" PINMODE1:     0x%X\n\r",LPC_PINCON->PINMODE1);
    vcom_printf(" PINMODE2:     0x%X\n\r",LPC_PINCON->PINMODE2);
    vcom_printf(" PINMODE3:     0x%X\n\r",LPC_PINCON->PINMODE3);
    vcom_printf(" PINMODE4:     0x%X\n\r",LPC_PINCON->PINMODE4);
    vcom_printf(" PINMODE5:     0x%X\n\r",LPC_PINCON->PINMODE5);
    vcom_printf(" PINMODE6:     0x%X\n\r",LPC_PINCON->PINMODE6);
    vcom_printf(" PINMODE7:     0x%X\n\r",LPC_PINCON->PINMODE7);
    vcom_printf(" PINMODE8:     0x%X\n\r",LPC_PINCON->PINMODE8);
    vcom_printf(" PINMODE9:     0x%X\n\r",LPC_PINCON->PINMODE9);
    vcom_printf(" PINMODE_OD0:  0x%X\n\r",LPC_PINCON->PINMODE_OD0);
    vcom_printf(" PINMODE_OD1:  0x%X\n\r",LPC_PINCON->PINMODE_OD1);
    vcom_printf(" PINMODE_OD2:  0x%X\n\r",LPC_PINCON->PINMODE_OD2);
    vcom_printf(" PINMODE_OD3:  0x%X\n\r",LPC_PINCON->PINMODE_OD3);
    vcom_printf(" PINMODE_OD4:  0x%X\n\r",LPC_PINCON->PINMODE_OD4);

    vcom_printf(" output_cache_array_addr: 0x%X (this should be from RAM2 at 0x2007D000)\n\r",&output_cache);

}


bool validate_pin_id(uint8_t pin_id)
 {

  if( ((pin_id > 9) && (pin_id < 18)) || ((pin_id > 19) && (pin_id < 28)) || ((pin_id > 29) && (pin_id < 38)) || ((pin_id > 39) && (pin_id < 48)) )
    return true;
  else
   {
     vcom_printf( "ERROR: %d is not a valid pin ID. Valid are: 10-17, 20-27, 30-37, 40-47\n\r",pin_id);
     return false;
   }
 }

