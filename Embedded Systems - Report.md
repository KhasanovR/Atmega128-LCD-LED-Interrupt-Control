# Embedded Systems 
> Home assignment #2

## Report

### Team:

| Name                       | ID       |
| -------------------------- | -------- |
| Bokhodir Urinboev (Leader) | U1610249 |
| Oybek Amanov               | U1610176 |
| Rakhmatjon Khasanov        | U1610183 |

## Task

Write a program that works like this:
1. Use HW Interrupt 0 ~ 2
2. Initially, the LEDs are turning on one by one from left to right.
3. Using Int0 falling edge trigger: When INT0 happens sequentially turn on LED from right
    to left. Display ‘INT0 FE’ on the LCD and moving the sentence to left
4. Use INT1 Rising edge trigger. When INT1 happens blinks LED 5 times. Display ‘INT1 RE’
    on the LCD and blink it 5 times
5. Use INT2 low level trigger. When INT2 happens turn off LEDs. Display ‘INT2 LL’on the
    LCD and shift the sentence sequentially from the first line to the next line.

#### Run: ####

```shell
avr-gcc -Wall -g -Os -mmcu=atmega128 -o main.bin main.c

avr-objcopy -j .text -j .data -O ihex main.bin main.hex

simulide
```

Demo video can be found in the folder `Demo.mp4`.

## Algorithms explanation

In our approach, we split the total features into three separate files:

* LCD_LED.h ->  the LCD connected to the PORT A and PORT G, and the LED connected to the PORT B
* interrupts.h -> for doing tasks from 3 to 5
* main.c -> all parts joined into one main file

```c
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LCD_LED.h"
#include "interrupts.h"

#define F_CPU 14745600UL
```

The general CPU frequency in our case is 14.74 MHz and  all the required libraries are provided, such as operating with AVR input / output, allowing delays in our programs, allowing interrupts and interrupt handlers in our system and user-defined "interrupts.h" and "LCD LED.h."

```c
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
```

We need to activate Interrupt 0 falling edge trigger, Interrupt 1 rising edge trigger, and Interrupt 2 low level trigger. For this reason, we need to send the value 0x01 to the EIMSK register to allow INT0, INT1, INT2.

| Bit           | 7    | 6    | 5    | 4    | 3    | 2    | 1    | 0    |       |
| ------------- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ----- |
|               | INT7 | INT6 | INT5 | INT4 | INT3 | INT2 | INT1 | INT0 | EIMSK |
| Read/Write    | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  |       |
| Initial Value | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    |       |

Then the value 0x0E (0000 1110) has to be sent to the Register EICRA, which is responsible for allowing different level interrupts:

| Bit           | 7    | 6    | 5    | 4    | 3    | 2    | 1    | 0    |       |
| ------------- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ----- |
|               | ISC7 | ISC6 | ISC5 | ISC4 | ISC3 | ISC2 | ISC1 | ISC0 | EICRA |
| Read/Write    | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  | R/W  |       |
| Initial Value | 0    | 0    | 0    | 0    | 0    | 0    | 0    | 0    |       |

| ISCn1 | ISCn0 | Description                                                  |
| ----- | ----- | ------------------------------------------------------------ |
| 0     | 0     | The low level of INTn generates an Interrupt request         |
| 0     | 1     | Reserved                                                     |
| 1     | 0     | The falling edge of INTn generates asynchronously an Interrupt request |
| 1     | 1     | The raising edge of INTn generates asynchronously an Interrupt request |

Finally, the Global Interrupt Enable bit is activated which is the I bit in the SREG register (Status register) by
calling the sei() function or by manually: SREG |= 0x80.

| Bit  | D7   | D6   | D5   | D4   | D3   | D2   | D1   | D0   |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| SREG | I    | T    | H    | S    | V    | N    | Z    | C    |

In order to do this, we have to switch the LEDs from right to left, then show the text "INT0 FE" on the LCD and shift it from right to left. To do this, we call a special function in the LCD_LED.h that allows us to transform the LEDs from right to left. Then we set the initial location of the text on the first line and cell number 0.  After that, we call the function LCD_Shift() (that is specified in LCD_LED.h, which transfers the contents of the display to the side that is given as a parameter (right or left). If you can see, we're changing our LCD output to the left 15 times. Before that, we have a short pause of 200ms to make sure that everything in the LCD is done and only then can we clear the screen and move the cursor back to the original location of (0, 0) by calling the Cursor_Home() feature. Finally, we need to get back from the controller to finish the execution.

```c
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
```

The main objective of this feature is to enable the LED to transform from right to left with a delay of 300ms. In order to do this, the corresponding values for the 8 LEDs must be issued to the PORTB. Next, we describe the path of performance by providing the value 0xFF to the DDRB. After this, using the loop to write down the value of the LED variable to the PORTB. Since PORTB often has an 8-bit register, it is really convenient to deal with variables to adjust the state of each Lead. Then we'll only change the value of the LED variable to make sure the leftmost bit is set to 1.

```c
void LED_Right_Shift(void)
{
	char LED;
	int i;
	LED = 0x7f; 
	for (i =0 ; i <8 ; i++)
	{
		PORTB = LED;
		LED = (LED>>1) | 0x80;
		_delay_ms(300);
	}
}
```

This feature helps one to compose the next character at every stage on the LCD pad. The algorithm works by sending a special order to the LCD by defining the corresponding address of the given bit that can be determined using the following formula: (col + row * 0x40). This method can be proved by the way the LCD screen address is built.

```c
void LCD_pos(unsigned char row, unsigned char col)
{
	LCD_Comm(0x80 | (row + col*0x40));	
}
```

These functions enable us to write a particular character on the LCD pad. LCD only recognizes the ASCII value for the character that we can send to the LCD by assigning a particular meaning to the predefined bits.

```C
void LCD_CHAR(char c)
{
	LCD_Data(c);
	_delay_ms(2);
}
```

LCD_Str() calls LCD_Char() function until it reaches the end of the string character ‘\0’.

```c
void LCD_STR(char *str)
{
	while(*str !=0)
	{
		LCD_CHAR(*str);
		str++;
	}
}
```

The corresponding feature functions in the same way that after setting 3 necessary bits, which are Allow Bit, RS Bit (Register Select) and RW Bit (Read / Write), the correct data is transferred to the PORTA. Enable bit should be set to 1 to encourage the LCD to be read or written to and set to 0 again at the end of the operation. The RS bit specifies what we're going to write, either a command or a record. The RW bit determines what has to be associated with the data recorded, interpreted or sent to the PORTA. In our instances, we need to write to the LCD, so RW = 0, while for LCD Data) (the RS bit is equal to 1 and for LCD Comm() it is equal to 0. It should be remembered that before and after writing the data to the PORTA, a certain amount of time should be waited for the different errors to be prevented when setting the above-mentioned bits. When the value of 0x01 is sent to the LCD screen as a signal, its contents will be removed. When submitting a value of 0x02, set the direction of the cursor to be at the start of the screen at the address (0, 0).

```c
void LCD_Data(char ch)
{	
	LCD_CTRL |=(1<<LCD_RS);
	LCD_CTRL &= ~(1<< LCD_RW);
	LCD_CTRL |= (1<<LCD_EN);
	_delay_us(50);
	LCD_WDATA = ch;
	_delay_us(50);
	LCD_CTRL &= ~(1<<LCD_EN);
}

void LCD_Comm(char ch)
{
	LCD_CTRL &= ~(1<<LCD_RS);
	LCD_CTRL &= ~(1<<LCD_RW);
	LCD_CTRL |= (1<<LCD_EN);
	_delay_us(50);
	LCD_WINST=ch;
	_delay_us(50);
	LCD_CTRL &= ~(1<<LCD_EN);	
}

void LCD_Clear(void)
{
	LCD_Comm(0x01);
	_delay_ms(2);
}

void Cursor_Home(void)
{
	LCD_Comm(0x02);
	_delay_ms(2);
}
```

This feature operates in the same manner as other functions calling the feature LCD Comm() with a different value of 0x18 for shifting the contents of the device to the right and 0x1C to the left. The LCD_Shift() (function recognizes one parameter that can be either 0 for Right or 1 for Left.

```c
void LCD_Shift(char p)
{
	if(p==RIGHT)
	{
		LCD_Comm(0x1c);
		_delay_ms(1);		
	}
	else if(p == LEFT)
	{
		LCD_Comm(0x18);
		_delay_ms(1);
	}
}

```

We are needed to blink the LEDs 5 times, to show the text "INT1 RE" on the computer, and also to blink it 5 times when the INT1 occurs. We build an interrupt handler that consists of calling LED blink() function specified in IOcontrolling.h, showing text by calling LCD STR() function, and blinking by calling isplay Blink() function.

```c
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

```

LED_blink() function turns off all LEDs (PORTB = 0xFF) to make sure that no LED is turned on. 

Following this, changing the value of 0xFF (turn off) and 0x00 (turn on) to the PORTB with a small delay of 300ms gives us the effect of blinking.

```c
void LED_Blink(void)
{
	PORTB = 0xff;
	_delay_ms(300);
	PORTB = 0x00;
	_delay_ms(300);	
}
```

To create a flickering effect on the projector, the LCD can be turned on and off many times. LCD_Blink() executes this operation by transmitting the commands 0x0C (turn on) and 0x08 (turn off) to the computer with a small delay of 300ms. During the process, the contents on the panel are not removed, which is what we need, and we get the effect of blinking the text on the LCD monitor.

```c
void LCD_Blink(void)
{	
	LCD_Comm(0x08);
	_delay_ms(300);
	LCD_Comm(0x0e);
	_delay_ms(2);	
}
```

This feature would switch on the LCD device by sending the 0x0C command to the computer. It is important to insure that the LCD is still on after such manipulations.

```c
void LCD_ON(void)
{
	LCD_Comm(0x0c);
	_delay_ms(200);
}
```

We need to switch off all the LEDs, show the text "INT2 LL" on the LCD panel, and transfer it from the first line to the second when moving to the right when the INT2 is activated. To do so, we call the following functions: LED_OFF(), LCD_LED.h, LCD_pos(), LCD_STR(), LCD_Shift() and LCD_Clear(). We display INT2 LL in position(0,0) and then position(0,1), and then we're changing our LCD output to the left 15 times. Before that, we have a short pause of 200ms to make sure that everything in the LCD is done and only then can we clear the screen and move the cursor back to the original location of (0, 0) by calling the Cursor_Home() feature. Finally, we need to get back from the controller to finish the execution.

```c
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
```

In order to switch off all the LEDs, we only need to send the value 0xFF to the PORTB responsible for the LEDs. Even before that, we need to set the path to be Input and make sure that we do not interpret bits.

```c
void LED_OFF(void)
{
	DDRB = 0xff;
	PORTB = 0xff;
	_delay_ms(100);
}
```

In main() in main.c, call Interrupt Init() to allow all forms of necessary interrupts, LCD Init() to start the device, and in the endless loop, call the feature LED Right Shift() which is assumed to be the default operation where the LEDs switch from left to right one by one.

Through writing the program in this manner, we may be confident that every time during the execution of the default operation, the CPU pauses and saves the next instruction code, and only after that the appropriate interrupt handler can be performed. After execution, it goes back to the spot where it left and proceeds to call the default feature.

```c
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
```

The LCD_Init() function begins the computer and lets you get the commands from the outside. The Port_Init() function initializes all necessary ports and their path for further usage. To start the panel, we need to send a set of commands to the LCD, which are 0x38, 0x0E, 0x06, informing the LCD to be 8-bit, 2 rows, and have 5x7 dot characters (0x38). 0x0E advises the cursor to click on the screen, switch on the mouse, and click on the mouse to blink. 0x06 Let the LCD set the monitor change off to allow the cursor rise by default not decrease, which ensures that the cursor shifts to the right.

```c
void PortInit(void)
{
	DDRA = 0xff;
	DDRG = 0x0f;	
}

void LCD_Init(void)
{
	LCD_Comm(0x38);
	_delay_ms(2);
	LCD_Comm(0x38);
	_delay_ms(2);
	LCD_Comm(0x0e);
	_delay_ms(2);
	LCD_Comm(0x06);	
}
```

The main objective of this task is to enable the LED to switch from left to right with a delay of 100ms. In order to do this, the corresponding values for the 8 LEDs must be issued to the PORTB. First, we describe the path of performance by providing the value 0xFF to the DDRB. After this, using the loop to write down the value of the LED variable to the PORTB. Since PORTB often has an 8-bit register, it is really convenient to deal with variables to adjust the state of each Lead. So we'll only change the value of the LED variable to make sure the correct bit is set to 1.

```c
void LED_Left_Shift(void)
{
	char LED;
	int i;
	LED = 0xfe;
	for (i =0 ; i <8 ; i++)
	{
		PORTB = LED;
		LED = (LED<<1) | 0x01;
		_delay_ms(300);
	}	
}
```



## Team Contribution

Since this homework was the first realistic challenge assigned to us to operate with the microcontroller and the simulator, it was very challenging to adjust. Our team has also agreed to meet at the Zoom Video Communications Forum, which enables audio and video conferencing. At these sessions, we separated the duties in the following order: Bokhodir Urinboev – INT0 handler, Oybek Amanov – INT1 handler, Rakhmatjon Khasanov – INT2 handler. But, since we were at meetings and still on-line, we might ask each other questions in order to make sure that we completely understood the issue. At the final conference, we assembled all the pieces into one and published a paper.

