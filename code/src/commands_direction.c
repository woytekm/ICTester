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
                   if(G_pin_array[i].pin_id == 47)
                    {
                      vcom_printf( "ERROR: pin 47 (GPIO23) can be used only as INPUT\n\r");
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

}

void cli_set_direction_all(int argc, char **argv) {
    uint8_t direction,i;

    if (argc < 1) {
        vcom_printf( "usage: set direction all <I|O>\n\r");
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
      if( (G_pin_array[i].pin_id >= 10) && (G_pin_array[i].pin_id < 48) )
       if(G_pin_array[i].gpio_id != 255)
         {
            switch (direction) {
               case PIN_INPUT:
                   set_pin_read(G_pin_array[i].gpio_id);
                   G_pin_array[i].direction = PIN_INPUT;
                   break;
               case PIN_OUTPUT:
                   if(G_pin_array[i].pin_id == 47)
                    {
                      vcom_printf( "ERROR: pin 47 (GPIO23) can be used only as INPUT\n\r");
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
}

void cli_set_direction(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: set direction pin|bank|all <I|O>\n\r");
        return;
    }

    char *subcommand_set_direction = argv[0];

    if (strcmp(subcommand_set_direction, "pin") == 0) {
        cli_set_direction_pin(argc - 1, argv + 1);
    } 
    else if (strcmp(subcommand_set_direction, "bank") == 0) {
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
        vcom_printf( "usage: show direction pin|bank|all\n\r");
        return;
    }

    char *subcommand_show_direction = argv[0];

    if (strcmp(subcommand_show_direction, "pin") == 0) {
        cli_show_direction_pin(argc - 1, argv + 1);
    } 
    else if (strcmp(subcommand_show_direction, "bank") == 0) {
        cli_show_direction_bank(argc - 1, argv + 1);
    }
    else if (strcmp(subcommand_show_direction, "all") == 0) {
        cli_show_direction_all(argc - 1, argv + 1);
    }
    else {
        vcom_printf( "unknown subcommand for 'show direction': %s\n\r", subcommand_show_direction);
    }
}


void cli_set_direction_pin(int argc, char **argv) {
    if (argc < 2) {
        vcom_printf( "usage: set direction pin <pin ID> <I|O>\n\r");
        return;
    }

   
    int pin_id = atoi(argv[0]);

    if(!validate_pin_id(pin_id))
      return;

    char *direction_value = argv[1];
    vcom_printf( "setting pin %d direction to %s\n\r", pin_id, direction_value);
    if(direction_value[0] == 'I')
      {
       set_pin_read(G_pin_array[pin_id].gpio_id);
       G_pin_array[pin_id].direction = PIN_INPUT;
      }
    else if(direction_value[0] == 'O')
      {
       if(pin_id == 47)
        {
         vcom_printf( "ERROR: pin 47 (GPIO23) can be used only as INPUT\n\r");
         return;
        }
       set_pin_write(G_pin_array[pin_id].gpio_id);
       G_pin_array[pin_id].direction = PIN_OUTPUT;
      }
    else
      {
       vcom_printf( "ERROR: wrong pin direction. Should be: I or O\n\r");
      }
      
}

void cli_show_direction_pin(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: show direction pin <pin>\n\r");
        return;
    }

    int pin_id = atoi(argv[0]);

    if(!validate_pin_id(pin_id))
      return;

    if(G_pin_array[pin_id].direction == PIN_OUTPUT)
     {
       vcom_printf( "pin %d direction is O\n\r", pin_id);
     }
    else if(G_pin_array[pin_id].direction == PIN_INPUT)
     {
       vcom_printf( "pin %d direction is I\n\r", pin_id);
     }
}


