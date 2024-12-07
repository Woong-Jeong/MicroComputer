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

#define SW0 0xFE00		    //PH 8
#define SW7 0x7F00			//PH 15
#define JOY_PUSH	0x03C0	//PI5 0000 0011 1100 0000 
#define JOY_UP		0x03A0	//PI6 0000 0011 1010 0000 
#define JOY_DOWN	0x0360	//PI7 0000 0011 0110 0000 
#define JOY_RIGHT	0x02E0	//PI8 0000 0010 1110 0000 
#define JOY_LEFT	0x01E0	//PI9 0000 0001 1110 0000 

void _GPIO_Init(void);			  // GPIO 초기화 함수  
void _EXTI_Init(void);			  // 인터럽트 초기화 함수		
void DisplayInitScreen(void);	  // GLCD 초기화 함수
void System_Clear(void);		  // GPIO, EXTI, GLCD를 초기화하는 함수
uint16_t KEY_Scan(void);		  // SW입력 받는 함수
void Detect_Win(void);            // 오목 승패 판정 함수
uint16_t JOY_Scan(void);		  // JOYSTICK 입력 받는 함수
void BEEP(void);			   	  // 30ms동안 부저가 울리는 함수
void BEEP_3(void);                // 0.5초간격 부저 3회
void BEEP_5(void);                // 1초간격 부저 5회
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void USART1_Init(void);             // USART1 설정
void USART_BRR_Configuration(uint32_t USART_BaudRate);  //USART Baudrate 설정
void SerialSendChar(uint8_t Ch);    // 1문자 보내기 함수
void SerialSendString(char *str);   // 여러문자 보내기 함수

uint8_t Warning[20] = {'N','o','t',' ','r','u','n','n','i','n','g',' '};
uint8_t Input[5];           // PC로부터 받을 좌표정보를 저장하는 배열
int input_count = 0;        // 받은 문자 개수를 count하는 변수
uint8_t Rx;                 // 받은 문자를 임시로 저장하는 변수
uint8_t SendPc[12];          // PC에 보낼 좌표정보를 저장하는 배열

uint8_t Red_x = 5;          // 선택된 적돌 x좌표
uint8_t Red_y = 5;          // 선택된 적돌 y좌표
uint8_t Blue_x = 5;         // 선택된 청돌 x좌표
uint8_t Blue_y = 5;         // 선택된 청돌 y좌표
uint8_t Red_Turn = 0;       // 적돌 순서 알려주는 변수
uint8_t Blue_Turn = 0;      // 청돌 순서 알려주는 변수
uint8_t Move_flag = 0;      // joystick 움직임 가능
uint8_t red[10][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // 적돌이 착돌된 위치 저장하는 배열
                    ,{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

uint8_t blue[10][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // 청돌이 착돌된 위치 저장하는 배열
                    ,{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t Red_Score;          //FRAM에 저장된 적돌 점수
uint8_t Blue_Score;         //FRAM에 저장된 청돌 점수

int main(void)
{
	System_Clear();	              // 시스템 초기화
    USART1_Init();
	while(1) {
        if(Red_Turn == 1 && Move_flag == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left 입력시
                    BEEP();							//부저 1회
                    if(Red_x > 0)                   //Red x좌표가 1이상일때
                        Red_x -= 1;					//Red x좌표가 1 감소
                    //좌표 최신화
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_RIGHT:						//Joy_Left입력시
                    BEEP();							//부저 1회
                    if(Red_x < 9)                   //Red x좌표가 8이하일때
                        Red_x += 1;					//Red x좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_UP:						//Joy_UP 입력시
                    BEEP();							//부저 1회
                    if(Red_y > 0)                   //Red y좌표가 1이상 일때
                        Red_y -= 1;					//Red y좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                    DelayMS(10);
                break;
                case JOY_DOWN:						//Joy_UP 입력시
                    BEEP();							//부저 1회
                    if(Red_y < 9)                   //Red y좌표가 8이하일때
                        Red_y += 1;					//Red y좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                    DelayMS(10);
                break;
            }	//switch(JOY_Scan()) end
        }//if(Red_Turn == 1) end

        else if(Blue_Turn == 1 && Move_flag == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left 입력시
                    BEEP();						    //부저 1회
                    if(Blue_x > 0)                  //Blue x좌표가 1이상일때
                        Blue_x -= 1;	        	//Blue x좌표가 1 감소
                    //좌표 최신화
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_RIGHT:                     //Joy_Right 입력시
                    BEEP();							//부저 1회
                    if(Blue_x < 9)                  //Blue x좌표가 8이하일때
                        Blue_x += 1;			    //Blue x좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_UP:						//Joy_UP 입력시
                    BEEP();			    			//부저 1회
                    if(Blue_y > 0)                   //Blue y좌표가 1이상 일때
                        Blue_y -= 1;					//Blue y좌표가 1 감소
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);              
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                    DelayMS(10);
                break;
                case JOY_DOWN:						//Joy_DOWN 입력시
                    BEEP();							//부저 1회
                    if(Blue_y < 9)                  //Blue y좌표가 8이하일때
                        Blue_y += 1;		   		//Blue y좌표가 1 증가
                    /*좌표 최신화*/
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                    DelayMS(10);
                break;
            }	//switch(JOY_Scan()) end
        }//else if(Blue_Turn == 1) end
        DelayMS(10);        //화면 흐려짐 방지
	}  //whlie(1) end	
} // main() end

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

/* EXTI5~9 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
    //착돌 (EXTI5, JOY_PUSH(PI5))
	if(EXTI->PR & 0x0020)		    // EXTI5 발생
	{
		EXTI->PR |= 0x0020;			    // Pending bit Clear
        BEEP();								// 부저 1회
        if(Red_Turn == 1)    //적돌일때 착돌
        {
            if(red[Red_x][Red_y] == 1 || blue[Red_x][Red_y] == 1)// 이미 돌이 놓여있을때
            {
                DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else
            {
                EXTI->IMR  &= ~0x0100;		// EXTI8: Masked(적돌선택불가)
                EXTI->IMR  |= 0x8000;		// EXTI15: unMasked(청돌선택가능)
                LCD_SetBrushColor(RGB_RED);                // 사각형 배경색 설정
                LCD_DrawFillRect(22 + Red_x*10, 22 + Red_y*10, 7, 7);  		 // 사각형 내부 채우기
                Move_flag = 0;              // 돌 좌표 움직임 off
                red[Red_x][Red_y] = 1;      // 적돌이 착돌한 위치 저장
                SendPc[0] = 'R';            // 돌의 타입 ('R'은 적돌, 'B'는 청돌)
                SendPc[1] = Red_x+0x30;          // 돌의 x좌표
                SendPc[2] = Red_y+0x30;          // 돌의 y좌표
                SendPc[3] = ' ';            // 끝을 나타내는 구분 문자
                SerialSendString(SendPc);   // PC에 좌표값전송
            }
        }
        else if(Blue_Turn == 1)     //청돌일때 착돌
        {
            if(red[Blue_x][Blue_y] == 1 || blue[Blue_x][Blue_y] == 1) // 이미 돌이 놓여있을때
            {
                DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else
            {
                EXTI->IMR  &= ~0x8000;		// EXTI15: Masked(청돌선택불가)
                EXTI->IMR  |= 0x0100;		// EXTI8: unMasked(적돌선택가능)
                LCD_SetBrushColor(RGB_BLUE);                // 사각형 배경색 설정
                LCD_DrawFillRect(22 + Blue_x*10, 22 + Blue_y*10, 7, 7);  		 // 사각형 내부 채우기
                EXTI->IMR  &= ~0x0020;		// EXTI5: Masked (착돌불가)
                Move_flag = 0;              // 돌 좌표 움직임 off
                blue[Blue_x][Blue_y] = 1;   // 청돌이 착돌한 위치 저장
                SendPc[0] = 'B';            // 돌의 타입
                SendPc[1] = Blue_x+0x30;         // 돌의 x좌표
                SendPc[2] = Blue_y+0x30;         // 돌의 y좌표
                SendPc[3] = ' ';            // 끝을 나타내는 구분 문자
                SerialSendString(SendPc);   // PC에 좌표값전송
            }
        }
        Detect_Win();                   // 승패 판별 알고리즘
    }//if(EXTI->PR & 0x0020) end
	//적돌선택 (EXTI8,SW 0)
	else if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
        EXTI->IMR  |= 0x0020;		// EXTI5: unMasked(착돌 가능)
        EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (청돌선택불가)
        GPIOG->ODR |= 0x0001; 	    // LED0 ON
        GPIOG->ODR &= ~0x0080; 	    // LED7 OFF 
        Red_Turn = 1;               //적돌 Enable
        Blue_Turn = 0;
        Move_flag = 1;              // 돌 좌표 움직임 on
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayText(9, 0, "*");
		LCD_DisplayText(9, 18, " ");
    }
}

/* EXTI10~15 인터럽트 핸들러(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// 청돌선택 (EXTI15,SW 7)
	if(EXTI->PR & 0x8000)			// EXTI11 Interrupt Pending(발생) 여부?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
        EXTI->IMR  &= ~0x0100;		// EXTI8: Masked(적돌선택불가)
        EXTI->IMR  |= 0x0020;		// EXTI5: unMasked(착돌 가능)
        GPIOG->ODR &= ~0x0001; 	    // LED0 OFF
        GPIOG->ODR |= 0x0080; 	    // LED7 ON
		Red_Turn = 0;               //청돌 Enable
        Blue_Turn = 1;
        Move_flag = 1;              // 돌 좌표 움직임 on
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayText(9, 18, "*");
		LCD_DisplayText(9, 0, " ");
	}
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
    
	USART_BRR_Configuration(38400); // USART Baud rate Configuration: 38400
    
	USART1->CR1	|= (1<<12);			// USART_WordLength 9 Data bit

	USART1->CR1	|= (1<<10);			// USE USART_Parity
	USART1->CR1	|= (1<<9);	        // Odd_Parity config

	USART1->CR1	|= (1<<2);	// 0x0004, USART_Mode_RX Enable
	USART1->CR1	|= (1<<3);	// 0x0008, USART_Mode_Tx Enable

	USART1->CR2	&= ~(3<<12);	// 0b00, USART_StopBits_1
	USART1->CR3	= 0x0000;	// No HardwareFlowControl, No DMA
    
	USART1->CR1 	|= (1<<5);	// 0x0020, RXNE interrupt Enable
	NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37번)
	USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}

void USART1_IRQHandler(void)	
{
    if (USART1->SR & USART_SR_RXNE) // 데이터가 수신되었는지 확인
    {
        Rx = (uint8_t)(USART1->DR & 0x1FF); // 수신된 데이터 읽기
        Input[input_count++] = Rx;         // 데이터를 배열에 저장
        
    }
    // 좌표값을 모두 받아온 뒤 실행
    if(input_count == 3)
    {
        if(Input[0] == 'R')
        {
            Red_x = Input[1] - 0x30;        // x좌표값 저장
            Red_y = Input[2] - 0x30;        // y좌표값 저장
            if(red[Red_x][Red_y] == 1 || blue[Red_x][Red_y] == 1)
            {
                DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else if(Blue_Turn == 1)
            {
                DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else
            {
                BEEP();
                LCD_SetTextColor(RGB_RED);         
                LCD_DisplayChar(9, 2, Input[1]);    // x좌표값 표시
                LCD_DisplayChar(9, 4, Input[2]);    // y좌표값 표시
                LCD_DisplayText(9, 0, "*");
                LCD_DisplayText(9, 18, " ");
                LCD_SetBrushColor(RGB_RED);
                LCD_DrawFillRect(22 + Red_x*10, 22 + Red_y*10, 7, 7);  		 // 사각형 내부 채우기
                red[Red_x][Red_y] = 1;      // 적돌이 착돌한 위치 저장
                Red_Turn = 0;               // 청돌 Enable
                Blue_Turn = 1;
                GPIOG->ODR |= 0x0001; 	    // LED0 ON
                GPIOG->ODR &= ~0x0080; 	    // LED7 OFF
            }

        }
        else if(Input[0] == 'B')
        {
            Blue_x = Input[1] - 0x30;             // x좌표값 저장
            Blue_y = Input[2] - 0x30;             // y좌표값 저장
            if(red[Blue_x][Blue_y] == 1 || blue[Blue_x][Blue_y] == 1) // 이미 돌이 놓여있을때
            {
                //DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else if(Red_Turn == 1)
            {
                //DelayMS(1000);
                BEEP_3();                   // 1초간격 부저 3회
                SerialSendString(Warning);  // PC에 "Not running "전송
            }
            else
            {
                BEEP();
                LCD_SetTextColor(RGB_BLUE);         
                LCD_DisplayChar(9, 14, Input[1]);   // x좌표값 표시
                LCD_DisplayChar(9, 16, Input[2]);   // y좌표값 표시
                LCD_DisplayText(9, 18, "*");
                LCD_DisplayText(9, 0, " ");
                LCD_SetBrushColor(RGB_BLUE);                // 사각형 배경색 설정
                LCD_DrawFillRect(22 + Blue_x*10, 22 + Blue_y*10, 7, 7);  		 // 사각형 내부 채우기
                blue[Blue_x][Blue_y] = 1;   // 청돌이 착돌한 위치 저장
                Red_Turn = 1;               // 적돌 Enable
                Blue_Turn = 0;
                GPIOG->ODR &= ~0x0001; 	    // LED0 OFF
                GPIOG->ODR |= 0x0080; 	    // LED7 ON
            }
        }
        input_count = 0;                // input_count 초기화
        Detect_Win();                   // 승패 판별 알고리즘
    }
    
	// DR 을 읽으면 SR.RXNE bit(flag bit)는 clear 된다. 즉 clear 할 필요없음
}

void SerialSendChar(uint8_t Ch) // 1문자 보내기 함수
{
	// USART_SR_TXE(1<<7)=0?, TX Buffer NOT Empty? 
	// TX buffer Empty되지 않으면 계속 대기(송신 가능한 상태까지 대기)
    while((USART1->SR & USART_SR_TXE) == RESET); 
	USART1->DR = (Ch & 0x01FF);	// 전송 (최대 9bit 이므로 0x01FF과 masking)
}

void SerialSendString(char *str) // 여러문자 보내기 함수
{
	while (*str != '\0') // 종결문자가 나오기 전까지 구동, 종결문자가 나온후에도 구동시 메모리 오류 발생가능성 있음.
	{
		SerialSendChar(*str);	// 포인터가 가르키는 곳의 데이터를 송신
		str++; 			// 포인터 수치 증가
	}
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

//승패 판별 알고리즘
void Detect_Win(void)
{
    uint8_t Rx_cnt = 0;     //적돌x축 연속으로 놓였는지 확인하는 변수
    uint8_t Ry_cnt = 0;     //적돌y축 연속으로 놓였는지 확인하는 변수
    uint8_t Bx_cnt = 0;     //청돌x축 연속으로 놓였는지 확인하는 변수
    uint8_t By_cnt = 0;     //청돌y축 연속으로 놓였는지 확인하는 변수
    int i, j, k;
    for(i = 0; i < 10; i++)
    {
        for(j = 0; j < 10; j++)
        {
            if(red[j][i] == 1)  Rx_cnt += 1;         //x축에 적돌이 5개 놓였는지 확인
            else Rx_cnt = 0;
            if(Rx_cnt == 5)  
            {
                BEEP_5();
                if(Red_Score < 9)          //9다음은 0
                        Red_Score += 1;
                else if(Red_Score == 9)
                        Red_Score = 0;
                Fram_Write(300, Red_Score);
                System_Clear();
            }
            if(blue[j][i] == 1)  Bx_cnt += 1;         //x축에 청돌이 5개 놓였는지 확인
            else Bx_cnt = 0;
            if(Bx_cnt == 5) 
            {
                BEEP_5();
                if(Blue_Score < 9)          //9다음은 0
                        Blue_Score += 1;
                else if(Blue_Score == 9)
                        Blue_Score = 0;
                Fram_Write(301, Blue_Score);
                System_Clear();
            }
            if(red[i][j] == 1)  Ry_cnt += 1;         //y축에 적돌이 5개 놓였는지 확인
            else Ry_cnt = 0;
            if(Ry_cnt == 5) 
            {
                BEEP_5();
                if(Red_Score < 9)          //9다음은 0
                        Red_Score += 1;
                else if(Red_Score == 9)
                        Red_Score = 0;
                Fram_Write(300, Red_Score);
                System_Clear();
            }
            if(blue[i][j] == 1)  By_cnt += 1;         //y축에 청돌이 5개 놓였는지 확인
            else By_cnt = 0;
            if(By_cnt == 5) 
            {
                BEEP_5();
                if(Blue_Score < 9)          //9다음은 0
                        Blue_Score += 1;
                else if(Blue_Score == 9)
                        Blue_Score = 0;
                Fram_Write(301, Blue_Score);
                System_Clear();
            }
        }//for(j = 0; j < 10; j++)  end
    }//for(i = 0; i < 10; i++)  end
    //우하향대각선
    for(i = 0; i < 6; i++)
    {
        for(j = 0; j < 6; j++)
        {
            for(k = 0; k < 5; k++){
                if(red[i+k][j+k] == 1)  Rx_cnt += 1;         //적돌 우하향 대각선 5개 확인
                else Rx_cnt = 0;
                if(Rx_cnt == 5) 
                {
                    BEEP_5();
                    if(Red_Score < 9)          //9다음은 0
                        Red_Score += 1;
                    else if(Red_Score == 9)
                        Red_Score = 0;
                    Fram_Write(300, Red_Score);
                    System_Clear();
                }
                if(blue[i+k][j+k] == 1)  Bx_cnt += 1;         //청돌 우하향 대각선 5개 확인
                else Bx_cnt = 0;
                if(Bx_cnt == 5)  
                {
                    BEEP_5();
                    if(Blue_Score < 9)          //9다음은 0
                        Blue_Score += 1;
                    else if(Blue_Score == 9)
                        Blue_Score = 0;
                    Fram_Write(301, Blue_Score);
                    System_Clear();
                }
            }//for(k = 0; k < 5; k++) end
        }//for(j = 0; j < 6; j++)  end
    }//for(i = 0; i < 6; i++)  end
    //우상향대각선
    for(i = 0; i < 6; i++)
    {
        for(j = 4; j < 10; j++)
        {
            for(k = 0; k < 5; k++){
                if(red[i+k][j-k] == 1)  Rx_cnt += 1;         //적돌 우상향 대각선 5개 확인
                else Rx_cnt = 0;
                if(Rx_cnt == 5)  
                {
                    BEEP_5();
                    if(Red_Score < 9)          //9다음은 0
                        Red_Score += 1;
                    else if(Red_Score == 9)
                        Red_Score = 0;
                    Fram_Write(300, Red_Score);
                    System_Clear();
                }
                if(blue[i+k][j-k] == 1)  Bx_cnt += 1;         //청돌 우상향 대각선 5개 확인
                else Bx_cnt = 0;
                if(Bx_cnt == 5)  
                {
                    BEEP_5();
                    if(Blue_Score < 9)          //9다음은 0
                        Blue_Score += 1;
                    else if(Blue_Score == 9)
                        Blue_Score = 0;
                    Fram_Write(301, Blue_Score);
                    System_Clear();
                }
            }//for(k = 0; k < 5; k++) end
        }//for(j = 0; j < 6; j++)  end
    }//for(i = 0; i < 6; i++)  end
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
    Red_x = 5;          
    Red_y = 5;          
    Blue_x = 5;        
    Blue_y = 5;        
    Red_Turn = 0;
    Blue_Turn = 0;
    Move_flag = 0;
    //적돌 저장된 위치 초기화
    for (int i = 0; i < 10; i++) 
    {
        for (int j = 0; j < 10; j++)
        {
            red[i][j] = 0;
        }
    }
    ////청돌 저장된 위치 초기화
    for (int i = 0; i < 10; i++) 
    {
        for (int j = 0; j < 10; j++)
        {
            blue[i][j] = 0;
        }
    }
    GPIOG->ODR &= ~0x0081; 	    // LED0,7 OFF
    Fram_Init();                  // FRAM 초기화 H/W 초기화
	Fram_Status_Config(); 		  // FRAM 초기화 S/W 초기화
    Red_Score = Fram_Read(300);   // red팀 점수 Fram에서 불러오기
    Blue_Score = Fram_Read(301);  // Blue팀 점수 Fram에서 불러오기
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

void BEEP_5(void)
{ 	
    DelayMS(500);
    for(int i=0; i<5 ;i++)
    {
        GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
        DelayMS(30);			// Delay 30 ms
        GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
        DelayMS(1000);
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