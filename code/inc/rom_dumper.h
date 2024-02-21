#include<stdbool.h>
#include<stdint.h>

#define ROM_DFT_ADDR_WIDTH 16
#define ROM_DFT_DATA_WIDTH 8
#define ROM_CE_ACT 0
#define ROM_OE_ACT 0


typedef struct {
  uint8_t address_width;
  uint8_t data_width;
  uint8_t ce_active;
  uint8_t oe_active;
  char filename[50];
  char pin_aliases[MAX_ALIASES][5];
} rom_dumper_settings_t;

void init_rom_dumper(uint8_t addr_width, uint8_t data_width, bool ce, bool oe, char *filename, rom_dumper_settings_t *rom_dumper_settings);

extern rom_dumper_settings_t G_rom_dumper_settings;


