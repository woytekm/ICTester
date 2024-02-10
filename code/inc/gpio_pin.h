#include <stdint.h>

#define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

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

extern uint32_t SystemCoreClock;

// pin mapping for actual GPIO wiring on the Tester board

#define PIN_1_DIR 201
#define PIN_1_OE 15 
#define PIN_10 202 // 2[01]
#define PIN_11 203 // 2[02]
#define PIN_12 204
#define PIN_13 205
#define PIN_14 206
#define PIN_15 207
#define PIN_16 208
#define PIN_17 16

#define PIN_2_DIR 101
#define PIN_2_OE 4    // 0[8], PINSEL0 [17:16]
#define PIN_20  104     // 1[10]
#define PIN_21  108     // 1[15]
#define PIN_22  109
#define PIN_23  110
#define PIN_24  114
#define PIN_25  115
#define PIN_26  116
#define PIN_27  117

#define PIN_3_DIR 22
#define PIN_3_OE 126
#define PIN_30 213
#define PIN_31 11
#define PIN_32 10
#define PIN_33 1
#define PIN_34 0
#define PIN_35 129
#define PIN_36 128
#define PIN_37 127

#define PIN_4_DIR 120
#define PIN_4_OE 121
#define PIN_40 119
#define PIN_41 118
#define PIN_42 26
#define PIN_43 25
#define PIN_44 27
#define PIN_45 28
#define PIN_46 23
#define PIN_47 24

#define PIN_GREEN_LED   19
#define PIN_RED_LED     20
#define PIN_YELLOW_LED  21

#define PIN_DUT_POWER 125

#define PIN_USD_MOSI 9
#define PIN_USD_MISO 8
#define PIN_USD_SCK  7
#define PIN_USD_SSEL 6


#define SET_ADDR 1
#define CLR_ADDR 0
#define READ_ADDR 2

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
    uint32_t gpio_id;     // Actual GPIO number for LPC1769 GPIO mapping
    uint32_t gpio_pin_id;
    uint32_t *gpio_addrs[3];
} pin;

// Array of pin structs
#define PIN_COUNT 48

extern uint8_t G_gpio_map[];
extern pin G_pin_array[PIN_COUNT];
extern uint8_t G_clock_pin;
extern uint8_t G_clock_state;


