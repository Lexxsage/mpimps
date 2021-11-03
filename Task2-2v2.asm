.def flag=r16

.def port0=r17
.def port1=r18
.def temp=r19

.cseg
.org 0x00
jmp init

.org 0x1C//ADC Conversion Complete Handler
jmp ADC_handler

.org 100

ADC_handler:
	call handler_part
	reti

init:
	ldi temp, LOW(RAMEND)
	out SPL, temp
	ldi temp, HIGH(RAMEND)
	out SPH, temp 
	//LED
	sbi DDRB, 3 
	//another LED
	sbi DDRD, 7 

	ldi flag, 0 
	sei 
	
	ldi	temp, (1 << CS00) | (1 << WGM00)|(1 << WGM01)|(1 << COM01) // fast PWM
	out TCCR0, temp

	ldi	temp, (1 << CS20) | (1 << WGM20)|(1 << WGM21)|(1 << COM21) // fast PWM
	out TCCR2, temp

	ldi temp, (1 << REFS0) | (1 << ADLAR) // read 0 pin ADC
	out ADMUX, temp

	ldi temp, (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)
	out ADCSRA, temp

start:
	rjmp start

handler_part:
	cpi flag, 0
	brne first_port_flag
	
zero_port_flag:
	ldi temp, (1 << REFS0)|(1 << ADLAR)|(1 << MUX0) //change 0 to 1 port
	out ADMUX, temp
	in port0, ADCH
	out OCR0, port0
	ldi flag, 1
	ret

first_port_flag:
	ldi temp, (1 << REFS0)|(1 << ADLAR) //change 1 to 0 port
	out ADMUX, temp
	in port1, ADCH
	out OCR2, port1
	ldi flag, 0
	ret