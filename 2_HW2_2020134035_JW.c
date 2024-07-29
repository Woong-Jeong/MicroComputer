/////////////////////////////////////////////////////////////
// 과제명: HW2. 엘리베이터
// 과제개요: STM32F407 마이크로컨트롤러를 사용하여 스위치와 조이스틱 입력을 통해 
//           엘리베이터를 시뮬레이션하고, LED와 부저를 이용해 
//           현재 층과 상태를 표시하는 시스템을 구현
// 사용한 하드웨어(기능): GPIO, Switch, Joy-stick, LED, Buzzer
// 제출일: 2024. 05. 18. 
// 제출자 클래스: 수요일반
// 학번: 2020134035
// 이름: 정 웅
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"

//Switch 및 JoyStick 의 주소값을 변수로 선언
#define SW0        0xFE00  //PH8
#define SW1        0xFD00  //PH9
#define SW2        0xFB00  //PH10
#define SW3        0xF700  //PH11
#define SW4        0xEF00  //PH12
#define SW5        0xDF00  //PH13
#define SW6        0xBF00  //PH14
#define SW7        0x7F00  //PH15

#define JOY_RIGHT	0x02E0	//PI8 0000 0010 1110 0000 
#define JOY_LEFT	0x01E0	//PI9 0000 0001 1110 0000

void _GPIO_Init(void);             // GPIO 초기화 함수
uint16_t KEY_Scan(void);           // 스위치 입력을 스캔하는 함수
uint16_t JOY_Scan(void);           // 조이스틱 입력을 스캔하는 함수
void SW_Move(int Input, int Now);  // 층 이동 함수
void BEEP(void);                   // 부저 함수
void DelayMS(unsigned short wMS);  // 밀리초 단위 지연 함수
void DelayUS(unsigned short wUS);  // 마이크로초 단위 지연 함수

int main(void)
{
	_GPIO_Init();               //GPIO핀 초기화
	GPIOG->ODR |= ~0x00FF;	    // 초기값: LED0~7 Off
	GPIOG->ODR |= 0x0001; 	    // GPIOG->ODR.1 'H'(LED0 ON)
        
        int Now_flag = 0;           //현재 층의 값을 저장하는 변수선언(초기값: 0층)
        int Input_flag = 0;        //스위치 입력층을 저장하는 변수선언(초기값: 0층) 
        BEEP();                    //엘리베이터가 준비됐음을 알리는 부저
        
	while(1)
	{
                //Switch Input
		switch(KEY_Scan())
		{
			case SW0  : 	                    // SW입력시
				BEEP();                         // 부저 울림
                                Input_flag = 0;                 // 입력층 설정
                                SW_Move(Input_flag, Now_flag);  // 층 이동
                                Now_flag = 0;                   // 현재 층 업데이트
                        break;                                           
			case SW1  : 	
				BEEP();
                                Input_flag = 1;
                                SW_Move(Input_flag, Now_flag);                                                               
                                Now_flag = 1;
                        break;
                        case SW2  : 	
				BEEP();
                                Input_flag = 2;
                                SW_Move(Input_flag, Now_flag);                                                               
                                Now_flag = 2;
                        break;
                        case SW3  : 	
				BEEP();
                                Input_flag = 3;
                                SW_Move(Input_flag, Now_flag);                                                              
                                Now_flag = 3;
                        break;     
                        case SW4  : 	
				BEEP();
                                Input_flag = 4;
                                SW_Move(Input_flag, Now_flag);                                                              
                                Now_flag = 4;
                        break;
                        case SW5  : 	
				BEEP();
                                Input_flag = 5;
                                SW_Move(Input_flag, Now_flag);                                                               
                                Now_flag = 5;
                        break;
                        case SW6  : 	
				BEEP();
                                Input_flag = 6;
                                SW_Move(Input_flag, Now_flag);                                                              
                                Now_flag = 6;
                        break;        
                        case SW7  : 	
				BEEP();
                                Input_flag = 7;
                                SW_Move(Input_flag, Now_flag);                                                               
                                Now_flag = 7;
                        break;
		}
                //JoyStick Input
                switch(JOY_Scan())
		{
			case JOY_LEFT:	              //왼쪽 JoyStick 입력시
			        BEEP();                       //부저 1회 실행  
                                if(Now_flag > 0)              //현재층이 1~7층일때
                                {
                                        DelayMS(500);             //0.5초후에
                                        Input_flag -= 1;          //입력변수 -1
                                        GPIOG->ODR >>= 1;          //LED를 왼쪽으로 이동
                                        Now_flag = Input_flag;    //현재층 업데이트
                                }
                                else                          //현재층이 0층일때  
                                {
                                        DelayMS(200);             //0.2초후 부저1번 작동  
                                        BEEP();                           
                                }     
			break;
			case JOY_RIGHT:	              //오른쪽 JoyStick 입력시
				BEEP();                   //부저 1회 실행
                                if(Now_flag < 7)          //현재층이 0~6층일때
                                {       
                                        DelayMS(500);            //0.5초후에
                                        Input_flag += 1;         //입력변수 +1
                                        GPIOG->ODR <<= 1;        //LED를 오른쪽으로 이동
                                        Now_flag = Input_flag;   //현재층 업데이트
                                }
                                else                      //현재층이 7층일때
                                {
                                        DelayMS(200);            //0.2초후 부저1번 작동  
                                        BEEP();
                                }                               
			break;             
		}  
	}
}
//Switch를 눌렀을때 층을 이동하게 하는 함수
//엘리베이터의 현재 층을 나타내는 변수 Now_flag와
//입력받은 층을 나타내는 변수 Input_flag를 활용
void SW_Move(int Input, int Now)
{                
        if(Input == Now)        //현재층과 같은 SW입력시 0.2초후 부저 1회
        {
                DelayMS(200);
                BEEP();
        }       
        else if(Input > Now)    //SW입력이 현재 층수보다 높을때 실행
        {
                DelayMS(500);    //초기상태에서 0.5초후에 LED 이동
                //SW입력과 현재 층수가 같아질때까지 반복
                while(Input != Now)
                {
                        GPIOG->ODR <<= 1;    //LED를 오른쪽으로 이동               
                        Now += 1;            //현재층수 +1
                        DelayMS(500);        //간격: 0.5초                   
                }
                //while문을 벗어나면 도착한것이므로 0.5초 간격으로 부저 3회
                for(int j = 0; j < 3; j++)    
                {
                        BEEP();
                        DelayMS(500);
                }        
        }
        else if(Input < Now)    //SW입력이 현재 층수보다 낮을때 실행
        {     
                DelayMS(500);     //초기상태에서 0.5초후에 LED 이동
                //SW입력과 현재 층수가 같아질때까지 반복
                while(Input != Now)
                {
                        GPIOG->ODR >>= 1;    //LED를 오른쪽으로 이동
                        Now -= 1;            //현재층수 -1       
                        DelayMS(500);        //간격: 0.5초           
                }
                //while문을 벗어나면 도착한것이므로 0.5초 간격으로 부저 3회
                for(int j = 0; j < 3; j++)
                {
                        BEEP();
                        DelayMS(500);
                }             
        }
}

/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
//key_flag -> SW를 한번 눌렀는지 인지하기 위한 변수
uint8_t key_flag = 0;           
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
        // SW의 핀은 PH8~15이므로 PH0~7을 0으로 초기화
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{	if(key_flag == 0)
			return key;
		else
		{	DelayMS(10);
			key_flag = 0;
			return key;
		}
	}
        //else로 오면 키가 눌린것
	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
			return 0xFF00;
		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);    //혹시 모를 노이즈방지                      
			return key;
		}
	}
}

//JoyStick이 입력되었는지를 여부와 어떤 JoyStick이 입력되었는지의 정보를 return하는 함수
//joy_flag -> JoyStick을 한번 눌렀는지 인지하기 위한 변수
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick
{ 
        uint16_t key;
	key = GPIOI->IDR & 0x03E0;	// any key pressed ?
	if(key == 0x03E0)		// if no key, check key off
	{	if(joy_flag == 0)
			return key;
		else
		{	DelayMS(10);
			joy_flag = 0;
			return key;
		}
	}
	else				// if key input, check continuous key
	{	if(joy_flag != 0)	// if continuous key, treat as no key input
			return 0x03E0;
		else			// if new key,delay for debounce
		{	joy_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(JoyStick) 초기 설정 */
void _GPIO_Init(void)
{
	// LED (GPIO G) 설정 : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 

	// SW (GPIO H) 설정 : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 : Output mode 
	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
        GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
        
        //Joy Stick SW(PORT I) 설정
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000F0000;	// GPIOI 8,9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000F0000;	// GPIOI 8,9 : Floating input (No Pull-up, pull-down) (reset state)
}	

void BEEP(void)			/* beep for 30 ms */
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