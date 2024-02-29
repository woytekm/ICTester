#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
void cli_set_level(int argc, char **argv);
void cli_set_direction(int argc, char **argv); 
void cli_set_test(int argc, char **argv); 
void cli_show_direction(int argc, char **argv);
void cli_show_level(int argc, char **argv); 
void cli_show_test(int argc, char **argv); 
void cli_set_direction_pin(int argc, char **argv); 
void cli_set_direction_bank(int argc, char **argv); 
void set_direction_bank(uint8_t bank_id, uint8_t direction);
void cli_set_direction_all(int argc, char **argv); 
void cli_show_direction_pin(int argc, char **argv); 
void cli_show_direction_bank(int argc, char **argv);
void cli_show_direction_all(int argc, char **argv);
void cli_show_level_pin(int argc, char **argv); 
void cli_show_level_bank(int argc, char **argv); 
uint8_t read_level_bank(uint8_t bank_id);
void cli_show_level_all(int argc, char **argv); 
void cli_set_level_pin(int argc, char **argv); 
void cli_set_level_bank(int argc, char **argv); 
void set_level_bank(uint8_t bank_id,uint8_t bitmap);
void cli_set_level_all(int argc, char **argv); 
void cli_set_level_bitmap(int argc, char **argv);
void cli_set_io(int argc, char **argv); 
void cli_set_clock(int argc, char **argv);
void cli_set_clock_pin(int argc, char **argv);
void cli_set_clock_period(int argc, char **argv);
void cli_set_clock_state(int argc, char **argv);
void cli_set_test_pin_alias(int argc, char** argv);
void cli_show_test_states(int argc, char** argv);
void cli_run(int argc, char **argv);
void cli_show_io(void);
void cli_show_clock(void);
void cli_set_color(int argc, char **argv);
void cli_set_button_cmd(int argc, char **argv);

void cli_set_test_name(int argc, char** argv);
void cli_set_test_frame_interval(int argc, char** argv);
void cli_set_test_io_settings(int argc, char** argv);
void cli_set_test_frame(int argc, char** argv);
void cli_show_test(int argc, char** argv);
void cli_show_test_frame(int argc, char** argv);
void cli_show_test_name(int argc, char** argv);

void display_help();
void display_hwinfo();

void dispatch_cli_command(int cli_argc, char **cli_argv);
bool validate_pin_id(uint8_t pin_id);
bool parse_pin_alias_params(int argc, char** argv, uint8_t *bank_bitmap, char pin_aliases[48][5]);
uint8_t alias_to_pin_id(char *alias, char pin_aliases[48][5]);
