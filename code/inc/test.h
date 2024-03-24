#include<stdbool.h>
#include<stdint.h>


#define MAX_TEST_CMDS 50
#define MAX_CMD_LEN 128
#define TEST_NAME_LEN 32
#define MAX_TESTS 8
#define MAX_FRAMES 48
#define MAX_STATES 2048
#define MAX_ALIASES 48
#define MAX_COUNTERS 9
#define MAX_VAL_BITS 9

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
                            // values: 
                            //   1: init counter with value and apply to bank 
                            //   2: increment existing counter value and apply to bank 
                            //   3: keep existing counter value and apply to bank 

  uint8_t counter_to_bank_assignment[5]; // one byte for each bank - tells which one of eight test counters is assigned to bank (if any) 0 - no counter assigned
  bool done;
} test_frame_t;

typedef struct pin_set{                  // pins in pin_set represent value bits used in calculating expression value matched with defined output
  uint8_t pin_ids[MAX_VAL_BITS];         // first pin_set is output value matched with calculated value of an expression, rest of pin_set structs are values used in calculating value of an expression
  struct pin_set *next;
} pin_set_t;

typedef struct {
  uint8_t output_pin_id;
  uint8_t pin_ids[16];   // for value match
  pin_set_t *pin_sets;    // first pin set - value on the left side, rest of pin_sets - values on the left side of equation
  char *expression;
  uint16_t value;
  uint8_t type;  // 0 - eval logic expression: multiple pins to one output, 
                 // 1 - pin_ids is always = value (from frame X to frame Y), 
                 // 2 - pin_ids is counter 
                 // 3 - eval math expression: mutliple pins in to multiple out - pin_sets linked list is used here to store pin groups containing subsequent values
  uint16_t from_frame;
  uint16_t to_frame;
} test_criteria_t;

typedef struct {
    char test_name[TEST_NAME_LEN];
    uint8_t io_settings[5];  // each of 4 banks of 8 pins can be either input or output: 0: input, 1: output
    char pin_aliases[MAX_ALIASES][5];
    uint8_t clock_pin;
    uint8_t reset_pin;
    test_frame_t *test_frames[MAX_FRAMES];
    uint8_t test_states[MAX_STATES][5];
    uint8_t frame_count;
    uint16_t frame_interval_ms;
    uint32_t iterations_done;
    uint8_t counters[MAX_COUNTERS];
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
    MATCH_VALUE,      // check for value in bank
    MATCH_COUNTER1,   // check if bank acts as a couter
    MATCH_MEXPR       // check math expression
} test_criteria_type;

// Array of test configurations

extern test_data_t *G_test_array[MAX_TESTS];

void init_tests(void);


