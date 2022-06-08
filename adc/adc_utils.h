#ifndef ADC_UTILS_H_
#define ADC_UTILS_H_

#include <avr/io.h>
#include "logic_utils.h"

void InitADC(unsigned char reference, unsigned char division_factor);
unsigned int ReadADC(unsigned char channel);

void SetVref(unsigned char reference);
void SetPrescaler(unsigned char division_factor);
void SetEnable(unsigned char enable);
void SetChannel(unsigned char channel);

void RunConversion();

#endif /* ADC_UTILS_H_ */
