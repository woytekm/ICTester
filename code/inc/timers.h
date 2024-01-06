#include <stdint.h>

void init_timers(void);
void initTimer0(void);
void delayMS(unsigned int milliseconds); //Using Timer0

void initTimer1(void);
void Timer1enable(void);
void Timer1disable(void);
void TIMER1_IRQHandler(void);
void UpdateTimer1Prescale(uint32_t prescale);

extern uint32_t G_tim_1_prescale;

#define DEFAULT_TIM_0_PRESCALE (24000-1)
#define DEFAULT_TIM_1_PRESCALE (24000-1)
#define DEFAULT_TIM_2_PRESCALE (24000-1)
#define PERIPH_CLOCK 24000000

