/*
 * Mack Beverages.c
 *
 * Created: 12/9/2017 8:39:30 PM
 * Author : Marc
 */ 

#include <avr/io.h>
#include <util/delay.h>
#define rs PD0
#define rw PD1
#define e  PD2

void latch(){
	PORTD |=(1<<e);
	_delay_ms(250);
	PORTD &=~(1<<e);
}

int main(void)
{
    /* Main coursework code*/
	DDRD= 0xff;
	DDRE= 0xff;
	
    //while (1) 
    //{
		PORTD &=~(1<<rs);
		PORTE = 0xff;
		latch();
		PORTD |=(1<<rs);
		PORTE = 'A';
		latch();
		
		
    //}
}

