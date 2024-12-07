/////////////////////////////////////////////////////////////
//  과제번호: 마이컴응용 2024 HW2
//  과제명: 디지털 시계 제작(Stopwatch 기능 포함)
//  과제개요: 과제에 대한 간단한 설명과 사용되는 Timer 사양 등을 기재
//  사용한 하드웨어(기능): GPIO(LED, Buzzer), EXTI(Switch), LCD
//  제출일: 2024.10.06
//  제출자 클래스: 목요일반
//  학번: 2020134041
//  이름: 허태규 
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

/* *****************************************************************************/
/*                                                                             */
/*                                  USER Define                                */
/*                                                                             */
/* *****************************************************************************/

/* *******************************   Buzzer  ***********************************/
#define BUZZER 0x0200   // 0000 0010 0000 0000

/* *********************************  LED  *************************************/
#define LED_OFF 0x0000  // 0000 0000 0000 0000
#define LED_0 0x0001    // 0000 0000 0000 0001
#define LED_1 0x0002    // 0000 0000 0000 0010
#define LED_2 0x0004    // 0000 0000 0000 0100
#define LED_3 0x0008    // 0000 0000 0000 1000
#define LED_4 0x0010    // 0000 0000 0001 0000
#define LED_5 0x0020    // 0000 0000 0010 0000
#define LED_6 0x0040    // 0000 0000 0100 0000
#define LED_7 0x0080    // 0000 0000 1000 0000

/* *********************************  SW  **************************************/
#define SW_OFF 0xFF00   // 1111 1111 0000 0000
#define SW_0 0xFE00     // 1111 1110 0000 0000
#define SW_1 0xFD00     // 1111 1101 0000 0000
#define SW_2 0xFB00     // 1111 1011 0000 0000
#define SW_3 0xF700     // 1111 0111 0000 0000
#define SW_4 0xEF00     // 1110 1111 0000 0000
#define SW_5 0xDF00     // 1101 1111 0000 0000
#define SW_6 0xBF00     // 1011 1111 0000 0000
#define SW_7 0x7F00     // 0111 1111 0000 0000

/* *********************************  JS  **************************************/
#define JS_OFF 0x03E0   // 0000 0011 1110 0000
#define JS_PUSH 0x03C0  // 0000 0011 1100 0000
#define JS_UP 0x03A0    // 0000 0011 1010 0000
#define JS_DOWN 0x0360  // 0000 0011 0110 0000
#define JS_RIGHT 0x02E0 // 0000 0010 1110 0000
#define JS_LEFT 0x01E0  // 0000 0001 1110 0000

/* *******************************  EXTI  **************************************/
#define EXIT_OFF 0x0000   // 0000 0000 0000 0000
#define EXIT_0 0x0001     // 0000 0000 0000 0001
#define EXIT_1 0x0002     // 0000 0000 0000 0010
#define EXIT_2 0x0004     // 0000 0000 0000 0100
#define EXIT_3 0x0008     // 0000 0000 0000 1000
#define EXIT_4 0x0010     // 0000 0000 0001 0000
#define EXIT_5 0x0020     // 0000 0000 0010 0000
#define EXIT_6 0x0040     // 0000 0000 0100 0000
#define EXIT_7 0x0080     // 0000 0000 1000 0000
#define EXIT_8 0x0100     // 0000 0001 0000 0000
#define EXIT_9 0x0200     // 0000 0010 0000 0000
#define EXIT_10 0x0400    // 0000 0100 0000 0000
#define EXIT_11 0x0800    // 0000 1000 0000 0000
#define EXIT_12 0x1000    // 0001 0000 0000 0000
#define EXIT_13 0x2000    // 0010 0000 0000 0000
#define EXIT_14 0x4000    // 0100 0000 0000 0000
#define EXIT_15 0x8000    // 1000 0000 0000 0000

/* *******************************  FRAM  **************************************/
#define WIN 300
#define LOSE 301

/* *****************************************************************************/
/*                                                                             */
/*                          USER Function Prototypes                           */
/*                                                                             */
/* *****************************************************************************/

void _GPIO_Init(void);          // GPIO_INIT
void _EXTI_Init(void);          // EXTI_INIT
void DisplayInitScreen(void);   // DisplayScreen_INIT

void TIMER2_OC_Init(void);          // TIMER2_INIT  (UP_Counter mode)   (TIME)
void TIMER10_UP_Init(void);         // TIMER10_INIT  (UP_Counter mode )  (TIME)

void Initial_value(void);    // 변수 초기화
void time_LCD(void);    // time 시간 LCD 표시
void stw_LCD(void);     // stw 시간 LCD 표시

uint16_t KEY_Scan(void);        // Key_Scan
uint16_t JOY_Scan(void);        // Joy_Scan         
void BEEP(void);                // Buzzer
void DelayMS(unsigned short wMS);       // Delay_Millisecond
void DelayUS(unsigned short wUS);       // Delay_Microsecond

/* *****************************************************************************/
/*                                                                             */
/*                               USER Variables                                */
/*                                                                             */
/* *****************************************************************************/

uint8_t stw_State;    //  stw 상태 변수 (0:보통상태, 1:작동상태, 2:중단상태)
uint16_t time_cnt;   // time 변수    (0~239)
char time_char[5];  // time 값 LCD 출력 변수
uint16_t stw_cnt;    // stw 변수     (0~999)
char stw_char[5];    // stw 값 LCD 출력 변수
uint8_t SW3_Flag, SW4_Flag;     // SW Flag

/* *****************************************************************************/
/*                                                                             */
/*                                     Main                                    */
/*                                                                             */
/* *****************************************************************************/

void main()
{
    LCD_Init();              // LCD initialization
    DelayMS(10);
    _GPIO_Init();            // GPIO initialization
    _EXTI_Init();            // EXTI initialization

    TIMER2_OC_Init();           // TIME2 initialization
    TIMER10_UP_Init();           // TIME10 initialization

    // Fram_Init();             // FRAM H/W initialization   
    // Fram_Status_Config();    // FRAM S/W initialization

    DisplayInitScreen();     // User Display initialization

    while (1)
    {
        if(SW3_Flag == 1)
        {
            // stw를 작동상태로 변경 
            if(stw_State == 0 || stw_State == 2)    // stw가 보통상태, 중단상태인 경우만 가능
            {
                stw_State = 1;          // 작동상태로 변경
                GPIOG->ODR |= LED_3;    // LED_3 ON
                GPIOG->ODR |= LED_4;    // LED_4 ON
            }
            // stw를 중단상태로 변경
            else    // stw가 작동상태인 경우만 가능
            {
                stw_State = 2;          // 중단상태로 변경
                GPIOG->ODR &= ~LED_3;   // LED_3 OFF
                GPIOG->ODR |= LED_4;    // LED_4 ON
            }

            SW3_Flag = 0;   // SW3_Flag 초기화
        }

        if(SW4_Flag == 1)
        {
            // stw를 보통상태로 변경
            if(stw_State == 2)  // stw가 중단상태인 경우만 가능
            {
                stw_State = 0;  // 보통상태로 변경
                stw_cnt = 0;    // stw 초기화
                GPIOG->ODR &= ~LED_3;    // LED_3 OFF
                GPIOG->ODR &= ~LED_4;   // LED_4 OFF
            }
            SW4_Flag = 0;   // SW4_Flag 초기화
        }
        
        time_LCD();
        stw_LCD();
        DelayMS(10);
    }
}

/* *****************************************************************************/
/*                                                                             */
/*                          INIT Function Definitions                          */
/*                                                                             */
/* *****************************************************************************/

void _GPIO_Init(void)
{
    /* *******************************  Example  **************************************/
    /*  1) RCC->AHB1ENR                                                               */
    /*  2-1) Output mode = GPIOx->MODER, OSPEEDR, OTYPER                              */
    /*  2-2)  Input mode = GPIOx->MODER, PUPDR                                        */
    /* ********************************************************************************/

    /* *******************************  BUZZER  ***************************************/
    RCC->AHB1ENR |= 0x00000020;     // GPIOF Enable

    GPIOF->MODER &= ~0x000C0000;    // GPIOF->MODER PF9 Clear
    GPIOF->MODER |= 0x00040000;     // GPIOF->MODER PF9 output mode

    GPIOF->OSPEEDR &= ~0x000C0000;  // GPIOF->OSPEEDR PF9 Clear
    GPIOF->OSPEEDR |= 0x00040000;   // GPIOF->OSPEEDR PF9 25MHZ Medium speed 

    GPIOF->OTYPER &= ~0x0020;       // GPIOF->OTYPER PF9 Clear(Push-Pull)

    /* *******************************  LED  ******************************************/
    RCC->AHB1ENR |= 0x00000040;     // GPIOG Enable

    GPIOG->MODER &= ~0x000003C0;    // GPIOG->MODER PG3,4 Clear
    GPIOG->MODER |= 0x00000140;     // GPIOG->MODER PG3,4 output mode

    GPIOG->OSPEEDR &= ~0x000003C0;  // GPIOG->OSPEEDR PF3,4 Clear
    GPIOG->OSPEEDR |= 0x00000140;   // GPIOG->OSPEEDR PF3,4 25MHZ Medium speed 

    GPIOG->OTYPER &= ~0x0018;       // GPIOF->OTYPER PF3,4 Clear(Push-Pull)

    /* *******************************  SW  *******************************************/
    // RCC->AHB1ENR |= 0x00000080;     // GPIOH Enable

    // GPIOH->MODER &= ~0x03C00000;    // GPIOH->MODER PH8~15 Clear(input mode)

    // GPIOH->PUPDR &= ~0x03C00000;    // GPIOH->PUPDR PH8~15 Clear(Floating input)

    /* *******************************  JS  *******************************************/
    // RCC->AHB1ENR |= 0x00000100;     // GPIOI Enable

    // GPIOI->MODER &= ~0x000FFC00;    // GPIOI->MODER PI5~9 Clear(input mode)

    // GPIOI->PUPDR &= ~0x000FFC00;    // GPIOI->MODER PI5~9 Clear(Floating input)
}

void _EXTI_Init(void)
{
    /* *******************************  Example  **************************************/
    /*  1) RCC->APB2ENR                                                               */
    /*  2) RCC->AHB1ENR                                                               */
    /*  3) Input mode = GPIOx->MODER, PUPDR                                           */
    /*  4) SYSCFG->EXTICR[x] (x: 0~3)                                                 */
    /*  5) EXTI->FTSR or RTSR                                                         */
    /*  6) EXTI->IMR                                                                  */
    /*  7) NVIC->ISER[x] (x: 0~3)                                                     */
    /* ********************************************************************************/ 

    RCC->APB2ENR |= 0x00004000;     // System Configuration Controller Clock Enable

    /* *******************************  SW  *******************************************/
    RCC->AHB1ENR |= 0x00000080;     // GPIOH Enable

    GPIOH->MODER &= ~0x03C00000;    // GPIOH->MODER PH11,12 Clear(input mode)
    GPIOH->PUPDR &= ~0x03C00000;    // GPIOH->PUPDR PH11,12 Clear(Floating input)

    // GPIO -> EXTI
    SYSCFG->EXTICR[2] &= ~0xF000;   // SYSCFG->EXTICR EXTI3 Clear
    SYSCFG->EXTICR[2] |= 0x7000;    // SYSCFG->EXTICR EXTI3
    SYSCFG->EXTICR[3] &= ~0x000F;   // SYSCFG->EXTICR EXTI4 Clear
    SYSCFG->EXTICR[3] |= 0x0007;    // SYSCFG->EXTICR EXTI4

    // Trigger Falling, Rising
    EXTI->FTSR &= ~0x1800;          // EXTI->FTSR EXTI3,4 Clear
    EXTI->FTSR |= 0x1800;           // EXTI->FTSR EXTI3,4 Falling Trigger Enable 

    // Making
    EXTI->IMR &= ~0x1800;           // EXTI->IMR EXTI3,4 Clear
    EXTI->IMR |= 0x1800;            // EXTI->IMR EXTI3,4 Not Masked

    NVIC->ISER[1] |= (1<<(40-32));    // NVIC->ISER[0] Global Interrupt EXTI11,12 Enable

    /* *******************************  JS  *******************************************/
    // RCC->AHB1ENR |= 0x00000100;     // GPIOI Enable

    // GPIOI->MODER &= ~0x000FFC00;    // GPIOI->MODER PF5~9 Clear(input mode)
    // GPIOI->PUPDR &= ~0x000FFC00;    // GPIOI->MODER PF5~9 Clear(Floating input)

    // SYSCFG->EXTICR[1] &= ~0xF000;   // SYSCFG->EXTICR[1] EXTI7 Clear
    // SYSCFG->EXTICR[1] |= 0x8000;   // SYSCFG->EXTICR[1] EXTI7

    // EXTI->FTSR &= ~0x0080;          // EXTI->FTSR EXTI7 Clear
    // EXTI->FTSR |= 0x0080;           // EXTI->FTSR EXTI7 Falling Trigger Enable

    // EXTI->IMR &= ~0x0080;           // EXTI->IMR EXTI7 Clear
    // EXTI->IMR |= 0x0080;            // EXTI->IMR EXTI7 Not Masked

    // NVIC->ISER[0] |= 0x00800000;    // NVIC->ISER[0] Global Interrupt EXTI7 Enable */
}

void DisplayInitScreen(void)
{
    /* *******************************  Example  **************************************/
    /*  1)  Clear = LCD_Clear(RGB_Color)                                              */
    /* *******************************  Text  *****************************************/
    /*  1)  Font = LCD_SetFont(&Gulim8);                                              */
    /*  2)  BackColor = LCD_SetBackColor(RGB_Color);                                  */
    /*  3)  TextColor = LCD_SetTextColor(RGB_Color);                                  */
    /*  4)  Text = LCD_DisplayText(Col_x, Row_y, "array");                            */
    /*      Text = LCD_DisplayChar(Col_x, Row_y, 'char');                             */
    /* *******************************  Graphic  **************************************/
    /*  1)  PenColor = LCD_SetPenColor(RGB_Color);                                    */
    /*  2)  Crossline = LCD_DrawLine(x1,y1,x2,y2);                                    */
    /*        HorLine = LCD_DrawHorLine(x,y,width);                                   */
    /*        VerLine = LCD_DrawVerLine(x,y,height);                                  */
    /*      Rectangle = LCD_DrawRectangle(x,y,width,height);                          */
    /*           Text = LCD_DrawText(x,y,array);                                      */
    /*  3) BrushColor = LCD_SetBrushColor(RGB_Color);                                 */
    /*     FillRectangle = LCD_DrawFillRect(x,y,width,height);                        */
    /* ********************************************************************************/ 

    LCD_Clear(RGB_YELLOW);  // 전체 배경 색
    LCD_SetFont(&Gulim8);   // 글자 크기
    LCD_SetBackColor(RGB_YELLOW);   // Text 뒷 배경 색

    // 제목 칸 LCD 표시
    LCD_SetPenColor(RGB_BLACK);     // 제목 칸 색
    LCD_SetTextColor(RGB_BLACK);     // 칸 색
    LCD_DrawRectangle(5,5,149,40);  // 크기

    // 제목 LCD 표시
    LCD_SetTextColor(RGB_BLUE);      // 제목 색
    LCD_DrawText(10,10,"Digital Watch");  // 제목

    // 학번,이름 LCD 표시
    LCD_SetTextColor(RGB_GREEN);      // 학번,이름 색
    LCD_DrawText(10,25,"2020134041 HTG");  // 학번,이름

    // 변수 초기화
    Initial_value();    // 변수 초기화

    // TIME, STW 이름 LCD 표시
    LCD_SetTextColor(RGB_BLUE);      // TIME, STW 이름 색
    LCD_DrawText(10,50,"TIME");  // TIME
    LCD_DrawText(10,65,"STW");  // STW

    // TIME, STW 시간 LCD 표시
    time_LCD();
    stw_LCD();

    // LED all off
    GPIOG->ODR &= LED_OFF;  
}

void TIMER2_OC_Init(void)
{
    /* *******************************  Example  **************************************/
    /*  2. Output Compare mode (TIME4_CH1: PD12)                                      */
    /*    1) RCC->AHB1ENR                                                             */
    /*    2) Output mode = GPIOx->MODER, OSPEEDR, OTYPER, PUPDR                       */
    /*    3) GPIOx->AFR[y] (y: 0~1)                                                   */
    /*                                                                                */
    /*    1) RCC->APB1ENR (Enable Timer CLK)                                          */
    /*    2-1) TIMx->CR1 1_bit (UDIS: Update disable)                                 */
    /*    2-2) TIMx->CR1 2_bit (URS: Update request source)                           */
    /*    2-3) TIMx->CR1 3_bit (OPM: One-pulse mode)                                  */
    /*    2-4) TIMx->CR1 4_bit (DIR: Direction)                                       */
    /*    2-5) TIMx->CR1 5,6_bit (CMS: Center-aligned mode selection)                 */
    /*    2-6) TIMx->CR1 7_bit (ARPE: Auto-reload preload enable)                     */
    /*    2-7) TIMx->CR1 8,9_bit (CKD: Clock division)                                */
    /*                                                                                */
    /*    3) TIMx->PSC=a-1 Prescaler 84MHz/a=bHz, c(sec) (a: 1~65536)                 */
    /*    4) TIMx->ARR=d-1 Auto reload c*d=e(sec)                                     */
    /*                                                                                */
    /*    5) TIMx->EGR 0_bit (UG: Update generation)                                  */
    /*    6) TIMx->SR 0_bit (UIF: Update Interrupt Flag)                              */
    /*                                                                                */
    /*    x=1,2 -> y=1      x=3,4 -> y=2                                              */
    /*    7) TIMx->CCMRy 0_bit (CC1S: Capture/Compare 1 selection)                    */
    /*    8) TIMx->CCMRy 2_bit (OC1FE: Output compare 1 fast enable)                  */
    /*    9) TIMx->CCMRy 3_bit (OC1PE: Output compare 1 preload enable)               */
    /*    10) TIMx->CCMRy 4_bit (OC1M: Output compare 1 mode)                         */
    /*                                                                                */
    /*    11) TIM4->CCER 0_bit (CC1E: Capture/Compare 1 output enable)                */
    /*    12) TIM4->CCER 1_bit (CC1P: Capture/Compare 1 output Polarity)              */
    /*                                                                                */
    /*    13) TIM4->CCR1=g (g/e*100)= h%                                              */
    /*                                                                                */
    /*    14) TIMx->DIER 0_bit (UIE: Update interrupt enable)                         */
    /*    15) TIMx->DIER 1_bit (CC1IE: Capture/Compare 1 interrupt enable)            */
    /*    16) NVIC->ISER[x] (x: 0~3)                                                  */
    /*    17) TIMx->CR1 0_bit (CEN: Counter Enable)                                   */
    /* ********************************************************************************/

	RCC->AHB1ENR	|= (1<<1);	// GPIOB Enable

    GPIOG->MODER &= ~(3<<20);    // GPIOB->MODER PB10 Clear
    GPIOG->MODER |= (2<<20);     // GPIOB->MODER PB10 Alternate function mode
    GPIOG->OSPEEDR &= ~(3<<20);  // GPIOG->OSPEEDR PB10 Clear
    GPIOG->OSPEEDR |= (3<<20);   // GPIOG->OSPEEDR PB10 25MHZ Medium speed 
    GPIOG->OTYPER &= ~(1<<10);       // GPIOF->OTYPER PFB10 Clear(Push-Pull)
    GPIOG->AFR[0] |= (10<<4);    // PB10 AF1

    // Enable Timer CLK 
	RCC->APB1ENR |= (1<<0);	// RCC_APB1ENR TIMER2 Enable

    // Setting CR1: 0x0000 
    TIM2->CR1 &= ~(1<<1);   // UDIS=0 (UEV enable)
    TIM2->CR1 &= ~(1<<2);   // URS=0 (Update request source selection)  
    TIM2->CR1 &= ~(1<<3);   // OPM=0 (One-pulse mode)
    TIM2->CR1 &= ~(1<<4);   // DIR=0 (Up counter)
    TIM2->CR1 &= ~(3<<5);   // CMS=00 (edge aligned)
    TIM2->CR1 &= ~(1<<7);   // ARPE=0 (ARR in not buffered)
    TIM2->CR1 &= ~(3<<8);   // CKD=00 (No division)

	TIM2->PSC = 16800-1; 	// Prescaler 84,000,000 Hz/16800 = 5,000 Hz (1~65536)
	TIM2->ARR = 500-1;	// Auto reload  5,000 / 500 = 10 Hz (0.1s)

    // Setting CCMR2: 0x30
    TIM2->CCMR2 &= ~(3<<0);     // CC3S=00 (CC3 Configured as Output)
    TIM2->CCMR2 &= ~(1<<2);     // OC3FE=0 (Output compare 3 fast disable)
    TIM2->CCMR2 &= ~(1<<3);     // OC3PE=0 (Output compare 3 preload)
    TIM2->CCMR2 &= ~(3<<4);     // OC3M=011 (Output compare 3 mode: toggle)
    
    TIM2->CCER &= ~(1<<8);       // CC3E=0 (CC1 channel output disable)
    TIM2->CCER &= ~(1<<9);      // CC3P=0 (CC1 channel output Polarity: 반전없이 출력)

    TIM2->CCR3 = 500 ;     // 500/500*100=100% (ARR의 100%일 때 인터럽트 발생)

    TIM2->EGR |= (1<<0);	// UG(Update generation)=1      UEV 강제 발생 -> Counter 초기화
    TIM2->SR &= ~(1<<0);	// UIF(Interrupt Flag)=0        UG 설정 시 불필요한 인터럽트 초기화

    TIM2->DIER |= (1<<0);	// Enable the Tim2 Update interrupt     UEV 활성화
    TIM2->DIER |= (1<<3);	// Tim2 CC3 interrupt enable
    NVIC->ISER[0] |= (1<<28); 	// Enable Timer2 global Interrupt   NVIC에서 UEV 활성화

	TIM2->CR1 |= (1<<0);	// Enable the Tim2 Counter (clock enable)
}

void TIMER10_UP_Init(void)
{
    /* *******************************  Example  **************************************/
    /*  1. UP/DOWN Counting mode                                                      */
    /*    1) RCC->APB2ENR (Enable Timer CLK)                                          */
    /*                                                                                */
    /*    2-2) TIMx->CR1 1_bit (UDIS: Update disable)                                 */
    /*    2-3) TIMx->CR1 2_bit (URS: Update request source)                           */
    /*    2-4) TIMx->CR1 3_bit (OPM: One-pulse mode)                                  */
    /*    2-5) TIMx->CR1 4_bit (DIR: Direction)                                       */
    /*    2-6) TIMx->CR1 5,6_bit (CMS: Center-aligned mode selection)                 */
    /*    2-7) TIMx->CR1 7_bit (ARPE: Auto-reload preload enable)                     */
    /*    2-8) TIMx->CR1 8,9_bit (CKD: Clock division)                                */
    /*                                                                                */
    /*    3) TIMx->PSC=a-1 Prescaler 84MHz/a=bHz, c(sec) (a: 1~65536)                 */
    /*    4) TIMx->ARR=d-1 Auto reload c*d=e(sec)                                     */
    /*    5) TIMx->EGR 0_bit (UG: Update generation)                                  */
    /*    6) TIMx->SR 0_bit (UIF: Update Interrupt Flag)                              */
    /*    7) TIMx->DIER 0_bit (UIE: Update interrupt enable)                          */
    /*    8) NVIC->ISER[x] (x: 0~3)                                                   */
    /*    9) TIMx->CR1 0_bit (CEN: Counter Enable)                                    */
    /* ********************************************************************************/

    // Enable Timer CLK 
	RCC->APB2ENR |= (1<<17);	// RCC_APB2ENR TIMER10 Enable

    // Setting CR1 : 0x0000 
    TIM10->CR1 &= ~(1<<1);   // UDIS=0 (UEV enable)
    TIM10->CR1 &= ~(1<<2);   // URS=0 (Update request source selection)  
    TIM10->CR1 &= ~(1<<3);   // OPM=0 (One-pulse mode)
    TIM10->CR1 &= ~(1<<4);   // DIR=0 (Up counter)
    TIM10->CR1 &= ~(3<<5);   // CMS=00 (edge aligned)
    TIM10->CR1 &= ~(1<<7);   // ARPE=0 (ARR in not buffered)
    TIM10->CR1 &= ~(3<<8);   // CKD=00 (No division)

	TIM10->PSC = 840-1; 	// Prescaler 168,000,000 Hz/840 = 200,000 Hz (1~65536)
	TIM10->ARR = 20000-1;	// Auto reload  200,000 / 20,000 = 10 Hz (0.1s)

    TIM10->EGR |= (1<<0);	// UG(Update generation)=1      UEV 강제 발생 -> Counter 초기화
    TIM10->SR &= ~(1<<0);	// UIF(Interrupt Flag)=0        UG 설정 시 불필요한 인터럽트 초기화

    TIM10->DIER |= (1<<0);	// Enable the Tim10 Update interrupt     UEV 활성화
    NVIC->ISER[0] |= (1<<25); 	// Enable Timer10 global Interrupt   NVIC에서 UEV 활성화

	TIM10->CR1 |= (1<<0);	// Enable the Tim10 Counter (clock enable)
}

/* *****************************************************************************/
/*                                                                             */
/*                          USER Function Definitions                          */
/*                                                                             */
/* *****************************************************************************/

uint8_t key_flag = 0;   
uint16_t KEY_Scan(void)
{
    uint16_t key;
    key = GPIOH->IDR & 0xFF00;	// any key pressed ?
    if (key == 0xFF00)	    // if no key, check key off	
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
    else	                // if key input, check continuous key	                
    {
        if (key_flag != 0)	// if continuous key, treat as no key input
            return 0xFF00;  
        else			    // if new key,delay for debounce
        {
            key_flag = 1;   
            DelayMS(10);
            return key;     
        }
    }
}

uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	
{ 
	uint16_t key;
	key = GPIOI->IDR & 0x03E0;	// any key pressed ?
	if(key == 0x03E0)		// if no key, check key off
	{	
        if(joy_flag == 0)   
			return key;
		else
		{	DelayMS(10);
			joy_flag = 0;
			return key;
		}
	}
	else				    // if key input, check continuous key
	{	
        if(joy_flag != 0)	// if continuous key, treat as no key input
			return 0x03E0;
		else			    // if new key,delay for debounce
		{	joy_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}

void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		    // Delay 30 ms
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

void Initial_value(void)
{
    time_cnt = 200;   // 20:0 초기화
    stw_cnt = 0;    // 00:0 초기화
    stw_State = 0;  // 보통 상태 초기화
    SW3_Flag = 0;   // SW3 flag 초기화
    SW4_Flag = 0;   // SW4 flag 초기화
}

void time_LCD(void)
{
    LCD_SetBackColor(RGB_YELLOW);   // Text 뒷 배경 색
    LCD_SetTextColor(RGB_BLACK);      // TIME 색

    time_char[0] = time_cnt/100+0x30;       // 백의 자리
    time_char[1] = (time_cnt%100/10)+0x30;  // 십의 자리
    time_char[2] = ':'; // 시, 초 구분
    time_char[3] = (time_cnt%10)+0x30;      // 일의 자리

    LCD_DrawText(50,50, time_char);  // TIME 
}

void stw_LCD(void)
{
    LCD_SetBackColor(RGB_YELLOW);   // Text 뒷 배경 색
    LCD_SetPenColor(RGB_BLACK);      // STW 색

    stw_char[0] = stw_cnt/100+0x30;       // 백의 자리
    stw_char[1] = (stw_cnt%100/10)+0x30;  // 십의 자리
    stw_char[2] = ':'; // 시, 초 구분
    stw_char[3] = (stw_cnt%10)+0x30;      // 일의 자리

    LCD_DrawText(50, 65, stw_char);  // STW
}

/* *****************************************************************************/
/*                                                                             */
/*                              Handler                                        */
/*                                                                             */
/* *****************************************************************************/

//void EXTI0_IRQHandler(void)   /* EXTI0 (ISR: Interrupt Service Routine) */
//void EXTI1_IRQHandler(void)   /* EXTI1 (ISR: Interrupt Service Routine) */
//void EXTI2_IRQHandler(void)   /* EXTI2 (ISR: Interrupt Service Routine) */
//void EXTI3_IRQHandler(void)   /* EXTI3 (ISR: Interrupt Service Routine) */
//void EXTI4_IRQHandler(void)   /* EXTI4 (ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)   /* EXTI5~9 (ISR: Interrupt Service Routine) */
{
    // if (EXTI->PR & EXIT_5) // EXTI5 
    // {
    //     EXTI->PR |= EXIT_5; // EXTI->PR 1->0 Clear          
    // }
    // else if (EXTI->PR & EXIT_6) // EXTI6
    // {
    //     EXTI->PR |= EXIT_6;    
    // }
    // else if (EXTI->PR & EXIT_7) // EXTI7 
    // {
    //     EXTI->PR |= EXIT_7;    
    // }
    // else if (EXTI->PR & EXIT_8) // EXTI8 
    // {
    //     EXTI->PR |= EXIT_8;    
    // }
    // else if (EXTI->PR & EXIT_9) // EXTI9 
    // {
    //     EXTI->PR |= EXIT_9;    
    // }
}

void EXTI15_10_IRQHandler(void)   /* EXTI10~15 (ISR: Interrupt Service Routine) */
{
    // if (EXTI->PR & EXIT_10) // EXTI10 
    // {
    //     EXTI->PR |= EXIT_10; // EXTI->PR 1->0 Clear          
    // }
    if (EXTI->PR & EXIT_11) // EXTI11
    {
        EXTI->PR |= EXIT_11;
        SW3_Flag = 1;   // SW3_Flag 발생
    }
    else if (EXTI->PR & EXIT_12) // EXTI12 
    {
        EXTI->PR |= EXIT_12;
        SW4_Flag = 1;   // SW4_Flag 발생
    }
    // else if (EXTI->PR & EXIT_13) // EXTI13 
    // {
    //     EXTI->PR |= EXIT_13;    
    // }
    // else if (EXTI->PR & EXIT_14) // EXTI14
    // {
    //     EXTI->PR |= EXIT_14;    
    // }
    // else if (EXTI->PR & EXIT_15) // EXTI15
    // {
    //     EXTI->PR |= EXIT_15;    
    // }
}

void TIM2_IRQHandler(void)  	// 0.1s Interrupt
{
    if(TIM2->SR & TIM_SR_CC3IF)     // Capture Compare Interrupt Flag 발생
    {
        TIM2->SR &= ~(1<<3);	// CC3 Interrupt flag Clear

        if(stw_State == 1)
        {
            if(stw_cnt >= 999) stw_cnt=0;   // 최대 999
            else stw_cnt++; // cnt 증가
        }
    }

    if(TIM2->SR & TIM_SR_UIF)  // Update Interrupt Flag 발생
    {
        TIM2->SR &= ~(1<<0);	// Update Interrupt flag Clear
    }
}

void TIM1_UP_TIM10_IRQHandler(void)  	// 0.1s Interrupt
{
    if (TIM10->SR & TIM_SR_UIF)     // Update Interrupt Flag 발생
    {
        TIM10->SR &= ~(1<<0);	// Interrupt flag Clear

        if(time_cnt >= 239) time_cnt=0; // 최대 239
        else time_cnt++;    // cnt 증가
    }
}