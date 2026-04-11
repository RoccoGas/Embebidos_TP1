/***************************************************************************//**
  @file     shift.h
  @brief    shift register managment
  @author   Facundo Torres
 ******************************************************************************/


#ifndef _SHIFT_H_
#define _SHIFT_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * INCLUDE DEFINES
 ******************************************************************************/

#define SHIFTREGISTER_SIZE 16

/*******************************************************************************
 * INCLUDE PROTOTYPES
 ******************************************************************************/

/**
 * @brief Initialice shiftregister controller and corresponding peripheral
 */
void shiftInit (void);

/**
 * @brief allows to write on any bit of the 16.
 * @param value state of the bit to write
 * @param pos position of the bit to write
 */
void shiftWrite (bool value, uint16_t pos);

/**
 * @brief Starts the shiftregister data output
 * @return true = data has finished sending
 */
void shiftSend (void);


/**
 * @brief asks if the shiftregister has ended its tasks
 * @return true = data has finished sending
 */
bool shiftIsReady(void);


// void shiftOutputEnable (bool ena);



#endif

