#define F_CPU 8000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define BUFFER0_MAX_SIZE 80 // макс. длина строки
char buffer0[BUFFER0_MAX_SIZE]; // строка
unsigned int buffer0_size = 0; //текущий размер строки
bool buffer0_complete = false; //строка в буфере получена полностью

void USARTInit(unsigned int ubrr) {
	//  нормальный асинхронный двунаправленный режим работы
	//  UBRR = f / (16 * band)
	//  Установка скорости
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)(ubrr);
	
	//  RXC         - завершение приёма
	//  |TXC        - завершение передачи
	//  ||UDRE      - отсутствие данных для отправки
	//  |||FE       - ошибка кадра
	//  ||||DOR     - ошибка переполнение буфера
	//  |||||PE     - ошибка чётности
	//  ||||||U2X   - Двойная скорость
	//  |||||||MPCM - Многопроцессорный режим
	//  ||||||||
	//  76543210
	UCSRA = 0;
	
	//  RXCIE       - прерывание при приёме данных
	//  |TXCIE      - прерывание при завершение передачи
	//  ||UDRIE     - прерывание отсутствие данных для отправки
	//  |||RXEN     - разрешение приёма
	//  ||||TXEN    - разрешение передачи
	//  |||||UCSZ2  - UCSZ0:2 размер кадра данных
	//  ||||||RXB8  - 9 бит принятых данных
	//  |||||||TXB8 - 9 бит переданных данных
	//  ||||||||
	//  76543210
	
	//  разрешен приём и передача данных по UART
	UCSRB =  (1<<RXCIE) | 1<<RXEN | 1<<TXEN;
	
	//  URSEL        - всегда 1
	//  |UMSEL       - режим: 1-синхронный 0-асинхронный
	//  ||UPM1       - UPM0:  1 чётность
	//  |||UPM0      - UPM0:  1 чётность
	//  ||||USBS     - стоп биты: 0-1, 1-2
	//  |||||UCSZ1   - UCSZ0: 2 размер кадра данных
	//  ||||||UCSZ0  - UCSZ0: 2 размер кадра данных
	//  |||||||UCPOL - в синхронном режиме - тактирование
	//  ||||||||
	//  76543210
	//  8-битовая посылка, 2 стоп бита
	UCSRC = 1<<URSEL | 1<<UCSZ0 | 1<<UCSZ1;
}

//  Отправка байта
void USARTTransmitChar(char c) {
	//  Устанавливается, когда регистр свободен
	while(!( UCSRA & (1<<UDRE)));
	UDR = c;
}

//  Отправка строки
void USARTTransmitString(const char str[]) {
	register char i = 0;
	while(str[i]) {
		USARTTransmitChar(str[i++]);
	}
}

//  Отправка строки
void USARTTransmitStringLn(const char str[]) {
	USARTTransmitString(str);
	USARTTransmitChar((char)13);
	USARTTransmitChar((char)10);
}

ISR(USART_RXC_vect) {
		char ch = UDR;
		switch (ch){
			case (char)10://перевод строки игнорируем
			break;
			case (char)13://CR - окончание строки
			buffer0[buffer0_size] = 0; // добавляем ноль в конец строки (признак её окончания)
			buffer0_complete = true; // признак получения полной строки для гл. цикла
			break;
			default:
			buffer0[buffer0_size] = ch; // если символ - не конец строки - добавляем символ в буфер
			buffer0_size++;
			break;
		}
		if (buffer0_size == BUFFER0_MAX_SIZE) {//если строки размер - максимальный
			buffer0[buffer0_size] = 0; // добавляем ноль в конец строки (признак её окончания)
			buffer0_complete = true; // признак получения полной строки для гл. цикла
		}
}

//  Получение байта
/*char USARTReceiveChar(void) {
	unsigned char tmp;
	unsigned char saveState = SREG;
	tmp = usartRxBuf;
	usartRxBuf = 0;
	SREG = saveState;
	return tmp;
}*/

void clean_buffer0 () {
	buffer0[0] = 0; buffer0_size = 0; buffer0_complete = false; // очистка буфера
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