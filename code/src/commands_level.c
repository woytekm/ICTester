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

void cli_set_level(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: set level <value>\n\r");
        return;
    }

    char *subcommand_set_level = argv[0];

    if (strcmp(subcommand_set_level, "pin") == 0) {
        cli_set_level_pin(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "bank") == 0) {
        cli_set_level_bank(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "all") == 0) {
        cli_set_level_all(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "bitmap") == 0) {
        cli_set_level_bitmap(argc - 1, argv + 1);
    } else {
        vcom_printf( "unknown subcommand for 'set level': %s\n\r", subcommand_set_level);
    }

}

void cli_set_level_bitmap(int argc, char **argv) {

   char *args[2];
   uint8_t _argc = 2;
   char bank[3];

    if (argc < 4) {
        vcom_printf( "usage: set level bitmap <bank 1 bitmap> <bank 2 bitmap> <bank 3 bitmap> <bank 4 bitmap> bitmap in form 0bxxxxxxxx, where x is: 0|1|N\n\r");
        return;
    }

   strcpy(bank,"1");
   args[0] = (char *)&bank;
   args[1] = argv[0];
   cli_set_level_bank(_argc,args);

   strcpy(bank,"2");
   args[0] = (char *)&bank;
   args[1] = argv[1];
   cli_set_level_bank(_argc,args);

   strcpy(bank,"3");
   args[0] = (char *)&bank;
   args[1] = argv[2];
   cli_set_level_bank(_argc,args);

   strcpy(bank,"4");
   args[0] = (char *)&bank;
   args[1] = argv[3];
   cli_set_level_bank(_argc,args);

}

void cli_show_level(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: show level <subcommand>\n\r");
        return;
    }

    char *subcommand_show_level = argv[0];

    if (strcmp(subcommand_show_level, "pin") == 0) {
        cli_show_level_pin(argc - 1, argv + 1);
    } else if (strcmp(subcommand_show_level, "bank") == 0) {
        cli_show_level_bank(argc - 1, argv + 1);
    } else if (strcmp(subcommand_show_level, "all") == 0) {
        cli_show_level_all(argc - 1, argv + 1);
    } else {
        vcom_printf( "unknown subcommand for 'show level': %s\n\r", subcommand_show_level);
    }
}


void cli_show_level_pin(int argc, char **argv) {

    uint32_t pin_id = atoi(argv[0]),pin_level;

    if(!validate_pin_id(pin_id))
      return;

    pin_level = get_pin(G_pin_array[pin_id].gpio_id);

    vcom_printf( "pin %d is %d\n\r",pin_id,pin_level);
}

void cli_show_level_bank(int argc, char **argv) {

    uint8_t bank_id = atoi(argv[0]),pin_level,pin_dir,i,bit_ctr=7;
    char dir = 'O';;
    char bitmap_str[9];
    char states[2] ={ '0','1'};
    bool bitmap = false;
    bitmap_str[8] = 0x0;

    if (argc != 1) {
        if((argc == 2) && (strcmp(argv[1], "bitmap") == 0))
         {
          bitmap = true;
         }
        else
         {
           vcom_printf("usage: show level bank <1|2|3|4> <bitmap>\n\r");
           return;
         }
    }

    if(!bitmap) 
      vcom_printf("------------- pin level/direction bank %d ------------\n\r",bank_id);

    
    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= (bank_id*10)) && (G_pin_array[i].pin_id < ((bank_id*10)+10)) )
       if(G_pin_array[i].gpio_id != 255)
         {
          pin_level = get_pin(G_pin_array[i].gpio_id);
          pin_dir = get_pin_direction(G_pin_array[i].gpio_id);
          if(!bitmap)
           {
            if(pin_dir == 1)
             dir = 'O';
            else if(pin_dir == 0)
             dir = 'I';
            vcom_printf("pin %d (GPIO %3d, DIR:%c): %d\n\r",G_pin_array[i].pin_id,G_pin_array[i].gpio_id,dir,pin_level);
           }
          else
           {
            bitmap_str[bit_ctr] = states[pin_level];
            bit_ctr--;
           }
         }
     }

    if(!bitmap)
       vcom_printf("------------------------------------------------------\n\r");
    else
       vcom_printf("0b%s\n\r",bitmap_str);
}

void cli_show_level_all(int argc, char **argv) {

   uint8_t pin_level,pin_dir,i,bit_ctr=7;
   char dir = 'O';
   char bitmap_str[5][9];
   uint8_t bank_counter = 0;
   char states[2] ={'0','1'};
   bool bitmap = false;
   
   for(i=0; i<=4; i++)
     bitmap_str[i][8] = 0x0;

   if (argc > 0) {
        if((argc == 1) && (strcmp(argv[0], "bitmap") == 0))
         {
          bitmap = true;
         }
        else
         {
           vcom_printf("usage: show level all <bitmap>\n\r");
           return;
         }
    }


   if(!bitmap)
     vcom_printf("------------- pin level/direction all pins ------------\n\r");

   for(i=10;i<PIN_COUNT;i++)
     {
       if(((i>9)&&(i<18))||((i>19)&&(i<28))||((i>29)&&(i<38))||((i>39)&&(i<48))) // only bank pins
        if(G_pin_array[i].gpio_id != 255)
         {
          pin_level = get_pin(G_pin_array[i].gpio_id);
          pin_dir = get_pin_direction(G_pin_array[i].gpio_id);
          if(!bitmap)
           {
            if(pin_dir == 1)
             dir = 'O';
            else if(pin_dir == 0)
             dir = 'I';
            vcom_printf("pin %d (GPIO %3d, DIR:%c): %d\n\r",G_pin_array[i].pin_id,G_pin_array[i].gpio_id,dir,pin_level);
           }
          else
           {
            if((i % 10) == 0)
              {bank_counter++; 
               bit_ctr = 7;}
            bitmap_str[bank_counter][bit_ctr] = states[pin_level];
            bit_ctr--;
           }
         }
     }

    if(!bitmap)
     vcom_printf("------------------------------------------------------\n\r");
    else
    {
     vcom_printf(" --bank 1--      --bank 2--       --bank 3--       --bank 4-- \n\r");
     vcom_printf(" 0b%s      0b%s       0b%s       0b%s  \n\r",bitmap_str[1],bitmap_str[2],bitmap_str[3],bitmap_str[4]);
    }
}


void cli_set_level_pin(int argc, char **argv) {

    if (argc == 2) {
        uint8_t pin_id = atoi(argv[0]);
        pin_level level;

        if (strcmp(argv[1], "L") == 0) {
            level = PIN_LOW;
        } else if (strcmp(argv[1], "H") == 0) {
            level = PIN_HIGH;
        } else {
            vcom_printf( "set level pin: invalid level specification (should be L or H)\n\r");
            return;
        }

    if(!validate_pin_id(pin_id))
      return;

    if (pin_id >= 1 && pin_id <= sizeof(G_pin_array) / sizeof(G_pin_array[0])) {
                uint8_t gpio_id = G_pin_array[pin_id].gpio_id;
                if(G_pin_array[pin_id].direction != PIN_OUTPUT)
                  {
                   vcom_printf( "ERROR: pin %d is not set to OUTPUT\n\r",pin_id);
                   return;
                  }
                // Set the pin level based on the provided pin_state
                switch (level) {
                    case PIN_LOW:
                        set_pin_low(gpio_id,G_pin_array[pin_id].gpio_addrs[CLR_ADDR]);
                        G_pin_array[pin_id].level =  PIN_LOW;
                        break;
                    case PIN_HIGH:
                        set_pin_high(gpio_id,G_pin_array[pin_id].gpio_addrs[SET_ADDR]);
                        G_pin_array[pin_id].level =  PIN_HIGH;
                        break;
                    default:
                      break;
                }
               vcom_printf( "pin %d level set to %s\n\r",pin_id,argv[1]);
            } 
       else {
               vcom_printf( "set level pin: <pin id> (XY where X - bank, Y - pin in bank) <level> (L|H)\n\r");
            }
    } 
    else {
       vcom_printf( "set level pin: <pin id> (XY where X - bank, Y - pin in bank) <level> (L|H)\n\r");
    }
}


void cli_set_level_bank(int argc, char **argv) {
    uint8_t bank_id = atoi(argv[0]),level = 0,i;
    bool bitmap = false;
 
    if((bank_id < 1) || (bank_id > 4))
      {
        vcom_printf( "set level bank: invalid bank id: %d\n\r",bank_id);
        return; 
      }
    
    if (strcmp(argv[1], "L") == 0) {
            level = PIN_LOW;
    } else if (strcmp(argv[1], "H") == 0) {
            level = PIN_HIGH;
    } else if ((argv[1][0] == '0')&&(argv[1][1] == 'b'))
      {
       if(strlen(argv[1]) != 10)
        {
          vcom_printf( "set level bank: invalid bitmap length: %s\n\r",argv[1]);
          return;
        }
        bitmap = true;
        vcom_printf( "set level bank: setting bitmap %s to bank %d\n\r",argv[1],bank_id);
       }
       else {
           vcom_printf( "set level bank: <bank number> <L|H> OR <0bXXXXXXXX>\n\r");
           return;
       }
 
    uint8_t bit_ctr = 9; // start counting from less significant bit.

    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= (bank_id*10)) && (G_pin_array[i].pin_id < ((bank_id*10)+10)) )
       if(G_pin_array[i].gpio_id != 255)
         {
            if(G_pin_array[i].direction != PIN_OUTPUT)
              {
                vcom_printf( "ERROR: pin %d is not set to OUTPUT\n\r",G_pin_array[i].pin_id);
                continue;
              }
            if(!bitmap) 
              {
               switch (level) {
                case PIN_LOW:
                   set_pin_low(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[CLR_ADDR]);
                   G_pin_array[i].level =  PIN_LOW;
                   break;
                case PIN_HIGH:
                   set_pin_high(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[SET_ADDR]);
                   G_pin_array[i].level =  PIN_HIGH;
                   break;
                default:
                   break;
                }
               vcom_printf( "pin %d level set to %s\n\r",G_pin_array[i].pin_id,argv[1]);
              }
            else 
             {
               switch (argv[1][bit_ctr]) {
                case '0':
                   set_pin_low(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[CLR_ADDR]);
                   G_pin_array[i].level =  PIN_LOW;
                   break;
                case '1':
                   set_pin_high(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[SET_ADDR]);
                   G_pin_array[i].level =  PIN_HIGH;
                   break;
                default:
                   continue;
                }
               vcom_printf( "pin %d level set to %c\n\r",G_pin_array[i].pin_id,argv[1][bit_ctr]);
               bit_ctr--;
             }
         }
     }
}

void cli_set_level_all(int argc, char **argv) {
    uint8_t level,i;

        if (strcmp(argv[0], "L") == 0) {
            level = PIN_LOW;
        } else if (strcmp(argv[0], "H") == 0) {
            level = PIN_HIGH;
        } else {
            vcom_printf( "set level bank: invalid level specification (should be L or H)\n\r");
            return;
        }

    for(i=0;i<PIN_COUNT;i++)
     {
      if( (G_pin_array[i].pin_id >= 10) && (G_pin_array[i].pin_id < 48) ) 
       if(G_pin_array[i].gpio_id != 255)
         {
            if(G_pin_array[i].direction != PIN_OUTPUT)
              {
                vcom_printf( "ERROR: pin %d is not set to OUTPUT\n\r",G_pin_array[i].pin_id);
                continue;
              }
            switch (level) {
               case PIN_LOW:
                   set_pin_low(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[CLR_ADDR]);
                   G_pin_array[i].level =  PIN_LOW;
                   break;
               case PIN_HIGH:
                   set_pin_high(G_pin_array[i].gpio_id,G_pin_array[i].gpio_addrs[SET_ADDR]);
                   G_pin_array[i].level =  PIN_HIGH;
                   break;
               default:
                   break;
                }
            
            vcom_printf( "pin %d level set to %s\n\r",G_pin_array[i].pin_id,argv[0]);

         }
     }
}


void set_level_bank3(uint8_t bank_id,uint8_t bitmap) {
    uint8_t bit_ctr = 9; // start counting from less significant bit
    char bin_str[11];
     
    uint8_to_binary_string(bitmap,&bin_str); 

    for(uint8_t i=(bank_id*10) ;i<((bank_id*10)+8);i++)
     {
      if( (G_pin_array[i].pin_id >= (bank_id*10)) && (G_pin_array[i].pin_id < ((bank_id*10)+10)) )
       if(G_pin_array[i].gpio_id != 255)
         {
            if(G_pin_array[i].direction != PIN_OUTPUT)
              {
                continue;
              }
                switch (bin_str[bit_ctr]) {
                case '0':
                   set_pin_low_simple(G_pin_array[i].gpio_id);
                   G_pin_array[i].level =  PIN_LOW;
                   break;

               case '1':
                   set_pin_high_simple(G_pin_array[i].gpio_id);
                   G_pin_array[i].level =  PIN_HIGH;
                   break;
                default:
                   continue;
                }
               bit_ctr--;
         }
     }
  }

void set_level_bank2(uint8_t bank_id,uint8_t bitmap) {


    // G_pin_array[pin].gpio_pin_id         - precalculated GPIO number in physical pin register (0,1,2,3)
    // G_pin_array[pin].gpio_addrs[pinval]  - precalculated physical GPIO address for bit set/clear operations (pinval can be 0 or 1)

    uint8_t pin = bank_id*10,pinval;  // we are starting from pin x0 in device bank (if bank = 1, then start from pin id 10 - id from G_pin_array)
    uint8_t mask = 0b00000001;

    void (*ops[2]) (uint32_t gpio_id, uint32_t *gpio_addr);

    ops[0] = set_pin_low;
    ops[1] = set_pin_high;

    pinval = bitmap & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 1) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 2) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 3) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 4) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 5) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 6) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 7) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_id,G_pin_array[pin].gpio_addrs[pinval]);

  }


void set_level_bank(uint8_t bank_id,uint8_t bitmap) {
  

    // G_pin_array[pin].gpio_pin_id         - precalculated GPIO number in physical pin register (FIOSET0/FIOCLEAR0,FIOSET1/FIOCLEAR,FIOSET2/FIOCLEAR2,FIOSET3/FIOCLEAR3)
    // G_pin_array[pin].gpio_addrs[pinval]  - precalculated physical GPIO address for bit set/clear operations (pinval can be 0 or 1)
 
    uint8_t pin = bank_id*10,pinval;  // we are starting from pin x0 in device bank (if bank = 1, then start from pin id 10 - id from G_pin_array)
    uint8_t mask = 0b00000001;

    void (*ops[2]) (uint32_t gpio_id, uint32_t *gpio_addr);

    ops[0] = set_pin_low_fast;
    ops[1] = set_pin_high_fast;   

    pinval = bitmap & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);
  
    pin++;
    pinval = (bitmap >> 1) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 2) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);
  
    pin++; 
    pinval = (bitmap >> 3) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 4) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 5) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 6) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);

    pin++;
    pinval = (bitmap >> 7) & mask;
    (*ops[pinval])(G_pin_array[pin].gpio_pin_id,G_pin_array[pin].gpio_addrs[pinval]);
   
  }


uint8_t read_level_bank(uint8_t bank_id) {

 uint8_t pin = bank_id*10,bitmap=0;
 
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 0);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 1);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 2);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 3);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 4);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 5);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 6);
 pin++;
 bitmap |= ((((*G_pin_array[pin].gpio_addrs[READ_ADDR] & (1<<G_pin_array[pin].gpio_pin_id)) == 0) ? 0 : 1) << 7);
 return bitmap;

}

