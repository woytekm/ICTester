#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "test.h"
#include "gpio.h"
#include "gpio_pin.h"


char *logic_eval( char *expr, uint8_t *res ){
  enum { LEFT, OP1, MID, OP2, RIGHT } state = LEFT;
  enum { AND, OR, XOR } op = AND;
  int mid=0, NEG=0;
  uint8_t tmp = 0;

  for( ; ; expr++, state++, NEG=0 ){
    for( ;; expr++ )
         if( *expr == '!'     ) NEG = !NEG;
         else if( *expr != ' '     ) break;

    if( *expr == '0'     ){ tmp  =  NEG; }
    else if( *expr == '1'     ){ tmp  = !NEG; }
    else if( *expr == 'A'     ){ op   = AND; expr+=2; }
    else if( *expr == '&'     ){ op   = AND; expr+=1; }
    else if( *expr == 'O'     ){ op   = OR;  expr+=1; }
    else if( *expr == '|'     ){ op   = OR;  expr+=1; }
    else if( *expr == '('     ){ expr = logic_eval( expr+1, &tmp ); if(NEG) tmp=!tmp; }
    else if( *expr == '\0' ||
            *expr == ')'     ){ if(state == OP2) *res |= mid; return expr; }

    if( state == LEFT               ){ *res  = tmp;               }
    else if( state == MID   && op == OR  ){  mid  = tmp;               }
    else if( state == MID   && op == AND ){ *res &= tmp; state = LEFT; }
    else if( state == OP2   && op == OR  ){ *res |= mid; state = OP1;  }
    else if( state == RIGHT              ){  mid &= tmp; state = MID;  }
  }
}

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
  
  char logic_expr[MAX_EXPR_LEN];

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

        case MATCH_EXPRESSION: // expr
               {
                 if(G_test_array[tn]->test_criteria[cn]->to_frame != 0)
                     to_frame = G_test_array[tn]->test_criteria[cn]->to_frame;
                 else
                     to_frame = G_test_array[tn]->iterations_done-1;
 
                 for(i = G_test_array[tn]->test_criteria[cn]->from_frame; i<= to_frame; i++)
                  { 
                    logic_result = 255;
                    sprintf(logic_expr,G_test_array[tn]->test_criteria[cn]->logic_expression, 
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
                    logic_eval(logic_expr,&logic_result);
                    if(logic_result == 255)
                     {
                      vcom_printf("ERROR: logic expression evaluation failed\r\n");
                      return false;
                     }
                    if(logic_result != get_plfsa(G_test_array[tn]->test_criteria[cn]->output_pin_id,G_test_array[tn]->test_states[i]))
                     {
                      vcom_printf("    - test criteria %d failed at state frame %d\r\n",cn,i);
                      vcom_printf("       state frame: %d\r\n",i);
                      vcom_printf("       eval: %s to %d\r\n",logic_expr,logic_result);
                      vcom_printf("       output pin id: %d : %d\r\n",G_test_array[tn]->test_criteria[cn]->output_pin_id,get_plfsa(G_test_array[tn]->test_criteria[cn]->output_pin_id,G_test_array[tn]->test_states[i]));
                      return false;
                     }
       
                  }
                 vcom_printf("    + test criteria %d passed\r\n",cn);
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
                      vcom_printf("    - test criteria %d failed at state frame %d\r\n",cn,i);
                      vcom_printf("       state frame: %d\r\n",i);
                      vcom_printf("       value is 0x%X, and should be 0x%X\r\n",value,G_test_array[tn]->test_criteria[cn]->value);
                      return false;
                     }
                 }

                vcom_printf("    + test criteria %d passed\r\n",cn);
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
                            vcom_printf("    - test criteria %d failed at state frame %d\r\n",cn,i);
                            vcom_printf("       state frame: %d\r\n",i);
                            vcom_printf("       current counter val: 0x%X, prev counter val: 0x%X \r\n",value,value_prev);
                            return false;
                        }
                    }
                  value_prev = value;
                  first_pass = false;
                 }

                vcom_printf("    + test criteria %d passed\r\n",cn);
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
