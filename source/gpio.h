

#ifndef _GPIO_H_
#define _GPIO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// Ports
enum { PA, PB, PC, PD, PE };

// Convert port and number into pin ID
// Ex: PTB5  -> PORTNUM2PIN(PB,5)  -> 0x25
//     PTC22 -> PORTNUM2PIN(PC,22) -> 0x56
#define PORTNUM2PIN(p,n)    (((p)<<5) + (n))
#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F)


// Modes
#ifndef INPUT
#define INPUT               0
#define OUTPUT              1
#define INPUT_PULLUP        2
#define INPUT_PULLDOWN      3
#endif // INPUT


// Digital values
#ifndef LOW
#define LOW     0
#define HIGH    1
#endif // LOW

#ifndef __IRQ_TRIGGER_MODES__
#define __IRQ_TRIGGER_MODES
#define DISABLE				0x00
#define POSEDGE				0x9 // 0b1001
#define NEGEDGE             0xA // 0b1010
#define BOTH_EDGES	        0xB // 0b1011
#define LOW_LEVEL	 		0x8	// 0b1000
#define HIGH_LEVEL			0xC // 0b1100

#define SWITCH_PRESS NEGEDGE
#define SWITCH_RELEASE POSEDGE
#endif // IRQ_TRIGGER_MODES


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint8_t pin_t;

typedef void (*gpio_irq_callback_t)(void);


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 * @return true if the operation was successful, false otherwise
 */
bool gpioMode (pin_t pin, uint8_t mode);

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 * @return true if the operation was successful, false otherwise
 */
bool gpioWrite (pin_t pin, bool value);

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 * @return true if the operation was successful, false otherwise
 */
bool gpioToggle (pin_t pin);

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW, LOW on error
 */
bool gpioRead (pin_t pin);


bool gpioIRQ(pin_t pin, uint8_t mode, gpio_irq_callback_t fun, uint8_t priority);

/*******************************************************************************
 ******************************************************************************/

#endif // _GPIO_H_

