#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

void joystickInit();

uint8_t joystickReadX(uint8_t channel);

uint8_t joystickReadY(uint8_t channel);

uint8_t joystickReadButton(uint8_t button_pin);

#endif /* JOYSTICK_H_ */
