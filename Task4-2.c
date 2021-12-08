#define F_CPU 8000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

uint8_t temp = 0;

#define BUFFER0_MAX_SIZE 40 // ����. ����� ������
char buffer0[BUFFER0_MAX_SIZE]; // ������
unsigned int buffer0_size = 0; //������� ������ ������
bool buffer0_complete = false; //������ � ������ �������� ���������
char buffer_res[BUFFER0_MAX_SIZE];

bool show_result = false;
bool show_error = false;
bool is_need_minus = false;

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

//  ��������� �����
char USARTReceiveChar(void) {
	//  ���������������, ����� ������� ��������
	while(!(UCSRA & (1<<RXC)));
	return UDR;
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

void calc_result(){
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

void analyze_result(){
	if (buffer0[0]=='i')
	{
		for(int i = 0; i< buffer0_size; i++){
			buffer_res[i] = buffer0[i+1];
		}
		show_result = true;
		show_error = false;
	} else if (buffer0[0]=='d')
	{
		for(int i = 0; i <buffer0_size - 1; i++){
			buffer_res[i] = buffer0[buffer0_size-i-1];
		}
		show_result = true;
		show_error = false;
	} else if (buffer0[0]=='s')
	{
		if(buffer0[1] == 'u'){			
			for(int i = 2; i < buffer0_size; i++){
				for (int k = buffer0_size-1; k> i; k--){
					if (buffer0[k] < buffer0[k-1]){
						char let = buffer0[k];
						buffer0[k] = buffer0[k-1];
						buffer0[k-1] = let;
					}
				}
			}
			for(int i = 0; i< buffer0_size; i++){
					buffer_res[i] = buffer0[i+2];
			}
			show_result = true;
			show_error = false;
		} else if (buffer0[1] == 'd'){
			for(int i = 2; i < buffer0_size; i++){
				for (int k = buffer0_size-1; k> i; k--){
					if (buffer0[k] > buffer0[k-1]){
						char let = buffer0[k];
						buffer0[k] = buffer0[k-1];
						buffer0[k-1] = let;
					}
				}
			}
			for(int i = 0; i< buffer0_size; i++){
				buffer_res[i] = buffer0[i+2];
			}
			show_result = true;
			show_error = false;
		}
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

void clean_buffer_res() {
	buffer_res[0] = 0; // ������� ������ ����������
}

int main(void) {
	USARTInit(MYUBRR);
	sei();
	USARTTransmitStringLn("Hi");
	while (1) {
		if(buffer0_complete){
			analyze_result();
			USARTTransmitString("You print: ");
			USARTTransmitStringLn(buffer0);
			if(show_result){
				/*if(is_need_minus){
					USARTTransmitString("Your result: ");
					USARTTransmitChar('-');
					USARTTransmitChar(temp);
					USARTTransmitStringLn("");
				} else {*/
					USARTTransmitString("Your result: ");
					USARTTransmitStringLn(buffer_res);
					//USARTTransmitStringLn("");
				//}
			} else if (show_error){
				USARTTransmitStringLn("You got error");
			} 
			clean_buffer0();
			clean_buffer_res();
		} 
	}
}