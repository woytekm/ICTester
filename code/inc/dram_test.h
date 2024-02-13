#include<stdbool.h>
#include<stdint.h>

#define DRAM_DFT_ADDR_WIDTH 16
#define DRAM_DFT_DATA_WIDTH 8
#define DRAM_LOOPS 1000
#define DRAM_CE_LOW 1
#define DRAM_OE_LOW 1
#define DRAM_WE_LOW 1
#define DRAM_RAS_LOW 1
#define DRAM_CAS_LOW 1

#define DR_ADDR_LOW_BANK 1
#define DR_ADDR_HIGH_BANK 2
#define DR_DATA_BANK 3
#define DR_CTRL_BANK 4

typedef enum {SINGLE_PORT,DUAL_PORT,DUAL_2RAS} dram_type;

typedef struct {
  uint8_t dram_type ;       // this differs DRAM chips by the data bus type: two separate I/O ports or one multiplexed I/O port 
                            // dual port examples: 4116,4132,4164, single port: 4x64Kbit DRAM's with 8bit data bus (MB81464)
  uint8_t address_width;
  uint8_t data_width;
  bool ce_active_low;
  bool we_active_low;
  bool oe_active_low;
  bool ras_active_low;
  bool cas_active_low;
  uint8_t refresh_delay_ms;
  uint16_t loops;
  char pin_aliases[48][5];
} dram_test_settings_t;

void init_dram_test(uint8_t addr_width, uint8_t data_width, bool ce, bool we, bool oe, bool ras, bool cas, uint16_t loops, dram_test_settings_t *dram_test_settings);

extern dram_test_settings_t G_dram_test_settings;


