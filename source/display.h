
/***************************************************************************//**
  @file     display.h
  @brief    7 segments and leds managment
  @author   Facundo Torres
 ******************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/*******************************************************************************
 * INCLUDE DEFINES
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * INCLUDE PROTOTYPES
 ******************************************************************************/

void displayInit(void);

void displayChar(char num,uint8_t pos);

void displayStr(char str [4]);

void displayLed(uint8_t num);

/**
 * @brief sets brightness for display
 * @param brightness level of brightness 1min 16max
 */
void displayBrightness(uint8_t brightness);

#endif