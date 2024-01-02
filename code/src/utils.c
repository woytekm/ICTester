#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "cli_io.h"

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

