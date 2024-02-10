#include <stdint.h>

void vcom_printf(char *fmt, ...);
void replace_or_append_cmd_buff(char *cmd_start,uint8_t argc, char **argv);
void tokenize_string(char *input_string, uint8_t *argc, char ***argv);
void free_argv(uint8_t argc, char ***argv);
char **duplicate_argv(uint8_t argc, char **argv);
