#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define SPI_MISO 6
#define SPI_MOSI 5
#define SPI_SCK 7
#define SPI_SS 4
char dg = 8;

void SPI_init(void)
{
	DDRB |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);//|(0<<SPI_MISO);  //����� SPI �� �����
	PORTB |= (1<<SPI_MOSI)|(1<<SPI_SCK)|(1<<SPI_SS);//|(1<<SPI_MISO); //������ �������
	SPCR = (1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(1<<SPR1)|(0<<SPR0);////������� ����, ������� �������, �������� ������� ����� ������, SPI Mode 0,

}

//�������, ������� ����� ���������� ������� � ������ �������.
//���������� ����� ������������ ����, � � ���� ������� ������� ������ ���� � ������� SPDR � ����� ����������� �������� ������ ���� � ���� SPI ���������
void SPI_WriteByte(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));//���� ���������� �� SPI, ��������������� � 1 �� ��������� �������� ����� ������.
}

//������ ������� ����� ���������� ���� �� ������ � ����, � � ����������� ������� ����������. 
//�������� ����������� ����� ����� �������� � ���� ������
//������� ������� ����� CS, ����� ������� ���� ����������, ����� ��������� ���� � ������� ��������, � ����� ���� ������, �� � � ����� �������� CS.
void Sendto7219(uint8_t rg, uint8_t dt)
{
	PORTB &= ~(1<<SPI_SS);//�������� ����� SS ��� ������ ����������, ����� �������� ���� � ������� ��������, ����� ���� ������
	SPI_WriteByte(rg);
	SPI_WriteByte(dt);
	PORTB |= (1<<SPI_SS);
}

//��������� �� ��� ��������������� ������� ��������� ������ �������. � �������� ������ �������� � ��� ��� ����� ��������� ����� ��������������� ��������, 
//��������������� ��������� �������. � ���� 0xF � ��� � ��� ��� ������� ������� (� ������� ���� �� �������� ��� "blank").
void Clear_7219(void)
{
	char i = dg;
	do {
		Sendto7219(i, 0xF); //CHAR BLANK
	} while (--i);
}

uint8_t data[8] = {
	0b11101101,
	0b00010010,
	0b00001100,
	0b00010010,
	0b11101101,
	0b00010010,
	0b00001100,
	0b00010010
};

uint8_t data_num[8] = {
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8
};

int main(void)
{
	int helper;
	SPI_init();
	//Send_7219(0x09, 0xFF); //������� ����� �������������
	Sendto7219(0x0B, dg - 1); //������� �������� ����������
	Sendto7219(0x0A, 0x05); //�������
	Sendto7219(0x0C, 1); //������� ���������
	while(1)
	{
		for (int i=1;i<9;i++)
		{
			Sendto7219(i,data[i-1]);
		}
		_delay_ms(400);
		helper=data[0];
		for (int b=0;b<7;b++)
		{
			data[b]=data[b+1];
		}
		data[7]=helper;
		Clear_7219();
	}

}