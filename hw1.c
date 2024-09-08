/////////////////////////////////////////////////////////////
// 과제명: HW1. 오목게임
// 과제개요: STM32F407 마이크로컨트롤러를 이용해 오목게임을 구현
// 사용한 하드웨어(기능): GPIO, Switch, Joy-stick, LED, Buzzer, GLCD, EXTI, FRAM
// 제출일: 2024. 09. . 
// 제출자 클래스: 수요일반
// 학번: 2020134035
// 이름: 정 웅
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW0 0xFE00		    //PH 8
#define SW7 0x7F00			//PH 15
#define JOY_PUSH	0x03C0	//PI5 0000 0011 1100 0000 //EXTI 9
#define JOY_UP		0x03A0	//PI6 0000 0011 1010 0000 
#define JOY_DOWN	0x0360	//PI7 0000 0011 0110 0000 
#define JOY_RIGHT	0x02E0	//PI8 0000 0010 1110 0000 
#define JOY_LEFT	0x01E0	//PI9 0000 0001 1110 0000 

void _GPIO_Init(void);			  // GPIO 초기화 함수  
void _EXTI_Init(void);			  // 인터럽트 초기화 함수		
void DisplayInitScreen(void);	  // GLCD 초기화 함수
void System_Clear(void);		  // GPIO, EXTI, GLCD를 초기화하는 함수
uint16_t KEY_Scan(void);		  // SW입력 받는 함수
uint16_t JOY_Scan(void);		  //JOYSTICK 입력 받는 함수
void BEEP(void);			   	  //30ms동안 부저가 울리는 함수
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t Red_x = 5;
uint8_t Red_y = 5;
uint8_t Blue_x = 5;
uint8_t Blue_y = 5;
uint8_t Red_Turn = 0;
uint8_t Blue_Turn = 0;

uint8_t Red_Score;
uint8_t Blue_Score;
int main(void)
{
	Fram_Init();                  // FRAM 초기화 H/W 초기화
	Fram_Status_Config(); 		  // FRAM 초기화 S/W 초기화
    Red_Score = Fram_Read(300);   // red팀 점수 Fram에서 불러오기
    Blue_Score = Fram_Read(301);  // Blue팀 점수 Fram에서 불러오기
	System_Clear();	              // 시스템 초기화
	while(1) {
        if(Red_Turn == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left 입력시
                    BEEP();							//부저 1회
                    if(Red_x > 0)                   //Red x좌표가 1이상일때
                        Red_x -= 1;					//Red x좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                break;
                case JOY_RIGHT:						//Joy_Left입력시
                    BEEP();							//부저 1회
                    if(Red_x < 9)                   //Red x좌표가 8이하일때
                        Red_x += 1;					//Red x좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                break;
                case JOY_UP:						//Joy_UP 입력시
                    BEEP();							//부저 1회
                    if(Red_y < 9)                   //Red y좌표가 8이하일때
                        Red_y += 1;					//Red y좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                break;
                case JOY_DOWN:						//Joy_UP 입력시
                    BEEP();							//부저 1회
                    if(Red_y > 0)                   //Red y좌표가 1이상 일때
                        Red_y -= 1;					//Red y좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                break;
            }	//switch(JOY_Scan()) end
        }//if(Red_Turn == 1) end

        else if(Blue_Turn == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left 입력시
                    BEEP();						    //부저 1회
                    if(Blue_x > 0)                  //Blue x좌표가 1이상일때
                        Blue_x -= 1;	        	//Blue x좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                break;
                case JOY_RIGHT:                     //Joy_Right 입력시
                    BEEP();							//부저 1회
                    if(Blue_x < 9)                  //Blue x좌표가 8이하일때
                        Blue_x += 1;			    //Blue x좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                break;
                case JOY_UP:						//Joy_UP 입력시
                    BEEP();			    			//부저 1회
                    if(Blue_y < 9)                  //Blue y좌표가 8이하일때
                        Blue_y += 1;		   		//Blue y좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                break;
                case JOY_DOWN:						//Joy_DOWN 입력시
                    BEEP();							//부저 1회
                    if(Blue_y > 0)                   //Blue y좌표가 1이상 일때
                        Blue_y -= 1;					//Blue y좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);              
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                break;
            }	//switch(JOY_Scan()) end
        }//else if(Blue_Turn == 1) end
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

	// LED (GPIO G) 설정 : LED0(PG0), LED7(PG7) Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00004001;	// GPIOG 0,7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00004001;	// GPIOG 0,7 : Push-pull
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00004001;	// GPIOG 0,7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)

	// Switch (GPIO H) 설정 : SW0(PH8), SW7(PH15) Input mode 1100
	// RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable				
	// GPIOH->MODER 	&= ~0xC0010000;	// GPIOH 8,15 : Input mode				
	// GPIOH->PUPDR 	&= ~0xC0010000;	// GPIOH 8,15 : Floating Input (No Pull-up, pull-down) :reset state

	//Joy Stick SW(GPIO I) 설정 : Input mode(PI5 ~ PI9)
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FF000;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FF000;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI8,9(PH 8-SW0, NAVI_PUSH-PI5)
	EXTI15(PH15, SW7))초기 설정  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0xC0010000;	// GPIOH 8,15 : Input mode
    GPIOI->MODER	&= ~0x00000C00;	// GPIOI 5 : Input mode (reset state)
	// EXTI8 <- PH8, EXTI9 <- PI5, 
	// EXTI15 <- PH15 
	// EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])를 이용 
	// reset value: 0x0000
    SYSCFG->EXTICR[2] &= ~0x00FF;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x0087;	// EXTI8 -> GPIOH, EXTI9 -> GPIOI	
    SYSCFG->EXTICR[3] &= ~0xF000;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x7000;	// EXTI15 -> GPIOH
	
	EXTI->FTSR |= 0x8300;			// EXTI8,9,15: Falling Trigger Enable  
	EXTI->IMR  &= ~0x8300;			// EXTI8,9,15: IMR Reg 초기화
	EXTI->IMR  |= 0x8300;			// EXTI8,9,15: UnMasked (Interrupt Enable) 설정

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI8,9'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI15'
}

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
	//적돌선택 (EXTI8,SW 0)
	if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
        //EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (Interrupt Enable) 설정
        GPIOG->ODR |= 0x0001; 	    // LED0 ON
        GPIOG->ODR &= ~0x0080; 	    // LED7 OFF 
        Red_Turn = 1;               //적돌 Enable
        Blue_Turn = 0;
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayText(9, 0, "*");
		LCD_DisplayText(9, 18, " ");
    }
	//착돌 (EXTI9, JOY_PUSH(PI5))
	else if(EXTI->PR & 0x0200)		    // EXTI9 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0200;			    // Pending bit Clear
        /*if(Red_Turn == 1 && JOY_Scan()==JOY_PUSH)          //적돌일때 착돌
        {
            EXTI->IMR  &= ~0x0100;		// EXTI7: Masked (Interrupt Enable) 설정
            EXTI->IMR  |= 0x8000;		// EXTI15: unMasked (Interrupt Enable) 설정
            LCD_SetBrushColor(RGB_RED);                // 사각형 배경색 설정
            LCD_DrawFillRect(22 + Red_x*10, 22 + Red_y*10, 7, 7);  		 // 사각형 내부 채우기
        }
        else if(Blue_Turn == 1 && JOY_Scan()==JOY_PUSH)     //청돌일때 착돌
        {
            EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (Interrupt Enable) 설정
            EXTI->IMR  |= 0x0100;		// EXTI7: unMasked (Interrupt Enable) 설정
            LCD_SetBrushColor(RGB_BLUE);                // 사각형 배경색 설정
            LCD_DrawFillRect(22 + Blue_x*10, 22 + Blue_y*10, 7, 7);  		 // 사각형 내부 채우기
        }*/
		BEEP();								// 부저 1회
    }
}

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// 청돌선택 (EXTI15,SW 7)
	if(EXTI->PR & 0x8000)			// EXTI11 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
        //EXTI->IMR  &= ~0x0100;		// EXTI7: Masked (Interrupt Enable) 설정
        GPIOG->ODR &= ~0x0001; 	    // LED0 OFF
        GPIOG->ODR |= 0x0080; 	    // LED7 ON
		Red_Turn = 0;               //청돌 Enable
        Blue_Turn = 1;
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayText(9, 18, "*");
		LCD_DisplayText(9, 0, " ");
	}
}

/* GLCD 초기화면 설정 함수 */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_YELLOW);		    // 화면 클리어
	LCD_SetFont(&Gulim8);		    // 폰트 : 굴림 8
	//Title 작성
	LCD_SetBackColor(RGB_YELLOW);             // 글자배경색 : YELLOW
    LCD_SetTextColor(RGB_BLACK);              // 글자색 : BLACK
	LCD_DisplayText(0, 0, "Mecha-OMOK(JW)");  // Title 작성
	//좌표 표시
	LCD_DisplayText(1, 2, "0");	
	LCD_DisplayText(1, 9, "5");		
	LCD_DisplayText(1, 14, "9");
	LCD_DisplayText(5, 2, "5");		
	LCD_DisplayText(8, 2, "9");
    // 오목판 그리기       			
    LCD_SetBrushColor(RGB_YELLOW);                // 사각형 배경색 설정
	LCD_SetPenColor(RGB_BLACK);				 	 // 사각형 테두리 색 설정
    for(uint16 x = 0 ; x < 9; x++){
        for(uint16 y = 0; y < 9; y++){
            LCD_DrawRectangle(x * 10 + 25, y * 10 + 25, 10, 10);         	 // 사각형 그리기
        }//for(uint16 x = 0 ; x < 9; x++) end
    }//for(uint16 y = 0; y < 9; y++) end
    LCD_DrawRectangle(75 - 2, 75 - 2, 4, 4);    //(5,5) 표시
    LCD_SetBrushColor(RGB_BLACK);                // 사각형 배경색 설정
    LCD_DrawFillRect(75 - 1, 75 - 1, 3, 3);  		 // 사각형 내부 채우기
    // 사용자 선택 좌표 및 SCORE
    LCD_DisplayText(9, 8, "vs");
    LCD_SetTextColor(RGB_RED);              // 글자색 : RED
    LCD_DisplayText(9, 1, "( , )");
    LCD_DisplayChar(9, 2, Red_x + 0x30);
    LCD_DisplayChar(9, 4, Red_y + 0x30);
    LCD_DisplayChar(9, 7, Red_Score + 0x30);
    LCD_SetTextColor(RGB_BLUE);              // 글자색 : BLUE
    LCD_DisplayText(9, 13, "( , )");
    LCD_DisplayChar(9, 14, Blue_x + 0x30);
    LCD_DisplayChar(9, 16, Blue_y + 0x30);
    LCD_DisplayChar(9, 10, Blue_Score + 0x30);
}

//GPIO, EXTI, GLCD를 초기화하는 함수
void System_Clear(void)
{
	LCD_Init();	            // LCD 모듈 초기화
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