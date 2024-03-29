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
#include "test.h"
#include "state_ops.h"
#include "globals.h"
#include "utils.h"


#include "LPC17xx.h"

uint16_t cache_entries;

uint8_t G_cmd_cnt;

__DATA(RAM2) char G_command_buffer[MAX_TEST_CMDS][MAX_CMD_LEN];
__DATA(RAM2) uint8_t G_output_cache[MAX_STATES][4];  // let's keep this in AHB RAM2 (second 32KB block of SRAM available in LPC1769). First 4KB in this block is allocated by USB OTG data structures. 
__DATA(RAM2) uint8_t usb_structures[4096];            // preserve preallocated USB OTG data structures in AHB RAM2


void cli_set_test(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: set test <name|frame|frame-interval|io-settings>\n\r");
        return;
    }

    char *subcommand_set_level = argv[0];

    if (strcmp(subcommand_set_level, "name") == 0) {
        cli_set_test_name(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "frame") == 0) {
        cli_set_test_frame(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "frame-interval") == 0) {
        cli_set_test_frame_interval(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "io-settings") == 0) {
        cli_set_test_io_settings(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "pin-alias") == 0) {
        cli_set_test_pin_alias(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "criteria") == 0) {
        cli_set_test_criteria(argc - 1, argv + 1);
    } else {
        vcom_printf( "unknown subcommand for 'set test': %s\n\r", subcommand_set_level);
    }
}


// Function to set pin alias
void cli_set_test_pin_alias(int argc, char** argv) {
    if (argc != 2) {
        vcom_printf("set pin alias: <pin_number>=<pin_alias>, where pin number is: 10-17, 20-27, 30-37, 40-47, and pin_alias is string (max 4 chars).\r\n");
        return;
    }

    char* test_name = argv[0];
    char* alias_param = argv[1];
    
    // Find the test index
    int test_index = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\r\n", test_name);
        return;
    }

    char **argv_copy = duplicate_argv(argc,argv);

    // Extract pin number and alias from the parameter
    char* token = strtok(alias_param, "=");
    if (token == NULL) {
        vcom_printf("ERROR: invalid format for pin alias - use pin_number=pin_alias.\r\n");
        free_argv(argc,argv_copy);
        return;
    }

    char* pin_number_str = token;
    token = strtok(NULL, "=");
    if (token == NULL) {
        vcom_printf("ERROR: invalid format for pin alias - use pin_number=pin_alias.\r\n");
        free_argv(argc,argv_copy);
        return;
    }

    char* pin_alias = token;

    // Convert pin_number_str to uint8_t
    char* endptr;
    uint8_t pin_number = strtol(pin_number_str, &endptr, 10);

    // Check if conversion was successful and within the valid pin range
    if (*endptr != '\0' || (pin_number < 10 || pin_number > 47)) {
        vcom_printf("ERROR: invalid pin number: %s\r\n", pin_number_str);
        free_argv(argc,argv_copy);
        return;
    }

    // Check the length of pin_alias
    if (strlen(pin_alias) > 4) {
        vcom_printf("ERROR: pin alias can have a maximum of 4 characters.\r\n");
        free_argv(argc,argv_copy);
        return;
    }

    // Set pin alias in the appropriate cell of pin_aliases array
    strncpy(G_test_array[test_index]->pin_aliases[pin_number], pin_alias, 4);
    vcom_printf("pin alias set: pin %u, alias: %s \r\n", pin_number, pin_alias);

    replace_or_append_cmd_buff("set test pin-alias ",argc,argv_copy);
    free_argv(argc,argv_copy);
}


void cli_show_test(int argc, char **argv) {
    if (argc < 1) {
        vcom_printf( "usage: show test <name|frame>\n\r");
        return;
    }

    char *subcommand_set_level = argv[0];

    if (strcmp(subcommand_set_level, "name") == 0) {
        cli_show_test_name(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "frame") == 0) {
        cli_show_test_frame(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "states") == 0) {
        cli_show_test_states(argc - 1, argv + 1);
    }else {
        vcom_printf( "unknown subcommand for 'show test': %s\n\r", subcommand_set_level);
    }

}


void apply_test_frame(uint8_t *io_settings,test_frame_t *frame,uint8_t *test_states,uint8_t *test_counters)
 {
    uint8_t r_bitmap = 0;
    uint8_t use_counters[5];

    use_counters[1] = extract_bits(frame->use_counters,0,1);
    use_counters[2] = extract_bits(frame->use_counters,2,3);
    use_counters[3] = extract_bits(frame->use_counters,4,5);
    use_counters[4] = extract_bits(frame->use_counters,6,7);

    for(uint8_t i = 1; i < 5; i++)
     if(io_settings[i] == 0)
       {
         if(use_counters[i] == 0)    
          {
            set_level_bank(i,frame->bank_bitmap[i]);
            test_states[i] = frame->bank_bitmap[i];
          }
         else if(use_counters[i] == 1)  // load counter value from frame and apply to bank
          {
            test_counters[frame->counter_to_bank_assignment[i]] = frame->bank_bitmap[i];
            set_level_bank(i,frame->bank_bitmap[i]);
            test_states[i] = frame->bank_bitmap[i];
          }
         else if(use_counters[i] == 2) // increase counter value by 1 and apply to bank
          {
            test_counters[frame->counter_to_bank_assignment[i]]++;
            set_level_bank(i,test_counters[frame->counter_to_bank_assignment[i]]);
            test_states[i] = test_counters[frame->counter_to_bank_assignment[i]];
          }
       
       }
    //delayuS(1);
    for(uint8_t i = 1; i < 5; i++)
     if(io_settings[i] == 255)
       {
         r_bitmap = read_level_bank(i);
         frame->bank_bitmap[i] = r_bitmap;
         test_states[i] = r_bitmap;
       }

    frame->done = true;
 }


void apply_cached_frame(uint8_t *io_settings,uint8_t *test_states,uint16_t iter, uint8_t *read_banks, uint8_t *write_banks)
 {
    uint8_t ctr = 0;
    //SEGGER_RTT_printf(0,"apply_cached_frame: iteration %d\n",iter);

    while(write_banks[ctr] != 0x0)
     {
       //SEGGER_RTT_printf(0,"  apply_cached_frame: write %X to bank %d \n",G_output_cache[iter][write_banks[ctr]],write_banks[ctr]);
       set_level_bank(write_banks[ctr],G_output_cache[iter][write_banks[ctr]]);
       test_states[write_banks[ctr]] = G_output_cache[iter][write_banks[ctr]];
       ctr++;
     }

    ctr = 0;
    
    while(read_banks[ctr] != 0x0)
     {
       test_states[read_banks[ctr]] = read_level_bank(read_banks[ctr]);
       //SEGGER_RTT_printf(0,"  apply_cached_frame: read %X from bank %d \n",test_states[read_banks[ctr]],read_banks[ctr]);
       ctr++;
     }
 }


void cache_test_frame(uint8_t *io_settings,test_frame_t *frame,uint8_t *test_states,uint8_t *test_counters, uint16_t iter)
 {
    uint8_t use_counters[5];

    //SEGGER_RTT_printf(0,"cache_test_frame: iteration %d\n",iter);
    use_counters[1] = extract_bits(frame->use_counters,0,1);
    use_counters[2] = extract_bits(frame->use_counters,2,3);
    use_counters[3] = extract_bits(frame->use_counters,4,5);
    use_counters[4] = extract_bits(frame->use_counters,6,7);

    for(uint8_t i = 1; i < 5; i++)
     if(io_settings[i] == 0)
       {

         //SEGGER_RTT_printf(0,"  cache_test_frame: bank %d is set to output.\n",i);
         if(use_counters[i] == 0)
          {
           //SEGGER_RTT_printf(0,"  cache_test_frame: bank %d is not using counters, bitmap: %x.\n",i,frame->bank_bitmap[i]);
           G_output_cache[iter][i] = frame->bank_bitmap[i];
          }
         else if(use_counters[i] == 1)  // load counter value from frame and apply to bank
          {
            //SEGGER_RTT_printf(0,"  cache_test_frame: bank %d is setting counter %d to %d and applying to bank\n",i,frame->counter_to_bank_assignment[i],frame->bank_bitmap[i]);
            test_counters[frame->counter_to_bank_assignment[i]] = frame->bank_bitmap[i];
            G_output_cache[iter][i] = frame->bank_bitmap[i];
          }
         else if(use_counters[i] == 2) // increase counter value by 1 and apply to bank
          {
            test_counters[frame->counter_to_bank_assignment[i]]++;
            //SEGGER_RTT_printf(0,"  cache_test_frame: bank %d is incr. counter %d to %d and applying to bank\n",i,frame->counter_to_bank_assignment[i],test_counters[frame->counter_to_bank_assignment[i]]);
            G_output_cache[iter][i] = test_counters[frame->counter_to_bank_assignment[i]];
          }
         else if(use_counters[i] == 3) // take actual counter value and apply to bank
          {
            //SEGGER_RTT_printf(0,"  cache_test_frame: bank %d is applying current counter %d val %d to bank\n",i,frame->counter_to_bank_assignment[i],test_counters[frame->counter_to_bank_assignment[i]]);
            G_output_cache[iter][i] = test_counters[frame->counter_to_bank_assignment[i]];
          }

       }
    frame->done = true;
 }


void cli_run_test(int argc, char** argv) {

    char* test_name = argv[0];

    uint8_t read_banks[4];
    uint8_t write_banks[4];
    uint8_t counter_match_val[MAX_COUNTERS] = { 0 };
    uint8_t counter_to_match = 0;
    uint8_t frame_matched_counter = 0;
    bool reached_counter_match = false;
    bool in_counter_match_loop[MAX_COUNTERS] = { false };
    bool state_overflow = false;
    uint16_t loops = 0;

    // Find the test index
    int test_index = -1;
    for (uint8_t i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\n\r", test_name);
        return;
    }


    // set bank direction according to test settings
    vcom_cprintf("\e[0;36m*\e[0m running test setup...\n\r","* running test setup...\n\r");

    uint8_t r = 0,w = 0;

    for(uint8_t i = 1; i < 5; i++)
     {
      if(G_test_array[test_index]->io_settings[i] == 0)
        { 
          set_direction_bank(i,PIN_OUTPUT);
          write_banks[w++]=i;
        }
      else if(G_test_array[test_index]->io_settings[i] == 255)
        {
          set_direction_bank(i,PIN_INPUT);
          read_banks[r++]=i;
        }
     }

    write_banks[w] = read_banks[r] = 0x0;

    vcom_cprintf("\e[0;36m*\e[0m generating test data for %s...\n\r","* generating test data for %s...\n\r",G_test_array[test_index]->test_name);

    G_test_array[test_index]->iterations_done = 0;

    for (uint8_t i = 0; i < G_test_array[test_index]->frame_count; i++) {
    
     //SEGGER_RTT_printf(0,"test data gen: frame %d \n",i);
    
     if(G_test_array[test_index]->test_frames[i]->type == MATCH_COUNTER)
      frame_matched_counter = G_test_array[test_index]->test_frames[i]->bank_bitmap[1];      
 
     if(in_counter_match_loop[frame_matched_counter])
       if(G_test_array[test_index]->counters[frame_matched_counter] ==  
            counter_match_val[frame_matched_counter])
        {      
         reached_counter_match = true;
         //SEGGER_RTT_printf(0," test data gen: counter %d matched value %d\n",counter_to_match,counter_match_val);
        } 

     if(G_test_array[test_index]->test_frames[i]->type == MATCH_COUNTER)
      {
        //SEGGER_RTT_printf(0," test data gen: frame %d is MATCH_COUNTER\n",i);
        if(reached_counter_match)
         {
           //SEGGER_RTT_printf(0," test data gen: frame %d reached counter match\n",i);
           loops = 0;
           counter_match_val[frame_matched_counter] = counter_to_match = 0;
           reached_counter_match = in_counter_match_loop[frame_matched_counter] = false;
           G_test_array[test_index]->counters[frame_matched_counter] = 0;
           continue;
         }
        else
         {
          if(loops > 256) break; // this indicates that we are in a deadlock and we should break out from loop
          counter_match_val[frame_matched_counter] = 
                                     G_test_array[test_index]->test_frames[i]->bank_bitmap[2];          
          counter_to_match = frame_matched_counter;
          in_counter_match_loop[frame_matched_counter] = true;
          loops++;
          //SEGGER_RTT_printf(0," test data gen: frame %d counter match loop %d, no match, jump to frame %d\n",i,loops,G_test_array[test_index]->test_frames[i]->bank_bitmap[0]);
          i = G_test_array[test_index]->test_frames[i]->bank_bitmap[0]-1;   // counter will increase before executing next loop, therefore -1
         }
       }
     else if(G_test_array[test_index]->test_frames[i]->type == MATCH_LOOP)
      {
       //SEGGER_RTT_printf(0," test data gen: frame %d is MATCH_LOOP\n",i);
       if(loops > 256) break;
       if(loops >= G_test_array[test_index]->test_frames[i]->bank_bitmap[1])
         {
          //SEGGER_RTT_printf(0," test data gen: matched MATCH_LOOP counter, continue to next frame\n");
          loops = 0;
          continue;
         }
       else
         {
          i = G_test_array[test_index]->test_frames[i]->bank_bitmap[0]-1;
          loops++;
          //SEGGER_RTT_printf(0," test data gen: MATCH_LOOP counter, loop: %s , not matched, continue looping to frame %d\n",loops-1,G_test_array[test_index]->test_frames[i]->bank_bitmap[0]-1);
         }
       }
     else // regular frame with bank bitmaps
      {
        //SEGGER_RTT_printf(0," test data gen: frame %d is regular frame with bitmaps - generating cache data\n",i);
        G_test_array[test_index]->test_states[G_test_array[test_index]->iterations_done][0] = i;
        cache_test_frame(G_test_array[test_index]->io_settings,G_test_array[test_index]->test_frames[i],G_test_array[test_index]->test_states[G_test_array[test_index]->iterations_done],G_test_array[test_index]->counters,G_test_array[test_index]->iterations_done);
        G_test_array[test_index]->iterations_done++;
        if(G_test_array[test_index]->iterations_done > MAX_STATES)
         {
           vcom_printf("ERROR: state table overflow (%d). Aborting test.\r\n",MAX_STATES);
           state_overflow = true;
           break;
         }
        //delayuS(G_test_array[test_index]->frame_interval_ms);
       }
    }

    //SEGGER_RTT_printf(0,"test data gen: generation of test data finished\n");

    if(!state_overflow)
     {
 
      vcom_cprintf("\e[0;36m*\e[0m enabling banks...\n\r","* enabling banks...\n\r");
      
      for(uint8_t i = 1; i < 5; i++)
        set_io_bank(i,"enable");

      vcom_cprintf("\e[0;36m*\e[0m enabling DUT power...\n\r","* enabling DUT power...\n\r");
      set_dut_power("enable");

      delayMS(100);

      vcom_cprintf("\e[0;36m*\e[0m running test...\n\r","* running test...\n\r");

      uint8_t ctr = 0;

      NVIC_DisableIRQ(USB_IRQn);

      for(uint16_t k = 0; k < G_test_array[test_index]->iterations_done; k++)
        {
         
         //SEGGER_RTT_printf(0,"run test: iter %d\n",k);  
         ctr = 0;
         while(write_banks[ctr] != 0x0)
          {
           set_level_bank(write_banks[ctr],G_output_cache[k][write_banks[ctr]]);
           G_test_array[test_index]->test_states[k][write_banks[ctr]] = G_output_cache[k][write_banks[ctr]];
           //SEGGER_RTT_printf(0," run test: write %X to bank %d \n",G_output_cache[k][write_banks[ctr]],write_banks[ctr]);
           ctr++;
          }

         ctr = 0;
         while(read_banks[ctr] != 0x0)
          {
           G_test_array[test_index]->test_states[k][read_banks[ctr]] = read_level_bank(read_banks[ctr]);
           //SEGGER_RTT_printf(0," run test: read %X from bank %d \n",G_test_array[test_index]->test_states[k][read_banks[ctr]],read_banks[ctr]);
           ctr++;
          }

         //delayMS(G_test_array[test_index]->frame_interval_ms);
        }

      NVIC_EnableIRQ(USB_IRQn);

      delayMS(100);
 
      vcom_cprintf("\e[0;36m*\e[0m disabling DUT power...\n\r","* disabling DUT power...\n\r");
      set_dut_power("disable");

      vcom_cprintf("\e[0;36m*\e[0m disabling banks...\n\r","* disabling banks...\n\r");
      for(uint8_t i = 1; i < 5; i++)
        set_io_bank(i,"disable");

      vcom_cprintf("\e[0;36m*\e[0m checking test criteria...\n\r","* checking test criteria...\n\r");

      ctr = 0;
   
      while(ctr < MAX_CRITERIA)
       {
        if(G_test_array[test_index]->test_criteria[ctr] != NULL)
          check_test_criteria(test_index,ctr);
        ctr++;
       }

      if(ctr == 0)
        vcom_cprintf("\e[0;36m*\e[0m no criteria defined.\n\r","* no criteria defined.\n\r");
        
      vcom_cprintf("\e[0;36m*\e[0m test run finished (%d iterations).\n\r","* test run finished (%d iterations).\n\r",G_test_array[test_index]->iterations_done);

     }
 
}


// Function to show information about a specific test frame
void cli_show_test_frame(int argc, char** argv) {
    if (argc != 2) {
        vcom_printf("ERROR: Invalid number of arguments for show test frame.\n\r");
        return;
    }

    char binary_strings[5][11];
    char* test_name = argv[0];
    char* frame_number_str = argv[1];

    // Find the test index
    int test_index = -1;
    for (uint8_t i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\n\r", test_name);
        return;
    }

    // Convert frame_number_str to uint8_t
    char* endptr;
    uint8_t frame_number = strtol(frame_number_str, &endptr, 10);

    if (*endptr != '\0' || frame_number >= G_test_array[test_index]->frame_count) {
        vcom_printf("ERROR: invalid frame number: %s\n\r", frame_number_str);
        return;
    }


    if(G_test_array[test_index]->test_frames[frame_number]->type == MATCH_LOOP)
       vcom_printf("frame %u : loop to frame %d until %d loops\r\n",frame_number,G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[0],
                                                                    G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[1]);
    else if (G_test_array[test_index]->test_frames[frame_number]->type == MATCH_COUNTER)
      {
       char bin_str[11];
       uint8_to_binary_string(G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[2], bin_str);
       vcom_printf("frame %u : loop to frame %d until bank %d matches %s (0x%X)\r\n",frame_number,G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[0],
                                                                                     G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[1],
                                                                                     bin_str,
                                                                                     G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[2]);
      }
    else
     {
      for(uint8_t i = 1; i < 5; i++)
        uint8_to_binary_string(G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[i],binary_strings[i]);

      // Display information for the specified test frame
      vcom_printf("test name: %s | frame number: %u | ", G_test_array[test_index]->test_name, frame_number);
      vcom_printf("bank bitmap: %s %s %s %s | ",
                binary_strings[1],
                binary_strings[2],
                binary_strings[3],
                binary_strings[4]);
      vcom_printf("| done: %s\n\r", G_test_array[test_index]->test_frames[frame_number]->done ? "true" : "false");
     }
}


void dealloc_test(test_data_t **test)
 {
    for(uint8_t i = 0; i < MAX_FRAMES; i++) {
      if((*test)->test_frames[i] != NULL) 
        free((*test)->test_frames[i]);
     }

    for(uint8_t i = 0; i < MAX_CRITERIA; i++) {
      if((*test)->test_criteria[i] != NULL) 
       {
         if((*test)->test_criteria[i]->type == MATCH_MEXPR)
            free((*test)->test_criteria[i]->expression);
         free((*test)->test_criteria[i]);
       }
    }

    free(*test);
    *test = NULL;
    G_cmd_cnt = 0;

    for(uint8_t i = 0; i < MAX_TEST_CMDS; i++)
       G_command_buffer[i][0] = '\0';
 }

void cli_clear_test(int argc, char** argv) {
    if (argc != 1) {
        vcom_printf("ERROR: invalid number of arguments for clear test <test_name>\n\r");
        return;
    }

    char* test_name = argv[0];

    for (int i = 0; i < MAX_TESTS; i++) {
        if ((G_test_array[i] != NULL) && (strcmp(G_test_array[i]->test_name, test_name) == 0)) {
            dealloc_test(&G_test_array[i]);
            vcom_printf("test deleted from memory\r\n");
            return;
        }
    }
  }

// Function to set test name
void cli_set_test_name(int argc, char** argv) {
    if (argc != 1) {
        vcom_printf("ERROR: invalid number of arguments for set test name.\n\r");
        return;
    }

    char* new_test_name = argv[0];

    // Check if a test with the same name already exists
    for(int i = 0; i < MAX_TESTS; i++) {
        if (G_test_array[i] != NULL) 
            if(strcmp(G_test_array[i]->test_name, new_test_name) == 0) {
             vcom_printf("ERROR: test with name '%s' already exists.\n\r", new_test_name);
             return;
          }
    }

    // Clear test table, currently used SoC has to little memory to have more than one test active
    for (int i = 0; i < MAX_TESTS; i++) {
        if (G_test_array[i] != NULL) {
          dealloc_test(&G_test_array[i]);
        }
    }

    int available_slot = 0;

    if (available_slot != -1) {
        // Allocate a new test_data_t structure
        test_data_t* new_test = malloc(sizeof(test_data_t));

        // Set the test_name
        strcpy(new_test->test_name, new_test_name);

        // Initialize other fields if needed

        // Append the new test to the test_array
        G_test_array[available_slot] = new_test;
        
        new_test->clock_pin = 0;
        new_test->reset_pin = 0;
        new_test->frame_count = 0;
        new_test->frame_interval_ms = 0;
        new_test->iterations_done = 0;
        memset(new_test->io_settings,0,5);
        memset(new_test->test_frames,0,sizeof(new_test->test_frames));
        memset(new_test->pin_aliases,0x0,sizeof(new_test->pin_aliases));
        memset(new_test->test_criteria,0x0,sizeof(new_test->test_criteria));

        replace_or_append_cmd_buff("set test name ",argc,argv);

    } else {
        vcom_printf("ERROR: maximum number of tests reached.\n\r");
    }
}


// Function to set test frame interval
void cli_set_test_frame_interval(int argc, char** argv) {
    if (argc != 2) {
        vcom_printf("ERROR: invalid number of arguments for set test frame interval.\n\r");
        return;
    }

    char* test_name = argv[0];
    char* frame_interval_str = argv[1];

    // Check if the test exists
    int test_index = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\n\r", test_name);
        return;
    }

    // Convert frame_interval_str to uint16_t
    char* endptr;
    uint16_t frame_interval_ms = strtol(frame_interval_str, &endptr, 10);

    if (*endptr != '\0') {
        vcom_printf("ERROR: invalid frame interval value: %s\n\r", frame_interval_str);
        return;
    }
 
    // Set the frame_interval_ms in the test_data_t structure
    G_test_array[test_index]->frame_interval_ms = frame_interval_ms;
    vcom_printf("frame interval set for test %s: %u ms\n\r", test_name, frame_interval_ms);

    replace_or_append_cmd_buff("set test frame-interval ",argc,argv);
}


void cli_set_test_io_settings(int argc, char** argv) {
    if (argc != 5) {
        vcom_printf("usage: set test <name> io-settings <I|O> <I|O> <I|O> <I|O>: set banks to either Input or Output\n\r");
        return;
    }

    uint8_t io_setting;
    char* test_name = argv[0];

    // Check if the test exists
    int test_index = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s", test_name);
        return;
    }

    // Convert subsequent parameters to integers and fill in io_settings
    for (int i = 1; i <= 4; ++i) {
        if(argv[i][0] == 'I') 
          io_setting = 255;
        else if(argv[i][0] == 'O') 
          io_setting = 0;
        else
          {
            vcom_printf("ERROR: invalid I/O setting for bank %d: %s\n\r", i, argv[i]);
            return;
          }
        G_test_array[test_index]->io_settings[i] = io_setting;
    }

    vcom_printf("I/O settings set for test %s\n\r", test_name);
    replace_or_append_cmd_buff("set test io-settings ",argc,argv);
}


bool parse_loop_condition(const char *input, loop_conditions_t *conditions) {
    
    char counter_str[2];

    conditions->loop_type = NO_LOOP;
    conditions->matched_value = 0;
    conditions->counter_parameter = 0;
 
    if (strncmp(input, "loop=", 5) == 0) {
        conditions->loop_type = MATCH_LOOP;             // Set loop type to LOOP
        conditions->matched_value = atoi(input + 5);    // Extract integer value after "loop="
        return true;
    } else if ((strncmp(input, "CTR", 3) == 0) && input[3] > '0' && input[3] <= '8' && input[4] == '=') {
        conditions->loop_type = MATCH_COUNTER;             // Set loop type to COUNTER
        counter_str[0] = input[3];
        counter_str[1] = '\0';
        conditions->counter_parameter = atoi(counter_str);    // Extract integer value after "CTRx="
        conditions->matched_value = parse_uint8_string(input + 5);
        return true;
    }
    else
      vcom_printf("parse_loop_condition: incorrect loop condition params \r\n");

    return false;
}



bool handle_loop_settings(test_frame_t *frame, char *loop_param1, char *loop_param2,char  *loop_param3)
{

  if(strcmp(loop_param2, "until") == 0)
   {
     loop_conditions_t loop_conditions; 
     parse_loop_condition(loop_param3,&loop_conditions);

     if(loop_conditions.loop_type == NO_LOOP)
       return false;
     if(loop_conditions.loop_type == MATCH_LOOP) // loop count match
      {
       frame->type = MATCH_LOOP;
       frame->bank_bitmap[0] = atoi(loop_param1);
       frame->bank_bitmap[1] = loop_conditions.matched_value;
       frame->bank_bitmap[2] = 0;
       frame->bank_bitmap[3] = 0;
       frame->bank_bitmap[4] = 0;
       return true;
      }
     else if(loop_conditions.loop_type == MATCH_COUNTER) // bank match
      {
       frame->type = MATCH_COUNTER;
       frame->bank_bitmap[0] = atoi(loop_param1);
       frame->bank_bitmap[1] = loop_conditions.counter_parameter;
       frame->bank_bitmap[2] = loop_conditions.matched_value;
       frame->bank_bitmap[3] = 0;
       frame->bank_bitmap[4] = 0;
       return true;
      }
   }

  return false;
}


bool parse_bank_setting(char *arg, uint8_t *bank_bitmap, uint8_t *use_counters, uint8_t *counter_to_bank, uint8_t test_index, uint8_t frame_number, uint8_t bank_id)
 {
      uint8_t bank_setting,counter;

      char *equal_sign = strchr(arg, '=');

      if(equal_sign != NULL) // parameter in form of CTRx=0b11001100 - initialize counter x with a value and set the same value in the bank
         {
          equal_sign = 0x0;  // cut arg in two: "CTRx" and "0b00110011"
          bank_setting = parse_bank_bitmap(equal_sign+1);

          counter = atoi(arg+3);

          if((counter < 1) || (counter > 8))
            {
             vcom_printf("ERROR: invalid counter specified: %d (should be between 1 and 8)\n\r", counter);
             return false;
            }
 
          switch(bank_id) {
            case 1:
              set_bits(use_counters,0,1,1);
              break;
            case 2:
              set_bits(use_counters,2,3,1);
              break;
            case 3:
              set_bits(use_counters,4,5,1);
              break;
            case 4:
              set_bits(use_counters,6,7,1);
              break;
          }
          *counter_to_bank = counter;
         }
       else if((strlen(arg) == 5) && (arg[strlen(arg)-1] == '+'))  // parameter in form of CTRx+, where x is integer from 1 to 8 - increment counter x by 1 and set the value in the bank
         {
          arg[strlen(arg)-1] = 0x0;  // cut "+" off from "CTRx+"
          counter = atoi(arg+3);     // this should be x from "CTRx+" parameter
          arg[strlen(arg)-1] = 0x0;
          if(strcmp(arg, "CTR") != 0)
           {
            vcom_printf("ERROR: %s should be a counter specification (CTRx+), where x is 1 to 8\n\r", arg);
            return false;
           }
          if((counter < 1) || (counter > 8))
            {
             vcom_printf("ERROR: invalid counter specified: %d (should be between 1 and 8)\n\r", counter);
             return false;
            }
          switch(bank_id) {
            case 1:
              set_bits(use_counters,0,1,2);
              break;
            case 2:
              set_bits(use_counters,2,3,2);
              break;
            case 3:
              set_bits(use_counters,4,5,2);
              break;
            case 4:
              set_bits(use_counters,6,7,2);
              break;
          }
          bank_setting = 0x0;
          *counter_to_bank = counter;
         }
       else if((strlen(arg) == 4) && (arg[0] == 'C') && (arg[1] == 'T') && (arg[2] == 'R'))  // parameter in form of CTRx, where x is integer from 1 to 8 - take counter x and set it's value in the bank
         {
          counter = atoi(arg+3);     // this should be x from "CTRx" parameter
          arg[strlen(arg)-1] = 0x0;
          if(strcmp(arg, "CTR") != 0)
           {
            vcom_printf("ERROR: %s should be a counter specification (CTRx), where x is 1 to 8\n\r", arg);
            return false;
           }
          if((counter < 1) || (counter > 8))
            {
             vcom_printf("ERROR: invalid counter specified: %d (should be between 1 and 8)\n\r", counter);
             return false;
            }
          switch(bank_id) {
            case 1:
              set_bits(use_counters,0,1,3);
              break;
            case 2:
              set_bits(use_counters,2,3,3);
              break;
            case 3:
              set_bits(use_counters,4,5,3);
              break;
            case 4:
              set_bits(use_counters,6,7,3);
              break;
          }
          bank_setting = 0x0;
          *counter_to_bank = counter;
         }
       else
         {
          bank_setting = parse_bank_bitmap(arg);
          *counter_to_bank = 0x0;
         }

       if (bank_setting > 255) {
            vcom_printf("ERROR: invalid bank setting: %s\n\r", arg);
            return false;
        }

       *bank_bitmap = bank_setting;

       return true;
 }


// Function to set test frame
void cli_set_test_frame(int argc, char** argv) {
    if (argc < 6) {
        vcom_printf("set test frame <test name> <frame number> <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>\n\r");
        vcom_printf("set test frame <test name> <frame number> <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4> <pin_alias_1=L|H> ...\n\r");
        vcom_printf("set test frame <test name> <frame number> CTR1=0b11001100 <bitmap 2> <bitmap 3> <bitmap 4> : set counter 1 to 0b11001100 and also set bank 1 to the same value (CTR1 - CTR8)\r\n"); 
        vcom_printf("set test frame <test name> <frame number> CTR5+ <bitmap 2> <bitmap 3> <bitmap 4> : increment counter 5 by 1 and set the value to bank 1 (CTR1 - CTR8)\r\n");
        vcom_printf("set test frame <test name> <frame number> loop <X> until loop=Y : do Y loops to frame X. Bank counters will not be reset.\n\r");
        vcom_printf("set test frame <test name> <frame number> loop <X> until bankY=Z : loop to frame X until bank Y matches Z value. Bank will not be reset.\n\r");
        vcom_printf("set test frame <test name> <frame number> loop <X> until CTRY=Z : loop to frame X until counter Y matches Z value. Bank will not be reset.\n\r");
        return;
    }

    char* test_name = argv[0];
    char* frame_number_str = argv[1];
    bool new_frame = false;

    // Check if the test exists
    int test_index = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
            test_index = i;
            break;
        }
    }

    if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\n\r", test_name);
        return;
    }

    // Convert frame_number_str to uint8_t
    char* endptr;
    uint8_t frame_number = strtol(frame_number_str, &endptr, 10);

    if (*endptr != '\0' || frame_number >= MAX_FRAMES) {
        vcom_printf("ERROR: invalid frame number: %s\n\r", frame_number_str);
        return;
    }

    // Check if the frame is already allocated
    if (G_test_array[test_index]->test_frames[frame_number] == NULL) {
        // Allocate a new test_frame_t
        new_frame = true;
        G_test_array[test_index]->test_frames[frame_number] = malloc(sizeof(test_frame_t));
        G_test_array[test_index]->test_frames[frame_number]->done = false;
        G_test_array[test_index]->test_frames[frame_number]->use_counters = 0;
        memset(G_test_array[test_index]->test_frames[frame_number]->counter_to_bank_assignment,0x0,sizeof(G_test_array[test_index]->test_frames[frame_number]->counter_to_bank_assignment));
        G_test_array[test_index]->frame_count++;
    }

    char **argv_copy = duplicate_argv(argc,argv);

    if (strcmp(argv[2], "loop") == 0) 
     {
      if(!handle_loop_settings(G_test_array[test_index]->test_frames[frame_number],argv[3],argv[4],argv[5]))
        {
         vcom_printf("ERROR: incorrect parameters specified for loop\r\n");
         if(new_frame)
          {
            free(G_test_array[test_index]->test_frames[frame_number]);
            G_test_array[test_index]->frame_count--;
          }
         free_argv(argc,argv_copy);
         return;
        }
     
      if(G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[0] >= frame_number)  // bank_bitmap[0] in frame with loop condition is used to specify frame number to loop to. Allow only to loop to previous frames.
        {
         vcom_printf("ERROR: can loop only to previous frame\r\n");
         if(new_frame)
          {
           free(G_test_array[test_index]->test_frames[frame_number]);
           G_test_array[test_index]->frame_count--;
          }
         free_argv(argc,argv_copy);
         return;
        }
     }
    else
     G_test_array[test_index]->test_frames[frame_number]->type = NO_LOOP;
    
    // Convert subsequent parameters to uint8_t and fill in bank_bitmap
    // If bank bitmap parameter is "CTRx+", this means that we should use test counter x instead of statically set value
    // If bank bitmap is CTRx=0b01010101 , this means - load test counter x with this value and register also with the same value
    // 

    if(G_test_array[test_index]->test_frames[frame_number]->type == NO_LOOP)
      for (int i = 0; i < 4; i++) 
       {
        if(!parse_bank_setting(argv[i + 2],
                           &G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[i + 1],
                           &G_test_array[test_index]->test_frames[frame_number]->use_counters,
                           &G_test_array[test_index]->test_frames[frame_number]->counter_to_bank_assignment[i + 1],
                           test_index,frame_number,i+1))
              {
               vcom_printf("ERROR: failed to parse bank settings\r\n");
               if(new_frame)
                 {
                  free(G_test_array[test_index]->test_frames[frame_number]);
                  G_test_array[test_index]->frame_count--;
                 }
               free_argv(argc,argv_copy);
               return;
               }

        // vcom_printf("  bank %d, bitmap: %X, use_counters: %X, counter_to_bank for bank: %X\n\r", i, G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[i + 1],
        //                                            G_test_array[test_index]->test_frames[frame_number]->use_counters,
        //                                            G_test_array[test_index]->test_frames[frame_number]->counter_to_bank_assignment[i + 1]);
       }
 
    if(argc > 6)
      {
      if(!parse_pin_alias_params(argc - 6, argv + 6, 
          G_test_array[test_index]->test_frames[frame_number]->bank_bitmap, 
          G_test_array[test_index]->pin_aliases))
              vcom_printf("ERROR during parsing pin alias params, frame %u\n\r", frame_number);
      }
    vcom_printf("frame set for test %s, frame %u\n\r", test_name, frame_number);
    replace_or_append_cmd_buff("set test frame ",argc,argv_copy);
    free_argv(argc,argv_copy);
   
}


uint8_t alias_to_pin_id(char *alias, char pin_aliases[48][5])
 {
   for (uint8_t i = 0; i < MAX_ALIASES; ++i) {
      if(strlen(pin_aliases[i]) != 0)
         {
           if (strcmp(alias, pin_aliases[i]) == 0) {
              return i; 
            }
         }
       } 
   return 255;
 }


bool parse_pin_alias_params(int argc, char** argv, uint8_t *bank_bitmap, char pin_aliases[48][5])
{

  bool alias_found = false;

  for (int i = 0; i < argc; ++i) {

        char *equal_sign = strchr(argv[i], '=');

        if (equal_sign != NULL) {
            *equal_sign = '\0';  // Null-terminate at the equal sign

            // Check if the alias exists in the pin_aliases array
            int alias_index = -1;
            for (int j = 0; j < MAX_ALIASES; ++j) {
             if(strlen(pin_aliases[j]) != 0)
              {
                if (strcmp(argv[i], pin_aliases[j]) == 0) {
                   alias_index = j;
                   alias_found = true;
                   break;
                }
              }
            }

         if(!alias_found)
            vcom_printf("ERROR: alias %s is not set in this test \r\n",argv[i]);

         if (alias_index != -1) {
                // Set or clear the corresponding bit in the bank_bitmap
                int bank_number = alias_index/10;
                int bit_position = alias_index % 10;;

                if ((strcmp(equal_sign + 1, "L") == 0)||
                   (strcmp(equal_sign + 1, "0") == 0)) {
                    // Set the bit to 0 for logic low
                    bank_bitmap[bank_number] &= ~(1 << bit_position);
                } else if ((strcmp(equal_sign + 1, "H") == 0)|| 
                           (strcmp(equal_sign + 1, "1") == 0)){
                    // Set the bit to 1 for logic high
                    bank_bitmap[bank_number] |= (1 << bit_position);
                }

                // Print information for demonstration purposes
                vcom_printf("alias: %s, bank: %d, bit position: %d, value: %s\r\n", argv[i], bank_number, bit_position, equal_sign + 1);
            }
        }
      else
        vcom_printf("ERROR: bad extra parameter (%d) for \"set test frame\": %s \r\n",i,argv[i]);
    }

    return true;
}

// Function to show test aliases
void show_test_aliases(char pin_aliases[48][5]) {

    char left_alias[5];
    char right_alias[5];

    // Display aliases in two columns
    vcom_cprintf("| \e[0;36mpin\e[0m  | \e[0;36malias\e[0m   | \e[0;36mpin\e[0m     | \e[0;36malias\e[0m   |\r\n","| pin  | alias   | pin     | alias   |\r\n");
    vcom_printf("+------+---------+-------------------+\r\n");

    for (int i = 0; i < 8; ++i) {
        int left_pin = 47 - i;
        int right_pin = 20 + i;

         if (left_pin >= 40 && left_pin <= 47) {
            if(strlen(pin_aliases[left_pin]))
              strcpy(left_alias,pin_aliases[left_pin]);
            else
              strcpy(left_alias,"none");
          } 
         if (right_pin >= 20 && right_pin <= 27) {
            if(strlen(pin_aliases[right_pin]))
              strcpy(right_alias,pin_aliases[right_pin]);
            else
              strcpy(right_alias,"none");
          }
        vcom_cprintf("|\e[0;33m%-4d\e[0m  | %-4s    |   \e[0;33m%-4d\e[0m  | %-4s    |\r\n","|%-4d  | %-4s    |   %-4d  | %-4s    |\r\n", left_pin, left_alias, right_pin, right_alias);
    }

    vcom_printf("\r\n");

    for (int i = 0; i < 8; ++i) {
        int left_pin = 37 - i;
        int right_pin = 10 + i;

        if (left_pin >= 30 && left_pin <= 37) {
            if(strlen(pin_aliases[left_pin]))
              strcpy(left_alias,pin_aliases[left_pin]);
            else
              strcpy(left_alias,"none");
          } 
         if (right_pin >= 10 && right_pin <= 17) {
            if(strlen(pin_aliases[right_pin]))
              strcpy(right_alias,pin_aliases[right_pin]);
            else
              strcpy(right_alias,"none");
          } 
        vcom_cprintf("|\e[0;33m%-4d\e[0m  | %-4s    |   \e[0;33m%-4d\e[0m  | %-4s    |\r\n","|%-4d  | %-4s    |   %-4d  | %-4s    |\r\n", left_pin, left_alias, right_pin, right_alias);
    }

  vcom_printf("+------+---------+-------------------+\r\n");

}


void display_aliased_pin_header(char pin_aliases[MAX_ALIASES][5]) {
    vcom_printf("+- %-4s -+", "Frame");

    for (int i = 0; i < MAX_ALIASES; ++i) {
        if (strlen(pin_aliases[i]) > 0) {
            vcom_printf("-+- %-4s -+", pin_aliases[i]);
        }
    }
    vcom_printf("-+\n\r");
}


void display_aliased_pins_state(test_data_t *test_data, uint16_t state_index) 
{
    vcom_printf("|%-5d    ", state_index);

    for (int i = 0; i < MAX_ALIASES; ++i) {
        if (strlen(test_data->pin_aliases[i]) > 0) {
            uint8_t bank_number = i / 10;
            uint8_t pin_in_bank = i % 10;
            uint8_t state = (test_data->test_states[state_index][bank_number] >> (pin_in_bank)) & 0x01;
            vcom_printf("|   %-5s  ", (state == 1) ? "H" : "L");
        }
    }
    vcom_printf("|\n\r");
}


void display_aliased_pin_footer(char pin_aliases[MAX_ALIASES][5]) {
    vcom_printf("+---------+");

    for (int i = 0; i < MAX_ALIASES; ++i) {
        if (strlen(pin_aliases[i]) > 0) {
            vcom_printf("-+--------+", pin_aliases[i]);
        }
    }
    vcom_printf("-+\n\r");
}


#define OUTPUT_BANK_COLOR "\e[0;33m"
#define INPUT_BANK_COLOR "\e[0;36m"

void bank_state_to_colored_string(uint8_t test_index, uint16_t frame_index, uint8_t bank, char *str)
 {
   if(G_test_array[test_index]->io_settings[bank] == 0)
     uint8_to_colored_binary_string(G_test_array[test_index]->test_states[frame_index][bank],OUTPUT_BANK_COLOR,str);
   else
     uint8_to_colored_binary_string(G_test_array[test_index]->test_states[frame_index][bank],INPUT_BANK_COLOR,str);
 }

void cli_show_test_states(int argc, char** argv){

   char* test_name = argv[0];
   bool aliased_only = false;

   if (argc == 0) {
        vcom_printf("show test states <test_name> : show pin states recorded during test\n\r", test_name);
        vcom_printf("show test states <test_name> aliased-only: show only pins with aliases \n\r", test_name);
        return;
    }

   if((argc == 2) && (strcmp(argv[1], "aliased-only") == 0))
    {
     aliased_only = true;
     vcom_printf("args %d, %s, %s \n\r", argc, argv[0],argv[1]);
    }

   char bank_1[40];
   char bank_2[40];
   char bank_3[40];
   char bank_4[40];
   char msg[80];

   // Check if the test exists
   int test_index = -1;

   for (int i = 0; i < MAX_TESTS; ++i) {
       if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
           test_index = i;
           break;
       }
   }

   if (test_index == -1) {
        vcom_printf("ERROR: test not found: %s\n\r", test_name);
        return;
    }

   if(!aliased_only)
    {
     if(G_use_color)
      {
        sprintf(msg,"\r\nTester input bank - %s###\e[0m , Tester output bank - %s###\e[0m\r\n",INPUT_BANK_COLOR,OUTPUT_BANK_COLOR);
        vcom_printf("%s \r\n",msg);
      }  

     vcom_printf("                                BANK1      BANK2      BANK3      BANK4        B1   B2   B3   B4 \r\n");
     for(uint16_t i = 0; i < G_test_array[test_index]->iterations_done; i++)
      {
       if(G_use_color)
        {
         bank_state_to_colored_string(test_index,i,1,bank_1);
         bank_state_to_colored_string(test_index,i,2,bank_2);
         bank_state_to_colored_string(test_index,i,3,bank_3);
         bank_state_to_colored_string(test_index,i,4,bank_4);
        }
       else
        {
         uint8_to_binary_string(G_test_array[test_index]->test_states[i][1], bank_1);
         uint8_to_binary_string(G_test_array[test_index]->test_states[i][2], bank_2);
         uint8_to_binary_string(G_test_array[test_index]->test_states[i][3], bank_3);
         uint8_to_binary_string(G_test_array[test_index]->test_states[i][4], bank_4);
        }
        vcom_printf("iteration: %4d (frame: %3d): %s %s %s %s | ",i,G_test_array[test_index]->test_states[i][0],bank_1,bank_2,bank_3,bank_4);  
        vcom_printf("0x%02X 0x%02X 0x%02X 0x%02X |\r\n",G_test_array[test_index]->test_states[i][1],
                                              G_test_array[test_index]->test_states[i][2],
                                              G_test_array[test_index]->test_states[i][3],
                                              G_test_array[test_index]->test_states[i][4]);
      }
    }
   else
    {
     for(uint16_t i = 0; i < G_test_array[test_index]->iterations_done; i++)
      {
       if((i % 20) == 0)
         {
          display_aliased_pin_header(G_test_array[test_index]->pin_aliases);
         }
       display_aliased_pins_state(G_test_array[test_index],i);
      }
     display_aliased_pin_footer(G_test_array[test_index]->pin_aliases);
    }

}


// Function to show information about defined tests
void cli_show_test_name(int argc, char** argv) {

    char binary_strings[5][11];
    uint8_t use_counters[5];    

    if (argc > 0) {
        // Show detailed information for a specific test
        char* test_name = argv[0];

        // Find the test index
        int test_index = -1;
        for (int i = 0; i < MAX_TESTS; ++i) {
            if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, test_name) == 0) {
                test_index = i;
                break;
            }
        }

        if (test_index == -1) {
            vcom_printf("ERROR: test not found: %s\n\r", test_name);
            return;
        }

        if(argc == 2)
          if(strcmp(argv[1],"commands") == 0)
           {
            display_command_buffer();
            return;
           }        

        // Display detailed information for the specified test
        vcom_cprintf("test name: \e[0;36m%s\e[0m\n\r","test name: %s\n\r", G_test_array[test_index]->test_name);
        vcom_printf("frame count: %u\n\r", G_test_array[test_index]->frame_count);
        vcom_printf("frame interval (ms): %u\n\r", G_test_array[test_index]->frame_interval_ms);
        vcom_printf("\n\r--------- aliases --------------------\r\n");
        show_test_aliases(G_test_array[test_index]->pin_aliases);
        vcom_printf("\n\r--------- frames ---------------------\r\n");
        vcom_printf("I/O settings:                     %c                 %c                 %c                %c\n\r",  
                                                 (G_test_array[test_index]->io_settings[1] == 255) ? 'I' : 'O', 
                                                 (G_test_array[test_index]->io_settings[2] == 255) ? 'I' : 'O',
                                                 (G_test_array[test_index]->io_settings[3] == 255) ? 'I' : 'O',
                                                 (G_test_array[test_index]->io_settings[4] == 255) ? 'I' : 'O');
        // Display information for each defined frame
        if(G_test_array[test_index]->frame_count == 0)
          {
            vcom_printf("no frames defined \r\n\r\n");
            return;
          }

        for (int i = 0; i < G_test_array[test_index]->frame_count; ++i) {

            if(G_test_array[test_index]->test_frames[i]->type == MATCH_LOOP)
               vcom_printf("frame %u : loop to frame %d until %d loops\r\n",i,G_test_array[test_index]->test_frames[i]->bank_bitmap[0],
                                                                            G_test_array[test_index]->test_frames[i]->bank_bitmap[1]);
            else if (G_test_array[test_index]->test_frames[i]->type == MATCH_COUNTER)
              {
                char bin_str[11];
                uint8_to_binary_string(G_test_array[test_index]->test_frames[i]->bank_bitmap[2], bin_str);
                vcom_printf("frame %u : loop to frame %d until counter %d matches %s (0x%X)\r\n",i,G_test_array[test_index]->test_frames[i]->bank_bitmap[0],
                                                                                              G_test_array[test_index]->test_frames[i]->bank_bitmap[1],
                                                                                              bin_str,
                                                                                              G_test_array[test_index]->test_frames[i]->bank_bitmap[2]);
              }
            else
             {
               use_counters[1] = extract_bits(G_test_array[test_index]->test_frames[i]->use_counters,0,1);
               use_counters[2] = extract_bits(G_test_array[test_index]->test_frames[i]->use_counters,2,3);
               use_counters[3] = extract_bits(G_test_array[test_index]->test_frames[i]->use_counters,4,5);
               use_counters[4] = extract_bits(G_test_array[test_index]->test_frames[i]->use_counters,6,7);

               for(uint8_t j = 1; j< 5; j++)
                  uint8_to_binary_string(G_test_array[test_index]->test_frames[i]->bank_bitmap[j],binary_strings[j]);

               vcom_printf("frame %u : bank bitmap: ",i);

               for(uint8_t k = 1; k < 5; k++)
                 if(use_counters[k] == 0)
                    vcom_printf("      %s  ",binary_strings[k]);
                 else if(use_counters[k] == 1)
                    vcom_printf(" CTR%d=%s  ",G_test_array[test_index]->test_frames[i]->counter_to_bank_assignment[k],binary_strings[k]);
                 else if(use_counters[k] == 2)
                    vcom_printf("        CTR%d+     ",G_test_array[test_index]->test_frames[i]->counter_to_bank_assignment[k]);
                 else if(use_counters[k] == 3)
                    vcom_printf("        CTR%d      ",G_test_array[test_index]->test_frames[i]->counter_to_bank_assignment[k]);
 
               vcom_printf("|");
               vcom_printf(" done: %s\n\r", G_test_array[test_index]->test_frames[i]->done ? "true" : "false");
             }
           
        }

      vcom_printf("\n\r--------- criteria ------------------\r\n");

      for(uint8_t j = 0; j < MAX_CRITERIA; j++)
        if(G_test_array[test_index]->test_criteria[j] != NULL)
          {
            vcom_printf("%d: ",j);
           switch (G_test_array[test_index]->test_criteria[j]->type)
             {
               case MATCH_MEXPR: 
                   vcom_printf("match math expression\n\r");
                   break;
               case MATCH_VALUE:
                   vcom_printf("match value\n\r");
                   break;
               case MATCH_COUNTER1:
                   vcom_printf("match counter\n\r");
                   break;
             }

          }

    } 
    else {
        // Show a list of defined tests with general information
        uint8_t crit_no = 0;
        for (int i = 0; i < MAX_TESTS; ++i) {
            if (G_test_array[i] != NULL) {

               for(uint8_t j = 0; j < MAX_CRITERIA; j++) 
                 if(G_test_array[i]->test_criteria[j] != NULL)
                    crit_no++;

                vcom_printf("test name: %s | frame count: %u | I/O settings: %u %u %u %u | frame interval (ms): %u | criteria %d |\n\r",
                            G_test_array[i]->test_name, G_test_array[i]->frame_count,
                            G_test_array[i]->io_settings[1], G_test_array[i]->io_settings[2],
                            G_test_array[i]->io_settings[3], G_test_array[i]->io_settings[4],
                            G_test_array[i]->frame_interval_ms, crit_no);
                              
            }
        }
    }
}


