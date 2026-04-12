/***************************************************************************//**
  @file     timer.h
  @brief    Timer driver. Advance implementation, Non-Blocking services
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TIMER_TICK_US           50U    // 50 micro
#define TIMER_MS2TICKS(ms)      ((ms) * 1000U / TIMER_TICK_US) //para no romper las q ya estaban
#define TIMER_US2TICKS(us)      ((us) / TIMER_TICK_US)

#define TIMERS_MAX_CANT     16
#define TIMER_INVALID_ID    255


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

// Timer Modes
enum { TIM_MODE_SINGLESHOT, TIM_MODE_PERIODIC, CANT_TIM_MODES };

// Timer alias
typedef uint32_t tim_tick_t;
typedef uint8_t tim_id_t;
typedef void (*tim_callback_t)(void);


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialice timer and corresponding peripheral
 */
void timerInit(void);


/**
 * @brief Request an timer
 * @return ID of the timer to use
 */
tim_id_t timerGetId(void);


/**
 * @brief Begin to run a new timer
 * @param id ID of the timer to start
 * @param ticks time until timer expires, in ticks
 * @param mode SINGLESHOT or PERIODIC
 * @param callback Function to be call when timer expires (NULL if no necessary)
 * @return true = timer start succeed
 */
bool timerStart(tim_id_t id, tim_tick_t ticks, uint8_t mode, tim_callback_t callback);


/**
 * @brief Finish to run a timer
 * @param id ID of the timer to stop
 */
void timerStop(tim_id_t id);


/**
 * @brief Verify if a timer has run timeout AND LOWERS FLAG if true
 * @param id ID of the timer to check for expiration
 * @return true = timer expired
 */
bool timerExpired(tim_id_t id);

/**
 * @brief Verify if a timer has run timeout and DOES NOT put down the flag
 * @param id ID of the timer to check for expiration
 * @return true = timer expired
 */
bool peekTimerExpired(tim_id_t id);
/**
 * @brief Free up the space in the timer array and free
 * @param id ID of the timer to destroy
 */
void timerDestroy(tim_id_t id);


/**
 * @brief Call respective callbacks if timeout ocurrs. Must be call from main loop.
 */
void timerUpdate(void);


tim_tick_t timerGetGlobalTicks(void);

/*******************************************************************************
 ******************************************************************************/

#endif // _TIMER_H_
