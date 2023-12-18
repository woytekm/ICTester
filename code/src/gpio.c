#include <stdint.h>
#include "LPC17xx.h"
#include "gpio_pin.h"
#include "gpio.h"

uint8_t G_gpio_map[] = {
    [1] = PIN_1_OE,
    [10] = PIN_10,  
    [11] = PIN_11,
    [12] = PIN_12,
    [13] = PIN_13,
    [14] = PIN_14,
    [15] = PIN_15,
    [16] = PIN_16,
    [17] = PIN_17,
    [2] = PIN_2_OE,
    [20] = PIN_20,
    [21] = PIN_21,
    [22] = PIN_22,
    [23] = PIN_23,
    [24] = PIN_24,
    [25] = PIN_25,
    [26] = PIN_26,
    [27] = PIN_27,
    [30] = PIN_30,
    [31] = PIN_31,
    [32] = PIN_32,
    [33] = PIN_33,
    [34] = PIN_34,
    [35] = PIN_35,
    [36] = PIN_36,
    [37] = PIN_37,
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
};


void init_pin_array(void) {
    for (uint8_t i = 1; i < sizeof(G_gpio_map) / sizeof(G_gpio_map[0]); ++i) {
        uint8_t pin_id = i;
        uint8_t gpio_id = G_gpio_map[pin_id];

        G_pin_array[pin_id].pin_id = pin_id;
        G_pin_array[pin_id].gpio_id = gpio_id;
        G_pin_array[pin_id].direction = PIN_OUTPUT;
        G_pin_array[pin_id].level = PIN_LOW;
    }
}

void init_pins(void)
{
   
    uint8_t bank,pin;

    // set ethernet pins to GPIO: 
    LPC_PINCON->PINSEL2 = 0x0;

    // set all pins used for tester I/O to FUNC0 - GPIO, pullup inactive
    for(uint8_t i = 1; i < sizeof(G_gpio_map) / sizeof(G_gpio_map[0]); ++i) 
      {
        bank = G_gpio_map[i]/100;
        pin = G_gpio_map[i] - bank * 100;
        Chip_IOCON_PinMux(LPC_IOCON, bank, pin, IOCON_MODE_INACT, IOCON_FUNC0);
      }

    // set all pins used for tester I/O to output/LOW
    for (uint8_t i = 1; i < sizeof(G_gpio_map) / sizeof(G_gpio_map[0]); ++i) {
        uint8_t pin_id = i;
        
        if(G_pin_array[pin_id].direction == PIN_OUTPUT)
         {
          set_pin_write(G_pin_array[pin_id].gpio_id);
          if(G_pin_array[pin_id].level == PIN_LOW)
            set_pin_low(G_pin_array[pin_id].gpio_id);
          else if (G_pin_array[pin_id].level == PIN_HIGH)
            set_pin_high(G_pin_array[pin_id].gpio_id);      
         }
        else if (G_pin_array[pin_id].direction == PIN_INPUT)
          set_pin_read(G_pin_array[pin_id].gpio_id);
    }
}

void init_leds(void)
{
 set_pin_write(PIN_GREEN_LED); 
 set_pin_write(PIN_RED_LED);
 set_pin_write(PIN_YELLOW_LED);

 set_pin_high(PIN_GREEN_LED);
 set_pin_high(PIN_RED_LED);
 set_pin_high(PIN_YELLOW_LED);

 SEGGER_RTT_printf(0,"PINSEL0: %X\n",LPC_PINCON->PINSEL0 );
 SEGGER_RTT_printf(0,"PINSEL1: %X\n",LPC_PINCON->PINSEL1 );
 SEGGER_RTT_printf(0,"PINSEL2: %X\n",LPC_PINCON->PINSEL2 );
 SEGGER_RTT_printf(0,"PINSEL3: %X\n",LPC_PINCON->PINSEL3 );
 SEGGER_RTT_printf(0,"PINSEL4: %X\n",LPC_PINCON->PINSEL4 );
 SEGGER_RTT_printf(0,"PINSEL5: %X\n",LPC_PINCON->PINSEL5 );


}

void set_pin_write(uint16_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIODIR |= 1 << (uint8_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIODIR |= 1 << (uint8_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIODIR |= 1 << (uint8_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIODIR |= 1 << (uint8_t)(bank_pin);
 }

void set_pin_read(uint16_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIODIR &= ~(1 << (uint8_t)(bank_pin - 300));
   else if(bank_pin>200)
     LPC_GPIO2->FIODIR &= ~(1 << (uint8_t)(bank_pin - 200));
   else if(bank_pin>100)
     LPC_GPIO1->FIODIR &= ~(1 << (uint8_t)(bank_pin - 100));
   else
     LPC_GPIO0->FIODIR &= ~(1 << (uint8_t)(bank_pin));
 }


void set_pin_high(uint16_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIOSET = 1 << (uint8_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIOSET = 1 << (uint8_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIOSET = 1 << (uint8_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIOSET = 1 << (uint8_t)(bank_pin);
 }


void set_pin_low(uint16_t bank_pin)
 {
   if(bank_pin>300)
     LPC_GPIO3->FIOCLR = 1 << (uint8_t)(bank_pin - 300);
   else if(bank_pin>200)
     LPC_GPIO2->FIOCLR = 1 << (uint8_t)(bank_pin - 200);
   else if(bank_pin>100)
     LPC_GPIO1->FIOCLR = 1 << (uint8_t)(bank_pin - 100);
   else
     LPC_GPIO0->FIOCLR = 1 << (uint8_t)(bank_pin);
 }


uint8_t get_pin(uint16_t bank_pin)
 {
   uint8_t pin_state;
   
   if(bank_pin>300)
     pin_state = LPC_GPIO0->FIOPIN & (1<<(bank_pin - 300));
   else if(bank_pin>200)
     pin_state = LPC_GPIO0->FIOPIN & (1<<(bank_pin - 200));
   else if(bank_pin>100)
     pin_state = LPC_GPIO0->FIOPIN & (1<<(bank_pin - 100));
   else
     pin_state = LPC_GPIO0->FIOPIN & (1<<bank_pin);

  return pin_state;
 }

