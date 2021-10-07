.org 0x00
    rjmp main
.org 0x01C
    rjmp adc_int

main:
    ; init stack
    ldi r16, low(RAMEND)
    out SPL, r16
    ldi r16, high(RAMEND)
    out SPH, r16

    ; init port, data direction
    sbi DDRB, 3

    ; init timer 1, 1MHz, Fast PWM
    ldi r16, ((1 << WGM00)|(1 << WGM01) | (1 << COM01) | (1 << CS01))
    ldi r17, (1 << TOIE0)
    out TCCR0, r16
    sts TIMSK, r17
	;ldi r18,0
    ;out OCR0,r18

    ; ADC setup
    ldi r16, ((1 << ADLAR) | (1 << REFS0)|(1<<REFS1) ) 
    ; Setup ADC with internal AREF, left adgusted ADC value on pin PORTA[0]
    out ADMUX, r16

    ldi r16, (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)
    ; 0b11101111 Enable ADC, start conversion, auto trigger, prescaler 128
    out ADCSRA, r16

    sei

loop:
    rjmp loop

adc_int:
    ;in r18, ADCH
	;out OCR0, r18
    in r16, ADCH
    out OCR0, r16
    reti
	;выход с прерывания