#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ATmega128_v20m.H"
#define BUZZER_ON PORTG |= (1<<PG3) // buzzer ON
#define BUZZER_OFF PORTG &= ~(1<<PG3) // buzzer OFF
#define SW1 !(PINB & (1<<PB0)) // MODE2
#define SW2 !(PINB & (1<<PB1)) // MODE3
#define cjs PORTE = 0b00001110; //천의자리 간편화
#define qor PORTE = 0b00001101; //백의자리 간편화
#define tlq PORTE = 0b00001011; //십의자리 간편화
#define dlf PORTE = 0b00000111; //일의자리 간편화


unsigned char digitPatterns[] = { //7세그먼트 배열 사용하여 0=0 5=5 간편하게 숫자 활용가능
	0b00111111, // 0
	0b00000110, // 1
	0b01011011, // 2
	0b01001111, // 3
	0b01100110, // 4
	0b01101101, // 5
	0b01111101, // 6
	0b00000111, // 7
	0b01111111, // 8
	0b01101111  // 9
};

unsigned char digitPatterns2[] = { //7세그먼트 내부에서 십의 자리 에 dp 핀의 dot 사용 따라서 MSB가 모두 1인것을 확인가능
	0b10111111, // 0
	0b10000110, // 1
	0b11011011, // 2
	0b11001111, // 3
	0b11100110, // 4
	0b11101101, // 5
	0b11111101, // 6
	0b10000111, // 7
	0b11111111, // 8
	0b11101111  // 9
};

// 함수선언
void MCU_initialize(void);
void LCD_initialize(void);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_print(char *str);
void ADC_initialize(void);
uint16_t ADC_read(uint8_t channel);
void display_voltage(uint16_t adc_value);
void LED_control(uint16_t voltage);

int main(void)
{
	DDRB |= 0xf0; // 7~4LED, 3~0 SW
    DDRD = 0xFF;  // 4-digit 7-segment display
    DDRE = 0xFF;  // 4-digit 7-segment display	
	
	
////////////////////////       부팅시작 (MODE1)     ///////////////////////////
	MCU_initialize(); // MCU 초기화
	LCD_initialize(); // LCD 초기화
	ADC_initialize(); // ADC 초기화
	
// 초기부팅시 부팅음 부저2회 출력
BUZZER_ON;
Delay_ms(100);
BUZZER_OFF;
Delay_ms(100);
BUZZER_ON;
Delay_ms(100);
BUZZER_OFF;
Delay_ms(100);
// 초기부팅시 화면 및 팀로고 출력
LCD_string(0x80,"DC Motor Test ");
LCD_string(0xC0,"Tested By AACL ");
Delay_ms(2000);
LCD_string(0x80,"                ");
LCD_string(0xC0,"                ");
Delay_ms(300);
LCD_string(0x80,"                ");
LCD_string(0xC0,"    OOO   OOO   ");
Delay_ms(150);
LCD_string(0x80,"    OOO   OOO   ");
LCD_string(0xC0,"   O   O O   O");
Delay_ms(150);
LCD_string(0x80,"   O   O O   O");
LCD_string(0xC0,"  O     O     O ");
Delay_ms(150);
LCD_string(0x80,"  O     O     O ");
LCD_string(0xC0,"  O           O ");
Delay_ms(150);
LCD_string(0x80,"  O           O ");
LCD_string(0xC0,"  O           O ");
Delay_ms(150);
LCD_string(0x80,"  O           O ");
LCD_string(0xC0,"   O         O  ");
Delay_ms(150);
LCD_string(0x80,"   O         O  ");
LCD_string(0xC0,"    O       O   ");
Delay_ms(150);
LCD_string(0x80,"    O       O   ");
LCD_string(0xC0,"     O     O    ");
Delay_ms(150);
LCD_string(0x80,"     O     O    ");
LCD_string(0xC0,"      O   O     ");
Delay_ms(150);
LCD_string(0x80,"      O   O     ");
LCD_string(0xC0,"       O O      ");
Delay_ms(150);
LCD_string(0x80,"       O O      ");
LCD_string(0xC0,"        O       ");
Delay_ms(150);
LCD_string(0x80,"        O       ");
LCD_string(0xC0,"                ");
Delay_ms(150);
LCD_string(0x80," press S/W 1or2 ");
LCD_string(0xC0,"    thank you   ");
/////////////////////////////////////////////////////////////////////////////////////////

	while (1)
	{
		if(SW1) //SW1을 누르면 MODE2 실행
			{
				while(1<<PB0)
				{
							uint16_t adc_value = ADC_read(6); // ADC6 값 읽기
							display_voltage(adc_value);      // 전압 값 LCD에 표시
							LED_control(adc_value);          // LED 및 부저 제어
							
				}
			}
		if(SW2) //SW2을 누르면 MODE3 실행
		{
			while(1<<PB1)
			{		uint16_t adc_value = ADC_read(6); // ADC6 값 읽기
					display_voltage(adc_value);      // 전압 값 LCD에 표시

					
// 4-digit 7세그먼트 디스플레이에 voltage 값 표시
int voltage_int; //변수 선언(LCD에 표시되는 값)
int digit1, digit2, digit3, digit4; //변수 선언
float voltage2 = adc_value * (5.0 / 1023.0); //LCD에 표시되고있는 float형태의 voltage값 

if (voltage2 == 0)
{
	voltage_int = 0;
	digit1 = 0;
	digit2 = 0;
	digit3 = 0;
	digit4 = 0;
}
else if (voltage2 == 5)
{
	voltage_int = 1000;
	digit1 = 1;
	digit2 = 0;
	digit3 = 0;
	digit4 = 0;
}
else
{
    voltage_int = (int)(voltage2 * 200);  // 0에서 1000까지의 정수값으로 변환
    digit1 = voltage_int / 1000;            // 천의 자리
    digit2 = (voltage_int / 100) % 10;      // 백의 자리
    digit3 = (voltage_int / 10) % 10;       // 십의 자리
    digit4 = voltage_int % 10;              // 일의 자리
}

cjs
PORTD = digitPatterns[digit1];
_delay_ms(1);

qor
PORTD = digitPatterns[digit2];
_delay_ms(1);

tlq
PORTD = digitPatterns2[digit3];
_delay_ms(1);

dlf
PORTD = digitPatterns[digit4];
_delay_ms(1);
					
			}
		}
	}
	return 0;
}

void ADC_initialize(void)
{
	ADMUX = (1 << REFS0);    // AVCC를 기준 전압으로 사용
	ADCSRA = (1 << ADEN) |   // ADC 활성화
	(1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128 설정
}

uint16_t ADC_read(uint8_t channel)
{
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // 입력 채널 선택
	ADCSRA |= (1 << ADSC); // 변환 시작
	while (ADCSRA & (1 << ADSC))
	{
		; // 변환 완료 대기
	}
	return ADC;
}

void display_voltage(uint16_t adc_value)
{
	float voltage = adc_value * (5.0 / 1023.0); // ADC 값에 대응하는 전압 계산
	
		LCD_command(0x80);               // 첫번째 줄 이동
		LCD_print("DC Voltmeter       "); // 문구 출력
	
		LCD_command(0xC0);
		LCD_print("AD6:");          // 두번째 줄 이동
		LCD_1d2(voltage);           // 전압 값 출력
		LCD_print("[V]              ");             // 단위 출력
}

// ATmega128_v20m.H 에 LCD_string사용 해봤지만 오류가나서 직접 간단한 string LCD함수 추가
void LCD_print(char *str)
{
	while (*str)
	{
		LCD_data(*str++);
	}
}
//프린터에 주어진 소수number 표기 
void LCD_1d2(float number)
{
	int i, j;
	j = (int)(number * 100. + 0.5);
	i = j / 100;
	LCD_data(i + '0');
	LCD_data('.');
	i = (j % 100) / 10;
	LCD_data(i + '0');
	i = j % 10;
	LCD_data(i + '0');
} 
///////// 아래는 현재 값에 따른 LED 점등
void LED_control(uint16_t voltage)
{
	int voltage_int = voltage * 5 / 1023; // 전압 값을 정수로 변환
	
	if (voltage_int == 0)
	{
		PORTB = 0b00000000; // 정수값이 0볼트 일때 LED 꺼짐
	}
	else if (voltage_int == 1)
	{ 
		PORTB = 0b10000000; // 정수값이 1일때 LED 1개 켜짐
	}
	else if (voltage_int == 2)
	{
		PORTB = 0b11000000; // 정수값이 1일때 LED 2개 켜짐
	}
	else if (voltage_int == 3)
	{
		PORTB = 0b11100000; // 정수값이 1일때 LED 3개 켜짐
	}
	else if (voltage_int == 4)
	{
		PORTB = 0b11110000; // 정수값이 1일때 LED 4개 켜짐
	}
	else if (voltage_int == 5) // 정수값이 5일때 최대값을 알리는 300ms 주기로 LED 점멸 및 부저음
	{
		PORTB = 0b11110000;
		_delay_ms(300);
		PORTB = 0b00000000;
		_delay_ms(300);
		BUZZER_ON;
		Delay_ms(100);
		BUZZER_OFF;
		Delay_ms(100);
		
		BUZZER_ON;
		Delay_ms(100);
		BUZZER_OFF;
		Delay_ms(100);
		
		BUZZER_ON;
		Delay_ms(100);
		BUZZER_OFF;
		Delay_ms(1000);
	}
}