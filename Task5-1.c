#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#define RS 0
#define RW 1
#define E 2

// PORT A = data port
// PORT B = E RW RS

//Дрыгом напряжения на этой линии мы даем понять дисплею
// что нужно забирать/отдавать данные с/на шину данных.
void strob() {
	PORTB |= (1 << E);
	_delay_ms(0.5); // 5
	PORTB &= ~(1 << E);
}

//Очистка экрана.
void clear_cmd() {
	PORTA = (1 << 0); strob();
	_delay_ms(100);
}


//сброс сдвигов, Счетчик адреса на 0
void reset_cursor_cmd() {
	PORTA = (1 << 1); strob();
}

//инициализация экрана
void init() {
	// 5 - говорим что выбираем число линий/ ширину 4/8 бить и размер картинки
	uint8_t DL = 4; // data width: 8bit
	uint8_t N = 3;  // 2 string
	uint8_t F = 2;  // 5x8
	
	PORTB &= ~(1 << E);
	PORTA = (1 << 5) | (1 << DL) | (1 << N) | (0 << F);
	strob();

	reset_cursor_cmd();
}

// 2- Настройка сдвига экрана и курсора
void shift_screen_setting_cmd(uint8_t inc, uint8_t screen) {
	uint8_t ID = 1; // inc address counter
	uint8_t S = 0; 	// shift screen on
	PORTA = (1 << 2) | (inc << ID) | (screen << S);
	strob();
}

//3 - Настройка режима отображения
void display_setting_cmd(uint8_t onDisp, uint8_t onCursor, uint8_t sqCursor) {
	uint8_t D = 2; // 1 - on display
	uint8_t C = 1; // 1 - cursor
	uint8_t B = 0; // 1 - black square
	PORTA = (1 << 3) | (onDisp << D) | (onCursor << C) | (sqCursor << B);
	strob();
}

//4 - Сдвиг курсора или экрана, в зависимости от битов
void move_cursor_cmd(uint8_t dir) {
	PORTB &= ~(1 << E);
	uint8_t SC = 3; // 0 - move cursor
	uint8_t RL = 2; // 1/0 - right/left
	PORTA = 0;
	
	PORTA |= (1 << 4) | (0 << SC) | (dir << RL);
	strob();
}

void next_string() {
	for (int i = 0; i < 40; i++)
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


//добавляем смайлик =)
void add_new_symbol() {
	PORTB = (0 << RS);
	// Выбираем в CGRAM адрес 0х08 — как раз начало второго символа 
	// что на один символ уходит 8 байт
	PORTA = 0b01001000; strob();
	PORTB = (1 << RS);
	//сам =)
	PORTA = 0b00000010;	strob();
	PORTA = 0b00000001;	strob();
	PORTA = 0b00011001;	strob();
	PORTA = 0b00000001;	strob();
	PORTA = 0b00011001;	strob();
	PORTA = 0b00000001;	strob();
	PORTA = 0b00000010;	strob();
	PORTA = 0b00000000;	strob();

	PORTB = (0 << RS);
	//переключение адреса на DDRAM и указатель на адрес 0000000
	// — первый символ в первой строке.
	PORTA = 0b10000000; strob();
}

void print_ASCII_by_num(uint8_t num, uint8_t str, uint8_t coord) {
	reset_cursor_cmd();
	for (uint8_t i = 0; i < coord; i++)
		move_cursor_cmd(1);
	if (str)
		next_string();
	PORTB = (1 << RS);
	PORTA = num;
	strob();
	PORTB &= ~(1 << RS);
}

int main() {
	DDRB = 255; //0xff
	DDRA = 255; //0xff
	PORTB = 0;
	
	init();
	clear_cmd();
	shift_screen_setting_cmd(1, 0);
	display_setting_cmd(1, 0, 0);
	
	print_string("Counter: ");
	add_new_symbol();
	//данные (RS=1), код 01 — именно в него мы засунули наш =).
	print_ASCII_by_num(1, 0, 15);
	
	for (int i = 0; ;++i) {
		unsigned int a = i;
		int digits = 0;
		while(a) {
			// % - остаток от деления целых чисел
			print_ASCII_by_num(a % 10 + '0', 0, 14 - digits++);
			a /= 10;
		}
		if (i == 300)
		i = -1;
	}
	return 0;
}