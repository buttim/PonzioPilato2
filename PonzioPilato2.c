//!sdcc "%file%"
#include "STC15F2K60S2.h"
#include <stdbool.h>
#include <stdint.h>

#define FOSC		11059200U
#define ENABLE_IAP  	0x82	//if SYSCLK<20MHz
#define CMD_READ    	1
#define BV(x) (1<<(x))
#define _nop_() __asm nop __endasm
#define LED		P55
#define BUTTON		P32	//INT0
#define BUZZER_PLUS	P10
#define BUZZER_MINUS	P11

volatile bool f1ms,playing=false,stop=false;
volatile unsigned millis=0;
unsigned short tVal;
int nSongs;

void powerDown() {
	LED=0;
	BUZZER_MINUS=0;
	BUZZER_PLUS=0;
	PCON|=BV(1);			//power down
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	BUZZER_PLUS=1;
}

void LVD(void) __interrupt 6 __using 1 {
	EX0=0;			//disable INT0
	powerDown();
}

void int0(void) __interrupt 0 __using 1 {
	if (playing)
		stop=true;
}

void IapIdle() {
    IAP_CONTR = 0;
    IAP_CMD = 0;
    IAP_TRIG = 0;
    IAP_ADDRH = 0x80;
    IAP_ADDRL = 0;
}

uint8_t IapReadByte(uint16_t addr) {
    uint8_t dat;

    IAP_CONTR = ENABLE_IAP;
    IAP_CMD = CMD_READ;
    IAP_ADDRL = addr;
    IAP_ADDRH = addr >> 8;
    IAP_TRIG = 0x5a;
    IAP_TRIG = 0xa5;
    _nop_();
    dat = IAP_DATA;
    IapIdle();
    return dat;
}

uint16_t IapReadWord(uint16_t addr) {
	return IapReadByte(addr)+256*IapReadByte(addr+1);
}

void Timer0Init(void)	{	//1ms@11.0592MHz
	AUXR|=0x80;		//Timer clock is 1T mode
	TMOD&=0xF0;		//Set timer work mode
	TL0=0xCD;		//Initial timer value
	TH0=0xD4;		//Initial timer value
	TF0=0;			//Clear TF0 flag
	TR0=1;			//Timer0 start run
	ET0=1;
}

void Timer2Init(void) {
	AUXR|=0x04;		//timer clock is 1T mode
	IE2|=4;			//enable interrupt for timer 2
}

void tm2(void) __interrupt 12 __using 1 {
	T2L=tVal;
	T2H=tVal>>8;
	BUZZER_MINUS^=1;
	BUZZER_PLUS^=1;
}

int modulo(int n,int q) {
	while (n>=q) n-=q;
	return n;
}

void tm0(void) __interrupt 1 __using 1 {
	f1ms=true;
	unsigned q=100;
	
	millis++;
	
	if (millis>20000)
		LED=0;
	else {
		if (millis<5000) q=1000;
		else if (millis<10000) q=500;
		else if (millis<15000) q=250;
		LED=modulo(millis,q)<50;
	}
	TL0=0xCD;		//Initial timer value
	TH0=0xD4;		//Initial timer value
}

void Delay1ms(int n) {
	while (n--) {
		f1ms=false;
		while (!f1ms);
	}
}

void play(int n) {
	int i=IapReadWord(2+2*n);
	stop=false;
	while (!stop) {
		uint16_t t=IapReadWord(i),
			duration=IapReadWord(i+2);
		if (duration==0) break;
		if (t!=0) {
			tVal=t;
			AUXR|=BV(4);
		}
		Delay1ms(duration);
		AUXR&=~BV(4);
		Delay1ms(50);
		i+=4;
	}
}

void main() {
	PCON&=~BV(5);		//turn off low voltage detection flag

	P5M0|=BV(5);		//P5.5 in push-pull mode
	P5M1&=~BV(5);

	P1M0|=BV(0)|BV(1);	//P1.0 and P1.1 in push-pull mode
	P1M1&=~(BV(0)|BV(1));	
	BUZZER_MINUS=0;
	BUZZER_PLUS=1;

	Timer0Init();
	Timer2Init();
	
	IT0=1;			//INT0 on falling edge
	PLVD=1;			//LVD interrupt high priority
	ELVD=1;			//enable low voltage detection interrupt
	EX0=1;			//enable INT0
	EA=1;			//enable interrupts
	
	nSongs=IapReadWord(0);
	
	for (;;) {
		powerDown();
		Delay1ms(500);
		int n=TL0^TH0;
		n=modulo(n,nSongs);
		millis=0;
		playing=true;
		play(n);
		playing=false;
		Delay1ms(500);
	}
}