#ifndef IND_H
#define IND_H

#include "main.h"
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include "main.h"


#define CS_Pin GPIO_PIN_7
#define CS_GPIO_Port GPIOC

/* Software chip select implementation*/
inline void CS_SET(void)
{
	HAL_GPIO_WritePin(GPIOC, CS_Pin, GPIO_PIN_RESET);
}

inline void CS_RESET(void)
{
	HAL_GPIO_WritePin(GPIOC, CS_Pin, GPIO_PIN_SET);	
}

inline void CS_STROB(void)
{
	CS_RESET();
	CS_SET();	
}

void displayDigits(uint8_t digit1,uint8_t digit2,uint8_t digit3,uint8_t digit4);
void indicatorClear(void);
void displayInt(int16_t value, bool padZeroes,uint8_t dots);
void displayFloat(float value, uint8_t precision, bool padZeroes);
uint8_t getBit(uint8_t byte, uint8_t number);

#define NUM0 		0x81
#define NUM1 		0xF3
#define NUM2 		0x49
#define NUM3 		0x61
#define NUM4 		0x33
#define NUM5 		0x25
#define NUM6 		0x05
#define NUM7 		0xF1
#define NUM8 		0x01
#define NUM9 		0x21
#define CLEAR 	0xFF
#define DOT 		0xFE
#define MINUS   0x7F

#endif
