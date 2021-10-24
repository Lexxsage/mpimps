.equ pull_up_DDRD=0b00001000
.equ pull_up_portD=0b00000100//vcc
.equ output_portB=0b00001000
.def button_prev=r19
.def button_act=r20
.def LED_status=r21
.def tmp=r17
.def tmp2=r18

.org 0
jmp init

.org 100

init:
//инициализаци€ стека
	ldi r16,Low(RAMEND)
	out SPL,r16
	ldi r16,High(RAMEND)
	out SPH,r16

	cbi DDRD, 2
	sbi portd, 2
	sbi DDRD, 3
	cbi portd, 3

	//LED
	sbi DDRB, 3
	sbi PORTB,3 
	ldi button_prev, pull_up_portD

start:
	in button_act, PIND
	call Delay

	mov tmp2, button_act
	ldi tmp, pull_up_portD
	andi tmp2, pull_up_portD//надо ли €вно через логическое и с маской, выдел€ющей 1 бит в баттон_акт?
	eor tmp2, tmp//change necessary bit
	mov tmp, button_prev

	mov button_prev, button_act
	
	and tmp, tmp2 //prev & !act == true <=> prev == 1 && act == 0
	ldi tmp2, pull_up_portD
	eor tmp, tmp2//xor
	cpi tmp, pull_up_portD//equal if ((prev == 1 && act == 0)) -> else

	brne else
	rjmp start

else:
	;sleep
	ldi tmp, output_portB
	eor LED_status, tmp//xot bit
	out PORTB, LED_status
	rjmp start

Delay:
	ldi r22, 0xFF ;load constant 
	ldi r23, 0x0F

Delay_loop:
	subi r22, 1 ;-1
	sbci r23, 0 ;carry flag
	brne Delay_loop ;branch, if not equal (Z!=0)
	ret