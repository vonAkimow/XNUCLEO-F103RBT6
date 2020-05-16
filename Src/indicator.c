#include "indicator.h"


/*Global variables*/
extern uint8_t aTxBuffer[1];
extern SPI_HandleTypeDef hspi1;
extern inline void CS_STROB(void);
extern inline void CS_SET(void);
extern inline void CS_RESET(void);

const static uint8_t numerals[] = { NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9};


void displayDigits(uint8_t digit1,uint8_t digit2,uint8_t digit3,uint8_t digit4)
{
	
	uint8_t digitsArray[] = {digit1, digit2, digit3, digit4 };
	uint8_t* ptr = digitsArray;
	
	for (uint8_t i=0; i<4; i++)
	{
		HAL_SPI_Transmit(&hspi1, ptr+i, 1, 100);	
		CS_STROB();
	}
	
}

void indicatorClear(void)
{
	displayDigits(CLEAR,CLEAR,CLEAR,CLEAR);
}

void displayInt(int16_t value, bool padZeroes,uint8_t dots)
{
	uint8_t digitsArray[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	/*padZeroes - количество нулей перед введённым числом
	Пример: ввели (1,3) - на индикаторе 0001
	dots - точка после определенного бита полубайта:
	Пример:dots=8 - на индикаторе 0.001	
				 dots=4 - на индикаторе 00.01		*/
	
	if(!padZeroes && !value)/*Если введён ноль*/
	{
		digitsArray[3] = numerals[0];
	}
	else
	{
		bool negative = value < 0;
		value = abs(value);
		
		int8_t i;
		
		for(i = 4; i--;)
		{
			uint8_t digit = value % 10;
			digitsArray[i] = (value || padZeroes) ? numerals[digit] : 0xFF;
			value /= 10;
			if (!padZeroes && !value) break;
		}
		
		if (negative)
		{
			digitsArray[i-1] = MINUS;
		}
		if (getBit(dots,8))
		{
			digitsArray[3] &= DOT;
		}
		if (getBit(dots,7))
		{
			digitsArray[2] &= DOT;
		}
		if (getBit(dots,6))
		{
			digitsArray[1] &= DOT;
		}
		if (getBit(dots,5))
		{
			digitsArray[0] &= DOT;
		}
				
	}
	displayDigits(digitsArray[0],digitsArray[1],digitsArray[2],digitsArray[3]);
}

uint8_t getBit(uint8_t byte, uint8_t number)
{
	return ((byte >> (8-number)) & 1);
}

void displayFloat(float value, uint8_t precision, bool padZeroes)
{
	uint8_t dot = 1;
	
	while(precision)
	{
		value *= 10;
		--precision;
		dot <<= 1;
	}
	displayInt((int16_t)value,padZeroes,dot);
	
}
