.def temp = r16				
.def razr = r17		
.def value = r18
.def row = r19		
.def col = r20			

start:
	//инициализация стэка
    ldi temp, Low(RAMEND)	
	out SPL, temp			
	ldi temp, High(RAMEND)	
	out SPH, temp			
	//сегменты 7-ми сегментника на А
	ldi temp, 0xFF			
	out DDRA, temp		
	//разряд 7-ми сегментника на В	
	ldi temp, 0x01			
	out DDRB, temp
	//кнопки на D			
	ldi temp, 0x0F			
	out PORTD, temp			
	//значение разряда
	ldi value, 0			
	//записываем цифры в SRAM
	clr r27					//очищаем старший байт
	ldi r26, $60			//устанавливаем $60 в младший байт
	ldi temp, 0x3F			//записываем сегменты для 0
	st X+, temp				//сохраняем значение temp в SRAM по адресу $60
	ldi temp, 0x06			//записываем сегменты для 1
	st X+, temp				//сохраняем значение temp в SRAM по адресу $61
	ldi temp, 0x5B			//записываем сегменты для 2
	st X+, temp				//сохраняем значение temp в SRAM по адресу $62
	ldi temp, 0x4F			//записываем сегменты для 3
	st X+, temp				//сохраняем значение temp в SRAM по адресу $63
	ldi temp, 0x66			//записываем сегменты для 4
	st X+, temp				//сохраняем значение temp в SRAM по адресу $64
	rjmp loop				

//переписываем значение разряда
rewrite:			
	clr value				
	ldi temp, 2			//чтобы бегать по строкам	

//переписываю значение строки
rewrite_row:			
	add value, temp			//+2, значение на следующей строке(1=3, 2=4)
	lsr row					//логический сдвиг вправо(делю на 2)
	cpi row, 0b0000			//закончили проход по строкам?
	brne rewrite_row		//если нет, то продолжаем

//переписываю значение колонки
rewrite_column:			
	inc value				//+1, значение на следующей колонке(1=2, 3=4)
	lsr col					//логический сдвиг вправо(делю на 2)
	cpi col, 0b0010			//колонки закончились?
	brne rewrite_column		//если нет, то бежим дальше
	sub value, temp			//вычитаем лишнюю 2ку, поправка вычисленного значения 

loop:						
	//выводим значение на 7-сегментник
	mov temp, value				//копирую значение
	call Digit					//переходим в диджит и получаем нужный код для цифры
	out PORTA, temp				//выводим нужный код на А
	ldi temp, 0x01				//на первый разряд выводим цифру
	out PORTB, temp				
	
//инициализируем начальное значение строки
init_row:
	call Delay					
	ldi row, 0b0001				//начинаем с первой строки

//проход по строчкам
row_run:						
	ldi temp, 0b1111		//ставим 1
	eor temp, row			//1110
	out PORTD, temp			//активируем 1110
	out DDRD, row			//поставили 1 на DDRD
	ldi col, 0b0100 		//записали начальное значение колонки

//проход по колонкам
column_run:					
	in temp, PIND			//считываем значение с пина
	and temp, col			//делаем маску, чтобы проверить нажатие(там будет 0)
	eor temp, col			//перевели в 1
	cp temp, col			//сравнили с колонкой
	breq rewrite			//если равно то переписываем
	lsl col					//логический сдвиг влево(умножаю на 2)
	cpi col, 0b10000		//сравниваю с крайним значением(вышли за столбцы)
	breq incr_row		    //переходим к следующей строке если равно
	rjmp column_run			//продолжаю проход по столбцам

//переходим к следующей строке
incr_row:				
	lsl row					//логический сдвиг влево(умножаю на 2)
	cpi row, 0b0100			//сравниваю с крайним значением(вышли за строки)
	breq init_row		    //перезапускаю проход по строкам
	rjmp row_run			//продолжаю проход по строкам

Digit:
//получаем из памяти кодировку полученной цифры						
	clr r27					
	ldi r26, $60			
	add r26, temp		
	ld temp, X				
	ret						

Delay:						
	ldi temp, 0xFF			
	ldi razr, 0x0F		

Delay_loop:					
	subi temp, 1			
	sbci razr, 0			
	brne Delay_loop			
	ret		