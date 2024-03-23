#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <cr_section_macros.h>

#include "board.h"
#include "cdc_vcom.h"
#include "app_usbd_cfg.h"
#include "cli_io.h"
#include "test.h"
#include "globals.h"
#include "ff.h"



// Helper function to parse and convert bank bitmap from string to uint8_t
uint8_t parse_bank_bitmap(char* bitmap_str) {
    // Check if the string starts with "0b" or "0B"
    if (bitmap_str[0] == '0' && (bitmap_str[1] == 'b' || bitmap_str[1] == 'B')) {
        return strtol(bitmap_str + 2, NULL, 2);  // Convert binary string
    } else {
        return strtol(bitmap_str, NULL, 0);  // Automatically detect base (hex or decimal)
    }
}

uint8_t parse_uint8_string(const char* bitmap_str) {
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


bool lh_to_bool(char c)
 {
   if(c == 'L')
    return false;
   else if(c == 'H')
    return true;

   return false;
 }

uint8_t lh_to_int(char c)
 {
   if(c == 'L')
    return 0;
   else if(c == 'H')
    return 1;

   return 0;
 }

void vcom_printf(char *fmt, ...)
 {
  #define MAX_LOG_MSG_LEN 2048
  va_list ap;
  char message[MAX_LOG_MSG_LEN];
  
  va_start(ap, fmt);
  vsprintf(message, fmt, ap);
  va_end(ap);

  vcom_message(message);
 }

void vcom_cprintf(char *fmt_c, char *fmt_nc, ...)
 {
  #define MAX_LOG_MSG_LEN 2048
  va_list ap;
  char message[MAX_LOG_MSG_LEN];

  if(G_use_color)
   {
     va_start(ap, fmt_nc);
     vsprintf(message, fmt_c, ap);
     va_end(ap);
   }
  else if(!G_use_color)
   {
     va_start(ap, fmt_nc);
     vsprintf(message, fmt_nc, ap);
     va_end(ap);
   }

  vcom_message(message);
 }


char sh_lvl_set(uint8_t level_set)
 {
  if(level_set == 0)
   return 'L';
  else if(level_set == 1)
   return 'H';

  return 'N';
 }

void replace_or_append_cmd_buff(char *cmd_start,uint8_t argc, char **argv)
 {
  char message[MAX_CMD_LEN];
  message[0] = 0x0;

  strcat(message,cmd_start);

  for(uint8_t i = 0; i < argc; i++)
   {
     strcat(message,argv[i]);    
     strcat(message," ");
   }

  if(G_cmd_cnt == MAX_TEST_CMDS)
   {
    vcom_printf("ERROR: maximum number of commands per test exceeded. Command was accepted, but it will not be saved.\r\n");
    return;
   }
  strcpy(G_command_buffer[G_cmd_cnt],message);
  G_cmd_cnt++;
 }



char *strdup (const char *s)
{
  size_t len = strlen (s) + 1;
  void *new = malloc (len);
  if (new == NULL)
    return NULL;
  return (char *) memcpy(new, s, len);
}

// memory conserving version 

void tokenize_string(char *input_string, uint8_t *argc, char ***argv) {
    // vcom_printf("tokenize: %s\r\n",input_string);
    char c = ' ';
    uint16_t i = 0;
    bool non_space_char = false;

    *argc = 0;

    while(c != '\0')
     {
        c = input_string[i++];
        if(((c == ' ')||(c == '\t'))&&(i == 1))  // spaces on the beginning - do not count
          { non_space_char = false; continue; }
        else if(((c == ' ')||(c == '\t'))&&non_space_char)  // also skip multiple spaces
         {
           (*argc)++;
           non_space_char = false;
         }
        else
         non_space_char = true;
     }
    if(non_space_char == false)
      (*argc)--; // spaces on the end of the string - do not count

    // Allocate memory for the argument array
    *argv = (char **)malloc(sizeof(char *) * (*argc));
     
     uint8_t total_strlen = strlen(input_string);
     uint8_t token_start = 0, token = 0;

     for(uint8_t i = 0; i < total_strlen; i++)
      {
        if(!non_space_char)
          token_start = i;
        if((input_string[i] == ' ')||(input_string[i] == '\0'))
          {
            input_string[i] = '\0';
            (*argv)[token] = &input_string[token_start];
            //vcom_printf("   token: %s\r\n",(*argv)[token]);
            token++;
            non_space_char = false;
          }
        else 
          non_space_char = true;       
      }
}


void free_argv(uint8_t argc, char **argv[]) {
        free(argv);
}


char **duplicate_argv(uint8_t argc, char **argv) {
  size_t strlen_sum;
  char **argp;
  char *data;
  size_t len;
  int i;

  strlen_sum = 0;
  for (i = 0; i < argc; i++) strlen_sum += strlen(argv[i]) + 1;

  argp = malloc(sizeof(char *) * (argc + 1) + strlen_sum);
  if (!argp) return NULL;
  data = (char *) argp + sizeof(char *) * (argc + 1);

  for (i = 0; i < argc; i++) {
    argp[i] = data;
    len = strlen(argv[i]) + 1;
    memcpy(data, argv[i], len);
    data += len;
  }
  argp[argc] = NULL;

  return argp;
 }


// Courtesy of GPT 3.5

#define XXD_BUFFER_SIZE 16
#define PAGE 24

void hex_dump(const char *filename) {

    FATFS drive;
    FIL file;
    FRESULT fr;
    BYTE buffer[XXD_BUFFER_SIZE];
    UINT br;
    int offset = 0;
    int lines = 0;
    uint8_t byte;
    VCOM_DATA_T *pVcom = &g_vCOM;

    f_mount(0,&drive);
    // Open the file
    fr = f_open(&file, filename, FA_READ);

    if (fr != FR_OK) {
        vcom_printf("ERROR: failed to open file: %s\n", filename);
        return;
    }

    // Read and dump the file contents
    while (1) {
        // Read a chunk of data from the file
        fr = f_read(&file, buffer, XXD_BUFFER_SIZE, &br);
        if (fr != FR_OK || br == 0) {
            break; // End of file or error
        }

        // Print the offset in hex
        vcom_printf("%08X  ", offset);

        // Print the hexadecimal dump
        for (int i = 0; i < XXD_BUFFER_SIZE; i++) {
            if (i < br) {
                vcom_printf("%02X ", buffer[i]);
            } else {
                printf("   "); // Pad with spaces if end of file reached
            }

            // Print an extra space after 8 bytes
            if ((i + 1) % 8 == 0) {
                vcom_printf(" ");
            }
        }

        // Print the ASCII representation
        vcom_printf(" | ");
        for (int i = 0; i < br; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                vcom_printf("%c", buffer[i]); // Printable characters
            } else {
                vcom_printf("."); // Non-printable characters replaced with dots
            }
        }
        vcom_printf(" |\r\n");

        offset += br;

        lines++;

        if(lines >= PAGE)
         {
           vcom_printf("-- more --\r\n");

           byte = 0x0;

           while((byte != 'q') && (byte != ' '))
            {  
              delayMS(5);
              if(pVcom->rx_count == 0)
               __WFI();
              vcom_bread(&g_hUsb,&g_rxBuff[0], 256);
              byte = g_rxBuff[0];
            }

           if(byte == ' ')
             lines = 0;
           else if(byte == 'q')
             break;
         }
    }

    // Close the file
    f_close(&file);
    f_mount(0,NULL);

}


#define STRINGS_BUFFER_SIZE 128
#define MIN_STRING_LEN 5

void strings(const char *filename) {
    FATFS drive;
    FIL file;
    FRESULT fr;
    BYTE buffer[STRINGS_BUFFER_SIZE];
    UINT br;
    int string_start = -1;


    f_mount(0,&drive);
    // Open the file
    fr = f_open(&file, filename, FA_READ);
    if (fr != FR_OK) {
        vcom_printf("ERROR: failed to open file: %s\r\n", filename);
        return;
    }

    // Read the file contents
    while (1) {
        // Read a chunk of data from the file
        fr = f_read(&file, buffer, STRINGS_BUFFER_SIZE, &br);
        if (fr != FR_OK || br == 0) {
            break; // End of file or error
        }

        // Find printable strings in the buffer
        for (int i = 0; i < br; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                if (string_start == -1) {
                    // Start of a new string
                    string_start = i;
                }
            } else {
                  if (string_start != -1) {
                    if((i - string_start)>MIN_STRING_LEN)
                      {
                       for (int j = string_start; j < i; j++) {
                         vcom_printf("%c", buffer[j]);
                      }
                      vcom_printf("\r\n");
                     }
                   string_start = -1;
                }
            }
        }
    }

    // Close the file
    f_close(&file);
    f_mount(0,NULL);
}

