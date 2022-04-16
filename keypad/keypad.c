#include "keypad.h"

#define GET_BIT(x, pos) ((x & (1 << pos)) >> pos)

void keypadInit(uint8_t *rows, uint8_t *cols)
{
	//PUD = 0 (pull-up otpornici su ukljuceni)
	MCUCR &= ~0x10;
	
	for (int i = 0; i < 4; i++)
      	if (cols[i] < 8)
      		DDRD &= (0 << cols[i]);
      	else
      		DDRB &= (0 << (cols[i] - 8));
  	
  	// Inicijalizacija porta D
  	for (int i = 0; i < 4; i++)
      	if (cols[i] < 8)
      		PORTD |= (1 << cols[i]);
      	else
      		PORTB |= (1 << (cols[i] - 8));
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
        	if (rows[j] < 8)
      			DDRD &= (0 << rows[j]);
      		else
      			DDRB &= (0 << (rows[j] - 8));
        
		if (rows[i] < 8)
   			DDRD |= (1 << rows[i]);
   		else
   			DDRB |= (1 << (rows[i] - 8));
		
		_delay_ms(2);
      
      	uint8_t tmp = 0x00;
      	for (uint8_t j = 0; j < 4; j++)
          	if (cols[j] < 8)
        	{
              if (GET_BIT(PIND, cols[j]) == 1)
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
              if (GET_BIT(PINB, cols[j] - 8) == 1)
              {
                  tmp |= 1 << (3 - j);
              }
              else
              {
                  tmp &= 0 << (3 - j);
              }
            }
        
		
      	for (uint8_t i = 0; i < 4; i++)
          	if (cols[i] < 8)
      			PORTD |= (1 << cols[i]);
      		else
      			PORTB |= (1 << (cols[i] - 8));
      
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