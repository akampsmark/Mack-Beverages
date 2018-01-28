/*
 * Mack Beverages.c
 *
 * Author : GROUP GRACE
 GROUP DETAILS:
 EKISA RODNEY SIMON OSOMI		15/U/4965/PS
 AKAMPURIRA MARK RUGUMAMBAJU		15/U/3044/PS
 NADYOPE RICHARD			15/U/9114/PS
 NINSIIMA GRACE				15/U/1004
 MBABAZI PRISCA				15/U/7566/PS
 KABALI MICHAEL				15/U/5319/PS
 KIGGUNDU RONALD KIWANUKA		15/U/6344/PS
 KARANZI JOHNMARY			15/U/5785/PS
 */ 
#include <avr/io.h>
#define FOSC 8000000 // Clock Speed
#define F_CPU 8000000UL //FCPU
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
uint8_t count = 0, countrest = 0, seconds = 0, restseconds = 0;
#include <stdio.h>
#include <time.h>
#define TX_NEWLINE	{transmitByte(0x0d); transmitByte(0x0a);}
unsigned char *str;
#define e PB0
#define rw PB1
#define rs PB2
#define bit(x) 1 << x
float numOranges = 0;
float numMangoes = 0;
float numGuavas = 0;
float numApples = 0;
float initialOranges=0.0;
float initialMangoes=0.0;
float initialGuavas=0.0;
float initialApples=0.0;
float percentOranges = 0;
float percentMangoes = 0;
float percentGuavas = 0;
float percentApples = 0;
float totalnumlitres=0;
int numbottles300ml=0;
int numbottles500ml=0;
float numbottles300mlfilled=0.0;
float numbottles500mlfilled=0.0;
long int thenumber;
time_t start;
float EEMEM  my_eeprom_array[12]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

/*LCD CODE*/
void enable(){
	PORTB |= (1<<e);
	_delay_ms(25);
	PORTB &= ~(1<<e);
}
void lcdinit (){
	DDRA = 0xFF;
	DDRB = 0xFF;
	PORTB =0x00;
	//set interface length of 8 bits, small font , 2 lines
	PORTA= 0x3F;
	enable();
	//turn on the display , blink cursor
	PORTA= 0x0F;
	enable();
	// shift cursor to right
	PORTA= 0x1F;
	enable();	
}
 char lcdread(){
	char  data;
	DDRA = 0xFF;
	DDRB = 0xFD;
	PORTB =0x06;
	data = PORTA;
	lcdinit();
	return data;
}
void clearscreen(){
	//clear screen
	PORTA= 0x01;
	enable();
}
void lcd_position(uint8_t pos){
	//address
	if (pos==1)
	{
		PORTA = 0x80;
		enable();
	}else if (pos == 2)
	{
	PORTA = 0xC0;
	enable();
	}else if (pos == 3){
	
	PORTA = 0x94;
	enable();
	}else {
	PORTA = 0xD4;
	enable();
	}	
}
void write_char(unsigned char A){
	PORTB |= bit(rs);
	PORTA = A;
	enable();
	PORTB &=~ bit(rs);
}
void lcd_write_string(const char x[]) {
	uint8_t i = 0;
	while(x[i] != 0){
		write_char(x[i]);
		i++;
	}
}
void lcd_write_int(int in)
{
	char snum[5];
	itoa(in,snum,10);
	write_char(snum[0]);
	write_char(snum[1]);
	write_char(snum[2]);
	write_char(snum[3]);
	write_char(snum[4]);
}
void lcd_write_float(float value){
	char f[10] = "";
	dtostrf(value, 3, 1, f);
	write_char(f[0]);
	write_char(f[1]);
	write_char(f[2]);
	write_char(f[3]);
	write_char(f[4]);
write_char(f[5]);
}
void litreslcd(){
	clearscreen();
	lcd_position(1);
	lcd_write_string("Litres left-");
	lcd_write_float(totalnumlitres);
	lcd_position(2);
	lcd_write_string("300ml filled- ");
	lcd_write_float(numbottles300mlfilled);
	lcd_position(3);
	lcd_write_string("500ml filled- ");
	lcd_write_float(numbottles500mlfilled);
}

/*SERIAL TRANSMISSION AND RECEIVING CODE*/
void USART_Init()
{
	/* Set baud rate */;
	UBRR0L = 0x33;
	UBRR0H = 0x00;
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
	/* Enable Receiver and Transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
}
void transmitByte( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
void transmitString(char* string)
{
	while (*string)
	transmitByte(*string++);
}
unsigned char USART_Receive( void )
{
	unsigned char data, status;
	
	while(!(UCSR0A & (1<<RXC))); 	// Wait for incomming data
	
	status = UCSR0A;
	data = UDR0;

	return(data);
}
void transmitInt(int in){
	char snum[5];
	itoa(in,snum,10);
	transmitByte(snum[0]);
	transmitByte(snum[1]);
	transmitByte(snum[2]);
	transmitByte(snum[3]);
	transmitByte(snum[4]);
}
void floatprint(float value){
	char f[10] = "";
	dtostrf(value, 3, 1, f);
	transmitByte(f[0]);
	transmitByte(f[1]);
	transmitByte(f[2]);
	transmitByte(f[3]);
	transmitByte(f[4]);
transmitByte(f[5]);}

/*ROTATE MOTOR*/
void rotate(){
	PORTD=0b00000111;
	_delay_ms(250);
	PORTD=0b00001101;
	_delay_ms(250);
	PORTD=0b00011001;
	_delay_ms(250);
	PORTD=0b00010011;
	_delay_ms(250);
}

/*KEYPAD SELECT*/
void keypadpresspercent(){
	int counter=0;
	int pressed[2]={-1,-1};
	while(1){
		PORTF=0xFE; //CHECK ROW1
		if((PINF&(1<<PF4))==0){
			write_char('1');
			pressed[counter]=1;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF5))==0){
			write_char('2');
			pressed[counter]=2;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF6))==0){
			write_char('3');
			pressed[counter]=3;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		PORTF=0xFD; //CHECK ROW2
		if((PINF&(1<<PF4))==0){
			write_char('4');
			pressed[counter]=4;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF5))==0){
			write_char('5');
			pressed[counter]=5;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF6))==0){
			write_char('6');
			pressed[counter]=6;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		PORTF=0xFB; //CHECK ROW3
		if((PINF&(1<<PF4))==0){
			write_char('7');
			pressed[counter]=7;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF5))==0){
			write_char('8');
			pressed[counter]=8;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		else if((PINF&(1<<PF6))==0){
			write_char('9');
			pressed[counter]=9;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
			//break;
		}
		PORTF=0xF7; //CHECK ROW4
		if((PINF&(1<<PF4))==0){
			PORTB =0x00;
			PORTA= 0x13;
			enable();
			write_char(' ');
			counter--;
			_delay_ms(250);
			PORTB =0x00;
			PORTA= 0x13;
			enable();
		}
		else if((PINF&(1<<PF6))==0){
STOP:
			thenumber=0;
			int i;
			for (i=0;i<2;i++)
			{
				if(pressed[i]>= 0){
					thenumber=10*thenumber + pressed[i];
					//floatprint(thenumber);
				}
			}
			break;
		}
		else if((PINF&(1<<PF5))==0){
			write_char('0');
			pressed[counter]=0;
			counter++;
			if (counter>1)
			{
				goto STOP;
			}
			_delay_ms(250);
		}
	}
}

/*KEYPAD SELECT*/
void keypadpress(){
	int counter=0; 
	int pressed[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	while(1){
	PORTF=0xFE; //CHECK ROW1
	if((PINF&(1<<PF4))==0){
		write_char('1');
		pressed[counter]=1;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF5))==0){
		write_char('2');
		pressed[counter]=2;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF6))==0){
		write_char('3');
		pressed[counter]=3;
		counter++;
		_delay_ms(250);
		//break;
	}
	PORTF=0xFD; //CHECK ROW2
	if((PINF&(1<<PF4))==0){
		write_char('4');
		pressed[counter]=4;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF5))==0){
		write_char('5');
		pressed[counter]=5;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF6))==0){
		write_char('6');
		pressed[counter]=6;
		counter++;
		_delay_ms(250);
		//break;
	}
	PORTF=0xFB; //CHECK ROW3
	if((PINF&(1<<PF4))==0){
		write_char('7');
		pressed[counter]=7;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF5))==0){
		write_char('8');
		pressed[counter]=8;
		counter++;
		_delay_ms(250);
		//break;
	}
	else if((PINF&(1<<PF6))==0){
		write_char('9');
		pressed[counter]=9;
		counter++;
		_delay_ms(250);
		//break;
	}
	PORTF=0xF7; //CHECK ROW4
	if((PINF&(1<<PF4))==0){
		PORTB =0x00;
		PORTA= 0x13;
		enable();
		write_char(' ');
		counter--;
		_delay_ms(250);
		PORTB =0x00;
		PORTA= 0x13;
		enable();
	}
	else if((PINF&(1<<PF6))==0){
		
		int i;
		thenumber=0;
		for (i=0;i<10;i++)
		{
			if(pressed[i]>= 0){
			thenumber=10*thenumber + pressed[i];
			//floatprint(thenumber);
			}
		}
		break;
	}
	else if((PINF&(1<<PF5))==0){
		write_char('0');
		pressed[counter]=0;
		counter++;
		_delay_ms(250);
	}
}
}


/*AMOUNT SELECT*/
float amount(int valuep){
	floatprint(valuep);
	float va = (float)valuep;
	return va;
	
}


/*PERCENT SELECT*/
float percent(int value){
		floatprint(value);
		float num= (float)value;
		return num/100;
	}


/*CONFIGURATION OF INITIAL VALUES*/
void configure(){
	clearscreen();
	lcd_position(1);
	lcd_write_string("Quantity--ORANGES:");
	lcd_position(2);
	keypadpress();
	//sen = lcdread();
	numOranges=amount(thenumber);
	initialOranges=numOranges;
	transmitString(" Oranges");
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("Quantity--MANGOES:");
	lcd_position(2);
	keypadpress();
	//sen = thenumber;
	numMangoes=amount(thenumber);
	initialMangoes=numMangoes;
	transmitString(" Mangoes");
	lcd_position(2);
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("Quantity--GUAVAS:");
	lcd_position(2);
	keypadpress();
	//sen = lcdread();
	numGuavas=amount(thenumber);
	initialGuavas=numGuavas;
	transmitString(" Guavas");
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("Quantity--APPLES:");
	lcd_position(2);
	keypadpress();
	//sen = lcdread();
	numApples=amount(thenumber);
	initialApples=numApples;
	transmitString(" Apples");
	lcd_position(2);
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("PERCENTAGE ORANGES:");
	lcd_position(2);
	keypadpresspercent();
	//sen = lcdread();
	percentOranges=percent(thenumber);
	transmitString(" % Oranges");
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("PERCENTAGE MANGOES:");
	lcd_position(2);
	keypadpresspercent();
	//sen = lcdread();
	percentMangoes=percent(thenumber);
	transmitString(" % Mangoes");
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("PERCENTAGE GUAVAS:");
	lcd_position(2);
	keypadpresspercent();
	//sen = lcdread();
	percentGuavas=percent(thenumber);
	transmitString(" % Guavas");
	TX_NEWLINE
	clearscreen();
	lcd_position(1);
	lcd_write_string("PERCENTAGE APPLES:");
	lcd_position(2);
	keypadpresspercent();
	
	percentApples=percent(thenumber);
	transmitString(" % Apples");
	TX_NEWLINE
	clearscreen();
	lcd_position(2);
	lcd_write_string("CONFIGURATION DONE");
}

/*Amount fruit left*/
void fruitleft(){
	TX_NEWLINE;
floatprint(numApples);
transmitString(" Apples");
TX_NEWLINE;
floatprint(numMangoes);
transmitString(" Mangoes");
TX_NEWLINE;
floatprint(numOranges);
transmitString(" Oranges");
TX_NEWLINE;
floatprint(numGuavas);
transmitString(" Guavas");
TX_NEWLINE;
}

/*Plant Operation Time*/
void timediff(){
	TX_NEWLINE;
	floatprint(seconds);
	transmitString(" Seconds");
	TX_NEWLINE;
}

//plant rest time
void timediffrest(){
	TX_NEWLINE;
	floatprint(restseconds);
	transmitString(" Seconds");
	TX_NEWLINE;
}

/*Calculations*/
float numlitres(float numApples1, float numMangoes1,float numOranges1,float numGuavas1,float percentApples1,float percentOranges1,float percentGuavas1,float percentMangoes1){
	float litres;
	int numfruit=0;
	int j,v,x,y,z;
	for (j=1;j<10;j++)
	{
		v = numApples1*percentApples1;
		x = numMangoes1*percentMangoes1;
		y = numOranges1*percentOranges1;
		z = numGuavas1*percentGuavas1;
		
		numfruit= numfruit+v+x+y+z;
		numApples1=numApples1 -v;
		numMangoes1=numMangoes1-x;
		numOranges1=numOranges1-y;
		numGuavas1=numGuavas1-z;
	}
	litres=((float)numfruit*150.0)/3000.0;
	floatprint(litres);
	totalnumlitres=litres;
	numApples=numApples1;
	numMangoes=numMangoes1;
	numOranges=numOranges1;
	numGuavas=numGuavas1;
	return litres;
}
void numbottles(int ml, float totalnumlitres){
	int numbottles;
	numbottles=((totalnumlitres*1000)/ml);
	if (ml==300){
	numbottles300ml=numbottles;
	floatprint(numbottles);
	transmitString(" 300ml bottles can be filled");
	TX_NEWLINE;
	}
	else{
	numbottles500ml=numbottles;
	floatprint(numbottles);
	transmitString(" 500ml bottles can be filled");
	TX_NEWLINE;
	}
}
void saveSetting(){
	eeprom_write_float(&my_eeprom_array[0], numbottles300mlfilled);
	eeprom_write_float(&my_eeprom_array[1], numbottles500mlfilled);
	eeprom_write_float(&my_eeprom_array[2], totalnumlitres);
	eeprom_write_float(&my_eeprom_array[3], initialApples);
	eeprom_write_float(&my_eeprom_array[4], initialMangoes);
	eeprom_write_float(&my_eeprom_array[5], initialGuavas);
	eeprom_write_float(&my_eeprom_array[6], initialOranges);
	eeprom_write_float(&my_eeprom_array[7], percentApples);
	eeprom_write_float(&my_eeprom_array[8], percentMangoes);
	eeprom_write_float(&my_eeprom_array[9], percentGuavas);
	eeprom_write_float(&my_eeprom_array[10], percentOranges);
	//eeprom_write_float(&my_eeprom_array[11], (float)seconds);
	//eeprom_write_float(&my_eeprom_array[12], (float)restseconds);
	//eeprom_write_float(&my_eeprom_array[13], percentApples);
	if(eeprom_read_float(&my_eeprom_array[0])==numbottles300mlfilled){
		TX_NEWLINE;
		transmitString("SETTING SAVE COMPLETE");
		TX_NEWLINE;
	}
	else{
		TX_NEWLINE;
		transmitString("SETTING SAVE FAILED");
	}
}
void resumeSetting(){
	
	numbottles300mlfilled= eeprom_read_float(&my_eeprom_array[0]);
	numbottles500mlfilled=eeprom_read_float(&my_eeprom_array[1]);
	totalnumlitres=eeprom_read_float(&my_eeprom_array[2]);
	numApples=eeprom_read_float(&my_eeprom_array[3]);
	numMangoes=eeprom_read_float(&my_eeprom_array[4]);
	numGuavas=eeprom_read_float(&my_eeprom_array[5]);
	numOranges=eeprom_read_float(&my_eeprom_array[6]);
	percentApples=eeprom_read_float(&my_eeprom_array[7]);
	percentMangoes=eeprom_read_float(&my_eeprom_array[8]);
	percentGuavas=eeprom_read_float(&my_eeprom_array[9]);
	percentOranges=eeprom_read_float(&my_eeprom_array[10]);

	if(numbottles300mlfilled==eeprom_read_float(&my_eeprom_array[0])){
		TX_NEWLINE;
		transmitString("SETTING RESUME COMPLETE");
		litreslcd();
		TX_NEWLINE;
	}
	else{
		TX_NEWLINE;
		transmitString("SETTING RESUME FAILED");
		TX_NEWLINE;
	}
	
	}

/*BOTTLE- Filling, Sealing and Storage*/
void process(float bottlesize){
if((PIND&(1<<PD0))==0){
	_delay_ms(250);
	if (bottlesize==300)
	{
	transmitString("300ml Bottle operation selected");
	TX_NEWLINE;
}
else
{
transmitString("500ml Bottle operation selected");
TX_NEWLINE;
}
if (totalnumlitres>(bottlesize/1000))
{
	rotate();
	transmitString("Bottle Filling...");
	TX_NEWLINE;
	PORTG=0x02;
	_delay_ms(1000);
	PORTG=0x01;
	_delay_ms(1000);
	PORTG=0x00;
	rotate();
	totalnumlitres=totalnumlitres-(bottlesize/1000);
	if (bottlesize==300)
	{
		numbottles300ml=numbottles300ml-1;
		numbottles300mlfilled=numbottles300mlfilled+1.0;
	}
	else{
	numbottles500ml=numbottles500ml-1;
	numbottles500mlfilled=numbottles500mlfilled+1.0;
	}
	litreslcd();
	transmitString("Bottle Sealing...");
	TX_NEWLINE;
	PORTG=0x01;
	_delay_ms(1000);
	PORTG=0x00;
	rotate();
	rotate();
	transmitString("Bottle in Storage.");
	TX_NEWLINE;
	PORTG=0x01;
	_delay_ms(1000);
	PORTG=0x00;
}
else
{
	TX_NEWLINE;
	transmitString("Alert!! Juice level below minimal");
	TX_NEWLINE;
}
}
}

/*Admin Menu*/
void adminmenu(){
	unsigned char option;
	
	REDO:
	TX_NEWLINE;
	transmitString("> 1 : Display Amount left in Liters");
	TX_NEWLINE;
	transmitString("> 2 : Number of Bottles Filled");
	TX_NEWLINE;
	transmitString("> 3 : Amount of fruit left after mixing drink");
	TX_NEWLINE;
	transmitString("> 4 : Plant Operation Time");
	TX_NEWLINE;
	transmitString("> 5 : Plant Rest Time ");
	TX_NEWLINE;
	transmitString("> 6 : Save Current Settings ");
	TX_NEWLINE;
	transmitString("> 7 : Resume Previous Setting ");
	TX_NEWLINE;
	transmitString("> 8 : Re-Configure Plant "); 
	TX_NEWLINE;
	transmitString("> 9 : Resume Operation ");
	TX_NEWLINE;
	TX_NEWLINE;
	transmitString("> Select Option (1-9): ");
	
	option = USART_Receive();
	transmitByte(option);
	
	switch (option)
	{
		case '1': litreslcd();
		break;
		case '2': litreslcd();
		break;
		case '3': fruitleft();
		break;
		case '4': timediff();
		break;
		case '5': timediffrest();
		break;
		case '6': saveSetting();
		break;
		case '7': resumeSetting();
		break;
		case '8': TX_NEWLINE;
		main();
		TX_NEWLINE;
		break;
		case '9':
		break;
		default:  TX_NEWLINE;
		transmitString(" Invalid option!");
		goto REDO;
		TX_NEWLINE;
		
	}
	
	TX_NEWLINE;
	transmitString(" Finished..");
	TX_NEWLINE;
}

ISR(TIMER0_OVF_vect){
	count++;
	if (count >= 122)
	{
		// which makes one second
		seconds++;
		TCNT0 = 0; 
		count = 0; 
	}
}

ISR(TIMER2_OVF_vect){
	countrest++;
	if (countrest >= 122)
	{
		// which makes one second
		restseconds++;
		TCNT2 = 0;
		countrest = 0;
	}
}

void continuedo(){
	lcd_position(1);
	lcd_write_string("CONFIGURE SYSTEM");
	lcd_position(2);
	lcd_write_string("USE KEYPAD");
	lcd_position(3);
	lcd_write_string("After input press #");
	_delay_ms(2000);
	configure();
}

int main(void)
{
    /* Replace with your application code */
	DDRC=0b11111100;
	DDRD=0b11111110;
	DDRE=0b11111110;
	DDRF=0b10001111;
	DDRG=0b01111;
	PORTC=0b00000100;
	PORTD=0b00000001;
	//DDRC&=~(1<<PC6);
	//DDRC&=~(1<<PC7);
	cli();
	sei();
	
	
	
	USART_Init();
	transmitString("WELCOME TO MACK BEVERAGES");
	TX_NEWLINE;
	
	
	lcdinit();
	lcd_position(1);
	lcd_write_string("PLANT INITIALIZED");
	_delay_ms(1500);
	clearscreen();
	if (&my_eeprom_array[0]>0)
	{
		TX_NEWLINE;
		transmitString("PREVIOUS SETTINGS DETECTED");
		GT:
		TX_NEWLINE;
		transmitString("Resume this Setting?");
		TX_NEWLINE;
		transmitString("Y--(yes)        N--(no)");
		TX_NEWLINE;
		unsigned char option;
		option = USART_Receive();
		transmitByte(option);
		
		TX_NEWLINE;
		switch (option)
		{
			case 'y': resumeSetting();
			break;
			
			case 'Y': resumeSetting();
			break;
			
			case 'n': continuedo();
			break;
			case 'N': continuedo();
			break;
						
			default: TX_NEWLINE;
			transmitString("Invalid Choice");
			goto GT;
			TX_NEWLINE;
		}
	}
	else{
	lcd_position(1);
	lcd_write_string("CONFIGURE SYSTEM");
	lcd_position(2);
	lcd_write_string("USE KEYPAD");
	lcd_position(3);
	lcd_write_string("After input press #");
	_delay_ms(2000);
	configure();
	}
	transmitString("Configurations Complete");
	TX_NEWLINE;
	transmitString("Plant will process: ");
	numlitres(numApples,numMangoes,numOranges,numGuavas,percentApples,percentOranges,percentGuavas,percentMangoes);
	transmitString("Litres.");
	TX_NEWLINE;
	numbottles(300,totalnumlitres);
	numbottles(500,totalnumlitres);
	TCNT0 = 0;
	TCNT2 = 0;
    while (1) 
    {
		if((PINC&(1<<PC0))==0){
			PORTC=0b00010001;
			//cli();
			TCCR0 |= (1 << CS02);
			TIMSK |= (1 << TOIE0);
			//sei();
				process(300);
		}
		else if((PINC&(1<<PC1))==0){
			PORTC=0b00001001;
			//cli();
			TCCR0 |= (1 << CS02);
			TIMSK |= (1 << TOIE0);
			//sei();
			process(500);
			}
		else if(((PINC&(1<<PC0))==1)&&((PINC&(1<<PC0))==1)){
			TCCR2 |= (1 << CS02);
			TIMSK |= (1 << TOIE2);
			//sei();
		adminmenu();
		_delay_ms(1000);
			}
	}
}