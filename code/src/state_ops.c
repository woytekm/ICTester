#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "test.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "ceval/ceval.h"


// get pin level from state frame by pin ID
uint8_t get_plfsa(uint8_t pin_id,uint8_t state[5])
 {
   uint8_t pin_level;
   uint8_t bank,pin;

   bank = pin_id / 10;
   pin  = pin_id % 10;

   pin_level = state[bank] & (1<<pin);

   if(pin_level > 0)
     return 1;
   else return 0;
 }


// get pin level from state frame by pin alias
uint8_t get_aplfsa(char *pin_alias,char pin_aliases[48][5],uint8_t state[5])
 {
   uint8_t pin_level;
   uint8_t bank,pin,bank_pin;

   bank_pin = alias_to_pin_id(pin_alias,pin_aliases);

   if(bank_pin == 255)
     return 255;    

   bank = bank_pin / 10;
   pin  = bank_pin % 10;

   pin_level = state[bank] & (1<<pin);

   if(pin_level > 0)
     return 1;
   else return 0;
 }


bool check_test_criteria(uint8_t tn, uint8_t cn)
 {

  uint16_t i,to_frame;
  uint8_t logic_result;
  
  char expr[MAX_EXPR_LEN];

  if(G_test_array[tn] == NULL)
    {
      vcom_printf("ERROR: this test is not inialized.\r\n");
      return false;
    }

  if(G_test_array[tn]->iterations_done == 0)
    {
      vcom_printf("ERROR: this test has empty state array (no interations).\r\n");
      return false;
    }


  switch (G_test_array[tn]->test_criteria[cn]->type)
   {

        #define MAX_EXPR_VALS 17

        case MATCH_MEXPR: // math expression
               {
                uint16_t values[MAX_EXPR_VALS];
                uint8_t j = 0, k = 0, bitval;
                pin_set_t *pin_set;
                int16_t result_int16_t;
                uint16_t result = 0, result_bits = 0;

                if(G_test_array[tn]->test_criteria[cn]->to_frame != 0)
                     to_frame = G_test_array[tn]->test_criteria[cn]->to_frame;
                else
                     to_frame = G_test_array[tn]->iterations_done-1;

                // get values from pins in state frame
                for(i = G_test_array[tn]->test_criteria[cn]->from_frame; i<= to_frame; i++)
                 {

                  memset(&values,0,sizeof(values));

                  pin_set = G_test_array[tn]->test_criteria[cn]->pin_sets;

                  if(pin_set == NULL)
                    return false;

                  j = result_bits = 0;
                  while( (pin_set->pin_ids[j] != 0xFF) && (j < MAX_VAL_BITS) )  // count bits in result value
                   {
                    result_bits++;
                    j++;
                   }

                  while(pin_set != NULL)  // assemble all values from bits
                   {    
                    j = 0;
                    while( (pin_set->pin_ids[j] != 0xFF) && (j < MAX_VAL_BITS) )
                      {
                        bitval = get_plfsa(pin_set->pin_ids[j],G_test_array[tn]->test_states[i]);
                        if(G_test_array[tn]->pin_aliases[pin_set->pin_ids[j]][0] == '~')
                         {
                          if(bitval == 1) bitval = 0;
                          else bitval = 1;
                         }

                        values[k] |= (bitval << j);
                        j++;
                      }
                    k++;
                    pin_set = pin_set->next;
                   }

                  // calculate expression value (values[1] -> max values), values[0] is output value to match (first pin set in criteria)
                  sprintf(expr,G_test_array[tn]->test_criteria[cn]->expression,values[1],values[2],values[3],values[4],values[5],values[6],
                                                                              values[7],values[8],values[9],values[10],values[11],values[12],
                                                                              values[13],values[14],values[15],values[16]);     // this statically depends on MAX_EXPR_VALS length being more than 17 
                  result_int16_t = ceval_result(expr);
                  result = (uint16_t)result_int16_t;
                  result = result & (uint16_t)(pow(2,result_bits)-1);  // clear unnecessary bits from result

                  if(result != values[0])
                      {
                       vcom_cprintf("\e[0;31m    - test criteria %d failed at state frame %d\r\n\e[0m","    - test criteria %d failed at state frame %d\r\n",cn,i);
                       vcom_printf("       state frame: %d\r\n",i);
                       vcom_printf("       value is 0x%X, and should be 0x%X\r\n",values[0],result);
                       led_signal_test_fail();
                       return false;
                      }
                  k = 0;
                 }

                vcom_cprintf("\e[0;32m    + test criteria %d passed\r\n\e[0m","    + test criteria %d passed\r\n",cn);
                led_signal_test_ok();
                return true;
               }
               break;


        case MATCH_LEXPR: // lexpr
               {
                 if(G_test_array[tn]->test_criteria[cn]->to_frame != 0)
                     to_frame = G_test_array[tn]->test_criteria[cn]->to_frame;
                 else
                     to_frame = G_test_array[tn]->iterations_done-1;
 
                 for(i = G_test_array[tn]->test_criteria[cn]->from_frame; i<= to_frame; i++)
                  { 
                    logic_result = 255;
                    sprintf(expr,G_test_array[tn]->test_criteria[cn]->expression, 
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[0],G_test_array[tn]->test_states[i]),   // read pin level for pin_ids[0] (id deciphered from entered expression), for state frame i
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[1],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[2],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[3],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[4],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[5],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[6],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[7],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[8],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[9],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[10],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[11],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[12],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[13],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[14],G_test_array[tn]->test_states[i]),
                            get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[15],G_test_array[tn]->test_states[i]));
                    logic_result = (uint8_t)ceval_result(expr);

                    if(logic_result == 255)
                     {
                      vcom_printf("ERROR: logic expression evaluation failed\r\n");
                      return false;
                     }
                    if(logic_result != get_plfsa(G_test_array[tn]->test_criteria[cn]->output_pin_id,G_test_array[tn]->test_states[i]))
                     {
                      vcom_cprintf("\e[0;31m    - test criteria %d failed at state frame %d\r\n\e[0m","    - test criteria %d failed at state frame %d\r\n",cn,i);
                      vcom_printf("       state frame: %d\r\n",i);
                      vcom_printf("       eval: %s to %d\r\n",expr,logic_result);
                      vcom_printf("       output pin id: %d : %d\r\n",G_test_array[tn]->test_criteria[cn]->output_pin_id,get_plfsa(G_test_array[tn]->test_criteria[cn]->output_pin_id,G_test_array[tn]->test_states[i]));
                      led_signal_test_fail();
                      return false;
                     }
       
                  }
                 vcom_cprintf("\e[0;32m    + test criteria %d passed\r\n\e[0m","    + test criteria %d passed\r\n",cn);
                 led_signal_test_ok();
                 return true;
               }
               break;

        case MATCH_VALUE: // val
               {
                uint16_t value = 0;
                uint8_t j = 0,bitval;

                if(G_test_array[tn]->test_criteria[cn]->to_frame != 0)
                     to_frame = G_test_array[tn]->test_criteria[cn]->to_frame;
                else
                     to_frame = G_test_array[tn]->iterations_done-1;

                for(i = G_test_array[tn]->test_criteria[cn]->from_frame; i<= to_frame; i++)
                 {
                  while(((G_test_array[tn]->test_criteria[cn]->pin_ids[j] != 0xFF)) && (j < 16))
                    {
                      bitval = get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[j],G_test_array[tn]->test_states[i]);    
                      value |= (bitval << j);
                      j++;
                    }
                  if(value != G_test_array[tn]->test_criteria[cn]->value)
                      {
                      vcom_cprintf("\e[0;31m    - test criteria %d failed at state frame %d\r\n\e[0m","    - test criteria %d failed at state frame %d\r\n",cn,i);
                      vcom_printf("       state frame: %d\r\n",i);
                      vcom_printf("       value is 0x%X, and should be 0x%X\r\n",value,G_test_array[tn]->test_criteria[cn]->value);
                      led_signal_test_fail();
                      return false;
                     }
                 }

                vcom_cprintf("\e[0;32m    + test criteria %d passed\r\n\e[0m","    + test criteria %d passed\r\n",cn);
                led_signal_test_ok();
                return true;
               }
               break;

        case MATCH_COUNTER1: // ctr
               {

                uint16_t value = 0, value_prev = 0;
                uint8_t j = 0,bitval;
                bool first_pass = true;

                if(G_test_array[tn]->test_criteria[cn]->to_frame != 0)
                     to_frame = G_test_array[tn]->test_criteria[cn]->to_frame;
                else
                     to_frame = G_test_array[tn]->iterations_done-1;

                for(i = G_test_array[tn]->test_criteria[cn]->from_frame; i<= to_frame; i++)
                 {
                  while(((G_test_array[tn]->test_criteria[cn]->pin_ids[j] != 0xFF)) && (j < 16))
                    {
                      bitval = get_plfsa(G_test_array[tn]->test_criteria[cn]->pin_ids[j],G_test_array[tn]->test_states[i]);
                      value |= (bitval << j);
                      j++;
                    }
                  if(!first_pass)
                    {
                      if(value>0) // if value == 0 then there was a counter overflow
                       if(value != (value_prev-1))
                        {
                            vcom_cprintf("\e[0;31m    - test criteria %d failed at state frame %d\r\n\e[0m","    - test criteria %d failed at state frame %d\r\n",cn,i);
                            vcom_printf("       state frame: %d\r\n",i);
                            vcom_printf("       current counter val: 0x%X, prev counter val: 0x%X \r\n",value,value_prev);
                            led_signal_test_fail();
                            return false;
                        }
                    }
                  value_prev = value;
                  first_pass = false;
                 }

                vcom_cprintf("\e[0;32m    + test criteria %d passed\r\n\e[0m","    + test criteria %d passed\r\n",cn);
                led_signal_test_ok();
                return true;

               }
               break;

        default:
               {
                vcom_printf("ERROR: invalid criteria type (0x%X), cannot perform criteria checks!!\r\n",G_test_array[tn]->test_criteria[cn]->type);
                return false;
               }

   }

   return false;

 }
