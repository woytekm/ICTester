#include <stdint.h>
#include <string.h>
#include "LPC17xx.h"
//#include "iocon_17xx_40xx.h"
#include "gpio_pin.h"
#include "gpio.h"
#include "timers.h"
#include "SEGGER_RTT.h"
#include "stdbool.h"
#include "test.h"
#include "utils.h"


uint8_t G_gpio_map[] = {
    [0] = 255,
    [1] = PIN_1_OE,
    [10] = PIN_10,  
    [11] = PIN_11,
    [12] = PIN_12,
    [13] = PIN_13,
    [14] = PIN_14,
    [15] = PIN_15,
    [16] = PIN_16,
    [17] = PIN_17,
    [18] = 255,
    [19] = 255,
    [2] = PIN_2_OE,
    [20] = PIN_20,
    [21] = PIN_21,
    [22] = PIN_22,
    [23] = PIN_23,
    [24] = PIN_24,
    [25] = PIN_25,
    [26] = PIN_26,
    [27] = PIN_27,
    [28] = 255,
    [29] = 255,
    [30] = PIN_30,
    [31] = PIN_31,
    [32] = PIN_32,
    [33] = PIN_33,
    [34] = PIN_34,
    [35] = PIN_35,
    [36] = PIN_36,
    [37] = PIN_37,
    [38] = 255,
    [39] = 255,
    [3] = PIN_3_OE,
    [40] = PIN_40,
    [41] = PIN_41,
    [42] = PIN_42,
    [43] = PIN_43,
    [44] = PIN_44,
    [45] = PIN_45,
    [46] = PIN_46,
    [47] = PIN_47,
    [4] = PIN_4_OE,
    [5] = PIN_1_DIR,
    [6] = PIN_2_DIR,
    [7] = PIN_3_DIR,
    [8] = PIN_4_DIR,
    [9] = 255
};

uint8_t G_clock_pin,G_clock_state;

pin G_pin_array[PIN_COUNT];

void init_pin_array(void) {

    uint32_t gpio_bank_addrs[5] = {0x2009C000,0x2009C020,0x2009C040,0x2009C060,0x2009C080};
    uint8_t gpio_id;

    for (uint8_t i = 0; i < PIN_COUNT; ++i) {
        gpio_id = G_gpio_map[i];
        if(gpio_id != 255)
         {
          uint8_t bank = gpio_id / 100;

          G_pin_array[i].gpio_addrs[0] = (uint32_t *)(gpio_bank_addrs[bank] + 0x1C); // CLR (set 0)
          G_pin_array[i].gpio_addrs[1] = (uint32_t *)(gpio_bank_addrs[bank] + 0x18); // SET (set 1)
          G_pin_array[i].gpio_addrs[2] = (uint32_t *)(gpio_bank_addrs[bank] + 0x14); // PIN (read)
          G_pin_array[i].pin_id = i;
          G_pin_array[i].gpio_id = gpio_id;
          G_pin_array[i].gpio_pin_id = gpio_id % 100;
          G_pin_array[i].direction = PIN_INPUT;
          G_pin_array[i].level = PIN_LOW;
         }
    }

}


void init_leds(void)
{
 set_pin_write(PIN_GREEN_LED); 
 set_pin_write(PIN_RED_LED);
 set_pin_write(PIN_YELLOW_LED);

 set_pin_high_simple(PIN_GREEN_LED);
 set_pin_high_simple(PIN_RED_LED);
 set_pin_high_simple(PIN_YELLOW_LED);

 delayMS(200);
 set_pin_low_simple(PIN_GREEN_LED);
 delayMS(200);
 set_pin_low_simple(PIN_RED_LED);
 delayMS(200);
 set_pin_low_simple(PIN_YELLOW_LED);
}

void led_signal_test_fail(void)
{
 for(uint8_t i = 0; i < 5; i++)
  {
    set_pin_high_simple(PIN_RED_LED);
    delayMS(110);
    set_pin_low_simple(PIN_RED_LED);
    delayMS(110);
  }
}

void led_signal_test_ok(void)
{
 for(uint8_t i = 0; i < 5; i++)
  {
    set_pin_high_simple(PIN_GREEN_LED);
    delayMS(110);
    set_pin_low_simple(PIN_GREEN_LED);
    delayMS(110);
  }
}


void set_pin_write(uint32_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIODIR |= 1 << (uint32_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIODIR |= 1 << (uint32_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIODIR |= 1 << (uint32_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIODIR |= 1 << (uint32_t)(bank_pin);
 }

void set_pin_read(uint32_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIODIR &= ~(1 << (uint32_t)(bank_pin - 300));
   else if(bank_pin>200)
     LPC_GPIO2->FIODIR &= ~(1 << (uint32_t)(bank_pin - 200));
   else if(bank_pin>100)
     LPC_GPIO1->FIODIR &= ~(1 << (uint32_t)(bank_pin - 100));
   else
     LPC_GPIO0->FIODIR &= ~(1 << (uint32_t)(bank_pin));

   set_pin_low_simple(bank_pin);
 }

void set_pin_high_simple(uint32_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIOSET = 1 << (uint32_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIOSET = 1 << (uint32_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIOSET = 1 << (uint32_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIOSET = 1 << (uint32_t)(bank_pin);
 }


void set_pin_low_simple(uint32_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIOCLR = 1 << (uint32_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIOCLR = 1 << (uint32_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIOCLR = 1 << (uint32_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIOCLR = 1 << (uint32_t)(bank_pin);
 }


void set_pin_high(uint32_t bank_pin, uint32_t *gpio_set_address)
 {
   uint32_t pin = bank_pin % 100;
   *gpio_set_address = 1 << pin;
 }


void set_pin_low(uint32_t bank_pin, uint32_t *gpio_clr_address)
 {
   uint32_t pin = bank_pin % 100;
   *gpio_clr_address = 1 << pin;
 }

void set_pin_high_fast(uint32_t pin, uint32_t *gpio_set_address)
 {
   *gpio_set_address = 1 << pin;
 }


void set_pin_low_fast(uint32_t pin, uint32_t *gpio_clr_address)
 {
   *gpio_clr_address = 1 << pin;
 }


void toggle_pin(uint32_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIOPIN ^= 1 << (uint32_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIOPIN ^= 1 << (uint32_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIOPIN ^= 1 << (uint32_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIOPIN ^= 1 << (uint32_t)(bank_pin);
 }

uint32_t get_pin(uint32_t bank_pin)
 {
   uint32_t pin_state = 0;
   
   if(bank_pin>300)
     pin_state = LPC_GPIO3->FIOPIN & (1<<(bank_pin - 300));
   else if(bank_pin>200)
     pin_state = LPC_GPIO2->FIOPIN & (1<<(bank_pin - 200));
   else if(bank_pin>100)
     pin_state = LPC_GPIO1->FIOPIN & (1<<(bank_pin - 100));
   else
     pin_state = LPC_GPIO0->FIOPIN & (1<<bank_pin);

  if(pin_state > 0)
    return 1;
  else return 0;
 }


uint32_t get_pin_aliased(char *pin_alias,char pin_aliases[48][5])
 {

   uint8_t pin_state = 0,bank_pin;
   int alias_index = -1;

   for (int j = 0; j < MAX_ALIASES; ++j) {
     if(strlen(pin_aliases[j]) != 0)
       {
         if (strcmp(pin_alias, pin_aliases[j]) == 0) {
            alias_index = j;
            break;
         }
      }
    }
   
   bank_pin = G_gpio_map[alias_index];

   if(bank_pin>300)
     pin_state = LPC_GPIO3->FIOPIN & (1<<(bank_pin - 300));
   else if(bank_pin>200)
     pin_state = LPC_GPIO2->FIOPIN & (1<<(bank_pin - 200));
   else if(bank_pin>100)
     pin_state = LPC_GPIO1->FIOPIN & (1<<(bank_pin - 100));
   else
     pin_state = LPC_GPIO0->FIOPIN & (1<<bank_pin);

  if(pin_state > 0)
    return 1;
  else return 0;
 }



uint32_t get_pin_fast(uint32_t pin, uint32_t *fiopin_addr)
 {
  return (((*fiopin_addr & (1<<pin)) == 0) ? 0 : 1);
 }


uint32_t get_pin_direction(uint32_t bank_pin)
 {
   uint32_t pin_direction = 0;

   if(bank_pin>300)
     pin_direction = LPC_GPIO3->FIODIR & (1<<(bank_pin - 300));
   else if(bank_pin>200)
     pin_direction = LPC_GPIO2->FIODIR & (1<<(bank_pin - 200));
   else if(bank_pin>100)
     pin_direction = LPC_GPIO1->FIODIR & (1<<(bank_pin - 100));
   else
     pin_direction = LPC_GPIO0->FIODIR & (1<<bank_pin);

  if(pin_direction > 0)
    return 1;
  else return 0;
 }


//void gpio_testfunc(void)
// {
//   uint8_t i,j;
//
//   init_pin_array(); // initialize analyzer pin data structures
//   init_pins();      // initialize all pins to default state
//   init_timers();
//   init_leds();
//   
//   set_pin_write(G_pin_array[10].gpio_id);
//   
//   while(1)
//    {
//     set_pin_low(G_pin_array[10].gpio_id);
//     for(i = 0; i < 2; i++)
//      {j = i;}
//     set_pin_high(G_pin_array[10].gpio_id);
//     for(i = 0; i < 2; i++)
//      {j = i;}
//    }
// }


void init_pins(void)
{

    uint8_t bank,pin;

    // set ethernet pins to GPIO:
    LPC_PINCON->PINSEL2 = 0x0;

    // set all pins used for tester I/O to FUNC0 - GPIO, pulldown enabled (if input)
    for(uint8_t i = 10; i < PIN_COUNT; i++)
      {
        bank = G_gpio_map[i]/100;
        pin = G_gpio_map[i] - bank * 100;
        Chip_IOCON_PinMux(LPC_IOCON, bank, pin, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
      }

    // Init [0]25 (DUT power control) separately
    Chip_IOCON_PinMux(LPC_IOCON, 1, 25, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(125);
    set_pin_low_simple(125);

    // Init OE pins and set them to output/high
    Chip_IOCON_PinMux(LPC_IOCON, 0, 15, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_1_OE);
    set_pin_high_simple(PIN_1_OE);

    Chip_IOCON_PinMux(LPC_IOCON, 0, 4, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_2_OE);
    set_pin_high_simple(PIN_2_OE);

    Chip_IOCON_PinMux(LPC_IOCON, 1, 26, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_3_OE);
    set_pin_high_simple(PIN_3_OE);

    Chip_IOCON_PinMux(LPC_IOCON, 1, 21, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_4_OE);
    set_pin_high_simple(PIN_4_OE);

    // Init DIR pins and set them to output/low  (B -> A)
    Chip_IOCON_PinMux(LPC_IOCON, 2, 1, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_1_DIR);
    set_pin_low_simple(PIN_1_DIR);

    Chip_IOCON_PinMux(LPC_IOCON, 1, 1, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_2_DIR);
    set_pin_low_simple(PIN_2_DIR);

    Chip_IOCON_PinMux(LPC_IOCON, 0, 22, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_3_DIR);
    set_pin_low_simple(PIN_3_DIR);

    Chip_IOCON_PinMux(LPC_IOCON, 1, 20, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
    set_pin_write(PIN_4_DIR);
    set_pin_low_simple(PIN_4_DIR);
    
    // Init SPI pins for uSD card
    Chip_IOCON_PinMux(LPC_IOCON, 0, 9, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 8, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 7, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 6, IOCON_MODE_INACT, IOCON_FUNC0); // SSEL is normal GPIO pin

    // set all pins used for tester I/O to input/LOW
    for (uint8_t i = 1; i < sizeof(G_gpio_map) / sizeof(G_gpio_map[0]); ++i) {
        uint8_t pin_id = i;

        // skip OE pins for banks - they are already set up 
        if((i > 4) || (i < 9 ))
          continue;

        if(G_pin_array[pin_id].direction == PIN_OUTPUT)
         {
          set_pin_write(G_pin_array[pin_id].gpio_id);
          if(G_pin_array[pin_id].level == PIN_LOW)
            set_pin_low_simple(G_pin_array[pin_id].gpio_id);
          else if (G_pin_array[pin_id].level == PIN_HIGH)
            set_pin_high_simple(G_pin_array[pin_id].gpio_id);
         }
        else if (G_pin_array[pin_id].direction == PIN_INPUT)
         {
          set_pin_read(G_pin_array[pin_id].gpio_id);
          set_pin_low_simple(G_pin_array[pin_id].gpio_id);
         }
    }
    
}


