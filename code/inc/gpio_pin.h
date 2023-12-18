#include <stdint.h>

typedef struct {
#if defined(CHIP_LPC175X_6X)
        __IO uint32_t PINSEL[11];
        uint32_t RESERVED0[5];
        __IO uint32_t PINMODE[10];
        __IO uint32_t PINMODE_OD[5];
        __IO uint32_t I2CPADCFG;
#else
        __IO uint32_t p[5][32];
#endif
} LPC_IOCON_T;

#define LPC_IOCON_BASE            0x4002C000
#define LPC_IOCON                 ((LPC_IOCON_T            *) LPC_IOCON_BASE)

#define IOCON_FUNC0             0x0                             /*!< Selects pin function 0 */
#define IOCON_FUNC1             0x1                             /*!< Selects pin function 1 */
#define IOCON_FUNC2             0x2                             /*!< Selects pin function 2 */
#define IOCON_FUNC3             0x3                             /*!< Selects pin function 3 */
#define IOCON_MODE_INACT        (0x2 << 2)              /*!< No addition pin function */
#define IOCON_MODE_PULLDOWN     (0x3 << 2)              /*!< Selects pull-down function */
#define IOCON_MODE_PULLUP       (0x0 << 2)              /*!< Selects pull-up function */
#define IOCON_MODE_REPEATER     (0x1 << 2)              /*!< Selects pin repeater function */

// pin mapping for actual GPIO wiring on the Tester board

#define PIN_1_OE 16
#define PIN_10 208 // 2[08]
#define PIN_11 207 // 2[07]
#define PIN_12 206
#define PIN_13 205
#define PIN_14 204
#define PIN_15 203
#define PIN_16 202
#define PIN_17 201
#define PIN_2_OE 8   // 0[8], PINSEL0 [17:16]
#define PIN_20 7     // 0[7], PINSEL0 [15:14]
#define PIN_21 6     // 0[6], PINSEL0 [13:12]
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


