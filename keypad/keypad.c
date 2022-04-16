#include "keypad.h"

#define GET_BIT(x, pos) ((x & (1 << pos)) >> pos)

uint8_t _rows[4];
uint8_t _cols[4];

void keypadInit(uint8_t *rows, uint8_t *cols)
{
	// pravljenje lokalnih kopija
	memcpy(_rows, rows, 4);
	memcpy(_cols, cols, 4);

	//PUD = 0 (pull-up otpornici su ukljuceni)
	MCUCR &= ~0x10;

	for (uint8_t i = 0; i < 4; i++)
      	if (_cols[i] < 8)
      		DDRD &= (0 << _cols[i]);
      	else
      		DDRB &= (0 << (_cols[i] - 8));
  	
  	// Inicijalizacija porta D
  	for (uint8_t i = 0; i < 4; i++)
      	if (_cols[i] < 8)
      		PORTD |= (1 << _cols[i]);
      	else
      		PORTB |= (1 << (_cols[i] - 8));
}

int8_t keypadScan()
{
	uint8_t row = 0x00;
	int8_t key = 0x00;
	
  	row = 0x08;
  	for(uint8_t i = 0; i < 4; i++)
	{
		//aktiviranje vrste
     	for(uint8_t j = 0; j < 4; j++)
        	if (_rows[j] < 8)
      			DDRD &= (0 << _rows[j]);
      		else
      			DDRB &= (0 << (_rows[j] - 8));
        
		if (_rows[i] < 8)
   			DDRD |= (1 << _rows[i]);
   		else
   			DDRB |= (1 << (_rows[i] - 8));
		
		_delay_ms(2);
      
      	uint8_t tmp = 0x00;
      	for (uint8_t j = 0; j < 4; j++)
          	if (_cols[j] < 8)
        	{
              if (GET_BIT(PIND, _cols[j]) == 1)
              {
                  tmp |= 1 << (3 - j);
              }
              else
              {
                  tmp &= 0 << (3 - j);
              }
        	}
      		else
            {
              if (GET_BIT(PINB, (_cols[j] - 8)) == 1)
              {
                  tmp |= 1 << (3 - j);
              }
              else
              {
                  tmp &= 0 << (3 - j);
              }
            }
        
		
      	for (uint8_t i = 0; i < 4; i++)
          	if (_cols[i] < 8)
      			PORTD |= (1 << _cols[i]);
      		else
      			PORTB |= (1 << (_cols[i] - 8));
      
		//nizi nibl predstavlja vrstu, a visi stanja tastera:
      	uint8_t tmp1 = (~tmp) << 4;
		switch (tmp1 | row)
		{
			//prva vrsta:
			case 0x88:
				key	= '1'; break;
			case 0xC8:
				key	= '2'; break;
			case 0xE8:
				key	= '3'; break;
			case 0xF8:
				key	= 'A'; break;
			
			//druga	vrsta:
			case 0x84:
				key	= '4'; break;
			case 0xC4:
				key	= '5'; break;
			case 0xE4:	
				key	= '6'; break;
			case 0xF4:	
				key	= 'B'; break;
			
			//treca	vrsta:
			case 0x82:	
				key	= '7'; break;
			case 0xC2:	
				key	= '8'; break;
			case 0xE2:
				key	= '9'; break;
			case 0xF2:	
				key	= 'C'; break;
			
			//cetvrta vrsta:
			case 0x81:	
				key	= '*'; break;
			case 0xC1:	
				key	= '0'; break;
			case 0xE1:	
				key = '#'; break;
			case 0xF1:	
				key	= 'D'; break;
		}
      	row >>= 1;
	}

	return key;
}
