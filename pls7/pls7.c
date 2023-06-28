#include <avr/io.h>
#include <avr/interrupt.h>
#include "PLS7.h"

uint8_t DISP_BUFFER[5] = {0x00, 0xff, 0xff, 0xff, 0xff}; //bafer displeja
uint8_t disp = 4; //indeks trenutno aktivnog displeja

ISR(TIMER0_COMPA_vect)
{
	//prekid tajmera 2 usled dostizanja vrednosti registra OCR2A
	if (++disp > 4)
		disp = 0;

	PORTB = ~(1 << (4 - disp)); //ukljucenje tranzistora
	PORTD = DISP_BUFFER[disp]; //ispis na trenutno aktivan displej
}

void init()
{
	//inicijalizacija portova:
	DDRB = 0x3f; //PB5-PB0 -> izlazi
	DDRC = 0x20; //PC5 -> izlaz
	DDRD = 0xff; //port D -> izlaz

	//inicijalizacija tajmera 2:
	TCCR0A = 0x02; //tajmer 2: CTC mod
	TCCR0B = 0x03; //tajmer 2: fclk = fosc/64
	OCR0A = 249; //perioda tajmera 2: 250 Tclk (OCR2A + 1 = 250)
	TIMSK0 = 0x02; //dozvola prekida tajmera 2

	//usled dostizanja vrednosti registra OCR2A
	sei(); //I = 1 (dozvola prekida)
}

void writeDisplay(uint8_t display, uint8_t value)
{
	DISP_BUFFER[display] = value;
}

uint8_t readDisplay(uint8_t display)
{
	return DISP_BUFFER[display];
}

uint8_t buttonState(uint8_t button)
{
	if (PINC & (1 << button))
		return 0;
	else
		return 1;
}

//makroi za kontrolu pinova preko kojih je kontroler povezan sa 74HC165:
#define SCL_HI (PORTC |= (1<<5))
#define SCL_LO (PORTC &= ~(1<<5))
#define SDA (PINC & (1 << 4))
#define SHLD_HI (PORTB |= (1<<5))
#define SHLD_LO (PORTB &= ~(1<<5))

uint8_t readSwitches()
{
	uint8_t i, tmp = 0, mask = 0x80;

	//impuls za upis stanja prekidaca u registar
	SHLD_HI;
	SHLD_LO;
	SHLD_HI;

	for (i = 0; i < 8; i++)
	{
		if (SDA) //provera stanja ulaznog pina
			tmp |= mask;

		mask >>= 1;

		SCL_LO;
		SCL_HI; //generisanje aktivne ivice takta
	}
	return tmp;
}

uint8_t switchState(uint8_t sw)
{
	uint8_t tmp = readSwitches();
	return (tmp & (1 << sw)) >> sw;
}
