/////////////////////////////////////////////////////////////
// ������: HW2. ������ �ð�
// ��������: STM32F407 ����ũ����Ʈ�ѷ��� �̿��� �����нð踦 ����
// ����� �ϵ����(���): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD, EXTI, FRAM
// ������: 2024. 10. 07. 
// ������ Ŭ����: �����Ϲ�
// �й�: 2020134035
// �̸�: �� ��
///////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"

#define SW3 0xF700 // PH11
#define SW4 0xEF00 // PH12

void _GPIO_Init(void);			  // GPIO �ʱ�ȭ �Լ�  
void _EXTI_Init(void);			  // ���ͷ�Ʈ �ʱ�ȭ �Լ�
void TIMER10_Init(void);
void TIMER2_OC_Init(void);
void DisplayInitScreen(void);	  // GLCD �ʱ�ȭ �Լ�
uint16_t KEY_Scan(void);		  // SW�Է� �޴� �Լ�
void BEEP(void);			   	  //30ms���� ������ �︮�� �Լ�
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

int sec1 = 20;
int sec100m = 0;
int SW_sec1 = 0;
int SW_sec100m = 0;
int SW_flag = 0;
int debounce = 0;
int main(void)
{
    _GPIO_Init();
	_EXTI_Init();
	LCD_Init();
	DelayMS(10);
	BEEP();
	DisplayInitScreen();  // LCD �ʱ�ȭ��
    TIMER10_Init();
	TIMER2_OC_Init();
	GPIOG->ODR &= 0xFF00; // �ʱⰪ: LED0~7 Off
    while(1)
    {
    }

} // main() end

void TIMER10_Init(void)
{
	// Time base ����
	RCC->APB2ENR |= (1 << 17); // RCC_APB2ENR TIMER10 Enable // �ִ� 84MHz
	// Setting CR1 : 0x0000
	TIM10->CR1 &= ~(1 << 1); // UDIS=0(Update event Enabled): By one of following events
							//  Counter Overflow/Underflow,
                        	//  Setting the UG bit Set,
							//  Update Generation through the slave mode controller
							// UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM10->CR1 &= ~(1 << 2); // URS=0(Update event source Selection): one of following events
							//	Counter Overflow/Underflow,
							// Setting the UG bit Set,
							//	Update Generation through the slave mode controller
							// URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM10->CR1 &= ~(1 << 3); // OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM10->CR1 &= ~(1 << 4);	// DIR=1(Down counter)(reset state)
    TIM10->CR1 &= ~(3 << 5); 	// CMS(Center-aligned mode Sel)=00 (Edge-aligned mode) (reset state)
				// Center-aligned mode: The counter counts UP and DOWN alternatively
    TIM10->CR1 &= ~(1 << 7);	// ARPE=1(ARR is buffered): ARR Preload Enable
	TIM10->CR1 &= ~(3 << 8); // CKD(Clock division)=00(reset state)

	// Setting the Period
	TIM10->PSC = 840 - 1;	   // Prescaler=840, 168MHz/840 = 200KHz (5us)
	TIM10->ARR = 20000 - 1; // Auto reload  : 5us * 20K = 100ms(period) : ���ͷ�Ʈ�ֱ⳪ ��½�ȣ�� �ֱ� ����

	// Update(Clear) the Counter
	TIM10->EGR |= (1 << 0); // UG: Update generation
    TIM10->DIER |= (1 << 0); // UIE: Enable TIM10 Update interrupt
	NVIC->ISER[0] |= (1 << 25); // Enable Timer10 global Interrupt on NVIC

	// PSC���� ���� Ŭ���� Enable����, �� �����ϰ� �������� �����������
	TIM10->CR1 |= (1 << 0); // CEN: Enable the TIM10 Counter
}

void TIM1_UP_TIM10_IRQHandler(void) // RESET: 0
{
    if(TIM10->SR & 0x01)
    {
        TIM10->SR &= ~(1 << 0); // Update Interrupt Claer
        debounce = 0;			// ��ٿ�� ��
		sec100m++;
        GPIOG->ODR |= (1 << 6);
        if(sec100m >= 10)
        {
            sec100m = 0;
            sec1++;
            if(sec1 >= 24)	sec1 = 0;
            LCD_DisplayChar(3, 6, (sec1/10)+0x30);       // �� 10�� ���� ǥ��
            LCD_DisplayChar(3, 7, (sec1%10)+0x30);       // �� 1�� ���� ǥ��
        }
        LCD_DisplayChar(3, 9, sec100m+0x30);       // 1/10�� ǥ��
    }
}

void TIMER2_OC_Init(void)
{    
	// PA2: TIM2_CH3
	// PA2�� ��¼����ϰ� Alternate function(TIM2_CH3)���� ��� ����
	RCC->AHB1ENR |= (1 << 0); // RCC_AHB1ENR GPIOA Enable

	GPIOA->MODER |= (2 << 4);	 // GPIOA PIN2 Output Alternate function mode
	GPIOA->OSPEEDR |= (3 << 4);  // GPIOA PIN2 Output speed (100MHz High speed)
	GPIOA->OTYPER &= ~(1 << 2);  // GPIOA PIN2 Output type push-pull (reset state)
	GPIOA->PUPDR |= (1 << 4);	 // GPIOA PIN2 Pull-up
								 // PA2 ==> TIM2_CH3
	GPIOA->AFR[0] |= 0X0100; // Connect TIM2 pins(PA2) to AF1(TIM2)

	// Time base ����
	RCC->APB1ENR |= (1 << 0); // RCC_APB1ENR TIMER2 Enable // �ִ� 42MHz

	// Setting CR1 : 0x0000
	TIM2->CR1 &= ~(1 << 4); // DIR=0(Up counter)(reset state)
	TIM2->CR1 &= ~(1 << 1); // UDIS=0(Update event Enabled): By one of following events
							//  Counter Overflow/Underflow,
							//  Setting the UG bit Set,
							//  Update Generation through the slave mode controller
							// UDIS=1 : Only Update event Enabled by  Counter Overflow/Underflow,
	TIM2->CR1 &= ~(1 << 2); // URS=0(Update event source Selection): one of following events
							//	Counter Overflow/Underflow,
							// Setting the UG bit Set,
							//	Update Generation through the slave mode controller
							// URS=1 : Only Update Interrupt generated  By  Counter Overflow/Underflow,
	TIM2->CR1 &= ~(1 << 3); // OPM=0(The counter is NOT stopped at update event) (reset state)
	TIM2->CR1 &= ~(1 << 7);	// ARPE=0(ARR is not buffered)
	TIM2->CR1 &= ~(3 << 8); // CKD(Clock division)=00(reset state)
	TIM2->CR1 &= ~(3 << 5); // CMS(Center-aligned mode Sel)=00 : Edge-aligned mode(reset state)

	// Setting the Period
	// PSC, ARR�����ϴ°� ������ ����
	TIM2->PSC = 16800 - 1;	   // Prescaler=16800, 84MHz/16800 = 5000Hz (2ms)
	TIM2->ARR = 500 - 1;     // Auto reload  : 0.2ms * 500 = 100ms(period) : ���ͷ�Ʈ�ֱ⳪ ��½�ȣ�� �ֱ� ����

	// Update(Clear) the Counter
	TIM2->EGR |= (1 << 0); // UG: Update generation

	// Output Compare ����
	// CCMR2(Capture/Compare Mode Register 2) : Setting the MODE of Ch3 or Ch4
	TIM2->CCMR2 &= ~(3 << 0); // CC3S(CC3 channel) = '0b00' : Output
	TIM2->CCMR2 &= ~(1 << 2); // OC3FE=0: Output Compare 1 Fast disable
	TIM2->CCMR2 &= ~(1 << 3); // OC3PE=0: Output Compare 1 preload disable(CCR1�� �������� ���ο� ���� loading ����)
	TIM2->CCMR2 &= ~(3 << 4);  // OC3M=0b011 (Output Compare 1 Mode : toggle)

	// CCER(Capture/Compare Enable Register) : Enable "Channel 1"
	TIM2->CCER |= (1 << 8);	 // CC3E=1: CC3 channel Output Enable
							 // OC3(TIM2_CH3) Active: �ش���(100��)�� ���� ��ȣ���
	TIM2->CCER &= ~(1 << 9); // CC3P=0: CC3 channel Output Polarity (OCPolarity_High : OC3���� �������� ���)

    TIM2->CCR3 = 500; // TIM2 CCR3 TIM2_Pulse
	TIM2->DIER &= ~(1 << 0); // UIE: Disable TIM2 Update interrupt
	TIM2->DIER &= ~(1 << 3); // CC3IE: Disable the TIM2 CC3 interrupt

	NVIC->ISER[0] |= (1 << 28); // Enable Timer2 global Interrupt on NVIC

	// PSC���� ���� Ŭ���� Enable����, �� �����ϰ� �������� �����������
	TIM2->CR1 |= (1 << 0); // CEN: Enable the TIM2 Counter
}

void TIM2_IRQHandler(void) // RESET: 0
{
	if (TIM2->SR & 0x01) // Update interrupt flag
	{
		TIM2->SR &= ~(1 << 0); // Update Interrupt Claer
		SW_sec100m++;
		if(SW_sec100m >= 10)
		{
			SW_sec100m = 0;
			SW_sec1++;
			if(SW_sec1 >= 99)	SW_sec1 = 0;
			LCD_DisplayChar(4, 6, (SW_sec1/10)+0x30);       // �� 10�� ���� ǥ��
            LCD_DisplayChar(4, 7, (SW_sec1%10)+0x30);       // �� 1�� ���� ǥ��
		}
		LCD_DisplayChar(4, 9, SW_sec100m+0x30);       // 1/10�� ǥ��
	}
}

/* GPIO (GPIOG(LED), GPIOI(JoyStick), GPIOF(Buzzer)) �ʱ� ����	*/
void _GPIO_Init(void)
{
	// Buzzer (GPIO F) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable	
	GPIOF->MODER 	&= ~0x000B0000;	// GPIOF 9 : Clear						
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)	
	GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull 
	GPIOF->OSPEEDR  &= ~0x000B0000; // GPIOF 9 : Clear(0b00)	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	// LED (GPIO G) ���� : LED3(PG4), LED4(PG4) Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00000140;	// GPIOG 3,4 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00000140;	// GPIOG 3,4 : Push-pull
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00000140;	// GPIOG 3,4 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)
}

/*  EXTI11,12(SW3,4(PH11,12)) �ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000080;	// RCC_AHB1ENR GPIOH Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0x03C00000;	// GPIOH 11,12 : Input mode
    GPIOH->PUPDR    &= ~0x03C00000;    // GPIOH->PUPDR PH11,12 Clear(Floating input)

    // EXTI11 <- PH11, EXTI12 <- PH12 
	// EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])�� �̿� 
	// reset value: 0x0000
    SYSCFG->EXTICR[2] &= ~0xF000;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x7000;	// EXTI11 -> GPIOH
    SYSCFG->EXTICR[3] &= ~0x000F;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x0007;	// EXTI12 -> GPIOH
	
    EXTI->FTSR &= ~0x1800;		// EXTI11,12: CLEAR 
	EXTI->FTSR |= 0x1800;		// EXTI11,12: Falling Trigger Enable 

	EXTI->IMR  &= ~0x1800;		// EXTI11,12: IMR Reg �ʱ�ȭ
	EXTI->IMR  |= 0x1800;		// EXTI11,12: UnMasked (Interrupt Enable) ����

    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI11,12'
}

/* EXTI10~15 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	if(debounce == 0)				// ��ٿ�� �ƴҶ�
	{
		//(EXTI11,SW 3)
		if(EXTI->PR & 0x0800)			// EXTI11 Interrupt Pending(�߻�) ����?
		{
			EXTI->PR |= 0x0800;			// Pending bit Clear
			debounce = 1;				//��ٿ�� ����
			if(SW_flag == 0)	
			{
				GPIOG->ODR |= 0x18;
				TIM2->DIER |= (1 << 0); 	// Timer Interrupt Enable
				TIM2->DIER |= (1 << 3);
				SW_flag = 1;
			}
			else if(SW_flag == 1)	
			{
				GPIOG->ODR &= ~0x08;
				TIM2->DIER &= ~(1 << 0);	// Timer Interrupt Disable
				TIM2->DIER &= ~(1 << 3);
				SW_flag = 0;
			}
			BEEP();
		}

		//(EXTI12,SW 4)
		if(EXTI->PR & 0x1000)			// EXTI12 Interrupt Pending(�߻�) ����?
		{
			EXTI->PR |= 0x1000;			// Pending bit Clear
			debounce = 1;				//��ٿ�� ����
			if(SW_flag == 0)
			{
				GPIOG->ODR &= ~0x18;		// LED3,4 Off
				TIM2->DIER &= ~(1 << 0);	// Timer Interrupt Disable
				TIM2->DIER &= ~(1 << 3);
				SW_sec1 = 0;
				SW_sec100m = 0;
				LCD_DisplayChar(4, 6, (SW_sec1/10)+0x30);       // �� 10�� ���� ǥ��
				LCD_DisplayChar(4, 7, (SW_sec1%10)+0x30);       // �� 1�� ���� ǥ��
				LCD_DisplayChar(4, 9, SW_sec100m+0x30);       // 1/10�� ǥ��
				BEEP();
			}
		}
	}
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_YELLOW);		    // ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		    // ��Ʈ : ���� 8
	//Title �ۼ�
	LCD_SetBackColor(RGB_YELLOW);             // ���ڹ��� : YELLOW
    LCD_SetTextColor(RGB_BLUE);              // ���ڻ� : BLUE
	LCD_DisplayText(1, 1, "Digital Watch");  // Title �ۼ�
	LCD_DisplayText(3, 1, "TIME");
    LCD_DisplayText(4, 1, "STW");
    // ����� ���� ��ǥ �� SCORE
    LCD_SetTextColor(RGB_RED);              // ���ڻ� : RED
    LCD_DisplayText(2, 1, "2020134035 JW");
    LCD_SetPenColor(RGB_BLACK);                // �簢�� ���� ����
    LCD_DrawRectangle(6, 11, 120, 27);    //(5,5) ǥ��
    
    LCD_SetTextColor(RGB_BLACK);              // ���ڻ� : BLACK
    LCD_DisplayChar(3, 6, (sec1/10)+0x30);       // �� 10�� ���� ǥ��
    LCD_DisplayChar(3, 7, (sec1%10)+0x30);       // �� 1�� ���� ǥ��
    LCD_DisplayText(3, 8, ":");
    LCD_DisplayChar(3, 9, sec100m+0x30);       // 1/10�� ǥ��

    LCD_DisplayChar(4, 6, (SW_sec1/10)+0x30);       // �� ǥ��
    LCD_DisplayChar(4, 7, (SW_sec1%10)+0x30);       // �� ǥ��
    LCD_DisplayText(4, 8, ":");
    LCD_DisplayChar(4, 9, SW_sec100m+0x30);       // 1/10�� ǥ��

}

/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */
// key_flag -> SW�� �ѹ� �������� �����ϱ� ���� ����
uint8_t key_flag = 0;
uint16_t KEY_Scan(void) // input key SW0 - SW7
{
    uint16_t key; 
    // SW�� ���� PH8~15�̹Ƿ� PH0~7�� 0���� �ʱ�ȭ
    key = GPIOH->IDR & 0xFF00; // any key pressed ?
    if (key == 0xFF00)         // if no key, check key off
    {
        if (key_flag == 0)
            return key;
        else
        {
            DelayMS(10);
            key_flag = 0;
            return key;
        }
    }
    // else�� ���� Ű�� ������
    else // if key input, check continuous key
    {
        if (key_flag != 0) // if continuous key, treat as no key input
            return 0xFF00;
        else // if new key, delay for debounce
        {
            key_flag = 1;
            DelayMS(10); // Ȥ�� �� ���������
            return key;
        }
    }
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
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