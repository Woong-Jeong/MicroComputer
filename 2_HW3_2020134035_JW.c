/////////////////////////////////////////////////////////////
// 과제명: HW3. Great Escape(EXTI)
// 과제개요: STM32F407 마이크로컨트롤러의 인터럽트 기능을 활용하여
// 			 스위치와 조이스틱 입력을 처리하고
//			 GLCD, LED, 부저를 이용하여 방탈출 시뮬레이션을 구현
// 사용한 하드웨어(기능): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD
// 제출일: 2024. 06. 04. 
// 제출자 클래스: 수요일반
// 학번: 2020134035
// 이름: 정 웅
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"

void _GPIO_Init(void);			  // GPIO 초기화 함수  
void _EXTI_Init(void);			  // 인터럽트 초기화 함수		
void DisplayInitScreen(void);	  // GLCD 초기화 함수
void RoomOpen(int SwitchFlag);	  //Room개방여부를 표시하는 함수
void System_Clear(void);		  // GPIO, EXTI, GLCD를 초기화하는 함수
void BEEP(void);			   	  //30ms동안 부저가 울리는 함수
void BEEP_Clear(void);			  //500ms동안 부저가 울리는 함수
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

int main(void)
{
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);
	System_Clear();	//시스템 초기화
	while(1){}		//main함수 종료를 방지하기위한 반복문
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) 초기 설정	*/
void _GPIO_Init(void)
{
	// Buzzer (GPIO F) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable	
	GPIOF->MODER 	&= ~0x000B0000;	// GPIOF 9 : Clear						
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)	
	GPIOF->OTYPER 	&= ~0x00000200;	// GPIOF 9 : Push-pull 
	GPIOF->OSPEEDR  &= ~0x000B0000; // GPIOF 9 : Clear(0b00)	
	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	// LED (GPIO G) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x000000FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)

	// Switch (GPIO H) 설정 : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	//Joy Stick SW(GPIO I) 설정 : Input mode
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x0000C000;	// GPIOI 7 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x0000C000;	// GPIOI 7 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI (EXTI7(GPIOI.7, JOY_Down), 
	EXTI8,9(GPIOH.8~9, SW0~1)
	EXTI11~15(GPIOH.11~15, SW3~7))초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock

	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)	
	GPIOI->MODER	&= ~0x0000C000;	// GPIOI 7 : Input mode (reset state)

	// EXTI7 <- PI7, EXTI8 <- PH8, EXTI9 <- PH9, 
	// EXTI11 <- PH11, EXTI12 <- PH12, EXTI13 <- PH13,
	// EXTI14 <- PH14, EXTI15 <- PH15 
	// EXTICR2(EXTICR[1]), EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])를 이용 
	// reset value: 0x0000
	SYSCFG->EXTICR[1] &= ~0x00FF;	// EXTICR2 clear
	SYSCFG->EXTICR[1] |= 0x8000;	// EXTI7에 대한 소스 입력은 GPIOI로 설정
    SYSCFG->EXTICR[2] &= ~0x00FF;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x7077;	// EXTI8,9,11에 대한 소스 입력은 GPIOH로 설정	
    SYSCFG->EXTICR[3] &= ~0x00FF;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x7777;	// EXTI12~15에 대한 소스 입력은 GPIOH로 설정
	
	EXTI->FTSR |= 0xFF80;			// EXTI7~15: Falling Trigger Enable  
	EXTI->IMR  &= ~0xFF80;			// EXTI7~15: IMR Reg 초기화
	EXTI->IMR  |= 0x2000;			// EXTI13: UnMasked (Interrupt Enable) 설정

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI7,8,9'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI11~15'
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
	//EXTI7 인터럽트 발생, LED2 ON	
	if(EXTI->PR & 0x0080)			// EXTI7 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0080;			// Pending bit Clear
		EXTI->IMR  &= ~0x0080;		// EXTI7: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x0800;		// EXTI11: UnMasked (Interrupt Enable) 설정
		RoomOpen(2);				// Room2 개방 표시
		BEEP();
		GPIOG->ODR |= 0x04;			// LED2 On
	}
	//EXTI8 인터럽트 발생, LED0 ON
	else if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
		EXTI->IMR  &= ~0x0100;		// EXTI8: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x0200;		// EXTI9: UnMasked (Interrupt Enable) 설정
		RoomOpen(0);				// Room0 개방 표시
		BEEP();
		GPIOG->ODR |= 0x01;			// LED0 On
    }
	//EXTI9 인터럽트 발생, LED1 ON
	else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0200;			// Pending bit Clear
		EXTI->IMR  &= ~0x0200;		// EXTI9: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x0080;		// EXTI7: UnMasked (Interrupt Enable) 설정
		RoomOpen(1);				// Room1 개방 표시
		BEEP();
		GPIOG->ODR |= 0x02;			// LED1 On
    }    
}

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{		
	if(EXTI->PR & 0x0800)			// EXTI11 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0800;			// Pending bit Clear
		EXTI->IMR  &= ~0x0800;		// EXTI11: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x1000;		// EXTI12: UnMasked (Interrupt Enable) 설정
		RoomOpen(3);				// Room3 개방 표시
		BEEP();
		GPIOG->ODR |= 0x08;			// LED3 On
	}
	else if(EXTI->PR & 0x1000)		// EXTI12 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x1000;			// Pending bit Clear
		EXTI->IMR  &= ~0x1000;		// EXTI12: Masked (Interrupt Disable) 설정
		RoomOpen(4);				// Room4 개방 표시
		BEEP();
		GPIOG->ODR |= 0x10;			// LED4 On
		//동작종료문자 표시
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayText(2, 18,"C");
		DelayMS(1000);
		for(int i=0; i<3; i++)		// 간격 0.5초, 부저 3회
		{
			BEEP();
			DelayMS(500);
		}
		DelayMS(3000);				// 3초후
		BEEP_Clear();				// 부저 0.5초간 재생
		System_Clear();				// 처음 상태로 초기화
	}  
        else if(EXTI->PR & 0x2000)	// EXTI13 Interrupt Pending(발생) 여부
	{
		EXTI->PR |= 0x2000;			// Pending bit Clear
		EXTI->IMR  &= ~0x2000;		// EXTI13: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x4000;		// EXTI14: UnMasked (Interrupt Enable) 설정
		//동작시작표시
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayText(2, 18,"W");
		RoomOpen(5);				// Room5 개방 표시
		BEEP();
		GPIOG->ODR |= 0x20;			// LED5 On
	}
        else if(EXTI->PR & 0x4000)	// EXTI14 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x4000;			// Pending bit Clear
		EXTI->IMR  &= ~0x4000;		// EXTI14: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x8000;		// EXTI15: UnMasked (Interrupt Enable) 설정
		RoomOpen(6);				// Room6 개방 표시
		BEEP();	
		GPIOG->ODR |= 0x40;			// LED6 On
	}
        else if(EXTI->PR & 0x8000)	// EXTI15 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
		EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (Interrupt Disable) 설정
		EXTI->IMR  |= 0x0100;		// EXTI8: UnMasked (Interrupt Enable) 설정
		RoomOpen(7);				// Room7 개방 표시
		BEEP();
		GPIOG->ODR |= 0x80;			// LED7 On
	}
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
    int Rectangle_x = 15;	//첫 사각형 시작위치
	LCD_Clear(RGB_WHITE);		    // 화면 클리어
	LCD_SetFont(&Gulim8);		    // 폰트 : 굴림 8
    LCD_SetBackColor(RGB_WHITE);	// 글자배경색 : White
    //학생의 학번 및 이름 작성
	LCD_SetTextColor(RGB_BLUE);	    // 글자색 : Blue
	LCD_DisplayText(0, 0,"2020134035 JW");  	// 학생의 학번 및 이름
	//ROOM 번호 표시
	LCD_SetTextColor(RGB_BLACK);	  // 글자색 : Black
	LCD_DisplayText(1, 0,"R 0 1 2 3 4 5 6 7"); //Room 번호 표시
	//ROOM 개방 상황 표시
    LCD_SetPenColor(RGB_BLACK);
    for(int i=0; i<8; i++)
    {
        LCD_DrawRectangle(Rectangle_x, 27, 8, 8); //사각형 그리기
		Rectangle_x += 16;						  //사각형 시작점 X축으로 16이동
    }    
    //동작상황표시
	LCD_SetTextColor(RGB_BLACK);		//글자색: Black
	LCD_DisplayText(2, 18,"S");			//동작 상태: S
}

//Room개방여부를 표시하는 함수
void RoomOpen(int SwitchFlag)
{
    LCD_SetBrushColor(GET_RGB(255, 0, 255));			//배경색: Pink
    LCD_DrawFillRect(16 * (SwitchFlag + 1), 28, 7, 7); 	//사각형 내부 채우기
}	

//GPIO, EXTI, GLCD를 초기화하는 함수
void System_Clear(void)
{
	_GPIO_Init();			// GPIO 초기화
	_EXTI_Init();			// EXTI 초기화
	DisplayInitScreen();	// GLCD 초기화
	GPIOG->ODR &= ~0x00FF;	// 초기값: LED0~7 Off
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