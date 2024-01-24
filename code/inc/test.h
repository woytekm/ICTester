#include<stdbool.h>
#include<stdint.h>


#define MAX_TESTS 8
#define MAX_FRAMES 64
#define MAX_STATES 4096
#define MAX_ALIASES 48

#define MAX_CRITERIA 16
#define MAX_EXPR_LEN 128

#define STATE_ARRAY_ADDRESS 0x2008000000

typedef struct {
  //uint32_t LPC_GPIOX_FIOSET_bitmap[5];   // pin set map
  //uint32_t LPC_GPIOX_FIOCLR_bitmap[5];   // pin clear map
  //uint32_t LPC_GPIOX_FIOMASK_bitmap[5];  // which pins should we set/clear ?
  //uint32_t LPC_GPIOX_FIOPIN_MASK_bitmap[5];   // which pins should we read?
  //uint32_t LPC_GPIOX_FIOPIN_bitmap[5];   // actual pin state read from registers
  //uint32_t LPC_GPIOX_FIOPIN_expected[5]; // expected pin state

  uint8_t bank_bitmap[5];   // actual values assigned to pins in each bank (OR loop parameters as below:)
  uint8_t type;             // 0 - normal, contains 4 bank bitmaps in cells 1,2,3,4
                            // 1 - loop to frame number bank_bitmap[0] until counter[bank_bitmap[2]] = bank_bitmap[1] 
                            // 2 - loop to frame number bank_bitmap[0] until bank_bitmap[1] loops passed
  uint8_t use_counters;     // bitmap telling if we should init counter with value, or increment existing value (or do nothing)
                            // bits 0,1 - bank 1, bits 2,3 - bank 2, etc, 
                            // values: 1: init counter with value and apply to bank 2: increment existing value and apply to bank
  uint8_t counter_to_bank_assignment[5]; // one byte for each bank - tells which one of eight test counters is assigned to bank (if any) 0 - no counter assigned
  bool done;
} test_frame_t;

typedef struct {
  uint8_t output_pin_id;
  uint8_t pin_ids[16];
  char *logic_expression;
  uint16_t value;
  uint8_t type;  // 0 - eval logic expression: multiple pins to one output, 1 - pin_ids is always = value (from frame X to frame Y), 2 - pin_ids is counter 
  uint16_t from_frame;
  uint16_t to_frame;
} test_criteria_t;

extern uint8_t G_output_cache[MAX_STATES][4];

typedef struct {
    char test_name[20];
    uint8_t io_settings[5];  // each of 4 banks of 8 pins can be either input or output: 0: input, 1: output
    char pin_aliases[MAX_ALIASES][5];
    uint8_t clock_pin;
    uint8_t reset_pin;
    test_frame_t *test_frames[64];
    uint8_t test_states[MAX_STATES][5];
    uint8_t frame_count;
    uint16_t frame_interval_ms;
    uint32_t iterations_done;
    uint8_t counters[9];
    test_criteria_t *test_criteria[MAX_CRITERIA+1];
} test_data_t;

typedef struct {
    uint8_t loop_type;  // 1 COUNTER, 2 LOOP
    uint8_t matched_value;
    uint8_t counter_parameter;
} loop_conditions_t;

// Enumerated type for pin state
typedef enum {
    NO_LOOP,
    MATCH_LOOP,
    MATCH_COUNTER
} loop_condition_type;

// Enumerated type for pin state
typedef enum {
    MATCH_EXPRESSION,
    MATCH_VALUE,
    MATCH_COUNTER1
} test_criteria_type;

// Array of test configurations

extern test_data_t *G_test_array[MAX_TESTS];

void init_tests(void);


