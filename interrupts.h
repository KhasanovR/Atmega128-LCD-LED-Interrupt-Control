char interrupt;


ISR (INT2_vect){
	interrupt = 2;
}


ISR(INT1_vect){
	interrupt = 1;
}


ISR (INT0_vect){
	interrupt = 0;
}


void Interrupt_Init(void){
	EIMSK = 0x07; 
	EICRA = 0xFC;
	sei(); 
}


void INT0_func()
{
	LED_Right_Shift();

	char str[] = "INT0 FE";

	LCD_Clear();
	LCD_pos(0, 0); 
	LCD_STR(str);
	for (int i = 0; i <15; i++)
	{
		LCD_Shift(LEFT);
		_delay_ms(200);	
	}
	LCD_Clear();
	interrupt = 100;	
	Cursor_Home();
}


void INT1_func()
{	
	char str[] = "INT1 RE";

	LCD_Clear();
	LCD_pos(0,0); 
	LCD_STR(str);
	
	for (int i=0;i<5;i++)
	{
		LED_Blink();
		LCD_Blink();
	}

	LCD_ON();
	LCD_Clear();
	interrupt = 100;	
	Cursor_Home();
}


void INT2_func()
{
	LED_OFF();

	char str[] = "INT2 LL";
	LCD_Clear();
	LCD_pos(0,0); 
	LCD_STR(str);
	_delay_ms(700);
	


	LCD_Clear();
	LCD_pos(0,1); 
	LCD_STR(str);
	_delay_ms(700);
	
	for (int i = 0; i <15; i++)
	{
		LCD_Shift(LEFT);
		_delay_ms(200);	
	}
	
	LCD_Clear();
	interrupt = 100;	
	Cursor_Home();
}