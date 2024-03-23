#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <cr_section_macros.h>

#include "test.h"
#include "globals.h"
#include "utils.h"


bool is_logic_oper(char *token)
 {
   if((strcmp(token,"and") == 0) ||
      (strcmp(token,"AND") == 0) ||
      (strcmp(token,"or") == 0) ||
      (strcmp(token,"OR") == 0))
       return true;
   return false;
 }


bool is_lexpr_oper(char *token)
 {
   if((strcmp(token,"&&") == 0) ||
      (strcmp(token,"||") == 0))
       return true;
   return false;
 }


pin_set_t *parse_pin_set(char *in_str, uint8_t tn, uint8_t cn)
 {
  uint8_t pin_ids[16],i = 0,j = 0;
  char token[8] = {'\0'};
  char *cursor;
  pin_set_t *pin_set;

  //vcom_printf("parse_pin_set\r\n");

  memset(&pin_ids,255,sizeof(pin_ids)); 
  cursor = in_str;

  if((cursor[0] != '[') && (cursor[strlen(in_str)-1] != ']'))
    return NULL;

  cursor++; // skip leading '['

  while(*cursor != '\0') 
   {
    if((*cursor != ',')&&(*cursor != ']'))
     {
      token[i++] = *cursor; 
      //vcom_printf(" token add: %c\r\n",*cursor);
     }
    else
     {
       //vcom_printf(" end token\r\n");
       token[i] = '\0';
       //vcom_printf(" token is: %s\r\n",token);
       pin_ids[j++] = alias_to_pin_id(token,G_test_array[tn]->pin_aliases);
       if(pin_ids[j-1] == 255) // failed to parse pin alias
         return NULL;
       i = 0;
       memset(&token,0x0,sizeof(token));
     }
    cursor++;
   }
  
  pin_set = malloc(sizeof(pin_set_t));
  memcpy(pin_set->pin_ids,&pin_ids,sizeof(pin_ids));
  pin_set->next = NULL;

  //vcom_printf(" return %X\r\n",pin_set);
  return pin_set;
  
 }


bool parse_mexpr(char *str,uint8_t tn, uint8_t cn)
 {
    char token[32];
    char chr = 0x0;
    char *expr_ls, *match_mexpr_str;
    char *equal_sign = strchr(str, '=');
    pin_set_t *pin_set = NULL,*prev_pin_set;
    bool in_token = false;

    uint8_t j = 0, i = 0;

    G_test_array[tn]->test_criteria[cn]->expression  = malloc(MAX_EXPR_LEN);
    G_test_array[tn]->test_criteria[cn]->pin_sets = NULL;

    if (equal_sign != NULL) {
         *equal_sign = '\0';
         expr_ls = str;
         match_mexpr_str = equal_sign+1;

         prev_pin_set = G_test_array[tn]->test_criteria[cn]->pin_sets = parse_pin_set(expr_ls,tn,cn);  // parse left side     

         if(prev_pin_set == NULL)
           goto free_and_return;

         while(*match_mexpr_str != '\0')
          {
           chr = *match_mexpr_str;

           if(chr == '[')
             {
               i = 0;
               token[i++] = chr;
               in_token = true; 
               //vcom_printf("start token [\r\n");
               match_mexpr_str++;
               continue;
             }
           else if(chr == ']')
             {
               //vcom_printf("end token ]\r\n");
               token[i++] = chr;
               prev_pin_set->next = parse_pin_set(token,tn,cn);
               if(prev_pin_set->next == NULL)
                 break;
               prev_pin_set = prev_pin_set->next;
               memset(&token,0x0,sizeof(token));  
               G_test_array[tn]->test_criteria[cn]->expression[j++] = '%';
               G_test_array[tn]->test_criteria[cn]->expression[j++] = 'd';
               in_token = false;
             }
           else if(in_token)
             {
              //vcom_printf("in token %c\r\n",chr);
              token[i++] = chr; 
             }
           else if(chr == ' ')
             {}
           else
            {
              //vcom_printf("any other char: %c %c\r\n",chr);
              G_test_array[tn]->test_criteria[cn]->expression[j++] = chr;
            }
           match_mexpr_str++; 
          }

         if(!in_token)
          {
            G_test_array[tn]->test_criteria[cn]->expression[j] = 0x0;
            //vcom_printf("parsed mexpr to: %s\n\r", G_test_array[tn]->test_criteria[cn]->expression);           
            pin_set = G_test_array[tn]->test_criteria[cn]->pin_sets;
            while(pin_set != NULL)
              {
               //vcom_printf("pin set \r\n");
               pin_set = pin_set->next;
              }
            return true;
          }
    }
 
    free_and_return:
 
    if(G_test_array[tn]->test_criteria[cn]->pin_sets != NULL)
      {
        prev_pin_set = G_test_array[tn]->test_criteria[cn]->pin_sets;
        while(pin_set != NULL)
          {
            pin_set = prev_pin_set->next;
            free(prev_pin_set);
            prev_pin_set = pin_set;
          }
      }
    free(G_test_array[tn]->test_criteria[cn]->expression); 

    return false;
   
 }


bool parse_match_value_expression(char *str,uint8_t tn, uint8_t cn)
 {
    char token[8];
    char chr = 0x0;
    char *expr_ls, *match_value_str;

    char *equal_sign = strchr(str, '=');
    uint8_t j = 0, ip = 0, pin_id;

    if (equal_sign != NULL) {
         *equal_sign = '\0';
         expr_ls = str;
         match_value_str = equal_sign+1;

         if (match_value_str[0] == '0' && (match_value_str[1] == 'b' || match_value_str[1] == 'B')) {
           G_test_array[tn]->test_criteria[cn]->value = strtol(match_value_str + 2, NULL, 2);  
         } else {
           G_test_array[tn]->test_criteria[cn]->value = strtol(match_value_str, NULL, 0);  
         }

         while(*expr_ls != 0x0)
          {
            chr = *expr_ls;
            if(chr=='(') {}
            else if((chr==',')||(chr==')'))
             {
               token[j] = 0x0;
               pin_id = alias_to_pin_id(token,G_test_array[tn]->pin_aliases);
               if(pin_id == 255)
                  return false;
               G_test_array[tn]->test_criteria[cn]->pin_ids[ip] = pin_id;
               ip++;
               j = 0;
             }
            else
             {
              token[j++] = chr;
             }

            expr_ls++;
          }

         return true;        

     }
    else
     {
      vcom_printf("ERROR: cannot parse match value expression: %s\n\r", str);
      return false;
     }

    return false;
 }


bool parse_counter_pins(char *str,uint8_t tn, uint8_t cn)
 {
    char token[8];
    char chr = 0x0;
    char *expr;

    uint8_t j = 0, ip = 0, pin_id;

    expr = str;

    while(*expr != 0x0)
      {
         chr = *expr;
         if(chr=='(') {}
         else if((chr==',')||(chr==')'))
           {
             token[j] = 0x0;
             pin_id = alias_to_pin_id(token,G_test_array[tn]->pin_aliases);
             if(pin_id == 255)
                return false;
             G_test_array[tn]->test_criteria[cn]->pin_ids[ip] = pin_id;
             ip++;
             j = 0;
           }
         else
           {
            token[j++] = chr;
           }

         expr++;
      }
    return true;
 }


bool parse_lexpr(char *str, char **expr, uint8_t tn, uint8_t cn) {

    char tmp_str[MAX_EXPR_LEN];
    char token[8];
    char chr = 0x0;
    char *output_pin;
    char *expr_rs;

    char *equal_sign = strchr(str, '=');
    //bool token_end = false;
    uint8_t i = 0,j = 0, ip = 0, k = 0;

    //vcom_printf(" parse lexpr, will parse: %s \r\n",str);

    if (equal_sign != NULL) {
         *equal_sign = '\0';
         output_pin = str;
         expr_rs = equal_sign+1;

         G_test_array[tn]->test_criteria[cn]->output_pin_id = alias_to_pin_id(output_pin,G_test_array[tn]->pin_aliases);

         if(G_test_array[tn]->test_criteria[cn]->output_pin_id == 255)
           return false;

         if(strlen(expr_rs)>MAX_EXPR_LEN)
           return false;

         while(*expr_rs != 0x0)
          {

            chr = *expr_rs;

            //vcom_printf(" next char: %c \r\n",chr);

            if((chr == '!')||(chr == '('))
               tmp_str[i++] = chr;
            else if(chr == ' ')               // reached end of token, previous char had to be last char of the token
             {
              if(tmp_str[i-1] == ')')
                tmp_str[i++] = ' ';
              else
               {
                //token_end = true;
                token[j] = 0x0;
                //vcom_printf(" found token: %s \r\n",token);
                if(is_lexpr_oper(token))        // logic operator
                 {
                  //vcom_printf(" logic oper\r\n");
                  k = 0;
                  while(token[k] != 0x0)
                    tmp_str[i++] = token[k++];
                  tmp_str[i++] = ' ';
                  j = 0;
                 }
                else                            // pin id
                 {
                  uint8_t pin_id = alias_to_pin_id(token,G_test_array[tn]->pin_aliases);
                  //vcom_printf(" pin id: %d \r\n",pin_id);
                  if(pin_id == 255)
                    return false;
                  G_test_array[tn]->test_criteria[cn]->pin_ids[ip] = pin_id;
                  ip++;
                  tmp_str[i++] = '%';
                  tmp_str[i++] = 'd';
                  tmp_str[i++] = ' ';
                  j = 0;
                 }
               }
             }
            else if(chr == ')')               // previous char could be either ')' or last char of the token
             {
               if(tmp_str[i-1] == ')')
                 tmp_str[i++] = chr;
               else
                {
                  //token_end = true;
                  token[j] = 0x0;
                  //vcom_printf(" found token: %s \r\n",token);
                  if(is_lexpr_oper(token))    // logic operator
                   {
                    //vcom_printf(" logic oper\r\n");
                    k = 0;
                    while(token[k] != 0x0)
                      tmp_str[i++] = token[k++];
                    tmp_str[i++] = ')';
                    j = 0;
                   }
                  else                        // pin id
                   {
                    uint8_t pin_id = alias_to_pin_id(token,G_test_array[tn]->pin_aliases);
                    //vcom_printf(" pin id: %d \r\n",pin_id);
                    if(pin_id == 255)
                      return false;
                    G_test_array[tn]->test_criteria[cn]->pin_ids[ip] = pin_id;
                    ip++;
                    tmp_str[i++] = '%';
                    tmp_str[i++] = 'd';
                    tmp_str[i++] = ')';
                    j = 0;
                   }
                }
             }
            else
             {
              //token_end = false;
              token[j++] = chr;
             }
            expr_rs++;
          }

         tmp_str[i] = 0x0;

         *expr = malloc(strlen(tmp_str)+1);
         strcpy(*expr,tmp_str);
         return true;
     }
    else
     {
      vcom_printf("ERROR: cannot parse logic expression: %s\n\r", str);
      return false;
     }

    return false;

 }



void cli_set_test_criteria(int argc, char** argv) {
    if (argc < 3) {
        vcom_printf("set test criteria <test_name> <criteria_number up to 8> <type: lexpr|mexpr|val|ctr> <expr> from-frame <frame> to-frame <frame> \r\n");
        vcom_printf("lexpr example: Y1=!((A1 && B1 && C1) || (D1 && E1 && F1)) \r\n");
        vcom_printf("mexpr example: [F0,F1,F2,F3]=[A0,A1,A2,A3]+[B0,B1,B2,B3]  WARNING: values of pin aliases starting with '~' will be inverted during evaluation of expression\r\n");
        return;
    }

    const char *criteria_type[] = {"lexpr", "val", "ctr", "mexpr"};
    char *test_name = argv[0];
    char *criteria_idx_str = argv[1];
    char *type = argv[2];
    bool new_criteria = false;
    char *endptr;

    // Find the test index
    int ti = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            ti = i;
            break;
        }
    }

    if (ti == -1) {
        vcom_printf("ERROR: test not found: %s\r\n", test_name);
        return;
    }

    uint8_t ci = strtol(criteria_idx_str, &endptr, 10);

    if (*endptr != '\0' || ci >= MAX_CRITERIA) {
        vcom_printf("ERROR: invalid criteria index: %s (maximum number of expressions is %d)\n\r", criteria_idx_str,MAX_CRITERIA);
        return;
    }

   // Check if criteria struct is already allocated
    if (G_test_array[ti]->test_criteria[ci] == NULL) {
       G_test_array[ti]->test_criteria[ci] = malloc(sizeof(test_criteria_t));
       G_test_array[ti]->test_criteria[ci]->type = 255;
       G_test_array[ti]->test_criteria[ci]->from_frame = 0;
       G_test_array[ti]->test_criteria[ci]->to_frame = 0;
       memset(G_test_array[ti]->test_criteria[ci]->pin_ids,0xFF,sizeof(G_test_array[ti]->test_criteria[ci]->pin_ids));
       new_criteria = true;
    }
    else
      new_criteria = false;

    int i=255;
    int criteria_types = sizeof(criteria_type) / sizeof(criteria_type[0]);

    for (i = 0; i < criteria_types; ++i) {
        if (strcmp(type, criteria_type[i]) == 0) {
            break;
        }
    }

    char **argv_copy = duplicate_argv(argc,argv);

    switch (i) {

        case MATCH_LEXPR: // logic expr
               {
                 G_test_array[ti]->test_criteria[ci]->type = MATCH_LEXPR;
                 char expr[MAX_EXPR_LEN];
                 expr[0] = '\0';
                 for(uint8_t i=3; i < argc; i++)
                   {
                     if(strcmp("from-frame", argv[i]) == 0)
                      {
                       if(argc > i)
                        {
                         G_test_array[ti]->test_criteria[ci]->from_frame = atoi(argv[i+1]);
                         i++;
                        }
                       else
                        {
                          vcom_printf("ERROR: malformed frame parameters.\r\n");
                          goto free_and_return;
                        }
                      }
                     else if(strcmp("to-frame", argv[i]) == 0)
                      {
                       if(argc > i)
                        { 
                         G_test_array[ti]->test_criteria[ci]->to_frame = atoi(argv[i+1]);
                         i++;
                        }
                       else
                        {
                          vcom_printf("ERROR: malformed frame parameters.\r\n");
                          goto free_and_return;
                        }
                      }
                     else
                      {
                        strcat(expr,argv[i]);
                        strcat(expr," ");
                      }
                   }
                
                 if(!new_criteria)
                   free(G_test_array[ti]->test_criteria[ci]->expression);
                 if(!parse_lexpr(expr,&G_test_array[ti]->test_criteria[ci]->expression,ti,ci))
                    {
                      vcom_printf("ERROR: failed to parse logic expression: %s \r\n", expr);
                      vcom_printf("  - check if pin aliases are valid\r\n");
                      vcom_printf("  - check if expression is not longer than %d chars\r\n",MAX_EXPR_LEN);
                      vcom_printf("  example: Y1=!((A1 && B1 && C1) || (D1 && E1 && F1)) \r\n");
                      goto free_and_return;
                    }
                   else
                    {
                     vcom_printf("logic expression saved as: %s\r\n", G_test_array[ti]->test_criteria[ci]->expression);
                    }

               }
               break;

        case MATCH_VALUE: // val
               {
                 G_test_array[ti]->test_criteria[ci]->type = MATCH_VALUE; 
                 if(!parse_match_value_expression(argv[3],ti,ci))
                    {
                      vcom_printf("ERROR: failed to parse match value criteria: %s \r\n", argv[3]);
                      vcom_printf("  - check if pin aliases are valid\r\n");
                      vcom_printf("  - check if expression is not longer than %d chars\r\n",MAX_EXPR_LEN);
                      vcom_printf("  example: (Q1,Q2,Q3,Q4)=0xF from-frame 10 to-frame 10 \r\n");
                      goto free_and_return;
                    }
                   else
                    {
                     vcom_printf("match value criteria saved\r\n");
                    }

                 if(argc == 8)
                  {
                   if(strcmp("from-frame", argv[4]) == 0)
                     G_test_array[ti]->test_criteria[ci]->from_frame = atoi(argv[5]);
                   else
                   {
                     vcom_printf("ERROR: malformed frame range parameters \r\n");
                     goto free_and_return;
                   }

                   if(strcmp("to-frame", argv[6]) == 0)
                     G_test_array[ti]->test_criteria[ci]->from_frame = atoi(argv[7]);
                   else
                   {
                     vcom_printf("ERROR: malformed frame range parameters \r\n");
                     goto free_and_return;
                   }
                 }
               }
               break;

        case MATCH_COUNTER1: // ctr
               {
                 G_test_array[ti]->test_criteria[ci]->type = MATCH_COUNTER1;
                 if(!parse_counter_pins(argv[3],ti,ci))
                    {
                      vcom_printf("ERROR: failed to parse counter pin criteria: %s \r\n", argv[3]);
                      vcom_printf("  - check if pin aliases are valid\r\n");
                      vcom_printf("  - check if expression is not longer than %d chars\r\n",MAX_EXPR_LEN);
                      vcom_printf("  example: (Q1,Q2,Q3,Q4) from-frame 10 to-frame 10 \r\n");
                      goto free_and_return;
                    }
                   else
                    {
                     vcom_printf("counter pin criteria saved\r\n");
                    }
               }
               break;

        case MATCH_MEXPR: // math
               {
                 G_test_array[ti]->test_criteria[ci]->type = MATCH_MEXPR;
                 char expr[MAX_EXPR_LEN];
                 expr[0] = '\0';
                 for(uint8_t i=3; i < argc; i++)
                   {
                     if(strcmp("from-frame", argv[i]) == 0)
                      {
                       if(argc > i)
                        {
                         G_test_array[ti]->test_criteria[ci]->from_frame = atoi(argv[i+1]);
                         i++;
                        }
                       else
                        {
                          vcom_printf("ERROR: malformed frame parameters.\r\n");
                          goto free_and_return;
                        }
                      }
                     else if(strcmp("to-frame", argv[i]) == 0)
                      {
                       if(argc > i)
                        {
                         G_test_array[ti]->test_criteria[ci]->to_frame = atoi(argv[i+1]);
                         i++;
                        }
                       else
                        {
                          vcom_printf("ERROR: malformed frame parameters.\r\n");
                          goto free_and_return;
                        }
                      }
                     else
                      {
                        strcat(expr,argv[i]);
                        strcat(expr," ");
                      }
                   }

                 if(!new_criteria)
                   {
                     free(G_test_array[ti]->test_criteria[ci]->expression);
                     if(G_test_array[ti]->test_criteria[ci]->pin_sets != NULL)
                       {
                        pin_set_t *pin_set = G_test_array[ti]->test_criteria[ci]->pin_sets;
                        pin_set_t *prev_pin_set = G_test_array[ti]->test_criteria[ci]->pin_sets;
                        while(pin_set != NULL)
                        {
                         pin_set = prev_pin_set->next;
                         free(prev_pin_set);
                         prev_pin_set = pin_set;
                        }
                       }
                   }
                 if(!parse_mexpr(expr,ti,ci))
                    {
                      vcom_printf("ERROR: failed to parse math expression: %s \r\n", expr);
                      vcom_printf("  - check if pin aliases are valid\r\n");
                      vcom_printf("  - check if expression is not longer than %d chars\r\n",MAX_EXPR_LEN);
                      vcom_printf("  example: [C1,C2,C3,C4,C5] = [A1,A2,A3,A4] + [B1,B2,B3,B4] \r\n");
                      goto free_and_return;
                    }
                   else
                    {
                     vcom_printf("math expression saved as: %s\r\n", G_test_array[ti]->test_criteria[ci]->expression);
                    }

               }
               break;


        default:
               {
                vcom_printf("ERROR: invalid criteria type\r\n");
                goto free_and_return;
               }

    }

  replace_or_append_cmd_buff("set test criteria ",argc,argv_copy);
  free_argv(argc,argv_copy);
  return;

  free_and_return:
   if(new_criteria)  
     free(G_test_array[ti]->test_criteria[ci]);
   free_argv(argc,argv_copy);
   return;

 }

