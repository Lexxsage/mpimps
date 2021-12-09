#define F_CPU 8000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define BUFFER0_MAX_SIZE 80 // ����. ����� ������
char buffer0[BUFFER0_MAX_SIZE]; // ������
unsigned int buffer0_size = 0; //������� ������ ������
bool buffer0_complete = false; //������ � ������ �������� ���������

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
	UCSRB =  (1<<RXCIE) | 1<<RXEN | 1<<TXEN;
	
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
			case (char)13://CR - ��������� ������
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

//  ��������� �����
/*char USARTReceiveChar(void) {
	unsigned char tmp;
	unsigned char saveState = SREG;
	tmp = usartRxBuf;
	usartRxBuf = 0;
	SREG = saveState;
	return tmp;
}*/

void clean_buffer0 () {
	buffer0[0] = 0; buffer0_size = 0; buffer0_complete = false; // ������� ������
}

int main(void) {
	USARTInit(MYUBRR);
	sei();
	USARTTransmitStringLn("Hi");
	while (1) {
		if(buffer0_complete){
			USARTTransmitString("You send: ");
			USARTTransmitStringLn(buffer0);
			clean_buffer0();
		}
	}
}