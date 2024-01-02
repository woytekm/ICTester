#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "test.h"


// Helper function to parse and convert bank bitmap from string to uint8_t
uint8_t parse_bank_bitmap(const char* bitmap_str) {
    // Check if the string starts with "0b" or "0B"
    if (bitmap_str[0] == '0' && (bitmap_str[1] == 'b' || bitmap_str[1] == 'B')) {
        return strtol(bitmap_str + 2, NULL, 2);  // Convert binary string
    } else {
        return strtol(bitmap_str, NULL, 0);  // Automatically detect base (hex or decimal)
    }
}

void uint8_to_binary_string(uint8_t value, char* binary_str) {
    // Ensure the binary_str is large enough to store the binary representation
    snprintf(binary_str, 10, "0b");
    
    // Convert each bit to '0' or '1' and append to the string
    for (int i = 7; i >= 0; --i) {
        binary_str[2 + (7 - i)] = (value & (1 << i)) ? '1' : '0';
    }

    // Null-terminate the string
    binary_str[10] = '\0';
}

// Sets specified bits (bit1 and bit2) in the given byte to the specified value
void set_bits(uint8_t *byte, uint8_t bit1, uint8_t bit2, uint8_t value) {
    if (bit1 > 7 || bit2 > 7 || bit1 > bit2 || value > 3) {
        // Invalid bit positions or value
        return;
    }

    // Calculate the mask to modify the specified bits
    uint8_t mask = 0x0;
    mask |= (1 << bit1);
    mask |= (1 << bit2);
    // Clear the specified bits
    (*byte) &= ~mask;
    // Set the specified bits to the new value
    (*byte) |= (value << bit1);

}

uint8_t extract_bits(uint8_t byte, uint8_t bit1, uint8_t bit2) {
    if (bit1 > 7 || bit2 > 7 || bit1 > bit2) {
        // Invalid bit positions
        return 0;
    }

    // Calculate the mask to extract the specified bits
    uint8_t mask = ((1 << (bit2 - bit1 + 1)) - 1) << bit1;

    // Use bit manipulation to extract the bits
    return (byte & mask) >> bit1;
}


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

    // Extract pin number and alias from the parameter
    char* token = strtok(alias_param, "=");
    if (token == NULL) {
        vcom_printf("ERROR: invalid format for pin alias - use pin_number=pin_alias.\r\n");
        return;
    }

    char* pin_number_str = token;
    token = strtok(NULL, "=");
    if (token == NULL) {
        vcom_printf("ERROR: invalid format for pin alias - use pin_number=pin_alias.\r\n");
        return;
    }

    char* pin_alias = token;

    // Convert pin_number_str to uint8_t
    char* endptr;
    uint8_t pin_number = strtol(pin_number_str, &endptr, 10);

    // Check if conversion was successful and within the valid pin range
    if (*endptr != '\0' || (pin_number < 10 || pin_number > 47)) {
        vcom_printf("ERROR: invalid pin number: %s\r\n", pin_number_str);
        return;
    }

    // Check the length of pin_alias
    if (strlen(pin_alias) > 4) {
        vcom_printf("ERROR: pin alias can have a maximum of 4 characters.\r\n");
        return;
    }

    // Set pin alias in the appropriate cell of pin_aliases array
    strncpy(G_test_array[test_index]->pin_aliases[pin_number], pin_alias, 4);
    vcom_printf("pin alias set: pin %u, alias: %s \r\n", pin_number, pin_alias);
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



void apply_test_frame(uint8_t *io_settings,test_frame_t *frame,uint8_t *test_states,uint8_t *bank_counters)
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
            bank_counters[i] = frame->bank_bitmap[i];
            set_level_bank(i,frame->bank_bitmap[i]);
            test_states[i] = frame->bank_bitmap[i];
          }
         else if(use_counters[i] == 2) // increase counter value by 1 and apply to bank
          {
            bank_counters[i]++;
            set_level_bank(i,bank_counters[i]);
            test_states[i] = bank_counters[i];
          }
       }
     else if(io_settings[i] == 255)
       {
         //delayMS(1); // this will not work well when frame interval will be less than few ms (TODO)
         r_bitmap = read_level_bank(i);
         frame->bank_bitmap[i] = r_bitmap;
         test_states[i] = r_bitmap;
       }

    frame->done = true;
 }

void cli_run_test(int argc, char** argv) {

    char* test_name = argv[0];

    char *args[3];
    char dir[2];
    char bank[2];
    char cmd1[10];
    char cmd2[10];

    uint8_t bank_counters[5];
    uint8_t bank_match_val = 0;
    uint8_t bank_match_number = 0;
    bool reached_bank_match = false;
    bool in_bank_match_loop = false;
    uint16_t loops = 0;

    uint8_t _argc;

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
    vcom_printf("* running test setup...\r\n");

    for(uint8_t i = 1; i < 5; i++)
     {
      _argc = 2;
      sprintf(bank,"%d",i);
      args[0] = (char *)&bank;

      if(G_test_array[test_index]->io_settings[i] == 0)
        { 
          strcpy(dir,"O");
          args[1] = (char *)&dir;
          cli_set_direction_bank(_argc,args);
        }
      else if(G_test_array[test_index]->io_settings[i] == 255)
        {
          strcpy(dir,"I");
          args[1] = (char *)&dir;
          cli_set_direction_bank(_argc,args);
        }
     }

    vcom_printf("* enabling banks...\r\n");
    for(uint8_t i = 1; i < 5; i++)
     {
      _argc = 3;
      sprintf(bank,"%d",i);
      args[1] = (char *)&bank;
      sprintf(cmd1,"bank");
      args[0] = (char *)&cmd1;
      sprintf(cmd2,"enable");
      args[2] = (char *)&cmd2;
      cli_set_io(_argc,args);
     }

    vcom_printf("* enabling DUT power...\r\n");
    _argc = 1;
    sprintf(cmd1,"enable");
    args[0] = (char *)&cmd1;
    cli_set_dut_power(_argc,args);
 
    delayMS(1000); 

    vcom_printf("* running test %s...\r\n",G_test_array[test_index]->test_name);

    G_test_array[test_index]->iterations_done = 0;

    for (uint8_t i = 0; i < G_test_array[test_index]->frame_count; i++) {
   
    if(in_bank_match_loop)
     if(bank_counters[bank_match_number] == bank_match_val)
       reached_bank_match = true;
 
    if(G_test_array[test_index]->test_frames[i]->type == MATCH_BANK)
      {
        if(reached_bank_match)
         {
           loops = 0;
           bank_match_val = bank_match_number = 0;
           reached_bank_match = in_bank_match_loop = false;
           bank_counters[G_test_array[test_index]->test_frames[i]->bank_bitmap[1]] = 0;
           continue;
         }
        else
         {
          if(loops > 256) break; // this indicates that we are in a deadlock and we should break out from loop
          bank_match_val = G_test_array[test_index]->test_frames[i]->bank_bitmap[2];          
          bank_match_number = G_test_array[test_index]->test_frames[i]->bank_bitmap[1];
          in_bank_match_loop = true;
          loops++;
          i = G_test_array[test_index]->test_frames[i]->bank_bitmap[0]-1;
         }
      }
    else if(G_test_array[test_index]->test_frames[i]->type == MATCH_LOOP)
      {
       if(loops > 256) break;
       if(loops >= G_test_array[test_index]->test_frames[i]->bank_bitmap[1])
         {
          loops = 0;
          continue;
         }
       else
         {
          i = G_test_array[test_index]->test_frames[i]->bank_bitmap[0]-1;
          loops++;
         }
      }
     else // regular frame with bank bitmaps
      {
        G_test_array[test_index]->test_states[G_test_array[test_index]->iterations_done][0] = i;
        apply_test_frame(G_test_array[test_index]->io_settings,G_test_array[test_index]->test_frames[i],G_test_array[test_index]->test_states[G_test_array[test_index]->iterations_done],bank_counters);
        G_test_array[test_index]->iterations_done++;
        if(G_test_array[test_index]->iterations_done > MAX_STATES)
         {
           vcom_printf("ERROR: state table overflow (%d). Aborting test.\r\n",MAX_STATES);
           break;
         }
        delayMS(G_test_array[test_index]->frame_interval_ms);
       }
    }
 
    delayMS(1000);
 
    vcom_printf("* disabling DUT power...\r\n");
    _argc = 1;
    sprintf(cmd1,"disable");
    args[0] = (char *)&cmd1;
    cli_set_dut_power(_argc,args);

    vcom_printf("* disabling banks...\r\n");
    for(uint8_t i = 1; i < 5; i++)
     {
      _argc = 3;
      sprintf(bank,"%d",i);
      args[1] = (char *)&bank;
      sprintf(cmd1,"bank");
      args[0] = (char *)&cmd1;
      sprintf(cmd2,"disable");
      args[2] = (char *)&cmd2;
      cli_set_io(_argc,args);
     }

    vcom_printf("* test run finished.\r\n");
 
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
    else if (G_test_array[test_index]->test_frames[frame_number]->type == MATCH_BANK)
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



// Function to set test name
void cli_set_test_name(int argc, char** argv) {
    if (argc != 1) {
        printf("ERROR: invalid number of arguments for set test name.\n\r");
        return;
    }

    char* new_test_name = argv[0];

    // Check if a test with the same name already exists
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] != NULL && strcmp(G_test_array[i]->test_name, new_test_name) == 0) {
            printf("ERROR: test with name '%s' already exists.\n\r", new_test_name);
            return;
        }
    }

    // Find the first available slot in the test_array
    int available_slot = -1;
    for (int i = 0; i < MAX_TESTS; ++i) {
        if (G_test_array[i] == NULL) {
            available_slot = i;
            break;
        }
    }

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

        printf("test created: %s\n\r", new_test_name);
    } else {
        printf("ERROR: maximum number of tests reached.\n\r");
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
}


void cli_set_test_io_settings(int argc, char** argv) {
    if (argc != 5) {
        vcom_printf("usage: set test <name> io-settings <0|255> <0|255> <0|255> <0|255>, where 0 is all pins in bank = OUT, 255 - all pins in bank = IN\n\r");
        return;
    }

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
        char* endptr;
        uint8_t io_setting = strtol(argv[i], &endptr, 10);

        if (*endptr != '\0' || (io_setting != 0 && io_setting != 255)) {
            vcom_printf("ERROR: invalid I/O setting for bank %d: %s\n\r", i, argv[i]);
            return;
        }

        G_test_array[test_index]->io_settings[i] = io_setting;
    }

    vcom_printf("I/O settings set for test %s\n\r", test_name);
}


bool parse_loop_condition(const char *input, loop_conditions_t *conditions) {
    
    char bank_str[2];

    conditions->loop_type = NO_LOOP;
    conditions->matched_value = 0;
    conditions->bank_parameter = 0;
 
    if (strncmp(input, "loop=", 5) == 0) {
        conditions->loop_type = MATCH_LOOP;  // Set loop type to LOOP
        conditions->matched_value = atoi(input + 5);  // Extract integer value after "loop="
        return true;
    } else if (strncmp(input, "bank", 4) == 0 && input[4] >= '0' && input[4] <= '9' && input[5] == '=') {
        conditions->loop_type = MATCH_BANK;  // Set loop type to BANK
        bank_str[0] = input[4];
        bank_str[1] = '\0';
        conditions->bank_parameter = atoi(bank_str);  // Extract integer value after "bankX="
        conditions->matched_value = atoi(input + 6);
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
     else if(loop_conditions.loop_type == MATCH_BANK) // bank match
      {
       frame->type = MATCH_BANK;
       frame->bank_bitmap[0] = atoi(loop_param1);
       frame->bank_bitmap[1] = loop_conditions.bank_parameter;
       frame->bank_bitmap[2] = loop_conditions.matched_value;
       frame->bank_bitmap[3] = 0;
       frame->bank_bitmap[4] = 0;
       return true;
      }
   }

  return false;
}


// Function to set test frame
void cli_set_test_frame(int argc, char** argv) {
    if (argc != 6) {
        vcom_printf("set test frame <test name> <frame number> <bitmap 1> <bitmap 2> <bitmap 3> <bitmap 4>\n\r");
        vcom_printf("set test frame <test name> <frame number> loop <X> until loop=Y. Do Y loops to frame X. Bank counters will not be reset.\n\r");
        vcom_printf("set test frame <test name> <frame number> loop <X> until bankY=Z. Loop to frame X until bank Y matches Z value. Bank will not be reset.\n\r");
        return;
    }

    char* test_name = argv[0];
    char* frame_number_str = argv[1];

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
        G_test_array[test_index]->test_frames[frame_number] = malloc(sizeof(test_frame_t));
        G_test_array[test_index]->test_frames[frame_number]->done = false;
        G_test_array[test_index]->test_frames[frame_number]->use_counters = 0;
        G_test_array[test_index]->frame_count++;
    }

    if (strcmp(argv[2], "loop") == 0) 
     {
      if(!handle_loop_settings(G_test_array[test_index]->test_frames[frame_number],argv[3],argv[4],argv[5]))
        {
         vcom_printf("ERROR: incorrect parameters specified for loop\r\n");
         // we don't know if this is a new frame or existing one, let's leave it alone and return
         return;
        }
     
      if(G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[0] >= frame_number)  // bank_bitmap[0] in frame with loop condition is used to specify frame number to loop to. Allow only to loop to previous frames.
        {
         vcom_printf("ERROR: can loop only to previous frame\r\n");
         // we don't know if this is a new frame or existing one, let's leave it alone and return
         return;
        }
     }
    else
     G_test_array[test_index]->test_frames[frame_number]->type = NO_LOOP;
    
    uint8_t bank_setting;

    // Convert subsequent parameters to uint8_t and fill in bank_bitmap
    // If bank bitmap parameter is "+", this means that we should use bank_counter instead of statically set value
    // If bank bitmap is 0bXXXXXXXX+ , this means - load bank counter with this value and register also with the same value

    if(G_test_array[test_index]->test_frames[frame_number]->type == NO_LOOP)
      for (int i = 0; i < 4; i++) {

        if(strcmp(argv[i + 2], "+") == 0)
         {
          bank_setting = 0x0;
          switch(i + 2) {
            case 2:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,0,1,2);
              break;
            case 3:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,2,3,2);
              break;
            case 4:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,4,5,2);
              break;
            case 5:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,6,7,2);
              break;
          }
         }
        else if(argv[i + 2][strlen(argv[i + 2])-1] == '+')
         {
          bank_setting = parse_bank_bitmap(argv[i + 2]);
          switch(i + 2) {
            case 2:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,0,1,1);
              break;
            case 3:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,2,3,1);
              break;
            case 4:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,4,5,1);
              break;
            case 5:
              set_bits(&G_test_array[test_index]->test_frames[frame_number]->use_counters,6,7,1);
              break;
          }
         }
        else
         {
          bank_setting = parse_bank_bitmap(argv[i + 2]);
         }

        if (bank_setting > 255) {
            vcom_printf("ERROR: invalid bank setting: %s\n\r", argv[i + 2]);
            return;
        }
        G_test_array[test_index]->test_frames[frame_number]->bank_bitmap[i + 1] = bank_setting;
    }

    vcom_printf("frame set for test %s, frame %u\n\r", test_name, frame_number);
}

// Function to show test aliases
void show_test_aliases(uint8_t test_index) {
    if (test_index >= 8 || G_test_array[test_index] == NULL) {
        vcom_printf("Error: Invalid test index.");
        return;
    }

    test_data_t* test = G_test_array[test_index];

    char left_alias[5];
    char right_alias[5];

    // Display aliases in two columns
    vcom_printf("| pin  | alias   | pin     | alias   |\r\n");
    vcom_printf("+------+---------+-------------------+\r\n");

    for (int i = 0; i < 8; ++i) {
        int left_pin = 47 - i;
        int right_pin = 20 + i;

         if (left_pin >= 40 && left_pin <= 47) {
            if(strlen(test->pin_aliases[left_pin]))
              strcpy(left_alias,test->pin_aliases[left_pin]);
            else
              strcpy(left_alias,"none");
          } 
         if (right_pin >= 20 && right_pin <= 27) {
            if(strlen(test->pin_aliases[right_pin]))
              strcpy(right_alias,test->pin_aliases[right_pin]);
            else
              strcpy(right_alias,"none");
          }

        vcom_printf("|%-4d  | %-4s    |   %-4d  | %-4s    |\r\n", left_pin, left_alias, right_pin, right_alias);
    }

    vcom_printf("\r\n");

    for (int i = 0; i < 8; ++i) {
        int left_pin = 37 - i;
        int right_pin = 10 + i;

        if (left_pin >= 30 && left_pin <= 37) {
            if(strlen(test->pin_aliases[left_pin]))
              strcpy(left_alias,test->pin_aliases[left_pin]);
            else
              strcpy(left_alias,"none");
          } 
         if (right_pin >= 10 && right_pin <= 17) {
            if(strlen(test->pin_aliases[right_pin]))
              strcpy(right_alias,test->pin_aliases[right_pin]);
            else
              strcpy(right_alias,"none");
          } 

        vcom_printf("|%-4d  | %-4s    |   %-4d  | %-4s    |\r\n", left_pin, left_alias, right_pin, right_alias);
    }

  vcom_printf("+------+---------+-------------------+\r\n");

}

void cli_show_test_states(int argc, char** argv){

   char* test_name = argv[0];
   char bank_1[11];
   char bank_2[11];
   char bank_3[11];
   char bank_4[11];

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

   for(uint16_t i = 0; i < G_test_array[test_index]->iterations_done; i++)
    {
     uint8_to_binary_string(G_test_array[test_index]->test_states[i][1], bank_1);
     uint8_to_binary_string(G_test_array[test_index]->test_states[i][2], bank_2);
     uint8_to_binary_string(G_test_array[test_index]->test_states[i][3], bank_3);
     uint8_to_binary_string(G_test_array[test_index]->test_states[i][4], bank_4);
     vcom_printf("iteration: %4d (frame: %3d): %s %s %s %s | ",i,G_test_array[test_index]->test_states[i][0],bank_1,bank_2,bank_3,bank_4);  
     vcom_printf(" 0x%02X 0x%02X 0x%02X 0x%02X |\r\n",G_test_array[test_index]->test_states[i][1],
                                           G_test_array[test_index]->test_states[i][2],
                                           G_test_array[test_index]->test_states[i][3],
                                           G_test_array[test_index]->test_states[i][4]);
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

        // Display detailed information for the specified test
        vcom_printf("test name: %s\n\r", G_test_array[test_index]->test_name);
        vcom_printf("frame count: %u\n\r", G_test_array[test_index]->frame_count);
        vcom_printf("frame interval (ms): %u\n\r", G_test_array[test_index]->frame_interval_ms);
        vcom_printf("\n\r--------- aliases --------------------\r\n");
        show_test_aliases(test_index);
        vcom_printf("\n\r--------- frames ---------------------\r\n");
        vcom_printf("I/O settings:                %c            %c            %c            %c\n\r",  
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
            else if (G_test_array[test_index]->test_frames[i]->type == MATCH_BANK)
              {
                char bin_str[11];
                uint8_to_binary_string(G_test_array[test_index]->test_frames[i]->bank_bitmap[2], bin_str);
                vcom_printf("frame %u : loop to frame %d until bank %d matches %s (0x%X)\r\n",i,G_test_array[test_index]->test_frames[i]->bank_bitmap[0],
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
                    vcom_printf(" %s  ",binary_strings[k]);
                 else if(use_counters[k] == 1)
                    vcom_printf(" %s+ ",binary_strings[k]);
                 else if(use_counters[k] == 2)
                    vcom_printf("      +      ",binary_strings[k]);
 
               vcom_printf("|");
               vcom_printf(" done: %s\n\r", G_test_array[test_index]->test_frames[i]->done ? "true" : "false");
             }
        }
    } else {
        // Show a list of defined tests with general information
        for (int i = 0; i < MAX_TESTS; ++i) {
            if (G_test_array[i] != NULL) {
                vcom_printf("test name: %s | frame count: %u | I/O settings: %u %u %u %u | frame interval (ms): %u\n\r",
                            G_test_array[i]->test_name, G_test_array[i]->frame_count,
                            G_test_array[i]->io_settings[1], G_test_array[i]->io_settings[2],
                            G_test_array[i]->io_settings[3], G_test_array[i]->io_settings[4],
                            G_test_array[i]->frame_interval_ms);
            }
        }
    }
}


