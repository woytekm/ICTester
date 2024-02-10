#include "LPC17xx.h"                              /* LPC17xx definitions    */
#include "LPC17xx_spi.h"


/* bit definitions for register SSPCR0. */
#define SSPCR0_DSS      0
#define SSPCR0_CPOL     6
#define SSPCR0_CPHA     7
#define SSPCR0_SCR      8
/* bit definitions for register SSPCR1. */
#define SSPCR1_SSE      1
/* bit definitions for register SSPSR. */
#define SSPSR_TFE       0
#define SSPSR_TNF       1
#define SSPSR_RNE       2
#define SSPSR_RFF       3
#define SSPSR_BSY       4

/* Local functions */
static uint8_t LPC17xx_SPI_SendRecvByte (uint8_t byte_s);


/* Initialize the SSP0, SSP0_PCLK=CCLK=72MHz */
void LPC17xx_SPI_Init (void) 
{
	uint32_t dummy = 0;

	dummy = dummy; // avoid warning

	/* Initialize and enable the SSP0 Interface module. */		
	LPC_SC->PCONP |= (1 << 10);          /* Enable power to SSPI1 block  */
	
	/* SSEL is GPIO, output set to high. */
	LPC_GPIO0->FIODIR  |=  (1<<6);             /* P0.6 is output */
	LPC17xx_SPI_DeSelect ();		   /* set P0.6 high (SSEL inactiv) */
	
        LPC_PINCON->PINSEL0 &= ~(0x3F<<14);  /* P0.7,8,9 cleared  */
        /* ... an then set to function 2 */
        LPC_PINCON->PINSEL0 |=  (2UL<<14) | (2UL<<16) | (2UL<<18);
	
	/* PCLK_SSP0=CCLK */
	LPC_SC->PCLKSEL1 &= ~(3<<20);               /* PCLKSP1 = CCLK/4 (18MHz) */
	LPC_SC->PCLKSEL1 |=  (1<<20);               /* PCLKSP1 = CCLK   (72MHz) */

	LPC_SSP1->CR0  = 0x0007;                    /* 8Bit, CPOL=0, CPHA=0         */
	LPC_SSP1->CR1  = 0x0002;                    /* SSP1 enable, master          */

	LPC17xx_SPI_SetSpeed (SPI_SPEED_400kHz);

	/* wait for busy gone */
	while( LPC_SSP1->SR & ( 1 << SSPSR_BSY ) );

	/* drain SPI RX FIFO */
	while( LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) 
	{
		dummy = LPC_SSP1->DR;
	}	
}

/* Close SSP0 */
void LPC17xx_SPI_DeInit( void )
{
	// disable SPI
	LPC_SSP1->CR1  = 0;

	// Pins to GPIO
	LPC_PINCON->PINSEL0 &= ~(3UL<<30);
	LPC_PINCON->PINSEL1 &= ~((3<<2) | (3<<4));

	// disable SSP power
	LPC_SC->PCONP &= ~(1 << 10);
}


/* Set a SSP0 clock speed to desired value. */
void LPC17xx_SPI_SetSpeed (uint8_t speed)
{
	speed &= 0xFE;
	if ( speed < 2  ) {
		speed = 2 ;
	}
	LPC_SSP1->CPSR = speed;
}

/* SSEL: low */
void LPC17xx_SPI_Select ()
{
	LPC_GPIO0->FIOPIN &= ~(1<<6);	
}
/* SSEL: high */
void LPC17xx_SPI_DeSelect ()
{
	LPC_GPIO0->FIOPIN |= (1<<6);	
}

/* Send one byte then recv one byte of response. */
static uint8_t LPC17xx_SPI_SendRecvByte (uint8_t byte_s)
{
	uint8_t byte_r;

	LPC_SSP1->DR = byte_s;
	while (LPC_SSP1->SR & (1 << SSPSR_BSY) /*BSY*/); 	/* Wait for transfer to finish */
//	while( !( LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) );	/* Wait untill the Rx FIFO is not empty */
	byte_r = LPC_SSP1->DR;

	return byte_r;                      /* Return received value */	
}

/* Send one byte */
void LPC17xx_SPI_SendByte (uint8_t data)
{
	LPC17xx_SPI_SendRecvByte (data);	
}
/* Recv one byte */
uint8_t LPC17xx_SPI_RecvByte ()
{
	return LPC17xx_SPI_SendRecvByte (0xFF);	
}

/* Release SSP0 */
void LPC17xx_SPI_Release (void)
{
	LPC17xx_SPI_DeSelect ();
	LPC17xx_SPI_RecvByte ();
}


#if USE_FIFO
/* on LPC17xx the FIFOs have 8 elements which each can hold up to 16 bits */
#define FIFO_ELEM 8

/* Receive btr (must be multiple of 4) bytes of data and store in buff. */
void LPC17xx_SPI_RecvBlock_FIFO (uint8_t *buff,	uint32_t btr)
{
	uint32_t hwtr, startcnt, i, rec;

	hwtr = btr/2;  /* byte number in unit of short */
	if ( btr < FIFO_ELEM ) {
		startcnt = hwtr;
	} else {
		startcnt = FIFO_ELEM;
	}

	LPC_SSP1 -> CR0 |= 0x0f;  /* DSS to 16 bit */

	for ( i = startcnt; i; i-- ) {
		LPC_SSP1 -> DR = 0xffff;  /* fill TX FIFO, prepare clk for receive */
	}

	do {
		while ( !(LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) ) {
			// wait for data in RX FIFO (RNE set)
		}
		rec = LPC_SSP1->DR;
		if ( i < ( hwtr - startcnt ) ) {
			LPC_SSP1->DR = 0xffff;	/* fill TX FIFO, prepare clk for receive */
		}
		*buff++ = (uint8_t)(rec>>8);
		*buff++ = (uint8_t)(rec);
		i++;
	} while ( i < hwtr );

	LPC_SSP1->CR0 &= ~0x08;  /* DSS to 8 bit */
}

/* Send 512 bytes of data block (stored in buff). */
void LPC17xx_SPI_SendBlock_FIFO (const uint8_t *buff)
{
	uint32_t cnt;
	uint16_t data;

	LPC_SSP1->CR0 |= 0x0f;  /* DSS to 16 bit */

	/* fill the FIFO unless it is full */
	for ( cnt = 0; cnt < ( 512 / 2 ); cnt++ ) 
	{
		/* wait for TX FIFO not full (TNF) */
		while ( !( LPC_SSP1->SR & ( 1 << SSPSR_TNF ) ) );
		
		data  = (*buff++) << 8;
		data |= *buff++;
		LPC_SSP1->DR = data;
	}

	/* wait for BSY gone */
	while ( LPC_SSP1->SR & ( 1 << SSPSR_BSY ) );

	/* drain receive FIFO */
	while ( LPC_SSP1->SR & ( 1 << SSPSR_RNE ) ) {
		data = LPC_SSP1->DR; 
	}

	LPC_SSP1->CR0 &= ~0x08;  /* DSS to 8 bit */
}
#endif /* USE_FIFO */
