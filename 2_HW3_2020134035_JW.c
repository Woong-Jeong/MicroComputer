/////////////////////////////////////////////////////////////
// ������: HW3. Great Escape(EXTI)
// ��������: STM32F407 ����ũ����Ʈ�ѷ��� ���ͷ�Ʈ ����� Ȱ���Ͽ�
// 			 ����ġ�� ���̽�ƽ �Է��� ó���ϰ�
//			 GLCD, LED, ������ �̿��Ͽ� ��Ż�� �ùķ��̼��� ����
// ����� �ϵ����(���): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD
// ������: 2024. 06. 04. 
// ������ Ŭ����: �����Ϲ�
// �й�: 2020134035
// �̸�: �� ��
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"

void _GPIO_Init(void);			  // GPIO �ʱ�ȭ �Լ�  
void _EXTI_Init(void);			  // ���ͷ�Ʈ �ʱ�ȭ �Լ�		
void DisplayInitScreen(void);	  // GLCD �ʱ�ȭ �Լ�
void RoomOpen(int SwitchFlag);	  //Room���濩�θ� ǥ���ϴ� �Լ�
void System_Clear(void);		  // GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
void BEEP(void);			   	  //30ms���� ������ �︮�� �Լ�
void BEEP_Clear(void);			  //500ms���� ������ �︮�� �Լ�
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

int main(void)
{
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);
	System_Clear();	//�ý��� �ʱ�ȭ
	while(1){}		//main�Լ� ���Ḧ �����ϱ����� �ݺ���
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����	*/
void _GPIO_Init(void)
{
	// Buzzer (GPIO F) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable	
	GPIOF->MODER 	&= ~0x000B0000;	// GPIOF 9 : Clear						
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)	
	GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull 
	GPIOF->OSPEEDR  &= ~0x000B0000; // GPIOF 9 : Clear(0b00)	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	// LED (GPIO G) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x000000FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)

	// Switch (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	//Joy Stick SW(GPIO I) ���� : Input mode
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x0000C000;	// GPIOI 7 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x0000C000;	// GPIOI 7 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI (EXTI7(GPIOI.7, JOY_Down), 
	EXTI8,9(GPIOH.8~9, SW0~1)
	EXTI11~15(GPIOH.11~15, SW3~7))�ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock

	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)	
	GPIOI->MODER	&= ~0x0000C000;	// GPIOI 7 : Input mode (reset state)

	// EXTI7 <- PI7, EXTI8 <- PH8, EXTI9 <- PH9, 
	// EXTI11 <- PH11, EXTI12 <- PH12, EXTI13 <- PH13,
	// EXTI14 <- PH14, EXTI15 <- PH15 
	// EXTICR2(EXTICR[1]), EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])�� �̿� 
	// reset value: 0x0000
	SYSCFG->EXTICR[1] &= ~0x00FF;	// EXTICR2 clear
	SYSCFG->EXTICR[1] |= 0x8000;	// EXTI7�� ���� �ҽ� �Է��� GPIOI�� ����
    SYSCFG->EXTICR[2] &= ~0x00FF;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x7077;	// EXTI8,9,11�� ���� �ҽ� �Է��� GPIOH�� ����	
    SYSCFG->EXTICR[3] &= ~0x00FF;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x7777;	// EXTI12~15�� ���� �ҽ� �Է��� GPIOH�� ����
	
	EXTI->FTSR |= 0xFF80;			// EXTI7~15: Falling Trigger Enable  
	EXTI->IMR  &= ~0xFF80;			// EXTI7~15: IMR Reg �ʱ�ȭ
	EXTI->IMR  |= 0x2000;			// EXTI13: UnMasked (Interrupt Enable) ����

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI7,8,9'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI11~15'
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
	//EXTI7 ���ͷ�Ʈ �߻�, LED2 ON	
	if(EXTI->PR & 0x0080)			// EXTI7 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0080;			// Pending bit Clear
		EXTI->IMR  &= ~0x0080;		// EXTI7: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x0800;		// EXTI11: UnMasked (Interrupt Enable) ����
		RoomOpen(2);				// Room2 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x04;			// LED2 On
	}
	//EXTI8 ���ͷ�Ʈ �߻�, LED0 ON
	else if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
		EXTI->IMR  &= ~0x0100;		// EXTI8: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x0200;		// EXTI9: UnMasked (Interrupt Enable) ����
		RoomOpen(0);				// Room0 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x01;			// LED0 On
    }
	//EXTI9 ���ͷ�Ʈ �߻�, LED1 ON
	else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0200;			// Pending bit Clear
		EXTI->IMR  &= ~0x0200;		// EXTI9: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x0080;		// EXTI7: UnMasked (Interrupt Enable) ����
		RoomOpen(1);				// Room1 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x02;			// LED1 On
    }    
}

/* EXTI10~15 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{		
	if(EXTI->PR & 0x0800)			// EXTI11 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0800;			// Pending bit Clear
		EXTI->IMR  &= ~0x0800;		// EXTI11: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x1000;		// EXTI12: UnMasked (Interrupt Enable) ����
		RoomOpen(3);				// Room3 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x08;			// LED3 On
	}
	else if(EXTI->PR & 0x1000)		// EXTI12 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x1000;			// Pending bit Clear
		EXTI->IMR  &= ~0x1000;		// EXTI12: Masked (Interrupt Disable) ����
		RoomOpen(4);				// Room4 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x10;			// LED4 On
		//�������Ṯ�� ǥ��
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayText(2, 18,"C");
		DelayMS(1000);
		for(int i=0; i<3; i++)		// ���� 0.5��, ���� 3ȸ
		{
			BEEP();
			DelayMS(500);
		}
		DelayMS(3000);				// 3����
		BEEP_Clear();				// ���� 0.5�ʰ� ���
		System_Clear();				// ó�� ���·� �ʱ�ȭ
	}  
        else if(EXTI->PR & 0x2000)	// EXTI13 Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x2000;			// Pending bit Clear
		EXTI->IMR  &= ~0x2000;		// EXTI13: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x4000;		// EXTI14: UnMasked (Interrupt Enable) ����
		//���۽���ǥ��
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayText(2, 18,"W");
		RoomOpen(5);				// Room5 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x20;			// LED5 On
	}
        else if(EXTI->PR & 0x4000)	// EXTI14 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x4000;			// Pending bit Clear
		EXTI->IMR  &= ~0x4000;		// EXTI14: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x8000;		// EXTI15: UnMasked (Interrupt Enable) ����
		RoomOpen(6);				// Room6 ���� ǥ��
		BEEP();	
		GPIOG->ODR |= 0x40;			// LED6 On
	}
        else if(EXTI->PR & 0x8000)	// EXTI15 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
		EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (Interrupt Disable) ����
		EXTI->IMR  |= 0x0100;		// EXTI8: UnMasked (Interrupt Enable) ����
		RoomOpen(7);				// Room7 ���� ǥ��
		BEEP();
		GPIOG->ODR |= 0x80;			// LED7 On
	}
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
    int Rectangle_x = 15;	//ù �簢�� ������ġ
	LCD_Clear(RGB_WHITE);		    // ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		    // ��Ʈ : ���� 8
    LCD_SetBackColor(RGB_WHITE);	// ���ڹ��� : White
    //�л��� �й� �� �̸� �ۼ�
	LCD_SetTextColor(RGB_BLUE);	    // ���ڻ� : Blue
	LCD_DisplayText(0, 0,"2020134035 JW");  	// �л��� �й� �� �̸�
	//ROOM ��ȣ ǥ��
	LCD_SetTextColor(RGB_BLACK);	  // ���ڻ� : Black
	LCD_DisplayText(1, 0,"R 0 1 2 3 4 5 6 7"); //Room ��ȣ ǥ��
	//ROOM ���� ��Ȳ ǥ��
    LCD_SetPenColor(RGB_BLACK);
    for(int i=0; i<8; i++)
    {
        LCD_DrawRectangle(Rectangle_x, 27, 8, 8); //�簢�� �׸���
		Rectangle_x += 16;						  //�簢�� ������ X������ 16�̵�
    }    
    //���ۻ�Ȳǥ��
	LCD_SetTextColor(RGB_BLACK);		//���ڻ�: Black
	LCD_DisplayText(2, 18,"S");			//���� ����: S
}

//Room���濩�θ� ǥ���ϴ� �Լ�
void RoomOpen(int SwitchFlag)
{
    LCD_SetBrushColor(GET_RGB(255, 0, 255));			//����: Pink
    LCD_DrawFillRect(16 * (SwitchFlag + 1), 28, 7, 7); 	//�簢�� ���� ä���
}	

//GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
void System_Clear(void)
{
	_GPIO_Init();			// GPIO �ʱ�ȭ
	_EXTI_Init();			// EXTI �ʱ�ȭ
	DisplayInitScreen();	// GLCD �ʱ�ȭ
	GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);			// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}
/*Buzzer: Beep for 500 ms*/
void BEEP_Clear(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(500);			// Delay 500 ms
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