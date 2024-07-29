/////////////////////////////////////////////////////////////
// 과제명: TP1. 커피자동판매기
// 과제개요: STM32F407 마이크로컨트롤러를 이용해 커피자판기 시뮬레이션을 구현
// 사용한 하드웨어(기능): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD, EXTI, FRAM
// 제출일: 2024. 06. 16. 
// 제출자 클래스: 수요일반
// 학번: 2020134035
// 이름: 정 웅
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW2 0xFB00				  //PH 8
#define JOY_RIGHT 0x02E0		  //PI 8
#define JOY_LEFT 0x01E0			  //PI 9
#define JOY_UP 0x03A0			  //PI 6

void _GPIO_Init(void);			  // GPIO 초기화 함수  
void _EXTI_Init(void);			  // 인터럽트 초기화 함수		
void DisplayInitScreen(void);	  // GLCD 초기화 함수
/* GLCD에 사각형그리는 함수 */
void Rect(uint16_t col, uint16_t row, uint32_t PenColor, uint32_t BackColor);
/* GLCD에 글자를 포함한 사각형 그리는 함수*/
void RectInText(uint16_t col, uint16_t row, int TextNum, uint32_t PenColor, uint32_t TextColor, uint32_t BackColor);
/* 커피만드는 동작하는 함수*/
void MakeCoffee(uint8_t Choice);
void System_Clear(void);		  // GPIO, EXTI, GLCD를 초기화하는 함수
uint16_t KEY_Scan(void);		  // SW입력 받는 함수
uint16_t JOY_Scan(void);		  //JOYSTICK 입력 받는 함수
void BEEP(void);			   	  //30ms동안 부저가 울리는 함수
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t Input;			//입력금액
uint8_t Total;			//총금액
uint8_t Choice = 0;		//커피선택변수
uint8_t cup = 9;		// cup 재고변수
uint8_t sugar = 5;		// sugar 재고변수
uint8_t milk = 5;		// milk 재고변수
uint8_t coffee = 9;		// coffee 재고변수
uint8_t Noc;			//총팔린 커피 잔수
uint8_t RF = 0;			//리필 필요여부
int main(void)
{
	Fram_Init();                  // FRAM 초기화 H/W 초기화
	Fram_Status_Config(); 		  // FRAM 초기화 S/W 초기화
	Input = Fram_Read(601);		  // 입력값 FRAM에서 읽어오기
	Total = Fram_Read(602);		  // 총 판매액 FRAM에서 읽어오기
	Noc = Fram_Read(603);	      // 총 판매잔수 FRAM에서 읽어오기
	System_Clear();	              // 시스템 초기화
	while(1)	
	{
		//재고가 모두 1이상일때 JOY스틱 및 SW 입력값 스캔
		if(cup >= 1 && sugar >= 1 && milk >= 1 && coffee >= 1)
		{
			switch(JOY_Scan())
			{
				case JOY_LEFT:						//Joy_Left 입력시
					BEEP();							//부저 1회
					Choice = 1;						//Black coffee 선택
					/*Black coffee만 박스안 글자 빨간색*/
					LCD_SetBackColor(RGB_BLUE);
					LCD_SetTextColor(RGB_WHITE);
					LCD_DisplayText(2, 3, "S");
					LCD_DisplayText(3, 5, "M");
					LCD_SetTextColor(RGB_RED);
					LCD_DisplayText(3, 1, "B");
				break;
				case JOY_UP:						//Joy_UP 입력시
					BEEP();							//부저 1회
					Choice = 2;						//Sugar coffee 선택
					/*Sugar coffee만 박스안 글자 빨간색*/
					LCD_SetBackColor(RGB_BLUE);
					LCD_SetTextColor(RGB_WHITE);
					LCD_DisplayText(3, 1, "B");
					LCD_DisplayText(3, 5, "M");
					LCD_SetTextColor(RGB_RED);
					LCD_DisplayText(2, 3, "S");
				break;
				case JOY_RIGHT:						//Joy_Left입력시
					BEEP();							//부저 1회
					Choice = 3;						//Mix coffee 선택
					/*Mix coffee만 박스안 글자 빨간색*/
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
					// Black coffee를 선택, 입력 Coin 10원 이상, 리필불필요 상태일때 실행
					if(Choice == 1 && Input >= 1 && RF == 0)
					{
						BEEP();				//부저 1회
						MakeCoffee(1);		//Black coffee 제작
					}
					// Sugar coffee를 선택, 입력 Coin 20원 이상, 리필불필요 상태일때 실행
					else if(Choice == 2 && Input >= 2 && RF == 0)
					{
						BEEP();				//부저 1회
						MakeCoffee(2);		//Sugar coffee 제작
					} 
					// Mix coffee를 선택, 입력 Coin 30원 이상, 리필불필요 상태일때 실행
					else if(Choice == 3 && Input >= 3 && RF == 0)
					{
						BEEP();				//부저 1회
						MakeCoffee(3);		//Mix coffee 제작
					}
					break;
			}	//switch(KEY_Scan()) end
		} // if() end
		//재료 중 하나라도 0이면 동전입력 x
		else
		{
			EXTI->IMR  	&= ~0x0300;			// EXTI8,9: Disable
			EXTI->IMR	|= 0x1000;			// EXTI12 : Enable
			RF = 1;							//Refill이 필요
			Rect(15, 6, RGB_GREEN, RGB_RED);	//리필 사각형 빨간색으로 변함
		} // else end		
	}  //whlie(1) end	
} // main() end

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
	GPIOH->MODER 	&= ~0x0FFF0000;	// GPIOH 8~13 : Input mode				
	GPIOH->PUPDR 	&= ~0x0FFF0000;	// GPIOH 8~13 : Floating Input (No Pull-up, pull-down) :reset state

	//Joy Stick SW(GPIO I) 설정 : Input mode
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000F3000;	// GPIOI 6,8,9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000F3000;	// GPIOI 6,8,9 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI8,9(GPIOH.8~9, SW0~1)
	EXTI11~15(GPIOH.11~15, SW3~7))초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0x0FFF0000;	// GPIOH PIN8~PIN13 Input mode (reset state)	

	// EXTI8 <- PH8, EXTI9 <- PH9, 
	// EXTI11 <- PH11, EXTI12 <- PH12, EXTI13 <- PH13 
	// EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])를 이용 
	// reset value: 0x0000
    SYSCFG->EXTICR[2] &= ~0x00FF;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x7077;	// EXTI8,9,11에 대한 소스 입력은 GPIOH로 설정	
    SYSCFG->EXTICR[3] &= ~0x00FF;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x0077;	// EXTI12,13에 대한 소스 입력은 GPIOH로 설정
	
	EXTI->FTSR |= 0x3F00;			// EXTI8~13: Falling Trigger Enable  
	EXTI->IMR  &= ~0x3F00;			// EXTI8~13: IMR Reg 초기화
	EXTI->IMR  |= 0x2B00;			// EXTI8,9,11,13: UnMasked (Interrupt Enable) 설정

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI8,9'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI11~13'
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
	//10원 INPUT 발생 (EXTI8,SW 0)
	if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
		Rect(11, 3, RGB_GREEN, RGB_YELLOW);	// 노란색으로 10원 입력 표시
		BEEP();								// 부저 1회
		DelayMS(1000);						// 1초 유지 
		Rect(11, 3, RGB_GREEN, RGB_GRAY);	// 다시 회색 변경
		LCD_SetBackColor(RGB_BLACK);		// 글자배경색 : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// 글자색 : YELLOW
		//Input이 20미만인 경우만 Input +1, Input값 LCD최신화 
		if(Input < 20)
		{
			Input += 1;						//Input값 +1
			Fram_Write(601,Input);			// 값이 바뀐 INPUT값 FRAM에 저장
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100의 자리
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10의 자리
		}
    }
	//50원 Input 발생 (EXTI9,SW 1)
	else if(EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0200;			// Pending bit Clear
		Rect(11, 5, RGB_GREEN, RGB_YELLOW);	// 노란색으로 50원 입력 표시
		BEEP();								// 부저 1회
		DelayMS(1000);						// 1초 유지 
		Rect(11, 5, RGB_GREEN, RGB_GRAY);	// 다시 회색 변경
		LCD_SetBackColor(RGB_BLACK);		// 글자배경색 : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// 글자색 : YELLOW
		//Input이 15미만인 경우만 Input +5, Input값 LCD최신화
		if(Input < 15)
		{
			Input += 5;						//Input값 +5
			Fram_Write(601,Input);			// 값이 바뀐 INPUT값 FRAM에 저장
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100의 자리
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10의 자리
		}
		//Input이 15이상일 경우 Input == 20, Input값 LCD최신화
		else if(Input >= 15 && Input < 20)
		{
			Input = 20;						// Input값 == 20
			Fram_Write(601,Input);			// 값이 바뀐 INPUT값 FRAM에 저장
			LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100의자리
			LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10의자리
		}
    }    
}

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// 잔돈반환 (EXTI11,SW 3)
	if(EXTI->PR & 0x0800)			// EXTI11 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0800;			// Pending bit Clear
		BEEP();						// 부저 1회
		Input = 0;					// Input값 == 0
		Fram_Write(601,0);			// 값이 바뀐 INPUT값 FRAM에 저장
		// 변경된 Input값 최신화
		LCD_SetBackColor(RGB_BLACK);		// 글자배경색 : BLACK
		LCD_SetTextColor(RGB_YELLOW); 		// 글자색 : YELLOW
		LCD_DisplayChar(1,14,(Fram_Read(601) / 10) + 0x30);	//100의자리
		LCD_DisplayChar(1,15,(Fram_Read(601) % 10) + 0x30);	//10의자리
	}
	// Refill 실행 (EXTI12,SW 4)
	else if(EXTI->PR & 0x1000)		// EXTI12 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x1000;			// Pending bit Clear
		/*재고가 ‘0’인 재료만 최대값*/
		// cup의 재고가 0일때
		if(cup == 0)
		{
			cup = 9;					//cup 재고 최대값
			RF = 0;						//리필 불필요 상태
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//리필 불필요 LCD표시
		}
		// sugar의 재고가 0일때
		if(sugar == 0)
		{
			sugar = 5;					//sugar 재고 최대값
			RF = 0;						//리필 불필요 상태
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//리필 불필요 LCD표시
		}
		// milk의 재고가 0일때
		if(milk == 0)
		{
			milk = 5;					//milk 재고 최대값
			RF = 0;						//리필 불필요 상태
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//리필 불필요 LCD표시
		}
		// coffee의 재고가 0일때
		if(coffee == 0)
		{
			coffee = 9;					//coffee 재고 최대값
			RF = 0;						//리필 불필요 상태
			Rect(15, 6, RGB_GREEN, RGB_GREEN);	//리필 불필요 LCD표시
		}
		//동전 입력 재개
		EXTI->IMR  		|= 0x0300;			// EXTI8,9: Enable
		EXTI->IMR		&= ~0x1000;			// EXTI12 : Disable
		//리필 후 재고값 LCD최신화
		LCD_SetBackColor(RGB_WHITE);
		LCD_SetTextColor(RGB_BLACK);
		LCD_DisplayChar(6, 1, cup + 0x30);
		LCD_DisplayChar(6, 4, sugar + 0x30);
		LCD_DisplayChar(6, 7, milk + 0x30);
		LCD_DisplayChar(6, 10, coffee + 0x30);
		//0.5초 간격으로 부저 2번
		for(int i=0;i<2;i++)
		{
			BEEP();
			DelayMS(500);
		}
	}

	// CLEAR 실행(EXTI13, SW 5)
    else if(EXTI->PR & 0x2000)	// EXTI13 Interrupt Pending(발생) 여부
	{
		EXTI->PR |= 0x2000;			// Pending bit Clear
		BEEP();						// 부저 1회
		Total = 0;					// Total값 == 0
		Fram_Write(602,0);			// 값이 바뀐 Total값 FRAM에 저장
		// 변경된 Total값 최신화
		LCD_SetBackColor(RGB_BLACK);		// 글자배경색 : BLACK
		LCD_SetTextColor(RGB_YELLOW);		// 글자색 : YELLOW
		LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100의 자리
		LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10의 자리
		
		Noc = 0;					// Noc값 == 0
		Fram_Write(603,0);			// 값이 바뀐 Noc값 FRAM에 저장
		// 변경된 Noc값 최신화
		LCD_SetBackColor(RGB_YELLOW);		// 글자배경색 : YELLOW
		LCD_SetTextColor(RGB_BLACK);		// 글자색 : BLACK
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);		//10의 자리
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);		//1의 자리
	}
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);		    // 화면 클리어
	LCD_SetFont(&Gulim8);		    // 폰트 : 굴림 8
	//Title 작성
	RectInText(0, 0, 10, RGB_BLACK, RGB_BLACK, RGB_YELLOW);	//Title 사각형 생성
	LCD_SetBackColor(RGB_YELLOW);                         // 글자배경색 : YELLOW
    LCD_SetTextColor(RGB_BLACK);                          // 글자색 : BLACK
	LCD_DisplayText(0, 0, " JW coffee");				  // Title 작성
	LCD_DrawRectangle(0, 0, 82, 14);         			  // 글자배경으로 지워지는 사각형 그리기
	//커피선택 박스
	RectInText(1, 3, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// Black coffee 사각형 생성
	RectInText(3, 2, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// sugar coffee 사각형 생성
	RectInText(5, 3, 1, RGB_GREEN, RGB_WHITE, RGB_BLUE);	// mix coffee 사각형 생성
	LCD_DisplayText(3, 1, "B");			// Black coffee LCD표시
	LCD_DisplayText(2, 3, "S");			// sugar coffee LCD표시
	LCD_DisplayText(3, 5, "M");			// mix coffee LCD표시
	// 커피제조 박스
	RectInText(3, 4, 1, RGB_GREEN, RGB_WHITE, RGB_RED);		// 커피제조중 사각형 생성
	LCD_DisplayText(4, 3, "W");								// 커피제조중 LCD표시
	// 사용자가 넣은 금액 표시
	LCD_SetBackColor(RGB_WHITE);								// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK); 								// 글자색 : BLACK            			
    LCD_DisplayText(0, 14, "IN");								// INPUT LCD표시
	RectInText(14, 1, 3, RGB_GREEN, RGB_YELLOW, RGB_BLACK);		//INPUT 사각형 생성
	//INPUT 범위 0~20 이면 FRAM에 저장된값 표시
	if(Input <= 20)
	{
		LCD_DisplayChar(1, 14, (Input / 10) + 0x30);	//100의자리
		LCD_DisplayChar(1, 15, (Input % 10) + 0x30);	//10의자리
		LCD_DisplayChar(1, 16, 0+0x30);					//1의자리
	}
	//INPUT 범위 0~20 아니면 0표시
	else 
	{
		LCD_DisplayChar(1, 14, 0+0x30);		//100의자리
		LCD_DisplayChar(1, 15, 0+0x30);		//10의자리
		LCD_DisplayChar(1, 16, 0+0x30);		//1의자리
	}
	// 자판기 총 수입 표시
	LCD_SetBackColor(RGB_WHITE);					// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK); 					// 글자색 : BLACK
	LCD_DisplayText(2, 14, "TOT");					// TOTAL LCD표시
	RectInText(14, 3, 3, RGB_GREEN, RGB_YELLOW, RGB_BLACK); //TOTAL 사각형 생성
	//Total 범위 0~99 이면 FRAM에 저장된값 표시
	if(Total <= 99)
	{
		LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100의자리
		LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10의자리
		LCD_DisplayChar(3, 16, 0+0x30);					//1의자리
	}
	//Total 범위 0~99 아니면 0표시
	else
	{
		LCD_DisplayChar(3, 14, 0+0x30);		//100의자리
		LCD_DisplayChar(3, 15, 0+0x30);		//10의자리
		LCD_DisplayChar(3, 16, 0+0x30);		//1의자리
	}
	//동전투입 표시
	LCD_SetBackColor(RGB_WHITE);		// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK);        // 글자색 : BLACK           		   
	LCD_DisplayText(2, 10, "\\10");		// 10원 LCD표시
	LCD_DisplayText(4, 10, "\\50");		// 50원 LCD표시
	Rect(11, 3, RGB_GREEN, RGB_GRAY);   // 10원 사각형 그리기
	Rect(11, 5, RGB_GREEN, RGB_GRAY);	// 50원 사각형 그리기
	//재고표시
	LCD_SetBackColor(RGB_WHITE);		// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK);		// 글자색 : BLACK 
	LCD_DisplayText(7, 0, "cp");		//cup LCD표시
	LCD_DisplayText(7, 3, "sg");		//sugar LCD표시
	LCD_DisplayText(7, 6, "mk");		//milk LCD표시
	LCD_DisplayText(7, 9, "cf");		//coffee LCD표시
	RectInText(1, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//cup 사각형 그리기
	RectInText(4, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//sugar 사각형 그리기
	RectInText(7, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//milk 사각형 그리기
	RectInText(10, 6, 1, RGB_GREEN, RGB_BLACK, RGB_WHITE);	//coffee 사각형 그리기
	LCD_DisplayChar(6, 1, cup + 0x30);		//cup 재고 표시
	LCD_DisplayChar(6, 4, sugar + 0x30);	//coffee 재고 표시
	LCD_DisplayChar(6, 7, milk + 0x30);		//milk 재고 표시
	LCD_DisplayChar(6, 10, coffee + 0x30);	//coffee 재고 표시
	//리필여부표시
	LCD_DisplayText(6, 13, "RF");			//Refill LCD표시
	Rect(15, 6, RGB_GREEN, RGB_GREEN);		//Refill 사각형 그리기
	//총 커피 판매수
	LCD_SetBackColor(RGB_WHITE);		// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK);		// 글자색 : BLACK 
	LCD_DisplayText(7, 13, "NoC");		// 총 커피 판매수 LCD표시
	RectInText(14, 8, 2, RGB_GREEN, RGB_BLACK, RGB_YELLOW);	// 총 커피 판매수 사각형 그리기
	//Noc 범위 0~50 이면 FRAM에 저장된값 표시
	if(Noc <= 50)
	{
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);
	}
	//Noc 범위 0~50 아니면 0표시
	else
	{
		LCD_DisplayChar(8, 14, 0 + 0x30);
		LCD_DisplayChar(8, 15, 0 + 0x30);
	}
}

/* GLCD에 10x10 사각형그리는 함수 */
void Rect(uint16_t col, uint16_t row, uint32_t PenColor, uint32_t BackColor)
{
	uint16_t x = (col * 8);						 // 사각형 열 좌표 설정
    uint16_t y = (row * 13);					 // 사각형 행 좌표 설정
	LCD_SetBrushColor(BackColor);                // 사각형 배경색 설정
    LCD_DrawFillRect(x + 1, y + 1, 9, 9);  		 // 사각형 내부 채우기
	LCD_SetPenColor(PenColor);				 	 // 사각형 테두리 색 설정
    LCD_DrawRectangle(x, y, 10, 10);         	 // 사각형 그리기
}

/* GLCD에 글자를 포함한 사각형 그리는 함수*/
void RectInText(uint16_t col, uint16_t row, int TextNum, uint32_t PenColor, uint32_t TextColor, uint32_t BackColor)
{
	uint16_t x = (col * 8) - 2;				// 사각형 시작점 열 좌표 설정
    uint16_t y = (row * 13) - 2;			// 사각형 시작점 행 좌표 설정
	// 행과 열이 모두 0일때 사각형이 GLCD에서 벗어나는 현상 보정 
	if(col == 0 && row == 0)
	{
		x = 0;
		y = 0;
	}
	// Text가 길어지면 상자가 더 커지는 현상 보정
	if(TextNum >= 3)
	{
		uint16_t TextFlag = TextNum;
		//3의 배수일때마다 textnum 1씩 감소
		while(TextFlag < 3)
		{
			TextFlag -= 3;
			TextNum -= 1;
		}
	}
	LCD_SetBrushColor(BackColor);                        // 사각형 배경색 설정
    LCD_DrawFillRect(x + 1, y + 1, TextNum*8 + 1, 13);   // 사각형 내부 채우기
	LCD_SetBackColor(BackColor);                         // 글자 배경색 설정
    LCD_SetTextColor(TextColor);                         // 글자색 설정
	LCD_SetPenColor(PenColor);							 // 사각형 테두리 색 설정
    LCD_DrawRectangle(x, y, TextNum*8 + 2, 14);          // 사각형 그리기
}

/* 커피 제조 함수 */
void MakeCoffee(uint8_t Choice)
{
	EXTI->IMR  		&= ~0x2B00;			// EXTI8,9,11,13: Disable
	GPIOG->ODR		|= 0x0000FFFF;		// LED 0~7 On
	if(Choice == 1)		//black coffee
	{
		cup -= 1;			// cup 재고 -1
		coffee -= 1;		// coffee 재고 -1
		Input -= 1;			// 입력 Coin 값 -1
		Fram_Write(601,Input);	// 값이 바뀐 INPUT값 FRAM에 저장
		if(Total < 99)			// 총 판매액이 99 미만일때
		{
			Total += 1;			// 총 판매액 +1
			Fram_Write(602,Total);  // 값이 바뀐 Total값 FRAM에 저장
		}
	}
	else if(Choice == 2)	//sugar coffee
	{
		cup -= 1;			// cup 재고 -1
		sugar -= 1;			// sugar 재고 -1
		coffee -= 1;		// coffee 재고 -1
		Input -= 2;			// 입력 Coin 값 -2
		Fram_Write(601,Input);	// 값이 바뀐 INPUT값 FRAM에 저장
		if(Total < 98)			// 총 판매액이 98 미만일때
		{
			Total += 2;			// 총 판매액 +2
			Fram_Write(602,Total);  // 값이 바뀐 Total값 FRAM에 저장
		}
		else
		{
			Total = 99;				// 최대금액으로 표시
			Fram_Write(602,Total);  // 값이 바뀐 Total값 FRAM에 저장
		}
	}
	else if(Choice == 3)	//mix coffee
	{
		cup -= 1;			// cup 재고 -1
		sugar -= 1;			// sugar 재고 -1
		milk -= 1;			// milk 재고 -1
		coffee -= 1;		// coffee 재고 -1
		Input -= 3;			// 입력 Coin 값 -3
		Fram_Write(601,Input);	// 값이 바뀐 INPUT값 FRAM에 저장
		if(Total < 97)			// 총 판매액이 97 미만일때
		{
			Total += 3;			// 총 판매액 +3
			Fram_Write(602,Total);  // 값이 바뀐 Total값 FRAM에 저장
		}
		else
		{
			Total = 99;				// 최대금액으로 표시
			Fram_Write(602,Total);  // 값이 바뀐 Total값 FRAM에 저장
		}
	}
	//제조중 표시
	LCD_SetBackColor(RGB_RED);				// 글자배경색 : RED
	LCD_SetTextColor(RGB_WHITE);			// 글자색 : WHITE
	// 커피제조중 사각형 안 TEXT 1초 간격으로 0, 1, 2, W 변경
	// 그리고 3초후 LED0~7 OFF 및 0.5초간격 부저 3회 
    LCD_DisplayText(4, 3, "0");		
	DelayMS(1000);					//Delay 1초
	LCD_DisplayText(4, 3, "1");
	DelayMS(1000);					//Delay 1초
	LCD_DisplayText(4, 3, "2");
	DelayMS(1000);					//Delay 1초
	LCD_DisplayText(4, 3, "W");
	DelayMS(3000);					//Delay 3초
	GPIOG->ODR	&= ~0x0000FFFF;		// LED 0~7 OFF
	//0.5초 간격으로 비프음 3회
	for(int i=0;i<3;i++)
	{
		BEEP();
		DelayMS(500);
	}
	//재고 감소 LCD 최신화
	LCD_SetBackColor(RGB_WHITE);			// 글자배경색 : WHITE
	LCD_SetTextColor(RGB_BLACK);			// 글자색 : BLACK
	LCD_DisplayChar(6, 1, cup + 0x30);		// cup 재고 표시
	LCD_DisplayChar(6, 4, sugar + 0x30);	// coffee 재고 표시
	LCD_DisplayChar(6, 7, milk + 0x30);		// milk 재고 표시
	LCD_DisplayChar(6, 10, coffee + 0x30);	// coffee 재고 표시
	//Noc 0~50일때 LCD 최신화
	if(Noc < 50) 
	{
		LCD_SetBackColor(RGB_YELLOW);		// 글자배경색 : YELLOW
		LCD_SetTextColor(RGB_BLACK);		// 글자색 : BLACK
		Noc += 1;							// 총 커피 판매잔수 +1
		Fram_Write(603,Noc);				// 값이 바뀐 Noc값 FRAM에 저장
		LCD_DisplayChar(8, 14, (Noc / 10) + 0x30);	//10의;자리
		LCD_DisplayChar(8, 15, (Noc % 10) + 0x30);	//1 의자리
	}
	//IN LCD 최신화
	LCD_SetBackColor(RGB_BLACK);		// 글자배경색 : BLACK
	LCD_SetTextColor(RGB_YELLOW); 		// 글자색 : YELLOW
	LCD_DisplayChar(1, 14, (Input / 10) + 0x30);	//100의 자리
	LCD_DisplayChar(1, 15, (Input % 10) + 0x30);	//10의 자리
	//ToT LCD 최신화
	LCD_DisplayChar(3, 14, (Total / 10) + 0x30);	//100의 자리
	LCD_DisplayChar(3, 15, (Total % 10) + 0x30);	//10의 자리
	//선택화면 초기화
	LCD_SetBackColor(RGB_BLUE);			// 글자배경색 : BLUE
	LCD_SetTextColor(RGB_WHITE);		// 글자색 : WHITE
	LCD_DisplayText(3, 1, "B");			// Black coffee LCD표시
	LCD_DisplayText(2, 3, "S");			// sugar coffee LCD표시
	LCD_DisplayText(3, 5, "M");			// mix coffee LCD표시
	//choice변수 초기화
	Choice = 0;
	EXTI->IMR  |= 0x2B00;			// EXTI8,9,11,13: UnMasked (Interrupt Enable) 설정	
}

//GPIO, EXTI, GLCD를 초기화하는 함수
void System_Clear(void)
{
	LCD_Init();	// LCD 모듈 초기화
	DelayMS(10);
	_GPIO_Init();			// GPIO 초기화
	_EXTI_Init();			// EXTI 초기화
	DisplayInitScreen();	// GLCD 초기화
}

/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */
// key_flag -> SW를 한번 눌렀는지 인지하기 위한 변수
uint8_t key_flag = 0;
uint16_t KEY_Scan(void) // input key SW0 - SW7
{
    uint16_t key;
    // SW의 핀은 PH8~15이므로 PH0~7을 0으로 초기화
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
    // else로 오면 키가 눌린것
    else // if key input, check continuous key
    {
        if (key_flag != 0) // if continuous key, treat as no key input
            return 0xFF00;
        else // if new key, delay for debounce
        {
            key_flag = 1;
            DelayMS(10); // 혹시 모를 노이즈방지
            return key;
        }
    }
}

// JoyStick이 입력되었는지를 여부와 어떤 JoyStick이 입력되었는지의 정보를 return하는 함수
// joy_flag -> JoyStick을 한번 눌렀는지 인지하기 위한 변수
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