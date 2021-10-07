.include "m16def.inc" 

.def temp = r16
.def sys = r20
.def try = r21

.equ one=0x28 
.equ two=0xB3
.equ three=0xBA 
.equ four=0x78 
.equ five=0xDA
.equ six=0xDB
.equ seven=0xA8
.equ eight=0xFB
.equ nine=0xFA
.equ zero=0xEB

.dseg // сегмент ОЗУ
Show:
.byte 4
;резервируем в озу 4 байта 
.cseg 
.org 0 
jmp start 
.org $012 
jmp TIM0_OVF 

start: 
 // сохраняем в ОЗУ, 1 ячейка
ldi try, eight
sts Show+1, try
ldi try, zero
sts Show+2, try
ldi try, three
sts Show+3, try

ldi temp, high(RAMEND) 
out sph, temp
ldi temp, low(RAMEND)
out spl, temp

;Порты D и А на выход, на нем висит наш индикатор
ldi temp, 0xFF 
out DDRD, temp
out DDRA, temp

ldi temp, 0b00000011
out TCCR0, temp 
ldi temp, 0b00000001 ; TOV0 = 1 бит прерывания по переполнению таймера 0
out TIFR, temp
out TIMSK, temp
out TIFR, temp
out TIMSK, temp
ldi temp, 0xFF ;в тикающий регистр
out TCNT0, temp 
ldi sys, 0b10000000
sei 

Loop: 
ldi try, six
sts Show, try
call Delay
ldi try, nine
sts Show, try
call Delay
ldi try, three
sts Show, try
call Delay
ldi try, one
sts Show, try
call Delay
rjmp Loop 

Delay:
ldi r17, 255
ldi r18, 255
ldi r19, 5
Delay_loop :
dec r17
brne Delay_loop 
dec r18
brne Delay_loop 
dec r19
brne Delay_loop 
ret

TIM0_OVF: 
lsr sys 
cpi sys, 0b00001000 ;Проверяем, не ушли ли за пределы сегментов отображения на индикаторе
breq reset ; Если ушли, переходим на обнуление
out PORTD, sys ; Если нет, выводим значение sys в порт
ld temp, X+ ;загрузим значение из X(Visible) в temp
ld temp, X  
out PORTA, temp 

rebuild: 
ldi temp, 0x00 ;Взводим тикающий регистр
out TCNT0, temp
reti

reset:
ldi sys, 0b10000000 
out PORTD, sys 
ldi XH, high(Show) 
ldi XL, low(Show)
ld temp, X 
out PORTA, temp  
rjmp rebuild 
