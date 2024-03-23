#define RTT_DEBUG 1

extern __DATA(RAM2) char G_command_buffer[MAX_TEST_CMDS][MAX_CMD_LEN];
extern __DATA(RAM2) uint8_t G_output_cache[MAX_STATES][4];  // let's keep this in AHB RAM2 (second 32KB block of SRAM available in LPC1769). First 4KB in this block is allocated by USB OTG data structures.
extern __DATA(RAM2) uint8_t usb_structures[4096];
extern uint8_t G_cmd_cnt;
extern bool G_use_color;
extern char G_btn_cmd[64];
extern char G_set_btn_cmd[64];

