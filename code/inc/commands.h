#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void cli_set_level(int argc, char **argv);
void cli_show_level(int argc, char **argv); 
void cli_set_pin(int argc, char **argv); 
void cli_show_pin(int argc, char **argv);
void cli_set_pin_direction(int argc, char **argv); 
void cli_show_pin_direction(int argc, char **argv); 
void cli_show_level_pin(int argc, char **argv); 
void cli_show_level_bank(int argc, char **argv); 
void cli_show_level_all(int argc, char **argv); 
void cli_set_level_pin(int argc, char **argv); 
void cli_set_level_bank(int argc, char **argv); 
void cli_set_level_all(int argc, char **argv); 
void display_help();

void dispatch_cli_command(int cli_argc, char **cli_argv);


