/***************************************************************************//**
  @file     magtek.h
  @brief    Driver lector de banda magnética Track 2 - FRDM-K64F
  @details  DATA  → PA0 (LSB primero, paridad al final)
            CLOCK → PA1 (data estable en flanco negativo)
            ENABLE→ PA7 (activo en bajo mientras pasa la tarjeta)
 ******************************************************************************/

#ifndef _MAGTEK_H_
#define _MAGTEK_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

#define MAGTEK_MAX_CHARS    40

/* Pines del lector — modificar según cableado */
#define MAGTEK_PIN_DATA     PORTNUM2PIN(PD, 0)
#define MAGTEK_PIN_CLOCK    PORTNUM2PIN(PC, 4)
#define MAGTEK_PIN_ENABLE   PORTNUM2PIN(PA, 0)
#define MAGTEK_BITS_PER_CHAR 4     /* 4 bits de datos + 1 paridad = 5 bits/char */

/* Caracteres de control Track 2 */
#define MAGTEK_SS   0x0B            /* Start Sentinel  ';' → 01011     */
#define MAGTEK_FS   0x0D            /* Field Separator '=' → 01101     */
#define MAGTEK_ES   0x0F            /* End Sentinel    '?' → 01111     */

/* Estados de resultado */
typedef enum {
    MAGTEK_OK,
    MAGTEK_NO_DATA,         /* no se pasó ninguna tarjeta aún   */
    MAGTEK_ERR_PARITY,      /* error de paridad en algún carácter */
    MAGTEK_ERR_LRC,         /* error de LRC                     */
    MAGTEK_ERR_FORMAT,      /* sin SS o ES válido               */
    MAGTEK_ERR_OVERFLOW,    /* demasiados caracteres            */
} magtek_result_t;

/*******************************************************************************
 * FUNCIONES
 ******************************************************************************/

/**
 * @brief Inicializa el driver (GPIOs e IRQs).
 *        Llamar después de hw_Init() y hw_EnableInterrupts().
 */
void magtekInit(void);

/**
 * @brief Indica si hay una lectura completa disponible.
 */
bool magtekDataReady(void);

/**
 * @brief Obtiene los datos de la última pasada.
 * @param buf   Buffer destino (mínimo MAGTEK_MAX_CHARS+1 bytes)
 * @param len   Longitud de los datos escritos (sin '\0')
 * @return      MAGTEK_OK o código de error
 */
magtek_result_t magtekGetData(uint8_t *buf, uint8_t *len);

#endif /* _MAGTEK_H_ */

