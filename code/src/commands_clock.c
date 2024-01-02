#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "timers.h"

void cli_set_clock(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: set clock pin|period|state\n\r");
        return;
    }

    char *subcommand_set_clock = argv[0];

    if (strcmp(subcommand_set_clock, "pin") == 0) {
        cli_set_clock_pin(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_clock, "period") == 0) {
        cli_set_clock_period(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_clock, "state") == 0) {
        cli_set_clock_state(argc - 1, argv + 1);
    } else {
        vcom_printf( "unknown subcommand for 'set clock': %s\n\r", subcommand_set_clock);
    }

}


void cli_show_clock(void) {
  if(G_clock_state == 1)
    vcom_printf( "Clock pin is set to pin: %d. Clock period is %dKHz. Clock is enabled. \n\r",G_clock_pin,(PERIPH_CLOCK/G_tim_1_prescale)/1000);
  else if(G_clock_state == 0)
    vcom_printf( "Clock pin is set to pin: %d. Clock period is %dKHz. Clock is disabled. \n\r",G_clock_pin,(PERIPH_CLOCK/G_tim_1_prescale)/1000);
}


void cli_set_clock_pin(int argc, char **argv) {

    uint8_t pin_id = atoi(argv[0]);

    if (argc != 1) {
            vcom_printf( "usage: set clock pin <ID>\n\r");
            return;
        }

    if(!validate_pin_id(pin_id))
      return;

    if (pin_id >= 1 && pin_id <= sizeof(G_pin_array) / sizeof(G_pin_array[0])) {
                if(G_pin_array[pin_id].direction != PIN_OUTPUT)
                  {
                   vcom_printf( "ERROR: pin %d is not set to OUTPUT\n\r",pin_id);
                   return;
                  }
               G_clock_pin = pin_id;
               vcom_printf( "clock output set to pin %d\n\r",pin_id);
            } 
}

void cli_set_clock_period(int argc, char **argv){

 if (argc < 1) {
        vcom_printf( "usage: set clock period <integer> (sets clock to <integer> KHz)\n\r");
        return;
  }

 uint32_t prescale;
 uint32_t period_KHz = 1;

 period_KHz  = atoi(argv[0]);

 if((period_KHz < 1) || (period_KHz > 5000)) 
  {
   vcom_printf( "ERROR: clock period must be between 1 and 5000\n\r");
   return;
  }

 period_KHz *= 1000;

 prescale = (PERIPH_CLOCK / period_KHz) / 2;

 UpdateTimer1Prescale(prescale);

 G_tim_1_prescale = prescale;

 vcom_printf( "clock period updated to %dKHz (applied prescaler: %d)\n\r",period_KHz/1000,prescale);

}

void cli_set_clock_state(int argc, char **argv){

   if (argc < 1) {
        vcom_printf( "usage: set clock state <enable|disable>\n\r");
        return;
    }

    char *subcommand_state = argv[0];

    if (strcmp(subcommand_state, "enable") == 0) {
      Timer1enable();
      G_clock_state = 1;
    } 
    else if (strcmp(subcommand_state, "disable") == 0) {
      Timer1disable();
      G_clock_state = 0;
    } 
    else {
        vcom_printf( "unknown subcommand for 'set clock state': %s\n\r", subcommand_state);
    }


}


