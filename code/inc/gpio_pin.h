#include <stdint.h>

// pin mapping for actual GPIO wiring on the Tester board

#define PIN_1_OE 16
#define PIN_10 208
#define PIN_11 207
#define PIN_12 206
#define PIN_13 205
#define PIN_14 204
#define PIN_15 203
#define PIN_16 202
#define PIN_17 201
#define PIN_2_OE 8
#define PIN_20 7
#define PIN_21 6
#define PIN_22 5
#define PIN_23 4
#define PIN_24 117
#define PIN_25 116
#define PIN_26 115
#define PIN_27 110

#define PIN_30 213
#define PIN_31 10
#define PIN_32 1
#define PIN_33 0
#define PIN_34 129
#define PIN_35 128
#define PIN_36 127
#define PIN_37 126
#define PIN_3_OE 125
#define PIN_40 124
#define PIN_41 122
#define PIN_42 121
#define PIN_43 120
#define PIN_44 119
#define PIN_45 118
#define PIN_46 23
#define PIN_47 27
#define PIN_4_OE 28

#define PIN_GREEN_LED 109
#define PIN_RED_LED 101
#define PIN_YELLOW_LED 2


// Enumerated type for pin direction
typedef enum {
    PIN_INPUT,
    PIN_OUTPUT   
} pin_direction;

// Enumerated type for pin state
typedef enum {
    PIN_LOW,
    PIN_HIGH
} pin_level;

// Struct to describe a single pin
typedef struct {
    uint8_t pin_id;         // Pin ID in the format XY: X bank, Y pin in bank
    pin_level level;  // Pin direction: PIN_INPUT or PIN_OUTPUT
    pin_direction direction;         // Pin state: PIN_LOW or PIN_HIGH
    uint8_t gpio_id;     // Actual GPIO number for LPC1769 GPIO mapping
} pin;

// Array of 32 pin structs
static pin G_pin_array[47];


