//////////////////////////////////////////////////////////////////////////////
// 과제명: HW3. 온도(가변저항)연동 냉방기 제어
// 제출자: 2020134035, 정 웅
// 제출일: 2024. 11. 09.
// 과제개요: ADC3_IN1을 사용하여 가변저항 출력전압을 AD전환함. 
//			 변환 디지털 값으로부터 전압값 및 온도값 계산하여 LCD에 표시.
//			 ADC3_IN1의 Start trigger는 TIM1_CH3의 CC event 사용
//			 냉각제어 신호는 TIM14_CH1(PWM mode)를 사용, 부저를 출력으로 사용
//			 SW로 TIM14 출력 Enable/Disable
//////////////////////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

void DisplayTitle(void);
void TIMER1_Init(void);
void TIMER14_Init(void);
void _GPIO_Init(void);
void _ADC_Init(void);

void BEEP(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
uint16_t KEY_Scan(void);

uint16_t ADC_Value, Voltage;
uint16_t Temp, DR;
int SW7_flag = 0;

void ADC_IRQHandler(void)
{
	ADC3->SR &= ~(1<<1);		// EOC flag clear

	ADC_Value = ADC3->DR;		// Reading ADC result
	Voltage = ADC_Value * (3.3 * 100) / 4095;   // 3.3 : 4095 =  Volatge : ADC_Value 
                                                    // 100:  소수점아래 두자리까지 표시하기 위한 값  
	LCD_DisplayChar(2,4,Voltage/100 + 0x30);
	LCD_DisplayText(2,5,".");
	LCD_DisplayChar(2,6,Voltage%100/10 + 0x30);
	LCD_DisplayChar(2,7,Voltage%10 + 0x30);

	Temp = (3.5 * Voltage/100 * Voltage/100 + 1) * 10;	// 10: 소수점아래 한자리까지 표시하기 위한 값
	LCD_DisplayChar(3,4,Temp/100 + 0x30);
	LCD_DisplayChar(3,5,Temp%100/10 + 0x30);
	LCD_DisplayText(3,6,".");
	LCD_DisplayChar(3,7,Temp%10 + 0x30);

	DR = Temp * 2;									//DutyRatio 정의
	LCD_DisplayChar(4,3,DR/100 + 0x30);
	LCD_DisplayChar(4,4,DR%100/10 + 0x30);
	LCD_DisplayText(4,5,".");
	LCD_DisplayChar(4,6,DR%10 + 0x30);

	TIM14->CCR1 = DR;	// TIM14 CCR1 TIM14_Pulse update
	
	// 그래프 표시
	// 온도 1도당 그래프 길이 3.6으로 설정
	LCD_SetBrushColor(RGB_GREEN);
    LCD_DrawFillRect(10, 65, (Temp/10)*3.6, 12);
	LCD_SetBrushColor(RGB_YELLOW);
    LCD_DrawFillRect(10 + (Temp/10)*3.6, 65, 140-(Temp/10)*3.6, 12);
}

int main(void)
{
	LCD_Init();	// LCD 구동 함수
	DelayMS(10);	// LCD구동 딜레이
 	DisplayTitle();	//LCD 초기화면구동 함수
	_GPIO_Init();
	_ADC_Init();
	TIMER1_Init();
	TIMER14_Init();

	// NO SWSTART !!!!
	while(1)
	{
		switch (KEY_Scan())
		{
			case SW7_PUSH:
				if(SW7_flag == 0)	// Buzzer off일때
				{
					TIM14->CCER |= (1<<0);	// CC1E=1: CC1 channel Output Enable
					SW7_flag = 1;
				}	
				else if(SW7_flag == 1)	// Buzzer on일때
				{
					TIM14->CCER &= ~(1<<0);	// CC1E=1: CC1 channel Output Disable
					SW7_flag = 0;
				}
				
			break;
		}
    }
}

void DisplayTitle(void)
{
	LCD_Clear(RGB_YELLOW);
	LCD_SetFont(&Gulim7);		//폰트 
	LCD_SetBackColor(RGB_WHITE);	//글자배경색
	LCD_SetTextColor(RGB_BLACK);	//글자색
    LCD_DisplayText(0,0,"Air Conditioner Control");

    LCD_SetFont(&Gulim8);		//폰트 
    LCD_SetBackColor(RGB_YELLOW);	//글자배경색
    LCD_DisplayText(1,0,"2020134035 JW");
    LCD_DisplayText(2,0,"VOL:    V");
    LCD_DisplayText(3,0,"TMP:    C");
    LCD_DisplayText(4,0,"DR:    %");
	LCD_SetTextColor(RGB_BLUE);	//글자색

    LCD_SetBrushColor(RGB_GREEN);
	LCD_SetPenColor(RGB_GREEN);	
	LCD_DrawRectangle(9, 64, 141, 13);	//그래프 틀 그리기
    LCD_DrawFillRect(10, 65, Temp*36/10, 12);
}

void TIMER1_Init(void)
{
// TIM1_CH3 (PE13) : 200ms 이벤트 발생
// Clock Enable : GPIOE & TIMER1
	RCC->AHB1ENR	|= (1<<4);	// GPIOE Enable
	RCC->APB2ENR 	|= (1<<0);	// TIMER1 Enable 

// P을 출력설정하고 Alternate function(TIM1_CH3)으로 사용 선언 
	GPIOE->MODER 	|= (2<<2*13);	// PE13 Output Alternate function mode					
	GPIOE->OSPEEDR 	|= (3<<2*13);	// PE13 Output speed (100MHz High speed)
	GPIOE->OTYPER	&= ~(1<<13);	// PE13 Output type push-pull (reset state)
	GPIOE->AFR[1]	|= (1 << 4*(13-8)); 	// : Connect TIM1 pins(PE13) to AF1(TIM1/2)
					// PE13 ==> TIM1_CH3

	// Assign 'Interrupt Period' and 'Output Pulse Period'
	TIM1->PSC = 1680-1;	// Prescaler 168MHz/1680 = 0.1MHz (10us)
	TIM1->ARR = 20000-1;	// Auto reload  : 10us * 20K = 200ms(period)

	// CR1 : Up counting
	TIM1->CR1 &= ~(1<<4);	// DIR=0(Up counter)(reset state)
	TIM1->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM1->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM1->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM1->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM1->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	TIM1->CR1 &= ~(3<<5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
				// Center-aligned mode: The counter counts Up and DOWN alternatively

	// Event & Interrup Enable : UI  
	TIM1->EGR |= (1<<0);    // UG: Update generation    
    
	// Define the corresponding pin by 'Output'  
	TIM1->CCER |= (1<<8);	// CC3E=1: CC3 channel Output Enable
				// OC3(TIM1_CH3) Active: 해당핀을 통해 신호출력
	TIM1->CCER &= ~(1<<9);	// CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC3으로 반전없이 출력)  

	// 'Mode' Selection : Output mode, toggle  
	TIM1->CCMR2 &= ~(3<<0); // CC3S(CC3 channel) = '0b00' : Output 
	TIM1->CCMR2 &= ~(1<<3); // OC3P=0: Output Compare 3 preload disable
	TIM1->CCMR2 |= (3<<4);	// OC3M=0b011: Output Compare 3 Mode : toggle
				            // OC3REF toggles when CNT = CCR3

 	TIM1->CCR3 = 10000;	// TIM1 CCR3 TIM1_Pulse
	TIM1->BDTR |= (1<<15);  // main output enable

	TIM1->CR1 |= (1<<0);	// CEN: Enable the TIM1 Counter  					
}

void TIMER14_Init(void)
{
// TIM14_CH1 (PF9) : PWM 이벤트 발생
// Clock Enable : GPIOF & TIMER14
	RCC->AHB1ENR	|= (1<<5);	// GPIOF Enable
	RCC->APB1ENR 	|= (1<<8);	// TIMER14 Enable 

// PF9를 출력설정하고 Alternate function(TIM14_CH1)으로 사용 선언 
	GPIOF->MODER 	|= (2<<2*9);	// PF9 Output Alternate function mode					
	GPIOF->OSPEEDR 	|= (3<<2*9);	// PF9 Output speed (100MHz High speed)
	GPIOF->OTYPER	&= ~(1<<9);	// PF9 Output type push-pull (reset state)
	GPIOF->AFR[1]	|= (9 << 4*(9-8)); 	// : Connect TIM14 pins(PF9) to AF9(CAN1/CAN2, TIM12..14)
					// PF9 ==> TIM14_CH1

	// Assign 'Interrupt Period' and 'Output Pulse Period'
	TIM14->PSC = 84-1;	// Prescaler 84MHz/84 = 1MHz (1us)
	TIM14->ARR = 1000-1;	// Auto reload  : 1us * 1000 = 1ms(period),(1000Hz)

	// CR1 : Up counting
	TIM14->CR1 &= ~(1<<1);	// UDIS=0(Update event Enabled): By one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM14->CR1 &= ~(1<<2);	// URS=0(Update event source Selection): one of following events
				//	- Counter Overflow/Underflow, 
				// 	- Setting the UG bit Set,
				//	- Update Generation through the slave mode controller 
	TIM14->CR1 &= ~(1<<3);	// OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM14->CR1 &= ~(1<<7);	// ARPE=0(ARR is NOT buffered) (reset state)
	TIM14->CR1 &= ~(3<<8); 	// CKD(Clock division)=00(reset state)
	
	// Event & Interrup Enable : UI  
	TIM14->EGR |= (1<<0);    // UG: Update generation    
    
	// Define the corresponding pin by 'Output'  
	TIM14->CCER &= ~(1<<0);	// CC1E=1: CC1 channel Output Disable
				// OC3(TIM14_CH1) Active: 해당핀을 통해 신호출력
	TIM14->CCER &= ~(1<<1);	// CC1P=0: CC1 channel Output Polarity (OCPolarity_sHigh : OC1으로 반전없이 출력)  

	// 'Mode' Selection : Output mode, toggle  
	TIM14->CCMR1 &= ~(3<<0); // CC1S(CC1 channel) = '0b00' : Output 
	TIM14->CCMR1 &= ~(1<<3); // OC1PE=0: Output Compare 1 preload disable
	TIM14->CCMR1 |= (6<<4);	// OC1M=0b110:  PWM mode 1
				            // Channel 1 active at TIMx_CNT < TIMx_CCR1

 	TIM14->CCR1 = DR;	// TIM14 CCR1 TIM14_Pulse
	NVIC->ISER[1] |= (1 << (45-32));
	TIM14->CR1 |= (1<<0);	// CEN: Enable the TIM14 Counter  					
}

void _ADC_Init(void)
{   // ADC3_IN1: 가변저항 PA1(pin 41)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// (1<<0) ENABLE GPIOA CLK (stm32f4xx.h 참조)
	GPIOA->MODER |= (3<<2*1);				// CONFIG GPIOA PIN1(PA1) TO ANALOG IN MODE
						
	RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;	// (1<<10) ENABLE ADC3 CLK (stm32f4xx.h 참조)

	ADC->CCR &= ~(0X1F<<0);		// MULTI[4:0]: ADC_Mode_Independent
	ADC->CCR |=  (1<<16); 		// 0x00010000 ADCPRE:ADC_Prescaler_Div4 (ADC MAX Clock 36MHz, 84Mhz(APB2)/4 = 21MHz)
        
	ADC3->CR1 &= ~(3<<24);		// RES[1:0]= 0x00 : 12bit Resolution
	ADC3->CR1 &= ~(1<<8);		// SCAN=0 : ADC_ScanCovMode Disable
	ADC3->CR1 |=  (1<<5);		// EOCIE=1: Interrupt enable for EOC

	ADC3->CR2 &= ~(1<<1);		// CONT=0: ADC_Continuous ConvMode Disable
	ADC3->CR2 |=  (2<<28);		// EXTEN[1:0]: ADC_ExternalTrigConvEdge_Enable(Both Edge)
	ADC3->CR2 |= (2<<24);	    // TIM1 CC3 event
	ADC3->CR2 &= ~(1<<11);		// ALIGN=0: ADC_DataAlign_Right
	ADC3->CR2 &= ~(1<<10);		// EOCS=0: The EOC bit is set at the end of each sequence of regular conversions

	ADC3->SQR1 &= ~(0xF<<20);	// L[3:0]=0b0000: ADC Regular channel sequece length 
					// 0b0000:1 conversion)
 	//Channel selection, The Conversion Sequence of PIN1(ADC3_CH1) is first, Config sequence Range is possible from 0 to 17
	ADC3->SQR3 |= (1<<0);		// SQ1[4:0]=0b0001 : CH1
	ADC3->SMPR2 |= (0x7<<(3*1));	// ADC3_CH1 Sample TIme_480Cycles (3*Channel_1)
 	//Channel selection, The Conversion Sequence of PIN1(ADC3_CH1) is first, Config sequence Range is possible from 0 to 17

	NVIC->ISER[0] |= (1<<18);	// Enable ADC global Interrupt

	ADC3->CR2 |= (1<<0);		// ADON: ADC ON
}

void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
    RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state
}	

void BEEP(void)			// Beep for 20 ms 
{ 	
	GPIOF->ODR |= (1<<9);	// PF9 'H' Buzzer on
	DelayMS(20);		// Delay 20 ms
	GPIOF->ODR &= ~(1<<9);	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);   // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}

uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
			return key;
		else
		{	DelayMS(10);
			key_flag = 0;
			return key;
		}
	}
	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
			return 0xFF00;
		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}
