
//SRAM1 TO UART2_TX 

#include <stdint.h>
#include "stm32f446xx.h"


void button_init(void);
void uart2_init(void);
void send_some_data(void);
void dma1_init(void);
void dma1_interrupt_configuration(void);
void enable_dma1_stream6(void);

void 	HT_COMPLETE_callback(void);
void 	FT_COMPLETE_callback(void);
void 	TE_error_callback(void);
void 	FE_error_callback(void);
void 	DME_error_callback(void);




#define BASE_ADDR_OF_GPIOC_PERI GPIOC
char data_stream[] = "Hello World\r\n";


int main(void)
{
	button_init();
	uart2_init();
//	send_some_data();
	dma1_init();
	dma1_interrupt_configuration();
	enable_dma1_stream6();
	
	while(1)
	{
		
	}
  return 0;
}

void send_some_data(void)
{
	char data_stream[] = "Hello World\r\n";
	
	USART_TypeDef *pUART2;
	pUART2 = USART2;
	
	//make sure that in the status register TSE is set
	//if TXE is 1, put the byte
	
	uint32_t len = sizeof(data_stream);
	for(uint32_t i = 0 ; i<len ; i++)
	{
		
	//waiting for TXE to become 1
	while( ! (pUART2->SR & (1 << 7) ) );
	
	pUART2->DR = data_stream[i];
	}
}


void button_init(void)
{
	//button is connected to PC13. GPIOC pin number 13.
	
	GPIO_TypeDef *pGPIOC;
	pGPIOC = BASE_ADDR_OF_GPIOC_PERI;
	
	RCC_TypeDef *pRCC;
	pRCC = RCC;
	
	EXTI_TypeDef *pEXTI;
	pEXTI = EXTI;
	
	SYSCFG_TypeDef *pSYSCFG;
	pSYSCFG = SYSCFG; 
	
	//1. enable the peripheral clock for the GPIOC peripheral.
	pRCC ->AHB1ENR |= (1<<2);

	//2. Keep the gpio pin in input mod.
	pGPIOC->MODER &= ~ (0x3 << 26);
	
	//3. Enable the interrupt over that GPIO pin
	pEXTI->IMR  |= (1 << 13);
	
	//4. Enable clock for SYSCFG
	pRCC->APB2ENR |= (1 << 14);
	
	//5. Configuring SYSCFG CR4 register
	pSYSCFG->EXTICR[3] &= ~(0xF <<4); //clearing
	pSYSCFG->EXTICR[3] |=  (0x2 <<4); // setting
	
	//6. Configure the edge detection on that gpio pin
	pEXTI->FTSR |= (1 << 13);

	//7. Enable the IRQ related to that gpio pin in NVIC of the processor.
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void uart2_init(void)
{
	RCC_TypeDef *pRCC;
	pRCC = RCC;
	
	GPIO_TypeDef *pGPIOA;
	pGPIOA = GPIOA;
	
	USART_TypeDef *pUART2;
	pUART2 = USART2;
	
	
	
	//1. Enable the peripheral clock for the uart2 peripheral
	pRCC->APB1ENR |= (1 << 17);
	
	//2. Configure the gpio pins for uart_tx and uart_rx functionality
		//from Alternate function table from datasheet
		//PA2 as TX, PA3 as RX

	
	//lets configure PA2 as UART2 TX       PA2 and AF7
	
		//2.1Enable the clock for the GPIOA peripheral 
		pRCC->AHB1ENR |= (1 << 0);
		//2.2 Change the mode of the PA2 to alternate function mode make it USART2_TX
		pGPIOA->MODER &= ~(0x3 << 4);
		pGPIOA->MODER |=  (0x2 << 4);
		
		pGPIOA->AFR[0] &= ~(0xF << 8);
		GPIOA->AFR[0]  |=  (0x7 << 8);
		
		//2.3 Enable or disable Pull-up resistor of the gpio pin if required	
		GPIOA->PUPDR |= (0x1 << 4); //enables the pull up resistor 
	
		//First lets configure PA3 as UART2 RX
		//2.4 Change the mode of the PA3 to alternate function mode
		pGPIOA->MODER &= ~(0x3 << 6);
		pGPIOA->MODER |=  (0x2 << 6);
		
		pGPIOA->AFR[0] &= ~(0xF << 12);
		GPIOA->AFR[0]  |=  (0x7 << 12);
		//2.5 Enable or disable Pull-up resistor if required
		GPIOA->PUPDR |= (0x1 << 6);
	
	//3. Configure the baudrate
	pUART2->BRR = 0x8B; //set to 115.2KBps
		
	//4. Configure the data width, no of stop bits , etc.	
			// no config. for data with its default is set to , parity selection set to even as default thats ok.
			// stop bit set 1 as default thats ok.
			// no need config. we use default values.
			
	//5. Enable the TX engine of the uart peripheral 
	pUART2->CR1 |=	(1 << 3);	// transmitter enable
	
	//6. Enable the uart peripheral      always enabling of peripheral has to be done at the end,
	pUART2->CR1 |=	(1 << 13);	 			//after configuring all you are control parameters.
																			//So, don't you know start the engine and then configure the registers 
																			//The engine has to be off

}

void dma1_init(void)
{
	RCC_TypeDef *pRCC;
	pRCC = RCC;
	
	DMA_TypeDef *pDMA;
	pDMA = DMA1;
	
	DMA_Stream_TypeDef *pSTREAM6;
	pSTREAM6 = DMA1_Stream6;
	
	USART_TypeDef *pUART2;
	pUART2 = USART2;
	
	
	//1. Enable the peripheral clock for the dma1
	pRCC->AHB1ENR |= (1 << 21);
	
	//2. identify the stream which is suitable for your peripheral
			//Channel4 , stream6
	
	
	//3. identify the channel number which uart2 peripheral send dma request.
			//channel4
	pSTREAM6->CR &= ~(0x7 <<25);
	pSTREAM6->CR |=  (0x4 <<25);

	//4. Program the source address (memory)
	pSTREAM6->M0AR = (uint32_t) data_stream;
	
	//5. Program the destination address (peripheral->UART2 output data register)
	pSTREAM6->PAR = (uint32_t)&pUART2->DR;
	
	//6. Program number of data items to send
		uint32_t len = sizeof(data_stream);
		pSTREAM6->NDTR = len;
	
	//7. The direction of data transfer. m2p,p2m,m2m
		pSTREAM6->CR |= (1 << 6);//memory to peripheral
		
	//8. Program the source and destination data width.
		pSTREAM6->CR &= ~(0x3 << 13);	
		pSTREAM6->CR &= ~(0x3 << 11);

	//9.Enable memort auto increment
		pSTREAM6->CR |= (1 << 10);
		
	//10. Select direct mode or fifo mode
		pSTREAM6->FCR  |= (1 << 2);
		
	//11. Select the fifo threshold 
		pSTREAM6->FCR &= ~(0x3 << 0);
		pSTREAM6->FCR |=  (0x3 << 0);

	//12. enable the circular mode if required 
	  //let as default
	//13. single transfer or burst transfer
	  //let as default
	//14. Configure the stream priority
		 //let as default (low)
	//15. Enable the stream
		pSTREAM6->CR |= (1<<0);
	
}
void enable_dma1_stream6(void)
{
	DMA_Stream_TypeDef *pSTREAM6;
	pSTREAM6 = DMA1_Stream6;
		//15. Enable the stream
		pSTREAM6->CR |= (1<<0);
	
}
void dma1_interrupt_configuration(void)
{
	DMA_Stream_TypeDef *pSTREAM6;
	pSTREAM6 = DMA1_Stream6;
	//1. Half-transfer interrupt enable (HTIE)
	pSTREAM6->CR |= (1<<3);
	//2. Transfer complate interrupt enable (TCIE)
	pSTREAM6->CR |= (1<<4);

	//3. Transfer error interrupt enable (TEIE)
	pSTREAM6->CR |= (1<<2);

	//4. FIFO overrun/underrun interrupt enable (FEIE)
	pSTREAM6->FCR |= (1<<7);

	//5. Direct mode error interrupt enable	(DMEIE)
	pSTREAM6->CR |= (1<<1);

	//6. Enable the IRQ for DMA stream6 global interrupt in NVIC
	NVIC_EnableIRQ(DMA1_Stream6_IRQn); //ready to receive the interrupt from the DMA controller
}

void 	HT_COMPLETE_callback(void)
{
	
}
void 	FT_COMPLETE_callback(void)
{
	//Trigger request again after communication
	USART_TypeDef *pUART2;
	pUART2 = USART2;
	
	DMA_Stream_TypeDef *pSTREAM6;
	pSTREAM6 = DMA1_Stream6;
	
	//when complete callback NDTR value 0
	uint32_t len = sizeof(data_stream);
	pSTREAM6->NDTR = len;
	
	//stopping the DMA request
	pUART2->CR3 &= ~(1 << 7 );
	
	//enable stream
	enable_dma1_stream6();
	
}

void 	TE_error_callback(void)
{
	while(1);
}
void 	FE_error_callback(void)
{
	while(1);
}

void 	DME_error_callback(void)
{
	while(1);
}

