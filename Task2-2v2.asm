.def flag=r16

.def port0=r17
.def port1=r18
.def temp=r19

.macro outi
LDI 	temp,@1 			
OUT 	@0,temp 	
.endm

.cseg
.org 0
jmp init

.org 0x1C//ADC Conversion Complete Handler
jmp ADC_handler

.org 100

ADC_handler:
	push r15//CPI
	in r15, sreg
	
	call handler_part
	
	out sreg, r15
	pop r15
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
	
	outi TCCR0, (1 << CS00) | (1 << WGM00)|(1 << WGM01)|(1 << COM01) // WGM00 WGM01 == 11 => PWM

	outi TCCR2, (1 << CS20) | (1 << WGM20)|(1 << WGM21)|(1 << COM21) // WGM00 WGM01 == 11 => PWM

	outi ADMUX, (1 << REFS0) | (1 << ADLAR) //Изначально - с 0-го пина считываем АЦП

	outi ADCSRA, (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)

start:
	rjmp start

handler_part:
	cpi flag, 0
	brne first_port_flag
	
zero_port_flag:
	outi ADMUX, (1 << REFS0)|(1 << ADLAR)|(1 << MUX0)//change port configure - Muxes bits - change 0 to 1 port
	in port0, ADCH
	out OCR0, port0
	ldi flag, 1
	ret

first_port_flag:
	outi ADMUX, (1 << REFS0)|(1 << ADLAR) //change port configure - Muxes bits - change 1 to 0 port
	in port1, ADCH
	out OCR2, port1
	ldi flag, 0
	ret