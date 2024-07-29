/////////////////////////////////////////////////////////////
// ������: TP1. Ŀ���ڵ��Ǹű�
// ��������: STM32F407 ����ũ����Ʈ�ѷ��� �̿��� Ŀ�����Ǳ� �ùķ��̼��� ����
// ����� �ϵ����(���): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD, EXTI, FRAM
// ������: 2024. 06. 16. 
// ������ Ŭ����: �����Ϲ�
// �й�: 2020134035
// �̸�: �� ��
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW2 0xFB00				  //PH 8
#define JOY_RIGHT 0x02E0		  //PI 8
#define JOY_LEFT 0x01E0			  //PI 9
#define JOY_UP 0x03A0			  //PI 6

void _GPIO_Init(void);			  // GPIO �ʱ�ȭ �Լ�  
void _EXTI_Init(void);			  // ���ͷ�Ʈ �ʱ�ȭ �Լ�		
void DisplayInitScreen(void);	  // GLCD �ʱ�ȭ �Լ�
/* GLCD�� �簢���׸��� �Լ� */
void Rect(uint16_t col, uint16_t row, uint32_t PenColor, uint32_t BackColor);
/* GLCD�� ���ڸ� ������ �簢�� �׸��� �Լ�*/
void RectInText(uint16_t col, uint16_t row, int TextNum, uint32_t PenColor, uint32_t TextColor, uint32_t BackColor);
/* Ŀ�Ǹ���� �����ϴ� �Լ�*/
void MakeCoffee(uint8_t Choice);
void System_Clear(void);		  // GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
uint16_t KEY_Scan(void);		  // SW�Է� �޴� �Լ�
uint16_t JOY_Scan(void);		  //JOYSTICK �Է� �޴� �Լ�
void BEEP(void);			   	  //30ms���� ������ �︮�� �Լ�
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t Input;			//�Է±ݾ�
uint8_t Total;			//�ѱݾ�
uint8_t Choice = 0;		//Ŀ�Ǽ��ú���
uint8_t cup = 9;		// cup �����
uint8_t sugar = 5;		// sugar �����
uint8_t milk = 5;		// milk �����
uint8_t coffee = 9;		// coffee �����
uint8_t Noc;			//���ȸ� Ŀ�� �ܼ�
uint8_t RF = 0;			//���� �ʿ俩��
int main(void)
{
	Fram_Init();                  // FRAM �ʱ�ȭ H/W �ʱ�ȭ
	Fram_Status_Config(); 		  // FRAM �ʱ�ȭ S/W �ʱ�ȭ
	Input = Fram_Read(601);		  // �Է°� FRAM���� �о����
	Total = Fram_Read(602);		  // �� �Ǹž� FRAM���� �о����
	Noc = Fram_Read(603);	      // �� �Ǹ��ܼ� FRAM���� �о����
	System_Clear();	              // �ý��� �ʱ�ȭ
	while(1)	
	{
		//��� ��� 1�̻��϶� JOY��ƽ �� SW �Է°� ��ĵ
		if(cup >= 1 && sugar >= 1 && milk >= 1 && coffee >= 1)
		{
			switch(JOY_Scan())
			{
				case JOY_LEFT:						//Joy_Left �Է½�
					BEEP();							//���� 1ȸ
					Choice = 1;						//Black coffee ����
					/*Black coffee�� �ڽ��� ���� ������*/
					LCD_SetBackColor(RGB_BLUE);
					LCD_SetTextColor(RGB_WHITE);
					LCD_DisplayText(2, 3, "S");
					LCD_DisplayText(3, 5, "M");
					LCD_SetTextColor(RGB_RED);
					LCD_DisplayText(3, 1, "B");
				break;
				case JOY_UP:						//Joy_UP �Է½�
					BEEP();							//���� 1ȸ
					Choice = 2;						//Sugar coffee ����
					/*Sugar coffee�� �ڽ��� ���� ������*/
					LCD_SetBackColor(RGB_BLUE);
					LCD_SetTextColor(RGB_WHITE);
					LCD_DisplayText(3, 1, "B");
					LCD_DisplayText(3, 5, "M");
					LCD_SetTextColor(RGB_RED);
					LCD_DisplayText(2, 3, "S");
				break;
				case JOY_RIGHT:						//Joy_Left�Է½�
					BEEP();							//���� 1ȸ
					Choice = 3;						//Mix coffee ����
					/*Mix coffee�� �ڽ��� ���� ������*/
					LCD_SetBackColor(RGB_BLUE);
					LCD_SetTextColor(RGB_WHITE);
					LCD_DisplayText(3, 1, "B");
					LCD_DisplayText(2, 3, "S");
					LCD_SetTextColor(RGB_RED);
					LCD_DisplayText(3, 5, "M");
				break;
			}	//switch(JOY_Scan()) end
			switch (KEY_Scan())
			{
				case SW2:
					// Black coffee�� ����, �Է� Coin 10�� �̻�, ���ʺ��ʿ� �����϶� ����
					if(Choice == 1 && Input >= 1 && RF == 0)
					{
						BEEP();				//���� 1ȸ
						MakeCoffee(1);		//Black coffee ����
					}
					// Sugar coffee�� ����, �Է� Coin 20�� �̻�, ���ʺ��ʿ� �����϶� ����
					else if(Choice == 2 && Input >= 2 && RF == 0)
					{
						BEEP();				//���� 1ȸ
						MakeCoffee(2);		//Sugar coffee ����
					} 
					// Mix coffee�� ����, �Է� Coin 30�� �̻�, ���ʺ��ʿ� �����϶� ����
					else if(Choice == 3 && Input >= 3 && RF == 0)
					{
						BEEP();				//���� 1ȸ
						MakeCoffee(3);		//Mix coffee ����
					}
					break;
			}	//switch(KEY_Scan()) end
		} // if() end
		//��� �� �ϳ��� 0�̸� �����Է� x
		else
		{
			EXTI->IMR  	&= ~0x0300;			// EXTI8,9: Disable
			EXTI->IMR	|= 0x1000;			// EXTI12 : Enable
			RF = 1;							//Refill�� �ʿ�
			Rect(15, 6, RGB_GREEN, RGB_RED);	//���� �簢�� ���������� ����
		} // else end		
	}  //whlie(1) end	
} // main() end

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
	GPIOH->MODER 	&= ~0x0FFF0000;	// GPIOH 8~13 : Input mode				
	GPIOH->PUPDR 	&= ~0x0FFF0000;	// GPIOH 8~13 : Floating Input (No Pull-up, pull-down) :reset state

	//Joy Stick SW(GPIO I) ���� : Input mode
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000F3000;	// GPIOI 6,8,9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000F3000;	// GPIOI 6,8,9 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI8,9(GPIOH.8~9, SW0~1)
	EXTI11~15(GPIOH.11~15, SW3~7))�ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0x0FFF0000;	// GPIOH PIN8~PIN13 Input mode (reset state)	

	// EXTI8 <- PH8, EXTI9 <- PH9, 
	// EXTI11 <- PH11, EXTI12 <- PH12, EXTI13 <- PH13 
	// EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])�� �̿� 
	// reset value: 0x0000
    SYSCFG->EXTICR[2] &= ~0x00FF;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x7077;	// EXTI8,9,11�� ���� �ҽ� �Է��� GPIOH�� ����	
    SYSCFG->EXTICR[3] &= ~0x00FF;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x0077;	// EXTI12,13�� ���� �ҽ� �Է��� GPIOH�� ����
	
	EXTI->FTSR |= 0x3F00;			// EXTI8~13: Falling Trigger Enable  
	EXTI->IMR  &= ~0x3F00;			// EXTI8~13: IMR Reg �ʱ�ȭ
	EXTI->IMR  |= 0x2B00;			// EXTI8,9,11,13: UnMasked (Interrupt Enable) ����

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI8,9'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI11~13'
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
	//10�� INPUT �߻� (EXTI8,SW 0)
	if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
		Rect(11, 3, RGB_GREEN, RGB_YELLOW);	// ��������� 10�� �Է� ǥ��
		BEEP();								// ���� 1ȸ
		DelayMS(1000);						// 1�� ���� 
		Rect(11, 3, RGB_GREEN, RGB_GRAY);	// �ٽ� ȸ�� ����
		LCD_SetBackColor(RGB_BLACK);		// ���ڹ��� : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// ���ڻ� : YELLOW
		//Input�� 20�̸��� ��츸 Input +1, Input�� LCD�ֽ�ȭ 
		if(Input < 20)
		{
			Input += 1;						//Input�� +1
			Fram_Write(601,Input);			// ���� �ٲ� INPUT�� FRAM�� ����
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100�� �ڸ�
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10�� �ڸ�
		}
    }
	//50�� Input �߻� (EXTI9,SW 1)
	else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0200;			// Pending bit Clear
		Rect(11, 5, RGB_GREEN, RGB_YELLOW);	// ��������� 50�� �Է� ǥ��
		BEEP();								// ���� 1ȸ
		DelayMS(1000);						// 1�� ���� 
		Rect(11, 5, RGB_GREEN, RGB_GRAY);	// �ٽ� ȸ�� ����
		LCD_SetBackColor(RGB_BLACK);		// ���ڹ��� : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// ���ڻ� : YELLOW
		//Input�� 15�̸��� ��츸 Input +5, Input�� LCD�ֽ�ȭ
		if(Input < 15)
		{
			Input += 5;						//Input�� +5
			Fram_Write(601,Input);			// ���� �ٲ� INPUT�� FRAM�� ����
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100�� �ڸ�
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10�� �ڸ�
		}
		//Input�� 15�̻��� ��� Input == 20, Input�� LCD�ֽ�ȭ
		else if(Input >= 15 && Input < 20)
		{
			Input = 20;						// Input�� == 20
			Fram_Write(601,Input);			// ���� �ٲ� INPUT�� FRAM�� ����
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100���ڸ�
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10���ڸ�
		}
    }    
}

/* EXTI10~15 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// �ܵ���ȯ (EXTI11,SW 3)
	if(EXTI->PR & 0x0800)			// EXTI11 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0800;			// Pending bit Clear
		BEEP();						// ���� 1ȸ
		Input = 0;					// Input�� == 0
		Fram_Write(601,0);			// ���� �ٲ� INPUT�� FRAM�� ����
		// ����� Input�� �ֽ�ȭ
		LCD_SetBackColor(RGB_BLACK);		// ���ڹ��� : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// ���ڻ� : YELLOW
		LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100���ڸ�
		LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10���ڸ�
	}
	// Refill ���� (EXTI12,SW 4)
	else if(EXTI->PR & 0x1000)		// EXTI12 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x1000;			// Pending bit Clear
		/*��� ��0���� ��Ḹ �ִ밪*/
		// cup�� ��� 0�϶�
		if(cup == 0)
		{
			cup = 9;					//cup ��� �ִ밪
			RF = 0;						//���� ���ʿ� ����
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//���� ���ʿ� LCDǥ��
		}
		// sugar�� ��� 0�϶�
		if(sugar == 0)
		{
			sugar = 5;					//sugar ��� �ִ밪
			RF = 0;						//���� ���ʿ� ����
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//���� ���ʿ� LCDǥ��
		}
		// milk�� ��� 0�϶�
		if(milk == 0)
		{
			milk = 5;					//milk ��� �ִ밪
			RF = 0;						//���� ���ʿ� ����
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//���� ���ʿ� LCDǥ��
		}
		// coffee�� ��� 0�϶�
		if(coffee == 0)
		{
			coffee = 9;					//coffee ��� �ִ밪
			RF = 0;						//���� ���ʿ� ����
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//���� ���ʿ� LCDǥ��
		}
		//���� �Է� �簳
		EXTI->IMR  		|= 0x0300;			// EXTI8,9: Enable
		EXTI->IMR		&= ~0x1000;			// EXTI12 : Disable
		//���� �� ��� LCD�ֽ�ȭ
		LCD_SetBackColor(RGB_WHITE);
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayChar(6, 1, cup + 0x30);
		LCD_DisplayChar(6, 4, sugar + 0x30);
		LCD_DisplayChar(6, 7, milk + 0x30);
		LCD_DisplayChar(6, 10, coffee + 0x30);
		//0.5�� �������� ���� 2��
		for(int i=0;i<2;i++)
		{
			BEEP();
			DelayMS(500);
		}
	}

	// CLEAR ����(EXTI13, SW 5)
    else if(EXTI->PR & 0x2000)	// EXTI13 Interrupt Pending(�߻�) ����
	{
		EXTI->PR |= 0x2000;			// Pending bit Clear
		BEEP();						// ���� 1ȸ
		Total = 0;					// Total�� == 0
		Fram_Write(602,0);			// ���� �ٲ� Total�� FRAM�� ����
		// ����� Total�� �ֽ�ȭ
		LCD_SetBackColor(RGB_BLACK);		// ���ڹ��� : BLACK
		LCD_SetTextColor(RGB_YELLOW);		// ���ڻ� : YELLOW
		LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100�� �ڸ�
		LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10�� �ڸ�
		
		Noc = 0;					// Noc�� == 0
		Fram_Write(603,0);			// ���� �ٲ� Noc�� FRAM�� ����
		// ����� Noc�� �ֽ�ȭ
		LCD_SetBackColor(RGB_YELLOW);		// ���ڹ��� : YELLOW
		LCD_SetTextColor(RGB_BLACK);		// ���ڻ� : BLACK
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);		//10�� �ڸ�
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);		//1�� �ڸ�
	}
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		    // ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		    // ��Ʈ : ���� 8
	//Title �ۼ�
	RectInText(0, 0, 10, RGB_BLACK, RGB_BLACK, RGB_YELLOW);	//Title �簢�� ����
	LCD_SetBackColor(RGB_YELLOW);                         // ���ڹ��� : YELLOW
    LCD_SetTextColor(RGB_BLACK);                          // ���ڻ� : BLACK
	LCD_DisplayText(0, 0, " JW coffee");				  // Title �ۼ�
	LCD_DrawRectangle(0, 0, 82, 14);         			  // ���ڹ������ �������� �簢�� �׸���
	//Ŀ�Ǽ��� �ڽ�
	RectInText(1, 3, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// Black coffee �簢�� ����
	RectInText(3, 2, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// sugar coffee �簢�� ����
	RectInText(5, 3, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// mix coffee �簢�� ����
	LCD_DisplayText(3, 1, "B");			// Black coffee LCDǥ��
	LCD_DisplayText(2, 3, "S");			// sugar coffee LCDǥ��
	LCD_DisplayText(3, 5, "M");			// mix coffee LCDǥ��
	// Ŀ������ �ڽ�
	RectInText(3, 4, 1, RGB_GREEN, RGB_WHITE, RGB_RED);		// Ŀ�������� �簢�� ����
	LCD_DisplayText(4, 3, "W");								// Ŀ�������� LCDǥ��
	// ����ڰ� ���� �ݾ� ǥ��
	LCD_SetBackColor(RGB_WHITE);								// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK); 								// ���ڻ� : BLACK            			
    LCD_DisplayText(0, 14, "IN");								// INPUT LCDǥ��
	RectInText(14, 1, 3, RGB_GREEN, RGB_YELLOW, RGB_BLACK);		//INPUT �簢�� ����
	//INPUT ���� 0~20 �̸� FRAM�� ����Ȱ� ǥ��
	if(Input <= 20)
	{
		LCD_DisplayChar(1, 14, (Input / 10) + 0x30);	//100���ڸ�
		LCD_DisplayChar(1, 15, (Input % 10) + 0x30);	//10���ڸ�
		LCD_DisplayChar(1, 16, 0+0x30);					//1���ڸ�
	}
	//INPUT ���� 0~20 �ƴϸ� 0ǥ��
	else 
	{
		LCD_DisplayChar(1, 14, 0+0x30);		//100���ڸ�
		LCD_DisplayChar(1, 15, 0+0x30);		//10���ڸ�
		LCD_DisplayChar(1, 16, 0+0x30);		//1���ڸ�
	}
	// ���Ǳ� �� ���� ǥ��
	LCD_SetBackColor(RGB_WHITE);					// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK); 					// ���ڻ� : BLACK
	LCD_DisplayText(2, 14, "TOT");					// TOTAL LCDǥ��
	RectInText(14, 3, 3, RGB_GREEN, RGB_YELLOW, RGB_BLACK); //TOTAL �簢�� ����
	//Total ���� 0~99 �̸� FRAM�� ����Ȱ� ǥ��
	if(Total <= 99)
	{
		LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100���ڸ�
		LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10���ڸ�
		LCD_DisplayChar(3, 16, 0+0x30);					//1���ڸ�
	}
	//Total ���� 0~99 �ƴϸ� 0ǥ��
	else
	{
		LCD_DisplayChar(3, 14, 0+0x30);		//100���ڸ�
		LCD_DisplayChar(3, 15, 0+0x30);		//10���ڸ�
		LCD_DisplayChar(3, 16, 0+0x30);		//1���ڸ�
	}
	//�������� ǥ��
	LCD_SetBackColor(RGB_WHITE);		// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK);        // ���ڻ� : BLACK           		   
	LCD_DisplayText(2, 10, "\\10");		// 10�� LCDǥ��
	LCD_DisplayText(4, 10, "\\50");		// 50�� LCDǥ��
	Rect(11, 3, RGB_GREEN, RGB_GRAY);   // 10�� �簢�� �׸���
	Rect(11, 5, RGB_GREEN, RGB_GRAY);	// 50�� �簢�� �׸���
	//���ǥ��
	LCD_SetBackColor(RGB_WHITE);		// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK);		// ���ڻ� : BLACK 
	LCD_DisplayText(7, 0, "cp");		//cup LCDǥ��
	LCD_DisplayText(7, 3, "sg");		//sugar LCDǥ��
	LCD_DisplayText(7, 6, "mk");		//milk LCDǥ��
	LCD_DisplayText(7, 9, "cf");		//coffee LCDǥ��
	RectInText(1, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//cup �簢�� �׸���
	RectInText(4, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//sugar �簢�� �׸���
	RectInText(7, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//milk �簢�� �׸���
	RectInText(10, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//coffee �簢�� �׸���
	LCD_DisplayChar(6, 1, cup + 0x30);		//cup ��� ǥ��
	LCD_DisplayChar(6, 4, sugar + 0x30);	//coffee ��� ǥ��
	LCD_DisplayChar(6, 7, milk + 0x30);		//milk ��� ǥ��
	LCD_DisplayChar(6, 10, coffee + 0x30);	//coffee ��� ǥ��
	//���ʿ���ǥ��
	LCD_DisplayText(6, 13, "RF");			//Refill LCDǥ��
	Rect(15, 6, RGB_GREEN, RGB_GREEN);		//Refill �簢�� �׸���
	//�� Ŀ�� �Ǹż�
	LCD_SetBackColor(RGB_WHITE);		// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK);		// ���ڻ� : BLACK 
	LCD_DisplayText(7, 13, "NoC");		// �� Ŀ�� �Ǹż� LCDǥ��
	RectInText(14, 8, 2, RGB_GREEN, RGB_BLACK, RGB_YELLOW);	// �� Ŀ�� �Ǹż� �簢�� �׸���
	//Noc ���� 0~50 �̸� FRAM�� ����Ȱ� ǥ��
	if(Noc <= 50)
	{
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);
	}
	//Noc ���� 0~50 �ƴϸ� 0ǥ��
	else
	{
		LCD_DisplayChar(8, 14, 0 + 0x30);
		LCD_DisplayChar(8, 15, 0 + 0x30);
	}
}

/* GLCD�� 10x10 �簢���׸��� �Լ� */
void Rect(uint16_t col, uint16_t row, uint32_t PenColor, uint32_t BackColor)
{
	uint16_t x = (col * 8);						 // �簢�� �� ��ǥ ����
    uint16_t y = (row * 13);					 // �簢�� �� ��ǥ ����
	LCD_SetBrushColor(BackColor);                // �簢�� ���� ����
    LCD_DrawFillRect(x + 1, y + 1, 9, 9);  		 // �簢�� ���� ä���
	LCD_SetPenColor(PenColor);				 	 // �簢�� �׵θ� �� ����
    LCD_DrawRectangle(x, y, 10, 10);         	 // �簢�� �׸���
}

/* GLCD�� ���ڸ� ������ �簢�� �׸��� �Լ�*/
void RectInText(uint16_t col, uint16_t row, int TextNum, uint32_t PenColor, uint32_t TextColor, uint32_t BackColor)
{
	uint16_t x = (col * 8) - 2;				// �簢�� ������ �� ��ǥ ����
    uint16_t y = (row * 13) - 2;			// �簢�� ������ �� ��ǥ ����
	// ��� ���� ��� 0�϶� �簢���� GLCD���� ����� ���� ���� 
	if(col == 0 && row == 0)
	{
		x = 0;
		y = 0;
	}
	// Text�� ������� ���ڰ� �� Ŀ���� ���� ����
	if(TextNum >= 3)
	{
		uint16_t TextFlag = TextNum;
		//3�� ����϶����� textnum 1�� ����
		while(TextFlag < 3)
		{
			TextFlag -= 3;
			TextNum -= 1;
		}
	}
	LCD_SetBrushColor(BackColor);                        // �簢�� ���� ����
    LCD_DrawFillRect(x + 1, y + 1, TextNum*8 + 1, 13);   // �簢�� ���� ä���
	LCD_SetBackColor(BackColor);                         // ���� ���� ����
    LCD_SetTextColor(TextColor);                         // ���ڻ� ����
	LCD_SetPenColor(PenColor);							 // �簢�� �׵θ� �� ����
    LCD_DrawRectangle(x, y, TextNum*8 + 2, 14);          // �簢�� �׸���
}

/* Ŀ�� ���� �Լ� */
void MakeCoffee(uint8_t Choice)
{
	EXTI->IMR  		&= ~0x2B00;			// EXTI8,9,11,13: Disable
	GPIOG->ODR		|= 0x0000FFFF;		// LED 0~7 On
	if(Choice == 1)		//black coffee
	{
		cup -= 1;			// cup ��� -1
		coffee -= 1;		// coffee ��� -1
		Input -= 1;			// �Է� Coin �� -1
		Fram_Write(601,Input);	// ���� �ٲ� INPUT�� FRAM�� ����
		if(Total < 99)			// �� �Ǹž��� 99 �̸��϶�
		{
			Total += 1;			// �� �Ǹž� +1
			Fram_Write(602,Total);  // ���� �ٲ� Total�� FRAM�� ����
		}
	}
	else if(Choice == 2)	//sugar coffee
	{
		cup -= 1;			// cup ��� -1
		sugar -= 1;			// sugar ��� -1
		coffee -= 1;		// coffee ��� -1
		Input -= 2;			// �Է� Coin �� -2
		Fram_Write(601,Input);	// ���� �ٲ� INPUT�� FRAM�� ����
		if(Total < 98)			// �� �Ǹž��� 98 �̸��϶�
		{
			Total += 2;			// �� �Ǹž� +2
			Fram_Write(602,Total);  // ���� �ٲ� Total�� FRAM�� ����
		}
		else
		{
			Total = 99;				// �ִ�ݾ����� ǥ��
			Fram_Write(602,Total);  // ���� �ٲ� Total�� FRAM�� ����
		}
	}
	else if(Choice == 3)	//mix coffee
	{
		cup -= 1;			// cup ��� -1
		sugar -= 1;			// sugar ��� -1
		milk -= 1;			// milk ��� -1
		coffee -= 1;		// coffee ��� -1
		Input -= 3;			// �Է� Coin �� -3
		Fram_Write(601,Input);	// ���� �ٲ� INPUT�� FRAM�� ����
		if(Total < 97)			// �� �Ǹž��� 97 �̸��϶�
		{
			Total += 3;			// �� �Ǹž� +3
			Fram_Write(602,Total);  // ���� �ٲ� Total�� FRAM�� ����
		}
		else
		{
			Total = 99;				// �ִ�ݾ����� ǥ��
			Fram_Write(602,Total);  // ���� �ٲ� Total�� FRAM�� ����
		}
	}
	//������ ǥ��
	LCD_SetBackColor(RGB_RED);				// ���ڹ��� : RED
	LCD_SetTextColor(RGB_WHITE);			// ���ڻ� : WHITE
	// Ŀ�������� �簢�� �� TEXT 1�� �������� 0, 1, 2, W ����
	// �׸��� 3���� LED0~7 OFF �� 0.5�ʰ��� ���� 3ȸ 
    LCD_DisplayText(4, 3, "0");		
	DelayMS(1000);					//Delay 1��
	LCD_DisplayText(4, 3, "1");
	DelayMS(1000);					//Delay 1��
	LCD_DisplayText(4, 3, "2");
	DelayMS(1000);					//Delay 1��
	LCD_DisplayText(4, 3, "W");
	DelayMS(3000);					//Delay 3��
	GPIOG->ODR	&= ~0x0000FFFF;		// LED 0~7 OFF
	//0.5�� �������� ������ 3ȸ
	for(int i=0;i<3;i++)
	{
		BEEP();
		DelayMS(500);
	}
	//��� ���� LCD �ֽ�ȭ
	LCD_SetBackColor(RGB_WHITE);			// ���ڹ��� : WHITE
	LCD_SetTextColor(RGB_BLACK);			// ���ڻ� : BLACK
	LCD_DisplayChar(6, 1, cup + 0x30);		// cup ��� ǥ��
	LCD_DisplayChar(6, 4, sugar + 0x30);	// coffee ��� ǥ��
	LCD_DisplayChar(6, 7, milk + 0x30);		// milk ��� ǥ��
	LCD_DisplayChar(6, 10, coffee + 0x30);	// coffee ��� ǥ��
	//Noc 0~50�϶� LCD �ֽ�ȭ
	if(Noc < 50) 
	{
		LCD_SetBackColor(RGB_YELLOW);		// ���ڹ��� : YELLOW
		LCD_SetTextColor(RGB_BLACK);		// ���ڻ� : BLACK
		Noc += 1;							// �� Ŀ�� �Ǹ��ܼ� +1
		Fram_Write(603,Noc);				// ���� �ٲ� Noc�� FRAM�� ����
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);	//10��;�ڸ�
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);	//1 ���ڸ�
	}
	//IN LCD �ֽ�ȭ
	LCD_SetBackColor(RGB_BLACK);		// ���ڹ��� : BLACK
	LCD_SetTextColor(RGB_YELLOW); 		// ���ڻ� : YELLOW
	LCD_DisplayChar(1, 14, (Input / 10) + 0x30);	//100�� �ڸ�
	LCD_DisplayChar(1, 15, (Input % 10) + 0x30);	//10�� �ڸ�
	//ToT LCD �ֽ�ȭ
	LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100�� �ڸ�
	LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10�� �ڸ�
	//����ȭ�� �ʱ�ȭ
	LCD_SetBackColor(RGB_BLUE);			// ���ڹ��� : BLUE
	LCD_SetTextColor(RGB_WHITE);		// ���ڻ� : WHITE
	LCD_DisplayText(3, 1, "B");			// Black coffee LCDǥ��
	LCD_DisplayText(2, 3, "S");			// sugar coffee LCDǥ��
	LCD_DisplayText(3, 5, "M");			// mix coffee LCDǥ��
	//choice���� �ʱ�ȭ
	Choice = 0;
	EXTI->IMR  |= 0x2B00;			// EXTI8,9,11,13: UnMasked (Interrupt Enable) ����	
}

//GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
void System_Clear(void)
{
	LCD_Init();	// LCD ��� �ʱ�ȭ
	DelayMS(10);
	_GPIO_Init();			// GPIO �ʱ�ȭ
	_EXTI_Init();			// EXTI �ʱ�ȭ
	DisplayInitScreen();	// GLCD �ʱ�ȭ
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

// JoyStick�� �ԷµǾ������� ���ο� � JoyStick�� �ԷµǾ������� ������ return�ϴ� �Լ�
// joy_flag -> JoyStick�� �ѹ� �������� �����ϱ� ���� ����
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void) // input joy stick
{
    uint16_t key;
    key = GPIOI->IDR & 0x03E0; // any key pressed ?
    if (key == 0x03E0)         // if no key, check key off
    {
        if (joy_flag == 0)
            return key;
        else
        {
            DelayMS(10);
            joy_flag = 0;
            return key;
        }
    }
    else // if key input, check continuous key
    {
        if (joy_flag != 0) // if continuous key, treat as no key input
            return 0x03E0;
        else // if new key, delay for debounce
        {
            joy_flag = 1;
            DelayMS(10);
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