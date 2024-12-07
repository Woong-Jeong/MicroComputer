/////////////////////////////////////////////////////////////
// ������: HW4. �������(USART �̿�)
// ��������: STM32F407 ����ũ����Ʈ�ѷ��� �̿��� ��������� ����
// ����� �ϵ����(���): GPIO, Switch, Joy-stick, LED, Buzzer,
//                        GLCD, EXTI, FRAM, USART, PC
// ������: 2024. 11. 28. 
// ������ Ŭ����: �����Ϲ�
// �й�: 2020134035
// �̸�: �� ��
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

void _GPIO_Init(void);			  // GPIO �ʱ�ȭ �Լ�  
void _EXTI_Init(void);			  // ���ͷ�Ʈ �ʱ�ȭ �Լ�		
void DisplayInitScreen(void);	  // GLCD �ʱ�ȭ �Լ�
void System_Clear(void);		  // GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
uint16_t KEY_Scan(void);		  // SW�Է� �޴� �Լ�
void Detect_Win(void);            // ���� ���� ���� �Լ�
uint16_t JOY_Scan(void);		  // JOYSTICK �Է� �޴� �Լ�
void BEEP(void);			   	  // 30ms���� ������ �︮�� �Լ�
void BEEP_3(void);                // 0.5�ʰ��� ���� 3ȸ
void BEEP_5(void);                // 1�ʰ��� ���� 5ȸ
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

void USART1_Init(void);             // USART1 ����
void USART_BRR_Configuration(uint32_t USART_BaudRate);  //USART Baudrate ����
void SerialSendChar(uint8_t Ch);    // 1���� ������ �Լ�
void SerialSendString(char *str);   // �������� ������ �Լ�

uint8_t Warning[20] = {'N','o','t',' ','r','u','n','n','i','n','g',' '};
uint8_t Input[5];           // PC�κ��� ���� ��ǥ������ �����ϴ� �迭
int input_count = 0;        // ���� ���� ������ count�ϴ� ����
uint8_t Rx;                 // ���� ���ڸ� �ӽ÷� �����ϴ� ����
uint8_t SendPc[12];          // PC�� ���� ��ǥ������ �����ϴ� �迭

uint8_t Red_x = 5;          // ���õ� ���� x��ǥ
uint8_t Red_y = 5;          // ���õ� ���� y��ǥ
uint8_t Blue_x = 5;         // ���õ� û�� x��ǥ
uint8_t Blue_y = 5;         // ���õ� û�� y��ǥ
uint8_t Red_Turn = 0;       // ���� ���� �˷��ִ� ����
uint8_t Blue_Turn = 0;      // û�� ���� �˷��ִ� ����
uint8_t Move_flag = 0;      // joystick ������ ����
uint8_t red[10][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}   // ������ ������ ��ġ �����ϴ� �迭
                    ,{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

uint8_t blue[10][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // û���� ������ ��ġ �����ϴ� �迭
                    ,{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t Red_Score;          //FRAM�� ����� ���� ����
uint8_t Blue_Score;         //FRAM�� ����� û�� ����

int main(void)
{
	System_Clear();	              // �ý��� �ʱ�ȭ
    USART1_Init();
	while(1) {
        if(Red_Turn == 1 && Move_flag == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left �Է½�
                    BEEP();							//���� 1ȸ
                    if(Red_x > 0)                   //Red x��ǥ�� 1�̻��϶�
                        Red_x -= 1;					//Red x��ǥ�� 1 ����
                    //��ǥ �ֽ�ȭ
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_RIGHT:						//Joy_Left�Է½�
                    BEEP();							//���� 1ȸ
                    if(Red_x < 9)                   //Red x��ǥ�� 8�����϶�
                        Red_x += 1;					//Red x��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 2, Red_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_UP:						//Joy_UP �Է½�
                    BEEP();							//���� 1ȸ
                    if(Red_y > 0)                   //Red y��ǥ�� 1�̻� �϶�
                        Red_y -= 1;					//Red y��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                    DelayMS(10);
                break;
                case JOY_DOWN:						//Joy_UP �Է½�
                    BEEP();							//���� 1ȸ
                    if(Red_y < 9)                   //Red y��ǥ�� 8�����϶�
                        Red_y += 1;					//Red y��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_RED);
                    LCD_DisplayChar(9, 4, Red_y + 0x30);
                    DelayMS(10);
                break;
            }	//switch(JOY_Scan()) end
        }//if(Red_Turn == 1) end

        else if(Blue_Turn == 1 && Move_flag == 1) {
            switch(JOY_Scan()) {
                case JOY_LEFT:	                    //Joy_Left �Է½�
                    BEEP();						    //���� 1ȸ
                    if(Blue_x > 0)                  //Blue x��ǥ�� 1�̻��϶�
                        Blue_x -= 1;	        	//Blue x��ǥ�� 1 ����
                    //��ǥ �ֽ�ȭ
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_RIGHT:                     //Joy_Right �Է½�
                    BEEP();							//���� 1ȸ
                    if(Blue_x < 9)                  //Blue x��ǥ�� 8�����϶�
                        Blue_x += 1;			    //Blue x��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_BLUE);
                    LCD_DisplayChar(9, 14, Blue_x + 0x30);
                    DelayMS(10);
                break;
                case JOY_UP:						//Joy_UP �Է½�
                    BEEP();			    			//���� 1ȸ
                    if(Blue_y > 0)                   //Blue y��ǥ�� 1�̻� �϶�
                        Blue_y -= 1;					//Blue y��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_BLUE);              
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                    DelayMS(10);
                break;
                case JOY_DOWN:						//Joy_DOWN �Է½�
                    BEEP();							//���� 1ȸ
                    if(Blue_y < 9)                  //Blue y��ǥ�� 8�����϶�
                        Blue_y += 1;		   		//Blue y��ǥ�� 1 ����
                    /*��ǥ �ֽ�ȭ*/
                    LCD_SetTextColor(RGB_BLUE);         
                    LCD_DisplayChar(9, 16, Blue_y + 0x30);
                    DelayMS(10);
                break;
            }	//switch(JOY_Scan()) end
        }//else if(Blue_Turn == 1) end
        DelayMS(10);        //ȭ�� ����� ����
	}  //whlie(1) end	
} // main() end

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

	// LED (GPIO G) ���� : LED0(PG0), LED7(PG7) Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)						
	GPIOG->MODER 	|=  0x00004001;	// GPIOG 0,7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00004001;	// GPIOG 0,7 : Push-pull
	GPIOG->OSPEEDR 	&= ~0x0000FFFF;	// GPIOG 0~7 : Clear(0b00)
	GPIOG->OSPEEDR 	|=  0x00004001;	// GPIOG 0,7 : Output speed 25MHZ Medium speed 
	// PUPDR : Default (floating)
	
    //Joy Stick SW(GPIO I) ���� : Input mode(PI5 ~ PI9)
	RCC->AHB1ENR	|=  0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER	&= ~0x000FF000;	// GPIOI 6~9 : Input mode (reset state)
	GPIOI->PUPDR	&= ~0x000FF000;	// GPIOI 6~9 : Floating input (No Pull-up, pull-down) (reset state)
}

/*  EXTI5,8(JOY_PUSH(PI5),SW0(PH8))
	EXTI15(PH15, SW7))�ʱ� ����  */
void _EXTI_Init(void)
{
	RCC->AHB1ENR 	|= 0x00000180;	// RCC_AHB1ENR GPIOH,I Enable
	RCC->APB2ENR 	|= 0x00004000;	// Enable System Configuration Controller Clock
	GPIOH->MODER 	&= ~0xC0010000;	// GPIOH 8,15 : Input mode
    GPIOI->MODER	&= ~0x00000C00;	// GPIOI 5 : Input mode (reset state)
	// EXTI5 <- PI5, EXTI8 <- PH8 
	// EXTI15 <- PH15 
	// EXTICR2(EXTICR[1]), EXTICR3(EXTICR[2]), EXTICR4(EXTICR[3])�� �̿� 
	// reset value: 0x0000
    SYSCFG->EXTICR[1] &= ~0x00F0;	// EXTICR2 clear
	SYSCFG->EXTICR[1] |= 0x0080;	// EXTI5 -> GPIOI
    SYSCFG->EXTICR[2] &= ~0x000F;	// EXTICR3 clear
	SYSCFG->EXTICR[2] |= 0x0007;	// EXTI8 -> GPIOH
    SYSCFG->EXTICR[3] &= ~0xF000;	// EXTICR4 clear
    SYSCFG->EXTICR[3] |= 0x7000;	// EXTI15 -> GPIOH
	
	EXTI->FTSR |= 0x8120;			// EXTI5,8,15: Falling Trigger Enable  
	EXTI->IMR  &= ~0x8120;			// EXTI5,8,15: IMR Reg �ʱ�ȭ
	EXTI->IMR  |= 0x8100;			// EXTI8,15: UnMasked (Interrupt Enable) ����

	NVIC->ISER[0] |= ( 1 << 23  );			// Enable 'Global Interrupt EXTI5,8'
    NVIC->ISER[1] |= ( 1 << (40 - 32) );	// Enable 'Global Interrupt EXTI15'
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{	
    //���� (EXTI5, JOY_PUSH(PI5))
	if(EXTI->PR & 0x0020)		    // EXTI5 �߻�
	{
		EXTI->PR |= 0x0020;			    // Pending bit Clear
        BEEP();								// ���� 1ȸ
        if(Red_Turn == 1)    //�����϶� ����
        {
            if(red[Red_x][Red_y] == 1 || blue[Red_x][Red_y] == 1)// �̹� ���� ����������
            {
                DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else
            {
                EXTI->IMR  &= ~0x0100;		// EXTI8: Masked(�������úҰ�)
                EXTI->IMR  |= 0x8000;		// EXTI15: unMasked(û�����ð���)
                LCD_SetBrushColor(RGB_RED);                // �簢�� ���� ����
                LCD_DrawFillRect(22 + Red_x*10, 22 + Red_y*10, 7, 7);  		 // �簢�� ���� ä���
                Move_flag = 0;              // �� ��ǥ ������ off
                red[Red_x][Red_y] = 1;      // ������ ������ ��ġ ����
                SendPc[0] = 'R';            // ���� Ÿ�� ('R'�� ����, 'B'�� û��)
                SendPc[1] = Red_x+0x30;          // ���� x��ǥ
                SendPc[2] = Red_y+0x30;          // ���� y��ǥ
                SendPc[3] = ' ';            // ���� ��Ÿ���� ���� ����
                SerialSendString(SendPc);   // PC�� ��ǥ������
            }
        }
        else if(Blue_Turn == 1)     //û���϶� ����
        {
            if(red[Blue_x][Blue_y] == 1 || blue[Blue_x][Blue_y] == 1) // �̹� ���� ����������
            {
                DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else
            {
                EXTI->IMR  &= ~0x8000;		// EXTI15: Masked(û�����úҰ�)
                EXTI->IMR  |= 0x0100;		// EXTI8: unMasked(�������ð���)
                LCD_SetBrushColor(RGB_BLUE);                // �簢�� ���� ����
                LCD_DrawFillRect(22 + Blue_x*10, 22 + Blue_y*10, 7, 7);  		 // �簢�� ���� ä���
                EXTI->IMR  &= ~0x0020;		// EXTI5: Masked (�����Ұ�)
                Move_flag = 0;              // �� ��ǥ ������ off
                blue[Blue_x][Blue_y] = 1;   // û���� ������ ��ġ ����
                SendPc[0] = 'B';            // ���� Ÿ��
                SendPc[1] = Blue_x+0x30;         // ���� x��ǥ
                SendPc[2] = Blue_y+0x30;         // ���� y��ǥ
                SendPc[3] = ' ';            // ���� ��Ÿ���� ���� ����
                SerialSendString(SendPc);   // PC�� ��ǥ������
            }
        }
        Detect_Win();                   // ���� �Ǻ� �˰���
    }//if(EXTI->PR & 0x0020) end
	//�������� (EXTI8,SW 0)
	else if(EXTI->PR & 0x0100)		// EXTI8 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x0100;			// Pending bit Clear
        EXTI->IMR  |= 0x0020;		// EXTI5: unMasked(���� ����)
        EXTI->IMR  &= ~0x8000;		// EXTI15: Masked (û�����úҰ�)
        GPIOG->ODR |= 0x0001; 	    // LED0 ON
        GPIOG->ODR &= ~0x0080; 	    // LED7 OFF 
        Red_Turn = 1;               //���� Enable
        Blue_Turn = 0;
        Move_flag = 1;              // �� ��ǥ ������ on
        LCD_SetTextColor(RGB_RED);
        LCD_DisplayText(9, 0, "*");
		LCD_DisplayText(9, 18, " ");
    }
}

/* EXTI10~15 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
	// û������ (EXTI15,SW 7)
	if(EXTI->PR & 0x8000)			// EXTI11 Interrupt Pending(�߻�) ����?
	{
		EXTI->PR |= 0x8000;			// Pending bit Clear
        EXTI->IMR  &= ~0x0100;		// EXTI8: Masked(�������úҰ�)
        EXTI->IMR  |= 0x0020;		// EXTI5: unMasked(���� ����)
        GPIOG->ODR &= ~0x0001; 	    // LED0 OFF
        GPIOG->ODR |= 0x0080; 	    // LED7 ON
		Red_Turn = 0;               //û�� Enable
        Blue_Turn = 1;
        Move_flag = 1;              // �� ��ǥ ������ on
        LCD_SetTextColor(RGB_BLUE);
        LCD_DisplayText(9, 18, "*");
		LCD_DisplayText(9, 0, " ");
	}
}

// USART1 �⺻ ����
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
	NVIC->ISER[1]	|= (1<<(37-32));// Enable Interrupt USART1 (NVIC 37��)
	USART1->CR1 	|= (1<<13);	//  0x2000, USART1 Enable
}

void USART1_IRQHandler(void)	
{
    if (USART1->SR & USART_SR_RXNE) // �����Ͱ� ���ŵǾ����� Ȯ��
    {
        Rx = (uint8_t)(USART1->DR & 0x1FF); // ���ŵ� ������ �б�
        Input[input_count++] = Rx;         // �����͸� �迭�� ����
        
    }
    // ��ǥ���� ��� �޾ƿ� �� ����
    if(input_count == 3)
    {
        if(Input[0] == 'R')
        {
            Red_x = Input[1] - 0x30;        // x��ǥ�� ����
            Red_y = Input[2] - 0x30;        // y��ǥ�� ����
            if(red[Red_x][Red_y] == 1 || blue[Red_x][Red_y] == 1)
            {
                DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else if(Blue_Turn == 1)
            {
                DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else
            {
                BEEP();
                LCD_SetTextColor(RGB_RED);         
                LCD_DisplayChar(9, 2, Input[1]);    // x��ǥ�� ǥ��
                LCD_DisplayChar(9, 4, Input[2]);    // y��ǥ�� ǥ��
                LCD_DisplayText(9, 0, "*");
                LCD_DisplayText(9, 18, " ");
                LCD_SetBrushColor(RGB_RED);
                LCD_DrawFillRect(22 + Red_x*10, 22 + Red_y*10, 7, 7);  		 // �簢�� ���� ä���
                red[Red_x][Red_y] = 1;      // ������ ������ ��ġ ����
                Red_Turn = 0;               // û�� Enable
                Blue_Turn = 1;
                GPIOG->ODR |= 0x0001; 	    // LED0 ON
                GPIOG->ODR &= ~0x0080; 	    // LED7 OFF
            }

        }
        else if(Input[0] == 'B')
        {
            Blue_x = Input[1] - 0x30;             // x��ǥ�� ����
            Blue_y = Input[2] - 0x30;             // y��ǥ�� ����
            if(red[Blue_x][Blue_y] == 1 || blue[Blue_x][Blue_y] == 1) // �̹� ���� ����������
            {
                //DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else if(Red_Turn == 1)
            {
                //DelayMS(1000);
                BEEP_3();                   // 1�ʰ��� ���� 3ȸ
                SerialSendString(Warning);  // PC�� "Not running "����
            }
            else
            {
                BEEP();
                LCD_SetTextColor(RGB_BLUE);         
                LCD_DisplayChar(9, 14, Input[1]);   // x��ǥ�� ǥ��
                LCD_DisplayChar(9, 16, Input[2]);   // y��ǥ�� ǥ��
                LCD_DisplayText(9, 18, "*");
                LCD_DisplayText(9, 0, " ");
                LCD_SetBrushColor(RGB_BLUE);                // �簢�� ���� ����
                LCD_DrawFillRect(22 + Blue_x*10, 22 + Blue_y*10, 7, 7);  		 // �簢�� ���� ä���
                blue[Blue_x][Blue_y] = 1;   // û���� ������ ��ġ ����
                Red_Turn = 1;               // ���� Enable
                Blue_Turn = 0;
                GPIOG->ODR &= ~0x0001; 	    // LED0 OFF
                GPIOG->ODR |= 0x0080; 	    // LED7 ON
            }
        }
        input_count = 0;                // input_count �ʱ�ȭ
        Detect_Win();                   // ���� �Ǻ� �˰���
    }
    
	// DR �� ������ SR.RXNE bit(flag bit)�� clear �ȴ�. �� clear �� �ʿ����
}

void SerialSendChar(uint8_t Ch) // 1���� ������ �Լ�
{
	// USART_SR_TXE(1<<7)=0?, TX Buffer NOT Empty? 
	// TX buffer Empty���� ������ ��� ���(�۽� ������ ���±��� ���)
    while((USART1->SR & USART_SR_TXE) == RESET); 
	USART1->DR = (Ch & 0x01FF);	// ���� (�ִ� 9bit �̹Ƿ� 0x01FF�� masking)
}

void SerialSendString(char *str) // �������� ������ �Լ�
{
	while (*str != '\0') // ���Ṯ�ڰ� ������ ������ ����, ���Ṯ�ڰ� �����Ŀ��� ������ �޸� ���� �߻����ɼ� ����.
	{
		SerialSendChar(*str);	// �����Ͱ� ����Ű�� ���� �����͸� �۽�
		str++; 			// ������ ��ġ ����
	}
}

// Baud rate ����
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
		integerdivider = ((25 * APB2clock) / (2 * USART_BaudRate));  // ���Ŀ� 100�� ���� ����(�Ҽ��� �ι�°�ڸ����� �����ϱ� ����)  
	}
	else  // USART1->CR1.OVER8 = 0 (16 oversampling)
	{	// Computing 'Integer part' when the oversampling mode is 16 Samples 
		integerdivider = ((25 * APB2clock) / (4 * USART_BaudRate));  // ���Ŀ� 100�� ���� ����(�Ҽ��� �ι�°�ڸ����� �����ϱ� ����)    
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

//���� �Ǻ� �˰���
void Detect_Win(void)
{
    uint8_t Rx_cnt = 0;     //����x�� �������� �������� Ȯ���ϴ� ����
    uint8_t Ry_cnt = 0;     //����y�� �������� �������� Ȯ���ϴ� ����
    uint8_t Bx_cnt = 0;     //û��x�� �������� �������� Ȯ���ϴ� ����
    uint8_t By_cnt = 0;     //û��y�� �������� �������� Ȯ���ϴ� ����
    int i, j, k;
    for(i = 0; i < 10; i++)
    {
        for(j = 0; j < 10; j++)
        {
            if(red[j][i] == 1)  Rx_cnt += 1;         //x�࿡ ������ 5�� �������� Ȯ��
            else Rx_cnt = 0;
            if(Rx_cnt == 5)  
            {
                BEEP_5();
                if(Red_Score < 9)          //9������ 0
                        Red_Score += 1;
                else if(Red_Score == 9)
                        Red_Score = 0;
                Fram_Write(300, Red_Score);
                System_Clear();
            }
            if(blue[j][i] == 1)  Bx_cnt += 1;         //x�࿡ û���� 5�� �������� Ȯ��
            else Bx_cnt = 0;
            if(Bx_cnt == 5) 
            {
                BEEP_5();
                if(Blue_Score < 9)          //9������ 0
                        Blue_Score += 1;
                else if(Blue_Score == 9)
                        Blue_Score = 0;
                Fram_Write(301, Blue_Score);
                System_Clear();
            }
            if(red[i][j] == 1)  Ry_cnt += 1;         //y�࿡ ������ 5�� �������� Ȯ��
            else Ry_cnt = 0;
            if(Ry_cnt == 5) 
            {
                BEEP_5();
                if(Red_Score < 9)          //9������ 0
                        Red_Score += 1;
                else if(Red_Score == 9)
                        Red_Score = 0;
                Fram_Write(300, Red_Score);
                System_Clear();
            }
            if(blue[i][j] == 1)  By_cnt += 1;         //y�࿡ û���� 5�� �������� Ȯ��
            else By_cnt = 0;
            if(By_cnt == 5) 
            {
                BEEP_5();
                if(Blue_Score < 9)          //9������ 0
                        Blue_Score += 1;
                else if(Blue_Score == 9)
                        Blue_Score = 0;
                Fram_Write(301, Blue_Score);
                System_Clear();
            }
        }//for(j = 0; j < 10; j++)  end
    }//for(i = 0; i < 10; i++)  end
    //������밢��
    for(i = 0; i < 6; i++)
    {
        for(j = 0; j < 6; j++)
        {
            for(k = 0; k < 5; k++){
                if(red[i+k][j+k] == 1)  Rx_cnt += 1;         //���� ������ �밢�� 5�� Ȯ��
                else Rx_cnt = 0;
                if(Rx_cnt == 5) 
                {
                    BEEP_5();
                    if(Red_Score < 9)          //9������ 0
                        Red_Score += 1;
                    else if(Red_Score == 9)
                        Red_Score = 0;
                    Fram_Write(300, Red_Score);
                    System_Clear();
                }
                if(blue[i+k][j+k] == 1)  Bx_cnt += 1;         //û�� ������ �밢�� 5�� Ȯ��
                else Bx_cnt = 0;
                if(Bx_cnt == 5)  
                {
                    BEEP_5();
                    if(Blue_Score < 9)          //9������ 0
                        Blue_Score += 1;
                    else if(Blue_Score == 9)
                        Blue_Score = 0;
                    Fram_Write(301, Blue_Score);
                    System_Clear();
                }
            }//for(k = 0; k < 5; k++) end
        }//for(j = 0; j < 6; j++)  end
    }//for(i = 0; i < 6; i++)  end
    //�����밢��
    for(i = 0; i < 6; i++)
    {
        for(j = 4; j < 10; j++)
        {
            for(k = 0; k < 5; k++){
                if(red[i+k][j-k] == 1)  Rx_cnt += 1;         //���� ����� �밢�� 5�� Ȯ��
                else Rx_cnt = 0;
                if(Rx_cnt == 5)  
                {
                    BEEP_5();
                    if(Red_Score < 9)          //9������ 0
                        Red_Score += 1;
                    else if(Red_Score == 9)
                        Red_Score = 0;
                    Fram_Write(300, Red_Score);
                    System_Clear();
                }
                if(blue[i+k][j-k] == 1)  Bx_cnt += 1;         //û�� ����� �밢�� 5�� Ȯ��
                else Bx_cnt = 0;
                if(Bx_cnt == 5)  
                {
                    BEEP_5();
                    if(Blue_Score < 9)          //9������ 0
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

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
	LCD_Clear(RGB_YELLOW);		    // ȭ�� Ŭ����
	LCD_SetFont(&Gulim8);		    // ��Ʈ : ���� 8
	//Title �ۼ�
	LCD_SetBackColor(RGB_YELLOW);             // ���ڹ��� : YELLOW
    LCD_SetTextColor(RGB_BLACK);              // ���ڻ� : BLACK
	LCD_DisplayText(0, 0, "Mecha-OMOK(JW)");  // Title �ۼ�
	//��ǥ ǥ��
	LCD_DisplayText(1, 2, "0");	
	LCD_DisplayText(1, 9, "5");		
	LCD_DisplayText(1, 14, "9");
	LCD_DisplayText(5, 2, "5");		
	LCD_DisplayText(8, 2, "9");
    // ������ �׸���       			
    LCD_SetBrushColor(RGB_YELLOW);                // �簢�� ���� ����
	LCD_SetPenColor(RGB_BLACK);				 	 // �簢�� �׵θ� �� ����
    for(uint16 x = 0 ; x < 9; x++){
        for(uint16 y = 0; y < 9; y++){
            LCD_DrawRectangle(x * 10 + 25, y * 10 + 25, 10, 10);         	 // �簢�� �׸���
        }//for(uint16 x = 0 ; x < 9; x++) end
    }//for(uint16 y = 0; y < 9; y++) end
    LCD_DrawRectangle(75 - 2, 75 - 2, 4, 4);    //(5,5) ǥ��
    LCD_SetBrushColor(RGB_BLACK);                // �簢�� ���� ����
    LCD_DrawFillRect(75 - 1, 75 - 1, 3, 3);  		 // �簢�� ���� ä���
    // ����� ���� ��ǥ �� SCORE
    LCD_DisplayText(9, 8, "vs");
    LCD_SetTextColor(RGB_RED);              // ���ڻ� : RED
    LCD_DisplayText(9, 1, "( , )");
    LCD_DisplayChar(9, 2, Red_x + 0x30);
    LCD_DisplayChar(9, 4, Red_y + 0x30);
    LCD_DisplayChar(9, 7, Red_Score + 0x30);
    LCD_SetTextColor(RGB_BLUE);              // ���ڻ� : BLUE
    LCD_DisplayText(9, 13, "( , )");
    LCD_DisplayChar(9, 14, Blue_x + 0x30);
    LCD_DisplayChar(9, 16, Blue_y + 0x30);
    LCD_DisplayChar(9, 10, Blue_Score + 0x30);
}

//GPIO, EXTI, GLCD�� �ʱ�ȭ�ϴ� �Լ�
void System_Clear(void)
{
    Red_x = 5;          
    Red_y = 5;          
    Blue_x = 5;        
    Blue_y = 5;        
    Red_Turn = 0;
    Blue_Turn = 0;
    Move_flag = 0;
    //���� ����� ��ġ �ʱ�ȭ
    for (int i = 0; i < 10; i++) 
    {
        for (int j = 0; j < 10; j++)
        {
            red[i][j] = 0;
        }
    }
    ////û�� ����� ��ġ �ʱ�ȭ
    for (int i = 0; i < 10; i++) 
    {
        for (int j = 0; j < 10; j++)
        {
            blue[i][j] = 0;
        }
    }
    GPIOG->ODR &= ~0x0081; 	    // LED0,7 OFF
    Fram_Init();                  // FRAM �ʱ�ȭ H/W �ʱ�ȭ
	Fram_Status_Config(); 		  // FRAM �ʱ�ȭ S/W �ʱ�ȭ
    Red_Score = Fram_Read(300);   // red�� ���� Fram���� �ҷ�����
    Blue_Score = Fram_Read(301);  // Blue�� ���� Fram���� �ҷ�����
	LCD_Init();	            // LCD ��� �ʱ�ȭ
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