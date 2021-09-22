.org 0x00


.org 0x0C
jmp time1_overflow

ldi r16, Low(RAMEND) ; stack pointer
out SPL, r16
ldi r16, High(RAMEND)
out SPH, r16
sbi DDRB,3
start:

sbi portb,3
ldi r16,(1<<WGM00)|(1<<WGM01)|(1<<COM01)|(1<<CS00)
out tccr0,r16

ldi r16,128
out ocr0,r16

ldi r16,0
out tccr1a,r16

ldi r16, (1<<WGM12)|(1<<CS10)
out tccr1b,r16

ldi R16,0x7A
out OCR1AH,R16
ldi R16,12
out OCR1AL,R16

ldi R16,(1<<OCIE1A)
out TIMSK,R16

sei ; Enable interrupts

loop:
rjmp loop

time1_overflow:
cpi R16,30
BRNE time2_overflow
out OCR0,R17
inc R17
BRNE time_overflow_end
ldi R16,1
rjmp time_overflow_end

time2_overflow:
out OCR0,R17
dec R17
BRNE time_overflow_end
ldi R16,0

time_overflow_end:
reti