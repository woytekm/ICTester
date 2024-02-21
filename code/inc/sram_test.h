#include<stdbool.h>
#include<stdint.h>
#include "test.h"

#define SRAM_DFT_ADDR_WIDTH 16
#define SRAM_DFT_DATA_WIDTH 8
#define SRAM_LOOPS 1000
#define SRAM_CE_ACT 0
#define SRAM_OE_ACT 0
#define SRAM_WE_ACT 0


typedef struct {
  uint8_t address_width;
  uint8_t data_width;
  bool ce_active;
  bool we_active;
  bool oe_active;
  uint16_t loops;
  char pin_aliases[MAX_ALIASES][5];
} sram_test_settings_t;

void init_sram_test(uint8_t addr_width, uint8_t data_width, bool ce, bool we, bool oe, uint16_t loops, sram_test_settings_t *sram_test_settings);

extern sram_test_settings_t G_sram_test_settings;


