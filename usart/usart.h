/**
 * @file	usart.h
 * @brief	Funkcije za upravljanje serijskim portom
 * @author	Milan Lukic
 * @date	14-04-2019
 */

#ifndef USART_H_
#define USART_H_

#include <stdint.h>

/// Velicina prijemnog bafera (mora biti 2^n)
#define UART_BUFFER_SIZE 64

/**
 * Funkcija inicijalizuje USART modul.
 * @param baud brzina serijske komunikacije, izrazena u bitima u sekundi
 */
void usartInit(uint32_t baud);

/**
 * Funkcija vraca broj neocitanih karaktera u prijemnom baferu.
 * @return broj neocitanih karaktera u prijemnom baferu
 */
uint8_t usartAvailable();

/**
 * Funkcija salje zadati karakter preko serijskog porta.
 * @param c karakter za slanje
 */
void usartPutChar(int8_t c);

/**
 * Funkcija salje string (niz karaktera) smesten u RAM memoriju preko serijskog porta.
 * @param s pokazivac na string u RAM memoriji
 */
void usartPutString(int8_t *s);

/**
 * Funkcija salje string (niz karaktera) smesten u programsku FLASH memoriju preko serijskog porta.
 * @param s pokazivac na string u programskoj FLASH memoriji
 */
void usartPutString_P(const int8_t *s);

/**
 * Funkcija vraca sledeci neprocitani karakter iz prijemnog bafera. Ukoliko je prijemni bafer prazan, vraca -1.
 * @return sledeci neprocitani karakter iz prijemnog bafera
 */
int8_t usartGetChar();

/**
 * Funkcija ocitava sve karaktere iz prijemnog bafera i od njih formira string.
 * @param s pokazivac na bafer u koji se smesta string
 * @return broj ocitanih karaktera
 */
uint8_t usartGetString(int8_t *s);

#endif /* USART_H_ */
