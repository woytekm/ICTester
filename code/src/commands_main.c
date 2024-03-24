#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <cr_section_macros.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "LPC17xx.h"
#include "core_cm3.h"
#include "test.h"
#include "sdcard.h"
#include "globals.h"

// Top-level function to dispatch commands
void dispatch_cli_command(int cli_argc, char **cli_argv) {
  
    if(cli_argc == 0)
      return;

    char *command = cli_argv[0];
    
    if(command[0] == '#') // comment
      return;

    // Array of known top-level commands
    const char *top_level_commands[] = {"set","clear","show","run","help","hwinfo","reset",
                                        "dir","cat","xxd","strings","rm","mv","mkdir","load","save","join"};

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
            else if (strcmp(subcommand_set, "sram-test") == 0) {
                cli_set_sram_test(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "dram-test") == 0) {
                cli_set_dram_test(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "dram-type") == 0) {
                cli_set_dram_type(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "rom-dumper") == 0) {
                cli_set_rom_dumper(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "button-cmd") == 0) {
                cli_set_button_cmd(cli_argc - 2, cli_argv + 2);
            }
            else if (strcmp(subcommand_set, "color") == 0) {
                cli_set_color(cli_argc - 2, cli_argv + 2);
            }
            else {
                vcom_printf( "unknown subcommand for 'set': %s\n\r", subcommand_set);
            }
            break;

        case 1: // clear
            if (cli_argc < 2) {
                vcom_printf( "usage: %s clear <subcommand>\n\r", cli_argv[0]);
                return;
            }

            char *subcommand_clear = cli_argv[1];

            if (strcmp(subcommand_clear, "test") == 0) {
                cli_clear_test(cli_argc - 2, cli_argv + 2);
            }
            else {
                vcom_printf( "unknown subcommand for 'clear': %s\n\r", subcommand_clear);
            }
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
            }else if (strcmp(subcommand_show, "sram-test") == 0) {
                cli_show_sram_test(cli_argc - 2, cli_argv + 2);
            }else if (strcmp(subcommand_show, "dram-test") == 0) {
                cli_show_dram_test(cli_argc - 2, cli_argv + 2);
            }else if (strcmp(subcommand_show, "rom-dumper") == 0) {
                cli_show_rom_dumper(cli_argc - 2, cli_argv + 2);
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
            }else if (strcmp(subcommand_run, "sram-test") == 0) {
                cli_run_sram_test(cli_argc - 2, cli_argv + 2);
            }else if (strcmp(subcommand_run, "dram-test") == 0) {
                cli_run_dram_test(cli_argc - 2, cli_argv + 2);
            }else if (strcmp(subcommand_run, "rom-dumper") == 0) {
                cli_run_rom_dumper(cli_argc - 2, cli_argv + 2);
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
            vcom_printf("resetting the system... %s\n\r");
            vcom_printf("\n\r");
            NVIC_SystemReset();
            break;

        case 7: // dir
            if (cli_argc < 2) 
              usd_list_dir("/");
            else
              usd_list_dir(cli_argv[1]);
            break;

        case 8: // cat
            if (cli_argc != 2)
              {
               vcom_printf( "usage: cat <filename>\n\r");
               break;
              }
            usd_list_file_contents(cli_argv[1]);
            break;

        case 9: // xxd
            if (cli_argc != 2)
              {
               vcom_printf( "usage: xxd <filename>\n\r");
               break;
              }
            hex_dump(cli_argv[1]);
            break;

        case 10: // strings
            if (cli_argc != 2)
              {
               vcom_printf( "usage: strings <filename>\n\r");
               break;
              }
            strings(cli_argv[1]);
            break;

        case 11: // rm
            if (cli_argc != 2)
              {
               vcom_printf( "usage: rm <filename>|<dirname>\n\r");
               break;
              }
            usd_unlink_file(cli_argv[1]);
            break;

        case 12: // mv
            if (cli_argc != 3)
              {
               vcom_printf( "usage: mv <filename> <new_filename>\n\r");
               break;
              }
            usd_rename_file(cli_argv[1],cli_argv[2]);
            break;

        case 13: // mkdir
            if (cli_argc != 2)
              {
               vcom_printf( "usage: mkdir <name>\n\r");
               break;
              }
            usd_mkdir(cli_argv[1]);
            break;

        case 14: // load
            if (cli_argc != 2)
              {
               vcom_printf( "usage: load <filename>\n\r");
               break;
              }
            usd_load_file(cli_argv[1]);
            break;

        case 15: // save
            if (cli_argc != 5)
              {
               vcom_printf( "usage: save test <name> to <filename>\n\r");
               break;
              }
            usd_save_test_to_file(cli_argv[2],cli_argv[4]);
            break;

        case 16: // join
            if (cli_argc != 4)
              {
               vcom_printf( "usage: join <file 1> <file 2> <output file> - join even and odd ROM parts into one file\n\r");
               break;
              }
            usd_join(cli_argv[1],cli_argv[2],cli_argv[3]);
            break;

        default:
            // Unknown command
            vcom_printf( "unknown command: %s\n\r", command);
            break;
    }
}


void cli_set_button_cmd(int argc, char **argv){

 G_set_btn_cmd[0] = '\0';
 for(uint8_t i = 0; i < argc; i++)
   {
    strcat(G_set_btn_cmd,argv[i]);
    strcat(G_set_btn_cmd," ");
   }
  strcat(G_set_btn_cmd,"\n");
}


void display_help() {
    vcom_printf("available commands:\n\r");
    vcom_printf("\r\npin/bank level/direction:\r\n");
    vcom_printf(" set level <pin|bank|all> <L|H>: set pin/bank/all level to a value\n\r");
    vcom_printf(" set level bitmap <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>: set all pins in all banks to specified bitmaps\n\r");
    vcom_printf(" show level <pin|bank|all>: show level for bank/all\n\r");
    vcom_printf(" show direction <bank|all>: show pin/bank/all direction: input or ouptut\n\r");
    vcom_printf(" show io: show I/O status\n\r");
    vcom_printf(" show clock: show clock status and settings\n\r");
    vcom_printf(" set direction <bank|all> <I|O>: set bank/all direction to input/output\n\r");
    vcom_printf(" set io <bank|all> <enable|disable>: enable/disable I/O on bank/all\n\r");
    vcom_printf(" set clock pin <ID>: set pin for clock output\n\r");
    vcom_printf(" set clock period <kHz>: set clock frequency in Khz\n\r");
    vcom_printf(" set clock state <enable|disable>: enable or disable clock output\n\r");
    vcom_printf("\r\ntest commands:\r\n");
    vcom_printf(" set test <name>: create new test\r\n");
    vcom_printf(" clear test <name>: delete a test from memory\r\n");
    vcom_printf(" set test frame-interval <test_name> <ms>: set test frame interval to ms miliseconds\r\n");
    vcom_printf(" set test io-settings <test_name> <bank 1: I|O>  <bank 2: I|O>  <bank 3: I|O>  <bank 4: I|O>\r\n");
    vcom_printf(" set test frame <test_name> <number> <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>\r\n");
    vcom_printf(" show test <name> - show test configuration\r\n");
    vcom_printf(" show test frame <test_name> <number> - show test frame\r\n");
    vcom_printf(" show test states <test_name> - show bank states from test iterations\r\n");
    vcom_printf(" run test <name> - start a test\n\r");
    vcom_printf("\r\ntest criteria commands:\r\n");
    vcom_printf(" \r\n");
    vcom_printf(" set test criteria <test_name> <criteria number> mexpr <expr> from-frame <number> to-frame <number> - test math expression against bank set\r\n");
    vcom_printf("   mexpr example: [Y5,Y4,Y3,Y2,Y1]=[A4,A3,A2,A1]+[B4,B3,B2,B1] \r\n");
    vcom_printf("   where [Y5,Y4,Y3,Y2,Y1] = bits used to check expression result\r\n");
    vcom_printf("         [A4,A3,A2,A1] =  bits of used to represent input value A\r\n");
    vcom_printf("         [B4,B3,B2,B1] =  bits of used to represent input value B\r\n");
    vcom_printf("         WARNING: values of pin aliases starting with '~' will be inverted during evaluation of expression (eg \"~A4\") \r\n");
    vcom_printf("   other examples:  \r\n");
    vcom_printf("         [F0,F1,F2,F3]=([A0,A1,A2,A3]|[~B0,~B1,~B2,~B3]) \r\n");
    vcom_printf("         [Y1]=!(([A1] & [B1] & [C1]) | ([D1] & [E1] & [F1])) \r\n");
    vcom_printf(" \r\n");
    vcom_printf(" set test criteria <test_name> <criteria number> val <val pin aliases: \"(pin1,pin2,pin3,pin4,pin5)=0xA\"> from-frame <number> to-frame <number>\r\n");
    vcom_printf("   check if specified pins match bit value specified (from frame x to frame y)\r\n");
    vcom_printf(" \r\n");
    vcom_printf(" set test criteria <test_name> <criteria number> ctr <counter pin aliases: \"(pin1,pin2,pin3,pin4,pin5)\"> from-frame <number> to-frame <number>\r\n");
    vcom_printf("   check if bit value of specified pins (in order) is a counter incrementing by one (from frame x to frame y)\r\n");
    vcom_printf("\r\nSRAM test commands:\r\n");
    vcom_printf(" set sram-test <addr-bits> <data-bits> ce oe we <loops> where ce=L|H , oe=L|H, we=L|H\r\n");
    vcom_printf(" show sram-test\r\n");
    vcom_printf(" run sram-test\r\n");
    vcom_printf("\r\nDRAM test commands:\r\n");
    vcom_printf(" set dram-test <addr-bits> <data-bits> ce oe we ras cas <loops> where ce=L|H , oe=L|H, we=L|H, ras=L|H, cas=L|H\r\n");
    vcom_printf(" set dram-type <single-port|dual-port|single-port-2ras>\r\n");
    vcom_printf("   single-port: DRAM with one I/O data bus\r\n"); 
    vcom_printf("   dual-port: DRAM with separate I and O buses\r\n"); 
    vcom_printf("   single-port-2ras: single port DRAM with 2 RAS lines (double capacity)\r\n");
    vcom_printf(" show dram-test\r\n");
    vcom_printf(" run dram-test\r\n");
    vcom_printf("\r\nROM dumper commands:\r\n");
    vcom_printf(" set rom-dumper <addr-bits> <data-bits> ce oe <filename> where ce=L|H , oe=L|H\r\n");
    vcom_printf(" show rom-dumper\r\n");
    vcom_printf(" run rom-dumper\r\n");
    vcom_printf("\r\nfile commands:\r\n");
    vcom_printf(" dir <path> - show directory listing\n\r");
    vcom_printf(" cat <path> - list file contents\n\r");
    vcom_printf(" xxd <path> - hex dump file contents\n\r");
    vcom_printf(" strings <path> - show ASCII strings in file contents\n\r");
    vcom_printf(" join <file 1> <file 2> <output file> - join even and odd ROM parts into one file\n\r");
    vcom_printf(" rm <path> - delete a file\n\r");
    vcom_printf(" mv <path1> <path2> - rename a file\n\r");
    vcom_printf(" mkdir <path> - create directory\n\r");
    vcom_printf(" load <path> - load test from file\n\r");
    vcom_printf(" save test <name> to <filename> - save test <name> to a file <filename>\n\r");
    vcom_printf("\r\nother:\r\n");
    vcom_printf(" set button-cmd <command> - attach a CLI command to a device button\r\n");
    vcom_printf(" set color <enable|disable>: enable/disable color in CLI\n\r");
    vcom_printf(" set dut-power <enable|disable>: enable/disable power to DUT\n\r");
    vcom_printf(" hwinfo: show system information\n\r");
    vcom_printf(" reset: reset the system\n\r");
    vcom_printf(" help: display available commands\n\r");
    vcom_printf("\n\r");
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

    vcom_printf(" output_cache_array_addr: 0x%X (RAM2 addr at 0x2007D000)\n\r",&G_output_cache);

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


void cli_set_color(int argc, char **argv) {

  if(argc == 0)
    {
     vcom_printf( "usage: set color <enable|disable> \n\r");
     return;
    }

  char *subcommand_set_color = argv[0];

  if (strcmp(subcommand_set_color, "enable") == 0) {
         G_use_color = true;
      }
  else if(strcmp(subcommand_set_color, "disable") == 0)
      {
         G_use_color = false;
      }
  else
      vcom_printf( "ERROR: invalid parameter. Should be: <enable|disable>\n\r");
}

