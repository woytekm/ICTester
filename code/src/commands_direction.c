#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"


void cli_set_direction_bank(int argc, char **argv) {

    uint8_t bank_id = atoi(argv[0]),direction,i;

    if (argc < 2) {
        vcom_printf( "usage: set direction bank <1|2|3|4> <I|O>\n\r");
        return;
    }

    if (strcmp(argv[1], "I") == 0) {
         direction = PIN_INPUT;
     } else if (strcmp(argv[1], "O") == 0) {
         direction = PIN_OUTPUT;
     } else {
         vcom_printf( "set direction bank: invalid direction specification (should be I or O)\n\r");
         return;
     }

    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= (bank_id*10)) && (G_pin_array[i].pin_id < ((bank_id*10)+10)) )
       if(G_pin_array[i].gpio_id != 255)
         {
            switch (direction) {
               case PIN_INPUT:
                   set_pin_read(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_INPUT;
                   break;
               case PIN_OUTPUT:
                   if((G_pin_array[i].pin_id == 44)||(G_pin_array[i].pin_id == 45))
                    {
                      vcom_printf( "ERROR: pin %d can be used only as INPUT\n\r",G_pin_array[i].pin_id);
                      continue;
                    }
                   set_pin_write(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_OUTPUT;
                   break;
               default:
                   break;
                }
            vcom_printf( "pin %d direction set to %s\n\r",G_pin_array[i].pin_id,argv[1]);

         }
     }

    if(direction == PIN_INPUT)
      set_pin_low_simple(G_pin_array[4+bank_id].gpio_id);   // bank DIR GPIO's are in G_pin_array 5,6,7,8 
    else if(direction == PIN_OUTPUT)
      set_pin_high_simple(G_pin_array[4+bank_id].gpio_id);
}


void set_direction_bank(uint8_t bank_id, uint8_t direction) {

    uint8_t i;

    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= (bank_id*10)) && (G_pin_array[i].pin_id < ((bank_id*10)+10)) )
       if(G_pin_array[i].gpio_id != 255)
         {
            switch (direction) {
               case PIN_INPUT:
                   set_pin_read(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_INPUT;
                   break;
               case PIN_OUTPUT:
                   if((G_pin_array[i].pin_id == 44)||(G_pin_array[i].pin_id == 45))
                    {
                      vcom_printf( "ERROR: pin %d can be used only as INPUT\n\r",G_pin_array[i].pin_id);
                      continue;
                    }
                   set_pin_write(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_OUTPUT;
                   break;
               default:
                   break;
                }
         }
     }

    if(direction == PIN_INPUT)
      set_pin_low_simple(G_pin_array[4+bank_id].gpio_id);   // bank DIR GPIO's are in G_pin_array 5,6,7,8
    else if(direction == PIN_OUTPUT)
      set_pin_high_simple(G_pin_array[4+bank_id].gpio_id);
}



void cli_set_direction_all(int argc, char **argv) {
    uint8_t direction,i;

    if (argc < 1) {
        vcom_printf( "usage: set direction all <I|O>\n\r");
        return;
    }

    if (strcmp(argv[0], "I") == 0) {
         direction = PIN_INPUT;
    } else if (strcmp(argv[0], "O") == 0) {
         direction = PIN_OUTPUT;
    } else {
         vcom_printf( "set direction bank: invalid direction specification (should be I or O)\n\r");
         return;
    }

    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= 10) && (G_pin_array[i].pin_id < 48) )
       if(G_pin_array[i].gpio_id != 255)
         {
            switch (direction) {
               case PIN_INPUT:
                   set_pin_read(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_INPUT;
                   break;
               case PIN_OUTPUT:
                   if((G_pin_array[i].pin_id == 44)||(G_pin_array[i].pin_id == 45))
                    {
                      vcom_printf( "ERROR: pin %d can be used only as INPUT\n\r",G_pin_array[i].pin_id);
                      continue;
                    }
                   set_pin_write(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_OUTPUT;
                   break;
               default:
                   break;
                }
            vcom_printf( "pin %d direction set to %s\n\r",G_pin_array[i].pin_id,argv[0]);
         }
     }

    if(direction == PIN_INPUT)
     {
       set_pin_low_simple(G_pin_array[5].gpio_id);   // bank DIR GPIO's are in G_pin_array 5,6,7,8
       set_pin_low_simple(G_pin_array[6].gpio_id);
       set_pin_low_simple(G_pin_array[7].gpio_id);
       set_pin_low_simple(G_pin_array[8].gpio_id);
     }
    else if(direction == PIN_OUTPUT)
     {
       set_pin_high_simple(G_pin_array[5].gpio_id);
       set_pin_high_simple(G_pin_array[6].gpio_id);
       set_pin_high_simple(G_pin_array[7].gpio_id);
       set_pin_high_simple(G_pin_array[8].gpio_id);
     }
}



void cli_set_direction(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: set direction bank|all <I|O>\n\r");
        return;
    }

    char *subcommand_set_direction = argv[0];

    if (strcmp(subcommand_set_direction, "bank") == 0) {
        cli_set_direction_bank(argc - 1, argv + 1);
    }
    else if (strcmp(subcommand_set_direction, "all") == 0) {
        cli_set_direction_all(argc - 1, argv + 1);
    }
    else {
        vcom_printf( "unknown subcommand for 'set direction': %s\n\r", subcommand_set_direction);
    }

}


void cli_show_direction_all(int argc, char **argv) {
  cli_show_level_all(argc,argv);
}

void cli_show_direction_bank(int argc, char **argv) {
  cli_show_level_bank(argc,argv);
}


void cli_show_direction(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: show direction bank|all\n\r");
        return;
    }

    char *subcommand_show_direction = argv[0];

    if (strcmp(subcommand_show_direction, "bank") == 0) {
        cli_show_direction_bank(argc - 1, argv + 1);
    }
    else if (strcmp(subcommand_show_direction, "all") == 0) {
        cli_show_direction_all(argc - 1, argv + 1);
    }
    else {
        vcom_printf( "unknown subcommand for 'show direction': %s\n\r", subcommand_show_direction);
    }
}




