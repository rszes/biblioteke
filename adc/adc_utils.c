#include "adc_utils.h"

void InitADC(unsigned char reference, unsigned char division_factor)
{
	SetVref(reference); // set reference voltage
	SetPrescaler(division_factor); // set prescaler
	SetEnable(1); // set enable
}

unsigned ReadADC(unsigned char channel)
{
	SetChannel(channel); // set reading channel
	RunConversion(); // start conversion

	return ADC;
}

void SetVref(unsigned char reference)
{
	ADMUX = BitmaskClear(ADMUX, 0xC0);
	ADMUX = BitmaskSet(ADMUX, ShiftLeft(reference, 6));
}

void SetPrescaler(unsigned char division_factor)
{
	unsigned char bits = 0x00;

	switch (division_factor)
	{
		case 2:
			bits = 0x01;
			break;
		case 4:
			bits = 0x02;
			break;
		case 8:
			bits = 0x03;
			break;
		case 16:
			bits = 0x04;
			break;
		case 32:
			bits = 0x05;
			break;
		case 64:
			bits = 0x06;
			break;
		case 128:
			bits = 0x07;
			break;
		default:
			bits = 0x00;
	}

	ADCSRA = BitmaskClear(ADCSRA, 0x03);
	ADCSRA = BitmaskSet(ADCSRA, bits);
}

void SetEnable(unsigned char enable)
{
	if (enable > 0)
		ADCSRA = SetBit(ADCSRA, 7);
	else
		ADCSRA = ClearBit(ADCSRA, 7);
}


void SetChannel(unsigned char channel)
{
	ADMUX = BitmaskSet(ADMUX, BitmaskClear(channel, 0x0F));
}

void RunConversion()
{
	ADCSRA = SetBit(ADCSRA, 6);

	while(CheckBit(ADCSRA, 6))
	;
}

