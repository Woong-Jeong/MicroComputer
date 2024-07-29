/////////////////////////////////////////////////////////////
// ������: HW2. ����������
// ��������: STM32F407 ����ũ����Ʈ�ѷ��� ����Ͽ� ����ġ�� ���̽�ƽ �Է��� ���� 
//           ���������͸� �ùķ��̼��ϰ�, LED�� ������ �̿��� 
//           ���� ���� ���¸� ǥ���ϴ� �ý����� ����
// ����� �ϵ����(���): GPIO, Switch, Joy-stick, LED, Buzzer
// ������: 2024. 05. 18. 
// ������ Ŭ����: �����Ϲ�
// �й�: 2020134035
// �̸�: �� ��
///////////////////////////////////////////////////////////////

#include "stm32f4xx.h"

//Switch �� JoyStick �� �ּҰ��� ������ ����
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

void _GPIO_Init(void);             // GPIO �ʱ�ȭ �Լ�
uint16_t KEY_Scan(void);           // ����ġ �Է��� ��ĵ�ϴ� �Լ�
uint16_t JOY_Scan(void);           // ���̽�ƽ �Է��� ��ĵ�ϴ� �Լ�
void SW_Move(int Input, int Now);  // �� �̵� �Լ�
void BEEP(void);                   // ���� �Լ�
void DelayMS(unsigned short wMS);  // �и��� ���� ���� �Լ�
void DelayUS(unsigned short wUS);  // ����ũ���� ���� ���� �Լ�

int main(void)
{
	_GPIO_Init();               //GPIO�� �ʱ�ȭ
	GPIOG->ODR |= ~0x00FF;	    // �ʱⰪ: LED0~7 Off
	GPIOG->ODR |= 0x0001; 	    // GPIOG->ODR.1 'H'(LED0 ON)
        
        int Now_flag = 0;           //���� ���� ���� �����ϴ� ��������(�ʱⰪ: 0��)
        int Input_flag = 0;        //����ġ �Է����� �����ϴ� ��������(�ʱⰪ: 0��) 
        BEEP();                    //���������Ͱ� �غ������ �˸��� ����
        
	while(1)
	{
                //Switch Input
		switch(KEY_Scan())
		{
			case SW0  : 	                    // SW�Է½�
				BEEP();                         // ���� �︲
                                Input_flag = 0;                 // �Է��� ����
                                SW_Move(Input_flag, Now_flag);  // �� �̵�
                                Now_flag = 0;                   // ���� �� ������Ʈ
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
			case JOY_LEFT:	              //���� JoyStick �Է½�
			        BEEP();                       //���� 1ȸ ����  
                                if(Now_flag > 0)              //�������� 1~7���϶�
                                {
                                        DelayMS(500);             //0.5���Ŀ�
                                        Input_flag -= 1;          //�Էº��� -1
                                        GPIOG->ODR >>= 1;          //LED�� �������� �̵�
                                        Now_flag = Input_flag;    //������ ������Ʈ
                                }
                                else                          //�������� 0���϶�  
                                {
                                        DelayMS(200);             //0.2���� ����1�� �۵�  
                                        BEEP();                           
                                }     
			break;
			case JOY_RIGHT:	              //������ JoyStick �Է½�
				BEEP();                   //���� 1ȸ ����
                                if(Now_flag < 7)          //�������� 0~6���϶�
                                {       
                                        DelayMS(500);            //0.5���Ŀ�
                                        Input_flag += 1;         //�Էº��� +1
                                        GPIOG->ODR <<= 1;        //LED�� ���������� �̵�
                                        Now_flag = Input_flag;   //������ ������Ʈ
                                }
                                else                      //�������� 7���϶�
                                {
                                        DelayMS(200);            //0.2���� ����1�� �۵�  
                                        BEEP();
                                }                               
			break;             
		}  
	}
}
//Switch�� �������� ���� �̵��ϰ� �ϴ� �Լ�
//������������ ���� ���� ��Ÿ���� ���� Now_flag��
//�Է¹��� ���� ��Ÿ���� ���� Input_flag�� Ȱ��
void SW_Move(int Input, int Now)
{                
        if(Input == Now)        //�������� ���� SW�Է½� 0.2���� ���� 1ȸ
        {
                DelayMS(200);
                BEEP();
        }       
        else if(Input > Now)    //SW�Է��� ���� �������� ������ ����
        {
                DelayMS(500);    //�ʱ���¿��� 0.5���Ŀ� LED �̵�
                //SW�Է°� ���� ������ ������������ �ݺ�
                while(Input != Now)
                {
                        GPIOG->ODR <<= 1;    //LED�� ���������� �̵�               
                        Now += 1;            //�������� +1
                        DelayMS(500);        //����: 0.5��                   
                }
                //while���� ����� �����Ѱ��̹Ƿ� 0.5�� �������� ���� 3ȸ
                for(int j = 0; j < 3; j++)    
                {
                        BEEP();
                        DelayMS(500);
                }        
        }
        else if(Input < Now)    //SW�Է��� ���� �������� ������ ����
        {     
                DelayMS(500);     //�ʱ���¿��� 0.5���Ŀ� LED �̵�
                //SW�Է°� ���� ������ ������������ �ݺ�
                while(Input != Now)
                {
                        GPIOG->ODR >>= 1;    //LED�� ���������� �̵�
                        Now -= 1;            //�������� -1       
                        DelayMS(500);        //����: 0.5��           
                }
                //while���� ����� �����Ѱ��̹Ƿ� 0.5�� �������� ���� 3ȸ
                for(int j = 0; j < 3; j++)
                {
                        BEEP();
                        DelayMS(500);
                }             
        }
}

/* Switch�� �ԷµǾ������� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */ 
//key_flag -> SW�� �ѹ� �������� �����ϱ� ���� ����
uint8_t key_flag = 0;           
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
        // SW�� ���� PH8~15�̹Ƿ� PH0~7�� 0���� �ʱ�ȭ
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
        //else�� ���� Ű�� ������
	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
			return 0xFF00;
		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);    //Ȥ�� �� ���������                      
			return key;
		}
	}
}

//JoyStick�� �ԷµǾ������� ���ο� � JoyStick�� �ԷµǾ������� ������ return�ϴ� �Լ�
//joy_flag -> JoyStick�� �ѹ� �������� �����ϱ� ���� ����
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

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(JoyStick) �ʱ� ���� */
void _GPIO_Init(void)
{
	// LED (GPIO G) ���� : Output mode
	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 

	// SW (GPIO H) ���� : Input mode 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) ���� : Output mode 
	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
        GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
        
        //Joy Stick SW(PORT I) ����
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