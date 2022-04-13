#ifndef TIMER_0_H_
#define TIMER_0_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/*
* timer0DelayMs - Funkcija koja implementira pauzu u broju milisekundi koji je prosledjen kao parametar
*/
void timer0DelayMs(uint32_t delay_length);

/*
* timer0Millis - Funkcija koja, na bezbedan nacin, vraca kao povratnu vrednost broj milisekundi proteklih od pocetka merenja vremena
*/
uint32_t timer0Millis();

/*
* timer0Init - Funkcija koja inicijalizuje timer 0 tako da pravi prekide svake milisekunde
*/
void timer0Init();


#endif /* TIMER_0_ */