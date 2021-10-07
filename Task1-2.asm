.org 0x00
rjmp start

.equ zero=0xEB
.equ one=0x28
.equ two=0xB3
.equ three=0xBA
.equ four=0x78
.equ five=0xDA
.equ six=0xDB
.equ seven=0xA8
.equ eight=0xFB
.equ nine=0xFA

start: ;
ldi r16, Low(RAMEND) ; stack pointer
out SPL, r16
ldi r16, High(RAMEND)
out SPH, r16

ldi r16, 0xff
out DDRD,r16
out DDRA,r16

TurnOn:
ldi r17, seven
out porta,r17
call Delay
rjmp TurnOn

Delay:
ldi r16, 0xFF
ldi r17, 0xFF
ldi r18, 0x05
Delay_loop:
subi r16,1 
sbci r17,0
sbci r18,0
brne Delay_loop 
reti