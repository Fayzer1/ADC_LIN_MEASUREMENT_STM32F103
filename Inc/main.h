/*
 * main.h
 *
 *  Created on: Feb 7, 2026
 *      Author: A.E. Lisitsa
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f1xx.h"
#include "stdbool.h"

/* Прототипы функций */
void initClk(void);
void delay(uint32_t delay_value);
void init_ports(void);
void indicator(uint8_t number, uint8_t indicator_index);
void init_tim2(void);
void init_tim3(void);
void init_adc(void);
void convert(void);
uint16_t read_adc(uint8_t channel);
void indication(void);

#endif /* MAIN_H_ */
