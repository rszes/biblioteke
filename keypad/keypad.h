#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

/*
* keypadInit - Funkcija koja vrsi inicijalizaciju matricne tastature
*/
void keypadInit(uint8_t *rows, uint8_t *cols);

/*
* keypadScan - Funkcija koja implementira skeniranje matricne tastature
*/
int8_t keypadScan();

#endif /* KEYPAD_H_ */
