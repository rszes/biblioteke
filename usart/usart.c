#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "usart.h"

#define UART_BUFFER_SIZE 64

int8_t Rx_Buffer[UART_BUFFER_SIZE];		// Prijemni FIFO bafer
volatile uint8_t Rx_Buffer_Size = 0;	// Broj karaktera u prijemnom baferu
volatile uint8_t Rx_Buffer_First = 0;
volatile uint8_t Rx_Buffer_Last = 0;

ISR(USART_RX_vect)
{
	Rx_Buffer[Rx_Buffer_Last++] = UDR0;		// Ucitavanje primljenog karaktera
	Rx_Buffer_Last &= UART_BUFFER_SIZE - 1;	// Povratak na pocetak u slucaju prekoracenja
	if (Rx_Buffer_Size < UART_BUFFER_SIZE)
		Rx_Buffer_Size++;					// Inkrement brojaca primljenih karaktera
}

void usartInit(uint32_t baud)
{
	/*
	 * Inicijalizacija indikatora
	 * U2Xn = 0: Onemogucena dvostruka brzina
	 * MPCMn = 0: Onemogucen multiprocesorski rezim
	 */
	UCSR0A = 0x20;	

	/*
	 * RXCIEn = 1: Dozvola prekida izavanog okoncanjem prijema
	 * RXENn = 1: Dozvola prijema
	 * TXENn = 1: Dozvola slanja
	 */
	UCSR0B = 0x98;

	/*
	 * UMSELn[1:0] = 00: Asinroni rezim
	 * UPMn[1:0] = 00: Bit pariteta se ne koristi
	 * USBSn = 0: Koristi se jedan stop bit
	 * UCSzn[2:0] = 011: 8-bitni prenos
	 */
	UCSR0C = 0x06;

	uint16_t UBRRn = F_CPU / (16 * baud) - 1;
	UBRR0H = UBRRn >> 8;
	UBRR0L = UBRRn & 0xff;

	sei();	// I = 1 (dozvola prekida)
}

uint8_t usartAvailable()
{
	return Rx_Buffer_Size;		// Ocitavanje broja karaktera u prijemnom baferu
}

void usartPutChar(int8_t c)
{
	while(!(UCSR0A & 0x20));	// Ceka da se setuje UDREn (indikacija da je predajni bafer prazan)
	UDR0 = c;					// Upis karaktera u predajni bafer
}

void usartPutString(int8_t *s)
{
	while(*s != 0)				// Petlja se izvrsava do nailaska na nul-terminator
	{
		usartPutChar(*s);		// Slanje tekuceg karaktera
		s++;					// Azuriranje pokazivaca na tekuci karakter
	}
}

void usartPutString_P(const int8_t *s)
{
	while (1)
	{
		int8_t c = pgm_read_byte(s++);	// Citanje sledeceg karaktera iz programske memorije
		if (c == '\0')					// Izlazak iz petlje u slucaju
			return;						// Nailaska na terminator
		usartPutChar(c);				// Slanje karaktera
	}
}

int8_t usartGetChar()
{
	int8_t c;

	if (!Rx_Buffer_Size)						// Bafer je prazan?
		return -1;
	c = Rx_Buffer[Rx_Buffer_First++];			// Citanje karaktera iz prijemnog bafera
	Rx_Buffer_First &= UART_BUFFER_SIZE - 1;	// Povratak na pocetak u slucaju prekoracenja
	Rx_Buffer_Size--;							// Dekrement brojaca karaktera u prijemnom baferu

	return c;
}

uint8_t usartGetString(int8_t *s)
{
	uint8_t len = 0;

	while(Rx_Buffer_Size) 			// Ima karaktera u faferu?
		s[len++] = usartGetChar();	// Ucitavanje novog karaktera

	s[len] = 0;						// Terminacija stringa
	return len;						// Vraca broj ocitanih karaktera
}
