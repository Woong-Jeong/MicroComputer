/////////////////////////////////////////////////////////////
// 과제명: HW4. 오목게임(USART 이용)
// 과제개요: STM32F407 마이크로컨트롤러를 이용해 오목게임을 구현
// 사용한 하드웨어(기능): GPIO, Switch, Joy-stick, LED, Buzzer,
//                        GLCD, EXTI, FRAM, USART, PC
// 제출일: 2024. 11. 28. 
// 제출자 클래스: 수요일반
// 학번: 2020134035
// 이름: 정 웅
///////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"


/************************  User Define  ************************/

#define SW7 0x7F00			//PH 15

/************************  Function Define  ************************/

///////////////////////// ALARM /////////////////////////
void USART1_Init(void);                                 // USART1 설정
void USART_BRR_Configuration(uint32_t USART_BaudRate);  // USART Baudrate 설정
void TIMER6_Init(void);                                 // Timer6 초기화
void NowTime(void);                                     // 현재시각을 표시하는 함수
void AlarmTime(void);                                   // 알람시각을 표시하는 함수
void FirstPage(void);                                   // 1번화면 
uint16_t now_H = 0x0F;                                  // 현재시각 '시'
uint16_t now_M = 0x0A;                                  // 현재시각 '분'
uint16_t alarm_H;                                       // 알람시각 '시'
uint16_t Rx;                                            // PC로 부터 받은 데이터
///////////////////////// Ball /////////////////////////


///////////////////////// Thermostat /////////////////////////


///////////////////////// 공통 /////////////////////////
void _GPIO_Init(void);          // GPIO_INIT
void _EXTI_Init(void);          // EXTI_INIT
void DisplayInitScreen(void);   // DisplayScreen_INIT
void BEEP(void);                // Buzzer
void BEEP_3(void);              // Buzzer x 3
void DelayMS(unsigned short wMS);       // Delay_Millisecond
void DelayUS(unsigned short wUS);       // Delay_Microsecond

/************************  Main  ************************/

void main()
{
    LCD_Init();              // LCD initialization
    DelayMS(10);
    _GPIO_Init();            // GPIO initialization
    _EXTI_Init();            // EXTI initialization
    DisplayInitScreen();     // User Display initialization
    Fram_Init();                  // FRAM 초기화 H/W 초기화
	Fram_Status_Config(); 		  // FRAM 초기화 S/W 초기화
    Fram_Write(1200,0);
    alarm_H = Fram_Read(1200);    // 알람시각 '시'
    USART1_Init();           // USART1 INIT
    TIMER6_Init();           // TIMER6 INIT

    while (1)
    {
    }
}

/************************  INIT  ************************/
void DisplayInitScreen(void)
{
    FirstPage();
}

/* GPIO (GPIOG(LED), GPIOI(JoyStick), GPIOF(Buzzer)) 초기 설정	*/
void _GPIO_Init(void)
{
	// Buzzer (GPIO F) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable	
	GPIOF->MODER 	&= ~0x000B0000;	// GPIOF 9 : Clear						
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)	
	GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull 
	GPIOF->OSPEEDR  &= ~0x000B0000; // GPIOF 9 : Clear(0b00)	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	// LED (GPIO G) 설정 : LED0(PG0), LED7(PG7) Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00004001;	// GPIOG 0,7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00004001;	// GPIOG 0,7 : Push-pull
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00004001;	// GPIOG 0,7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)
	
    //Joy Stick SW(GPIO I) 설정 : Input mode(PI5 ~ PI9)
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FF000;	// GPIOI 6~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FF000;	// GPIOI 6~9 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI5,8(JOY_PUSH(PI5),SW0(PH8))
	EXTI15(PH15, SW7))초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0xC0010000;	// GPIOH 8,15 : Input mode
    GPIOI->MODER	&= ~0x00000C00;	// GPIOI 5 : Input mode (reset state)
	// EXTI5 <- PI5, EXTI8 <- PH8 
	// EXTI15 <- PH15 
	// EXTICR2(EXTICR[1]), EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])를 이용 
	// reset value: 0x0000
    SYSCFG->EXTICR[1] &= ~0x00F0;	// EXTICR2 clear
	SYSCFG->EXTICR[1] |= 0x0080;	// EXTI5 -> GPIOI
    SYSCFG->EXTICR[2] &= ~0x000F;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x0007;	// EXTI8 -> GPIOH
    SYSCFG->EXTICR[3] &= ~0xF000;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x7000;	// EXTI15 -> GPIOH
	
	EXTI->FTSR |= 0x8120;			// EXTI5,8,15: Falling Trigger Enable  
	EXTI->IMR  &= ~0x8120;			// EXTI5,8,15: IMR Reg 초기화
	EXTI->IMR  |= 0x8100;			// EXTI8,15: UnMasked (Interrupt Enable) 설정

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI5,8'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI15'
}

// USART1 기본 설정
void USART1_Init(void)
{
	// USART1 : TX(PA9)
	RCC->AHB1ENR	|= (1<<0);	    // RCC_AHB1ENR GPIOA Enable
	GPIOA->MODER	|= (2<<2*9);	// GPIOA PIN9 Output Alternate function mode					
	GPIOA->OSPEEDR	|= (3<<2*9);	// GPIOA PIN9 Output speed (100MHz Very High speed)
	GPIOA->AFR[1]	|= (7<<4);	    // Connect GPIOA pin9 to AF7(USART1)
    
	// USART1 : RX(PA10)
	GPIOA->MODER 	|= (2<<2*10);	// GPIOA PIN10 Output Alternate function mode
	GPIOA->OSPEEDR	|= (3<<2*10);	// GPIOA PIN10 Output speed (100MHz Very High speed
	GPIOA->AFR[1]	|= (7<<8);	    // Connect GPIOA pin10 to AF7(USART1)

	RCC->APB2ENR	|= (1<<4);	    // RCC_APB2ENR USART1 Enable
    
	USART_BRR_Configuration(9600); // USART Baud rate Configuration: 38400
    
	USART1->CR1	|= (1<<12);			// USART_WordLength 9 Data bit

	USART1->CR1	|= (1<<10);			// USE USART_Parity
	USART1->CR1	&= ~(1<<9);	        // Even_Parity config

	USART1->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
	USART1->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable

	USART1->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
	USART1->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
    
	USART1->CR1 	|= (1<<5);	// 0x0020, RXNE interrupt Enable
	NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37번)
	USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}
// TIMER6 기본 설정
void TIMER6_Init(void)
{
// Enable Timer CLK 

	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;	// RCC_APB1ENR TIMER6 Enable

	TIM6->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
				//   - Counter Overflow/Underflow, 
				//   - Setting the UG bit Set,
				//   - Update Generation through the slave mode controller 
				// UDIS=1 : Only Update event Enabled by Counter Overflow/Underflow,
	TIM6->CR1 |= (1<<2);	
				// URS=1 : Only Update Interrupt generated By Counter Overflow/Underflow,
	TIM6->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM6->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)

// Deciding the Period
	TIM6->PSC = 8400-1;	        // Prescaler 84,000,000Hz/8400 = 10,000 Hz (0.1ms)  (1~65536)
	TIM6->ARR = 10000-1;		// Auto reload  0.1ms * 10000 = 1s

// Clear the Counter
	TIM6->EGR |= (1<<0);	// UG(Update generation)=1 
				// Re-initialize the counter(CNT=0) & generates an update of registers   

// Setting an UI(UEV) Interrupt 
	NVIC->ISER[1] |= (1<<54-32); 	// Enable Timer6 global Interrupt
 	TIM6->DIER |= (1<<0);	// Enable the TIM6 Update interrupt

	TIM6->CR1 |= (1<<0);	// Enable the TIM6 Counter (clock enable)   
}
// 초기화면 설정

/************************   USER Function  ************************/
// 현재시각을 표시하는 함수
void NowTime(void)
{
    LCD_SetTextColor(RGB_BLUE);              // 글자색 : BLUE
    // '시'가 0~9일때와 아닐때를 구분해서 display
    LCD_DisplayChar(0, 13, (now_H < 10) ? (now_H + 0x30) : (now_H + 0x37));
	LCD_DisplayText(0, 14, ":");
    // '분'이 0~9일때와 아닐때를 구분해서 display
    LCD_DisplayChar(0, 15, (now_M < 10) ? (now_M + 0x30) : (now_M + 0x37));
}

// 알람시각을 표시하는 함수
void AlarmTime(void)
{
    LCD_SetTextColor(RGB_RED);                // 글자색 : RED
    // '시'가 0~9일때와 아닐때를 구분해서 display
    LCD_DisplayChar(1, 7, (alarm_H < 10) ? (alarm_H + 0x30) : (alarm_H+0x37));
    LCD_SetTextColor(RGB_BLACK);              // 글자색 : BLACK
    LCD_DisplayText(1, 8, ":");
    LCD_DisplayChar(1, 9, 0+0x30);
}

// 첫번째 화면 설정
void FirstPage(void)
{
    LCD_Clear(RGB_WHITE);		    // 화면 클리어
	LCD_SetFont(&Gulim8);		    // 폰트 : 굴림 8
	//Title 작성
	LCD_SetBackColor(RGB_WHITE);             // 글자배경색 : WHITE
    LCD_SetTextColor(RGB_BLACK);              // 글자색 : BLACK
	LCD_DisplayText(0, 0, "1.ALARM(JW)");  // Title 작성
    LCD_DisplayText(1, 0, "Alarm");  // Title 작성
	// 현재시각 표시
    NowTime();
    // 알람시각 표시
    AlarmTime();
}

// Baud rate 설정
void USART_BRR_Configuration(uint32_t USART_BaudRate)
{ 
	uint32_t tmpreg = 0x00;
	uint32_t APB2clock = 84000000;	//PCLK2_Frequency
	uint32_t integerdivider = 0x00;
	uint32_t fractionaldivider = 0x00;

	// Find the integer part 
	if ((USART1->CR1 & USART_CR1_OVER8) != 0) // USART_CR1_OVER8=(1<<15)
	//  #define  USART_CR1_OVER8 ((uint16_t)0x8000) // USART Oversampling by 8 enable   
	{       // USART1->CR1.OVER8 = 1 (8 oversampling)
		// Computing 'Integer part' when the oversampling mode is 8 Samples 
		integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));  // 공식에 100을 곱한 곳임(소수점 두번째자리까지 유지하기 위함)  
	}
	else  // USART1->CR1.OVER8 = 0 (16 oversampling)
	{	// Computing 'Integer part' when the oversampling mode is 16 Samples 
		integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));  // 공식에 100을 곱한 곳임(소수점 두번째자리까지 유지하기 위함)    
	}								     // 100*(f_CK) / (8*2*Buadrate) = (25*f_CK)/(4*Buadrate)	
	tmpreg = (integerdivider / 100) << 4;

	// Find the fractional part 
	fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

	// Implement the fractional part in the register 
	if ((USART1->CR1 & USART_CR1_OVER8) != 0)	
	{	// 8 oversampling
		tmpreg |= (((fractionaldivider * 8) + 50) / 100) & (0x07);
	}
	else	// 16 oversampling
	{
		tmpreg |= (((fractionaldivider * 16) + 50) / 100) & (0x0F);
	}

	// Write to USART BRR register
	USART1->BRR = (uint16_t)tmpreg;
}







/* Buzzer: Beep for 30 ms */
void BEEP(void)
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void BEEP_3(void)
{ 	
    for(int i=0; i<3 ;i++)
    {
        GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
        DelayMS(30);			// Delay 30 ms
        GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
        DelayMS(500);
    }
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}
/************************  Handler  ************************/

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// 청돌선택 (EXTI15,SW 7)
	if(EXTI->PR & 0x8000)			// EXTI11 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
	}
}

void USART1_IRQHandler(void)	
{
    if (USART1->SR & USART_SR_RXNE) // 데이터가 수신되었는지 확인
    {
        Rx = (uint8_t)(USART1->DR & 0xFF); // 수신된 데이터 읽기
        alarm_H = Rx;         // 데이터를 배열에 저장
        Fram_Write(1200,alarm_H);
        AlarmTime();
    }
    
	// DR 을 읽으면 SR.RXNE bit(flag bit)는 clear 된다. 즉 clear 할 필요없음
}

// TIM6 interrupt 설정
void TIM6_DAC_IRQHandler(void)
{
    if (TIM6->SR & 0x01) // Update interrupt flag
	{
		TIM6->SR &= ~(1 << 0); // Update Interrupt Claer
		now_M++;
		if(now_M > 0xF)
		{
			now_M = 0;
			now_H++;
			if(now_H >= 0xF)	now_H = 0;
		}
        NowTime();
	}
}