#include <stddef.h>
#include "integer.h"
#include <cr_section_macros.h>

#include "ff.h"
#include "utils.h"
#include "diskio.h"
#include "test.h"
#include "globals.h"


void usd_unlink_file(char *path)
 {
    FRESULT fr;     /* FatFs return code */
    FATFS drive;

    f_mount(0,&drive);

    fr = f_unlink(path);
    if (fr)
     {
      vcom_printf("ERROR: removing file %s failed (%d).\r\n",path,fr);
      f_mount(0,NULL);
      return;
     }

    f_mount(0,NULL);
    return;
 }


void usd_rename_file(char *path1, char *path2)
 {
    FRESULT fr;     /* FatFs return code */
    FATFS drive;

    f_mount(0,&drive);

    fr = f_unlink(path2);

    fr = f_rename(path1,path2);

    if (fr)
     {
      vcom_printf("ERROR: renaming/moving of file %s failed (%d).\r\n",path1,fr);
      f_mount(0,NULL);
      return;
     }

    f_mount(0,NULL);
    return;
 }


void usd_list_file_contents(char *path)
 {
    FIL fil;        /* File object */
    char line[128]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */
    FATFS drive;
	
    f_mount(0,&drive);

    /* Open a text file */
    fr = f_open(&fil, path, FA_READ);
    if (fr)
     {
      vcom_printf("ERROR: listing file %s failed (%d).\r\n",path,fr);
      f_mount(0,NULL);
      return; 
     }

    /* Read every line and display it */
    while (f_gets(line, sizeof line, &fil)) {
        vcom_printf(line);
        vcom_printf("\r");
    }

    vcom_printf("\n");

    /* Close the file */
    f_close(&fil);
    f_mount(0,NULL);

    return;
 }


void usd_list_dir(char *path)
  {
    FRESULT res;
    FATFS drive;            /* File system object for each logical drive */

    DIR dir;
    char string[192];
 
    f_mount(0,&drive);

    res = f_opendir(&dir, path);
 
    if (res != FR_OK)
      vcom_printf("ERROR: list_dir(): res = %d f_opendir\r\n", res);
 
    if (res == FR_OK)
    {
      while(1)
      {
        FILINFO fno;
        static char lfn[_MAX_LFN + 1];
        fno.lfname = lfn;
        fno.lfsize = sizeof(lfn);

        res = f_readdir(&dir, &fno);

        if (res != FR_OK)
          vcom_printf("ERROR: list_dir(): res = %d f_readdir\r\n", res);

        if ((res != FR_OK) || (fno.fname[0] == 0))
          break;

        if(strlen(fno.lfname) == 0)
          sprintf(string, "%c%c%c%c %10d %s/%s\r\n",
            ((fno.fattrib & AM_DIR) ? 'D' : '-'),
            ((fno.fattrib & AM_RDO) ? 'R' : '-'),
            ((fno.fattrib & AM_SYS) ? 'S' : '-'),
            ((fno.fattrib & AM_HID) ? 'H' : '-'),
            (int)fno.fsize, path, fno.fname);
        else
          sprintf(string, "%c%c%c%c %10d %s/%s\r\n",
            ((fno.fattrib & AM_DIR) ? 'D' : '-'),
            ((fno.fattrib & AM_RDO) ? 'R' : '-'),
            ((fno.fattrib & AM_SYS) ? 'S' : '-'),
            ((fno.fattrib & AM_HID) ? 'H' : '-'),
            (int)fno.fsize, path, fno.lfname);
 
        vcom_printf(string);

      }
    }

   f_mount(0,NULL);

  }


void usd_mkdir(char *path)
{
    FRESULT fr;     /* FatFs return code */
    FATFS drive;

    f_mount(0,&drive);

    fr = f_mkdir(path);

    if (fr)
     {
      vcom_printf("ERROR: creating directory %s failed (%d).\r\n",path,fr);
      f_mount(0,NULL);
      return;
     }

    f_mount(0,NULL);
    return;

}


void usd_load_file(char *path)
{
    uint8_t my_argc;
    char **my_argv;
    FIL fil;        /* File object */
    char line[MAX_CMD_LEN]; /* Line buffer */
    FRESULT fr;     /* FatFs return code */
    FATFS drive;

    f_mount(0,&drive);

    /* Open a text file */
    fr = f_open(&fil, path, FA_READ);
    if (fr)
     {
      vcom_printf("ERROR: loading file %s failed (%d).\r\n",path,fr);
      f_mount(0,NULL);
      return;
     }

    /* Read every line and display it */
    while (f_gets(line, sizeof line, &fil)) {
      tokenize_string(line,&my_argc,&my_argv); 
      dispatch_cli_command(my_argc, my_argv);   
      free_argv(my_argc,&my_argv);
    }

    /* Close the file */
    f_close(&fil);
    f_mount(0,NULL);

    return;
 }



void usd_save_test_to_file(char *test_name, char *path)
{

    FIL file;        /* File object */
    FRESULT fr;     /* FatFs return code */
    FATFS drive;

    f_mount(0,&drive);

    fr = f_unlink(path);

    fr = f_open(&file, path, FA_CREATE_NEW);
    fr = f_close(&file);
    fr = f_open(&file, path, FA_WRITE);

    if (fr)
     {
      vcom_printf("ERROR: cannot open %s for writing (%d).\r\n",path,fr);
      f_mount(0,NULL);
      return;
     }

    for(uint8_t i = 0; i < G_cmd_cnt; i++)
     {
       fr = f_printf(&file,"%s\n", G_command_buffer[i]);
       if (fr<0)
       {
        vcom_printf("ERROR: cannot write to file %s (%d).\r\n",path,fr);
        f_mount(0,NULL);
        return;
       }
     }

    fr = f_close(&file);

    f_mount(0,NULL);
    return;
}



