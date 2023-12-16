#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "commands.h"
#include "cli_io.h"
#include "gpio.h"
#include "gpio_pin.h"

// Predefined static char array for formatted messages
#define MAX_MESSAGE_LENGTH 256
static char message_buffer[MAX_MESSAGE_LENGTH];


// Top-level function to dispatch commands
void dispatch_cli_command(int cli_argc, char **cli_argv) {
  
    if(cli_argc == 0)
      return;

    char *command = cli_argv[0];

    // Array of known top-level commands
    const char *top_level_commands[] = {"set", "clear", "show", "help"};

    int i;
    int num_top_level_commands = sizeof(top_level_commands) / sizeof(top_level_commands[0]);

    for (i = 0; i < num_top_level_commands; ++i) {
        if (strcmp(command, top_level_commands[i]) == 0) {
            break;
        }
    }

    switch (i) {
        case 0: // set
            if (cli_argc < 2) {
                snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: set %s <subcommand>\n", cli_argv[1]);
                vcom_message(message_buffer);
                return;
            }

            char *subcommand_set = cli_argv[1];

            if (strcmp(subcommand_set, "level") == 0) {
                cli_set_level(cli_argc - 2, cli_argv + 2);
            } else if (strcmp(subcommand_set, "pin") == 0) {
                cli_set_pin(cli_argc - 2, cli_argv + 2);
            } else {
                snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'set': %s\n", subcommand_set);
                vcom_message(message_buffer);
            }
            break;

        case 1: // clear
            // Handle 'clear' command if needed
            snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Clear command not implemented\n");
            vcom_message(message_buffer);
            break;

        case 2: // show
            if (cli_argc < 2) {
                snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: %s show <subcommand>\n", cli_argv[0]);
                vcom_message(message_buffer);
                return;
            }

            char *subcommand_show = cli_argv[1];

            if (strcmp(subcommand_show, "level") == 0) {
                cli_show_level(cli_argc - 2, cli_argv + 2);
            } else if (strcmp(subcommand_show, "pin") == 0) {
                cli_show_pin(cli_argc - 2, cli_argv + 2);
            } else {
                snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'show': %s\n", subcommand_show);
                vcom_message(message_buffer);
            }
            break;

        case 3: // help
            display_help();
            break;

        default:
            // Unknown command
            snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown command: %s\n", command);
            vcom_message(message_buffer);
            break;
    }
}

// Sub-functions
void cli_set_level(int argc, char **argv) {
    if (argc < 1) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: set level <value>\n");
        vcom_message(message_buffer);
        return;
    }

    char *subcommand_set_level = argv[0];

    if (strcmp(subcommand_set_level, "pin") == 0) {
        cli_set_level_pin(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "bank") == 0) {
        cli_set_level_bank(argc - 1, argv + 1);
    } else if (strcmp(subcommand_set_level, "all") == 0) {
        cli_set_level_all(argc - 1, argv + 1);
    } else {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'show level': %s\n", subcommand_set_level);
        vcom_message(message_buffer);
    }

}


void cli_show_level(int argc, char **argv) {
    if (argc < 1) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: show level <subcommand>\n");
        vcom_message(message_buffer);
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
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'show level': %s\n", subcommand_show_level);
        vcom_message(message_buffer);
    }
}


// Sub-functions
void cli_set_pin(int argc, char **argv) {
    if (argc < 1) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: set level <value>\n");
        vcom_message(message_buffer);
        return;
    }

    char *subcommand_set_pin = argv[0];

    if (strcmp(subcommand_set_pin, "direction") == 0) {
        cli_set_pin_direction(argc - 1, argv + 1);
    } else {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'show level': %s\n", subcommand_set_pin);
        vcom_message(message_buffer);
    }

}


void cli_show_pin(int argc, char **argv) {
    if (argc < 1) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: show level <subcommand>\n");
        vcom_message(message_buffer);
        return;
    }

    char *subcommand_show_pin = argv[0];

    if (strcmp(subcommand_show_pin, "direction") == 0) {
        cli_show_pin_direction(argc - 1, argv + 1);
    } else {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Unknown subcommand for 'show level': %s\n", subcommand_show_pin);
        vcom_message(message_buffer);
    }
}



void cli_set_pin_direction(int argc, char **argv) {
    if (argc < 2) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: set pin direction <value>\n");
        vcom_message(message_buffer);
        return;
    }

    int pin_value = atoi(argv[0]);
    int direction_value = atoi(argv[1]);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Setting pin %d direction to %d\n", pin_value, direction_value);
    vcom_message(message_buffer);
    // Add code to perform the 'set pin direction' operation with the specified values
}

void cli_show_pin_direction(int argc, char **argv) {
    if (argc < 1) {
        snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Usage: show pin direction <pin>\n");
        vcom_message(message_buffer);
        return;
    }

    int pin_value = atoi(argv[0]);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing pin %d direction\n", pin_value);
    vcom_message(message_buffer);
    // Add code to perform the 'show pin direction' operation with the specified pin
}

void cli_show_level_pin(int argc, char **argv) {
    // Add code for 'show level pin' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing level pin\n");
    vcom_message(message_buffer);
}

void cli_show_level_bank(int argc, char **argv) {
    // Add code for 'show level bank' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing level bank\n");
    vcom_message(message_buffer);
}

void cli_show_level_all(int argc, char **argv) {
    // Add code for 'show level all' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing level all\n");
    vcom_message(message_buffer);
}

void cli_set_level_pin(int argc, char **argv) {
    // Add code for 'show level pin' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Setting pin level\n");
    vcom_message(message_buffer);

    if (argc == 2) {
        uint8_t pin_id = atoi(argv[0]);
        pin_level level;

        if (strcmp(argv[1], "L") == 0) {
            level = PIN_LOW;
        } else if (strcmp(argv[1], "H") == 0) {
            level = PIN_HIGH;
        } else {
            snprintf(message_buffer, MAX_MESSAGE_LENGTH, "set level pin: invalid level specification (should be L or H)\n");
            vcom_message(message_buffer); 
            return;
        }

        if (pin_id >= 1 && pin_id <= sizeof(G_pin_array) / sizeof(G_pin_array[0])) {
            uint8_t gpio_id = G_pin_array[pin_id].gpio_id;

                // Set the pin level based on the provided pin_state
                switch (level) {
                    case PIN_LOW:
                        set_pin_low(gpio_id);
                        break;
                    case PIN_HIGH:
                        set_pin_high(gpio_id);
                        break;
                    default:
                        // Handle invalid pin_state
                        break;
                }
            } 
       else {
                 snprintf(message_buffer, MAX_MESSAGE_LENGTH, "set level pin: <pin id> (XY where X - bank, Y - pin in bank) <level> (L|H)\n");
                 vcom_message(message_buffer);
            }
    } 
    else {
       snprintf(message_buffer, MAX_MESSAGE_LENGTH, "set level pin: <pin id> (XY where X - bank, Y - pin in bank) <level> (L|H)\n");
       vcom_message(message_buffer);
    }
}


void cli_set_level_bank(int argc, char **argv) {
    // Add code for 'show level bank' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing level bank\n");
    vcom_message(message_buffer);
}

void cli_set_level_all(int argc, char **argv) {
    // Add code for 'show level all' operation
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Showing level all\n");
    vcom_message(message_buffer);
}


void display_help() {
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "Available commands:\n");
    vcom_message(message_buffer);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "set level <pin|bank|all> <L|H>: Set pin/bank/all level to a value\n");
    vcom_message(message_buffer);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "show level <pin|bank|all>: Show level for pin/bank/all\n");
    vcom_message(message_buffer);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "show pin direction <pin>: Show pin direction: input or ouptut\n");
    vcom_message(message_buffer);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "set pin direction <I|O>: Set pin direction to input/output\n");
    vcom_message(message_buffer);
    snprintf(message_buffer, MAX_MESSAGE_LENGTH, "help: Display available commands\n");
    vcom_message(message_buffer);
}


