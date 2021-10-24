.include "m16def.inc"

.def result = r21
.def change = r20
.def temp = r16
.cseg
.org 0
jmp Reset
.org 0x01C
jmp ADC_handler

Reset:
ldi temp, high(RAMEND)
out SPH, r16
ldi temp, low(RAMEND)
out SPL, r16
ldi temp, 0xff
out DDRD, r16
out DDRB, r16
;out DDRC, r16
//////////--TIMER--////////////////////////
ldi r16, ((1 << WGM00)|(1 << WGM01) | (1 << COM01) | (1 << CS01))
out TCCR0, r16
ldi r23,0
out OCR0,r23
//////////--ADC0--////////////////////////
ldi temp,((1 << ADLAR) | (1 << REFS0))
out ADMUX, r16
ldi temp, ((1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
out ADCSRA, r16
////////////////////////////////////////
ldi change, 0 //for change channel
sei

main :
jmp main

ADC_handler:
cli
push r16
in r16, SREG
push r16
push result
;in temp, ADCL
in result, ADCH
cpi change, 0
breq ADC_0_set
ldi change, 0
//////////////////////////
ldi r16,0
out DDRD, r16
ldi r16,0xFF
out DDRB,r16
out ocr0, result
///////////////////////////
ldi temp, ((1 << ADLAR) | (1 << REFS0) | (1 << MUX0)) // ADC1 set
out ADMUX, r16
ADC_settings:
ldi r16, ((1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
out ADCSRA, r16
pop result
pop r16
out SREG, r16
pop r16
reti

ADC_0_set:
ldi r16 ,0
out DDRB,r16
ldi temp, 0xff
out DDRD, r16
ldi change, 1
///////////////////////////
out portd,result
///////////////////////////
ldi temp, ((1 << ADLAR) | (1 << REFS0)) // ADC0
out ADMUX, r16
rjmp ADC_settings