#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD_LED.h"
#include "interrupts.h"

#define F_CPU 14745600UL


int main (void)
{
	interrupt = 100;
	Interrupt_Init(); 
	DDRB = 0xFF; 
	DDRD = 0x00; 

	LCD_Init(); 
	LCD_pos(0, 0); 
	 
	while(1){
		switch(interrupt){
			case 0: 
			{
				INT0_func();
				break;
			}
			case 1:  
			{
				INT1_func();
				break;
			}
			case 2: 
			{
				INT2_func();
				break;
			}
			default:
				LED_Left_Shift();
				break;
		}
	}
	
	return 0;
}
