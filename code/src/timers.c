#include "timers.h"
#include "LPC17xx.h"
#include "gpio.h"
#include "gpio_pin.h"
#include "utils.h"

uint32_t G_tim_1_prescale;


void initTimer0(void)
{
	/*Assuming that PLL0 has been setup with CCLK = 96Mhz and PCLK = 24Mhz.*/
	LPC_SC->PCONP |= (1<<1); //Power up TIM0. By default TIM0 and TIM1 are enabled.
	//LPC_SC->PCLKSEL0 &= ~(0x3<<3); //Set PCLK for timer = CCLK/4 = 96/4 (default)

	LPC_TIM0->CTCR = 0x0;
	LPC_TIM0->PR = DEFAULT_TIM_0_PRESCALE; //Increment TC at every 23999+1 clock cycles
	//25000 clock cycles @25Mhz = 1 mS

	LPC_TIM0->TCR = 0x02; //Reset Timer
}


void delayMS(unsigned int milliseconds) //Using Timer0
{
	LPC_TIM0->TCR = 0x02; //Reset Timer

	LPC_TIM0->TCR = 0x01; //Enable timer

	while(LPC_TIM0->TC < milliseconds); //wait until timer counter reaches the desired delay

	LPC_TIM0->TCR = 0x00; //Disable timer
}

void delayuS(unsigned int microseconds) //Using Timer2
{
        LPC_TIM2->TCR = 0x02; //Reset Timer

        LPC_TIM2->TCR = 0x01; //Enable timer

        while(LPC_TIM2->TC < microseconds); //wait until timer counter reaches the desired delay

        LPC_TIM2->TCR = 0x00; //Disable timer
}


void initTimer2(void)
{
        /*Assuming that PLL0 has been setup with CCLK = 96Mhz and PCLK = 24Mhz.*/
        LPC_SC->PCONP |= (1<<22); //Power up TIM2.
        //LPC_SC->PCLKSEL0 &= ~(0x3<<3); //Set PCLK for timer = CCLK/4 = 96/4 (default)

        LPC_TIM2->CTCR = 0x0;
        LPC_TIM2->PR = DEFAULT_TIM_2_PRESCALE; //Increment TC at every 24 clock cycles

        LPC_TIM2->TCR = 0x02; //Reset Timer
}


void initTimer1(void)
{

        G_tim_1_prescale = DEFAULT_TIM_1_PRESCALE;
	/*Assuming that PLL0 has been setup with CCLK = 96Mhz and PCLK = 24Mhz.*/
	LPC_SC->PCONP |= 1; //Power up TIM1. By default TIM0 and TIM1 are enabled.
	//LPC_SC->PCLKSEL0 &= ~(0x0<<5); //Set PCLK for timer = CCLK/4 = 96/4 (default)
	
	LPC_TIM1->CTCR = 0x0;
	LPC_TIM1->PR = DEFAULT_TIM_1_PRESCALE;  // (24000-1)
	
	LPC_TIM1->MR0 = 1; //Toggle Time 
	LPC_TIM1->MCR |= (1<<0) | (1<<1); // Interrupt & Reset on MR0 match
	LPC_TIM1->TCR |= (1<<1); //Reset Timer0

	NVIC_EnableIRQ(TIMER1_IRQn); //Enable timer interrupt
	
	LPC_TIM1->TCR = 0x00; //Disable timer
 
}

void Timer1enable(void)
 {
  LPC_TIM1->TCR = 0x01;
  vcom_printf("LPC_TIM1->TCR set to 0x01 (%X)\n\r",LPC_TIM1->TCR);
  vcom_printf("LPC_TIM1 PR:(%X) MCR:(%X) MR0:(%X) TCR:(%X)\n\r",LPC_TIM1->PR,LPC_TIM1->MCR,LPC_TIM1->MR0,LPC_TIM1->TCR);
  vcom_printf("LPC_SC->PCLKSEL0:(%X), LPC_SC->PCLKSEL1:(%X)\r\n",LPC_SC->PCLKSEL0,LPC_SC->PCLKSEL1);
 }

void Timer1disable(void)
 {
  LPC_TIM1->TCR = 0x00;
  vcom_printf( "LPC_TIM1->TCR set to 0x00 (%X)\n\r",LPC_TIM1->TCR);
  vcom_printf( "LPC_TIM1 PR:(%X) MCR:(%X) MR0:(%X) TCR:(%X)\n\r",LPC_TIM1->PR,LPC_TIM1->MCR,LPC_TIM1->MR0,LPC_TIM1->TCR);
  vcom_printf("LPC_SC->PCLKSEL0:(%X), LPC_SC->PCLKSEL1:(%X)\r\n",LPC_SC->PCLKSEL0,LPC_SC->PCLKSEL1);
 }

void UpdateTimer1Prescale(uint32_t prescale)
 {
  uint32_t TCR_save = 0x0;
  
  if(LPC_TIM1->TCR != 0x00)
   {
    TCR_save = LPC_TIM1->TCR;
    LPC_TIM1->TCR = 0x00;
   }
  LPC_TIM1->PR = prescale;
  LPC_TIM1->TCR = TCR_save;
 }

void TIMER1_IRQHandler(void) 
{
  LPC_TIM1->IR |= (1<<0); //Clear MR0 Interrupt flag
  if((G_clock_pin&&G_clock_state) != 0)
    toggle_pin(G_pin_array[G_clock_pin].gpio_id);
}


void init_timers(void)
 {
  initTimer0();
  initTimer1();
  initTimer2();
 }



