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


// courtesy of GPT 3.5 

void tokenize_string(char *input_string, uint8_t *argc, char ***argv) {
    // Make a copy of the input string as strtok modifies the string
    char *input_copy = strdup(input_string);

    // Count the number of segments (words) in the input string
    *argc = 0;
    char *token = strtok(input_copy, " \t");
    while (token != NULL) {
        (*argc)++;
        token = strtok(NULL, " \t");
    }

    free(input_copy);

    // Allocate memory for the argument array
    *argv = (char **)malloc(sizeof(char *) * (*argc));

    // Reset strtok to tokenize the original string
    input_copy = strdup(input_string);

    // Copy each segment into the argument array
    *argc = 0;
    token = strtok(input_copy, " \t");
    while (token != NULL) {
        // Allocate memory and copy the segment into the argument array
        (*argv)[*argc] = strdup(token);
        (*argc)++;
        token = strtok(NULL, " \t");
    }

    // Free the temporary copy of the input string
    (*argc)--;
    free(input_copy);
}


void free_argv(uint8_t argc, char ***argv) {
    for (uint8_t i = 0; i < argc; i++) {
        //vcom_printf("free argument %d: %s (%X)\r\n",i,(*argv)[i],(*argv)[i]);
        free((*argv)[i]);
    }
    //vcom_printf("free pointer %X\r\n",*argv);
    free(*argv);
}


char **duplicate_argv(uint8_t argc, char **argv) {

    char **argv_copy = (char **)malloc(sizeof(char *) * argc);

    for (uint8_t i = 0; i < argc; i++) {
        argv_copy[i] = strdup(argv[i]);
    }

    return argv_copy;
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

