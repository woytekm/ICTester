#include<stdbool.h>
#include<stdint.h>


#define MAX_TESTS 8
#define MAX_FRAMES 64
#define MAX_STATES 2048

typedef struct {
  //uint32_t LPC_GPIOX_FIOSET_bitmap[5];   // pin set map
  //uint32_t LPC_GPIOX_FIOCLR_bitmap[5];   // pin clear map
  //uint32_t LPC_GPIOX_FIOMASK_bitmap[5];  // which pins should we set/clear ?
  //uint32_t LPC_GPIOX_FIOPIN_MASK_bitmap[5];   // which pins should we read?
  //uint32_t LPC_GPIOX_FIOPIN_bitmap[5];   // actual pin state read from registers
  //uint32_t LPC_GPIOX_FIOPIN_expected[5]; // expected pin state

  uint8_t bank_bitmap[5];
  uint8_t type;         // 0 - normal, contains 4 bank bitmaps in cells 1,2,3,4
                        // 1 - loop to frame number bank_bitmap[0] until bank_counter = bank_bitmap[1] 
                        // 2 - loop to frame number bank_bitmap[0] until bank_bitmap[1] loops passed
  uint8_t use_counters;
  bool done;
} test_frame_t;


typedef struct {
    char test_name[20];
    uint8_t io_settings[5];  // each of 4 banks of 8 pins can be either input or output: 0: input, 1: output
    char pin_aliases[48][5];
    uint8_t clock_pin;
    uint8_t reset_pin;
    test_frame_t *test_frames[64];
    uint8_t test_states[MAX_STATES][5];
    uint8_t frame_count;
    uint16_t frame_interval_ms;
    uint32_t iterations_done;
} test_data_t;


typedef struct {
    uint8_t loop_type;  // 0 for BANK, 1 for LOOP
    uint8_t matched_value;
    uint8_t bank_parameter;
} loop_conditions_t;

// Enumerated type for pin state
typedef enum {
    NO_LOOP,
    MATCH_LOOP,
    MATCH_BANK
} loop_condition_type;

// Array of test configurations

extern test_data_t *G_test_array[MAX_TESTS];

void init_tests(void);


