/***************************************************************************//**
  @file     SysTick.h
  @brief    SysTick driver
  @author   Nicol�s Magliola
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
//DRIVER DE SYSTICK. LA APP DEBERIA USAR EL  "TIMER"


#define SYSTICK_ISR_FREQUENCY_HZ 20000U // 50us


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef void (*pisr_callback_t) (void);

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/




/**
 * @brief Load fucntion that is called every period ms , si no esta inicializado systick lo inicializa
 * @param fun: function address that is executed,
 * @return Registration succeed
 */
bool pisrRegisterCallback(pisr_callback_t fun, int period);

/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_

