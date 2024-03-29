void set_pin_write(uint32_t bank_pin);
void set_pin_high_simple(uint32_t bank_pin);
void set_pin_low_simple(uint32_t bank_pin);
void set_pin_high(uint32_t bank_pin, uint32_t *gpio_set_address);
void set_pin_low(uint32_t bank_pin, uint32_t *gpio_clr_address);
void set_pin_high_fast(uint32_t pin, uint32_t *gpio_set_address);
void set_pin_low_fast(uint32_t pin, uint32_t *gpio_clr_address);
void toggle_pin(uint32_t bank_pin);
uint32_t get_pin(uint32_t bank_pin);
void init_pin_array(void);
void init_pins(void);
void led_signal_test_fail(void);
void led_signal_test_ok(void);

