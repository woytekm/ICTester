#include <stdint.h>
#include <stdbool.h>

void vcom_printf(char *fmt, ...);
void replace_or_append_cmd_buff(char *cmd_start,uint8_t argc, char **argv);
void tokenize_string(char *input_string, uint8_t *argc, char ***argv);
void free_argv(uint8_t argc, char **argv);
char **duplicate_argv(uint8_t argc, char **argv);
char sh_lvl_set(uint8_t level_set);
bool lh_to_bool(char c);
uint8_t lh_to_int(char c);
uint8_t parse_bank_bitmap(char* bitmap_str);
uint8_t parse_uint8_string(const char* bitmap_str); 
void uint8_to_binary_string(uint8_t value, char* binary_str); 
void set_bits(uint8_t *byte, uint8_t bit1, uint8_t bit2, uint8_t value); 
uint8_t extract_bits(uint8_t byte, uint8_t bit1, uint8_t bit2);


