#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>

//UART
#define F_UBR 8000000UL
#define BAUD 9600
#define MYUBRR F_UBR/16/BAUD-1

uint8_t temp = 0;

#define BUFFER0_MAX_SIZE 4 // ����. ����� ������
char buffer0[BUFFER0_MAX_SIZE]; // ������
unsigned int buffer0_size = 0; //������� ������ ������
bool buffer0_complete = false; //������ � ������ �������� ���������

bool show_result = false;
bool show_error = false;
bool is_need_minus = false;

//LCD DISLAY
#define DATA_PORT PORTA
#define DATA_DDR DDRA
#define DATA_PIN PINA

#define CTL_PORT PORTB
#define CTL_DDR DDRB
#define RS_PIN PORTB0
#define RW_PIN PORTB1
#define E_PIN PORTB2

int cur_pos = 0;

//UART
void USARTInit(unsigned int ubrr) {
	//  ���������� ����������� ��������������� ����� ������
	//  UBRR = f / (16 * band)
	//  ��������� ��������
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)(ubrr);
	
	//  RXC         - ���������� �����
	//  |TXC        - ���������� ��������
	//  ||UDRE      - ���������� ������ ��� ��������
	//  |||FE       - ������ �����
	//  ||||DOR     - ������ ������������ ������
	//  |||||PE     - ������ ��������
	//  ||||||U2X   - ������� ��������
	//  |||||||MPCM - ����������������� �����
	//  ||||||||
	//  76543210
	UCSRA = 0;
	
	//  RXCIE       - ���������� ��� ����� ������
	//  |TXCIE      - ���������� ��� ���������� ��������
	//  ||UDRIE     - ���������� ���������� ������ ��� ��������
	//  |||RXEN     - ���������� �����
	//  ||||TXEN    - ���������� ��������
	//  |||||UCSZ2  - UCSZ0:2 ������ ����� ������
	//  ||||||RXB8  - 9 ��� �������� ������
	//  |||||||TXB8 - 9 ��� ���������� ������
	//  ||||||||
	//  76543210
	
	//  �������� ���� � �������� ������ �� UART
	UCSRB = 1<<RXCIE | 1<<RXEN | 1<<TXEN;
	
	//  URSEL        - ������ 1
	//  |UMSEL       - �����: 1-���������� 0-�����������
	//  ||UPM1       - UPM0:  1 ��������
	//  |||UPM0      - UPM0:  1 ��������
	//  ||||USBS     - ���� ����: 0-1, 1-2
	//  |||||UCSZ1   - UCSZ0: 2 ������ ����� ������
	//  ||||||UCSZ0  - UCSZ0: 2 ������ ����� ������
	//  |||||||UCPOL - � ���������� ������ - ������������
	//  ||||||||
	//  76543210
	//  8-������� �������, 2 ���� ����
	UCSRC = 1<<URSEL | 1<<UCSZ0 | 1<<UCSZ1;
	//UCSRC = 1<<URSEL | 1<<USBS | 1<<UCSZ0 | 1<<UCSZ1;
}

//  �������� �����
void USARTTransmitChar(char c) {
	//  ���������������, ����� ������� ��������
	while(!( UCSRA & (1<<UDRE)));
	UDR = c;
}

//  �������� ������
void USARTTransmitString(const char str[]) {
	register char i = 0;
	while(str[i]) {
		USARTTransmitChar(str[i++]);
	}
}

//  �������� ������
void USARTTransmitStringLn(const char str[]) {
	USARTTransmitString(str);
	USARTTransmitChar((char)13);
	USARTTransmitChar((char)10);
}

ISR(USART_RXC_vect) {
	char ch = UDR;
	switch (ch){
		case (char)10://������� ������ ����������
		break;
		case (char)13:// CR - ��������� ������
		buffer0[buffer0_size] = 0; // ��������� ���� � ����� ������ (������� � ���������)
		buffer0_complete = true; // ������� ��������� ������ ������ ��� ��. �����
		break;
		default:
		buffer0[buffer0_size] = ch; // ���� ������ - �� ����� ������ - ��������� ������ � �����
		buffer0_size++;
		break;
	}

	
	if (buffer0_size == BUFFER0_MAX_SIZE) {//���� ������ ������ - ������������
		buffer0[buffer0_size] = 0; // ��������� ���� � ����� ������ (������� � ���������)
		buffer0_complete = true; // ������� ��������� ������ ������ ��� ��. �����
	}
}

void analyze_result(){
	if (buffer0[0]=='i')
	{
		is_need_minus = false;
		temp = ((buffer0[1] + buffer0[2]) & 0x0F) | 0x30;
		show_result = true;
		show_error = false;
	}
	else if (buffer0[0]=='d')
	{
		if(buffer0[1]>buffer0[2]){
			is_need_minus = false;
			temp = ((buffer0[1] - buffer0[2]) & 0x0F) | 0x30;
			} else {
			is_need_minus = true;
			temp = ((buffer0[2] - buffer0[1]) & 0x0F) | 0x30;
		}
		show_result = true;
		show_error = false;
	}
	else if (buffer0[0]=='m')
	{
		is_need_minus = false;
		temp = ((buffer0[1] * buffer0[2]) & 0x0F) | 0x30;
		show_result = true;
		show_error = false;
	}
	else
	{
		show_error = true;
		show_result = false;
	}
}

void clean_buffer0 () {
	buffer0[0] = 0; buffer0_size = 0; buffer0_complete = false; // ������� ������
}

//LCD DISLAY
#define F_CPU 16000000

#define RS 0
#define RW 1
#define E 2

// PORT A = data port
// PORT B = 0b0000 E RW RS 0

void strob() {
	PORTB |= (1 << E);
	_delay_ms(0.5); // 5
	PORTB &= ~(1 << E);
}

void clear_cmd() {
	PORTA = (1 << 0); strob();
	_delay_ms(100);
}

void reset_cursor_cmd() {
	PORTA = (1 << 1); strob();
}

void init() {
	uint8_t DL = 4; // data width: 4bit/8bit
	uint8_t N = 3;  // 1/2 string
	uint8_t F = 2;  // 5x8
	
	PORTB &= ~(1 << E);
	PORTA = (1 << 5) | (1 << DL) | (1 << N) | (0 << F);
	strob();

	reset_cursor_cmd();
}

void shift_screen_setting_cmd(uint8_t inc, uint8_t screen) {
	uint8_t ID = 1; // inc address counter 0/1 - dec/inc
	uint8_t S = 0; 	// shift screen 0/1 off/on
	PORTA = (1 << 2) | (inc << ID) | (screen << S);
	strob();
}

void display_setting_cmd(uint8_t onDisp, uint8_t onCursor, uint8_t sqCursor) {
	uint8_t D = 2; // on display
	uint8_t C = 1; // 1 - cursor
	uint8_t B = 0; // 1 - black square
	PORTA = (1 << 3) | (onDisp << D) | (onCursor << C) | (sqCursor << B);
	strob();
}

void move_cursor_cmd(uint8_t dir) {
	PORTB &= ~(1 << E);
	uint8_t SC = 3; // 1/0 - move cursor/screen
	uint8_t RL = 2; // 1/0 - right/left
	PORTA = 0;
	
	PORTA |= (1 << 4) | (0 << SC) | (dir << RL);
	strob();
}

void next_string() {
	for (int i = 0; i < 26; i++)
	move_cursor_cmd(1);
}

void print_ASCII(char sym) {
	PORTB = (1 << RS);
	PORTA = sym;
	strob();
	PORTB &= ~(1 << RS);
}

void print_string(const char* str) {
	unsigned char i = 0;

	while (str[i] != '\0')
	print_ASCII(str[i++]);
}

int main() {
	
	USARTInit(MYUBRR);
	sei();
	DDRB = 255;
	DDRA = 255;
	PORTB = 0;
	
	init();
	clear_cmd();
	shift_screen_setting_cmd(1, 0);
	display_setting_cmd(1, 0, 0);
	
	USARTTransmitStringLn("Hi");
	print_string("Hi");
	
	while (1) {
		if(buffer0_complete){
			clear_cmd();
			analyze_result();
			print_string("You print: ");
			print_string(buffer0);
			next_string();
			if(show_result){
				if(is_need_minus){
					USARTTransmitString("Your result: ");
					USARTTransmitChar('-');
					USARTTransmitChar(temp);
					USARTTransmitStringLn("");
					print_string("Your result: ");
					print_ASCII('-');
					print_ASCII((char)temp);
				} else {
					USARTTransmitString("Your result: ");
					USARTTransmitChar(temp);
					USARTTransmitStringLn("");
					print_string("Your result: ");
					print_ASCII((char)temp);
				}
				} else if (show_error){
				USARTTransmitStringLn("You got error");
				print_string("You got error");
			}
			clean_buffer0();
		}
	}
	return 0;
}