/***************************************************************************//**
  @file     magtek.c
  @brief    Driver lector de banda magnética Track 2 - FRDM-K64F
 ******************************************************************************/

#include "magtek.h"
#include "hardware.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

#define BITS_PER_CHAR   5       /* 4 datos + 1 paridad          */
#define MAX_BITS        200     /* 40 chars × 5 bits            */

#define CHAR_SS         0x0B   /* ;  Start Sentinel             */
#define CHAR_FS         0x0D   /* =  Field Separator            */
#define CHAR_ES         0x0F   /* ?  End Sentinel               */

/*******************************************************************************
 * VARIABLES
 ******************************************************************************/

static volatile uint8_t  raw[MAX_BITS]; /* bits crudos, 1 por entrada   */
static volatile uint16_t bit_idx = 0;  /* siguiente posición libre      */
static volatile bool     active  = false;
static volatile bool     ready   = false;

/*******************************************************************************
 * IRQ CALLBACKS
 ******************************************************************************/

/* ENABLE — ambos flancos */
static void enable_cb(void)
{
    if (!gpioRead(MAGTEK_PIN_ENABLE))   /* flanco negativo: empieza */
    {
        bit_idx = 0;
        active  = true;
        ready   = false;
    }
    else                                /* flanco positivo: termina */
    {
        active = false;
        if (bit_idx > 0)        // solo si realmente se leyó algo
            ready = true;
    }
}

/* CLOCK — flanco negativo: DATA estable en este momento
 * Solo guardamos si ENABLE está activo y hay DATA válida.
 * "no siempre hay data": solo leer cuando ENABLE está bajo */
static void clock_cb(void)
{
    if (!active)            return;
    if (bit_idx >= MAX_BITS) return;

    raw[bit_idx++] = gpioRead(MAGTEK_PIN_DATA) ? 1 : 0;
}

/*******************************************************************************
 * DECODIFICACIÓN
 ******************************************************************************/

static magtek_result_t decode(uint8_t *buf, uint8_t *len)
{
    uint16_t n_bits  = bit_idx;
    uint16_t n_chars = n_bits / BITS_PER_CHAR;

    if (n_chars < 3)                    /* mínimo SS + ES + LRC         */
        return MAGTEK_ERR_FORMAT;

    /* ---- Armar caracteres y verificar paridad ---- */
    uint8_t chars[40];
    uint8_t lrc = 0;

    for (uint8_t c = 0; c < n_chars; c++)
    {
        uint16_t base = c * BITS_PER_CHAR;

        /* LSB primero: bit 0 del raw es b0 del carácter */
        uint8_t ch = 0;
        for (uint8_t b = 0; b < BITS_PER_CHAR; b++)
            ch |= raw[base + b] << b;

        /* Paridad impar sobre los 5 bits */
        uint8_t ones = 0;
        for (uint8_t b = 0; b < BITS_PER_CHAR; b++)
            ones += (ch >> b) & 1;
        if ((ones & 1) == 0)
            return MAGTEK_ERR_PARITY;

        /* Los 4 bits de datos (sin el bit de paridad en b4) */
        chars[c] = ch & 0x0F;

        /* Acumular LRC (XOR de b0..b3 de todos excepto el LRC) */
        if (c < n_chars - 1)
            lrc ^= chars[c];
    }

    /* ---- Verificar estructura ---- */
    if (chars[0] != CHAR_SS)
        return MAGTEK_ERR_FORMAT;

    uint8_t es_idx = n_chars - 2;
    if (chars[es_idx] != CHAR_ES)
        return MAGTEK_ERR_FORMAT;

    /* ---- Verificar LRC ---- */
    if (lrc != chars[n_chars - 1])
        return MAGTEK_ERR_LRC;

    /* ---- Copiar datos como ASCII (sin SS, ES, LRC) ---- */
    uint8_t out_len = 0;
    for (uint8_t c = 1; c < es_idx; c++)
    {
        if      (chars[c] == CHAR_FS) buf[out_len++] = '=';
        else                          buf[out_len++] = chars[c] + '0';
    }
    buf[out_len] = '\0';
    *len = out_len;

    return MAGTEK_OK;
}

/*******************************************************************************
 * API PÚBLICA
 ******************************************************************************/

void magtekInit(void)
{
    bit_idx = 0;
    active  = false;
    ready   = false;

    gpioMode(MAGTEK_PIN_DATA, INPUT_PULLUP);   /* DATA   */
    gpioMode(MAGTEK_PIN_CLOCK, INPUT_PULLUP);   /* CLOCK  */
    gpioMode(MAGTEK_PIN_ENABLE, INPUT_PULLUP);   /* ENABLE */

    gpioIRQ(MAGTEK_PIN_CLOCK, GPIO_IRQ_MODE_FALLING_EDGE, clock_cb);
    gpioIRQ(MAGTEK_PIN_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES,   enable_cb);
}

bool magtekDataReady(void)
{
    return ready;
}

magtek_result_t magtekGetData(uint8_t *buf, uint8_t *len)
{
    if (!ready)
        return MAGTEK_NO_DATA;

    ready = false;
    return decode(buf, len);
}
