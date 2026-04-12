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

#define BITS_PER_CHAR   5       /* 4 datos + 1 paridad                  */
#define MAX_BITS        300     /* 40 chars válidos + margen pre-SS      */

#define CHAR_SS         0x0B   /* ;  Start Sentinel                     */
#define CHAR_FS         0x0D   /* =  Field Separator                    */
#define CHAR_ES         0x0F   /* ?  End Sentinel                       */

/* Valor de SS con paridad impar: 0b01011 → 3 unos → impar ✓
 * raw con LSB primero: b0=1 b1=1 b2=0 b3=1 b4=0(paridad) → 0b01011 = 0x0B */
#define SS_WITH_PARITY  0x0B

/*******************************************************************************
 * VARIABLES
 ******************************************************************************/

static volatile uint8_t  raw[MAX_BITS];
static volatile uint16_t bit_idx = 0;
static volatile bool     active  = false;
static volatile bool     ready   = false;

/*******************************************************************************
 * IRQ CALLBACKS
 ******************************************************************************/

static void enable_cb(void)
{
    if (!gpioRead(MAGTEK_PIN_ENABLE))
    {
        bit_idx = 0;
        active  = true;
        ready   = false;
    }
    else
    {
        active = false;
        ready  = true;
    }
}

static void clock_cb(void)
{
    if (!active)             return;
    if (bit_idx >= MAX_BITS) return;

    raw[bit_idx++] = gpioRead(MAGTEK_PIN_DATA) ? 0 : 1;
}

/*******************************************************************************
 * PRIVADAS
 ******************************************************************************/

/* Armar un carácter de 5 bits a partir de la posición 'start' en raw[] */
static uint8_t read_char(uint16_t start)
{
    uint8_t ch = 0;
    for (uint8_t b = 0; b < BITS_PER_CHAR; b++)
        ch |= raw[start + b] << b;
    return ch;
}

/* Paridad impar válida sobre los 5 bits */
static bool parity_ok(uint8_t ch)
{
    uint8_t ones = 0;
    for (uint8_t b = 0; b < BITS_PER_CHAR; b++)
        ones += (ch >> b) & 1;
    return (ones & 1) == 1;
}

/*******************************************************************************
 * DECODIFICACIÓN
 ******************************************************************************/

static magtek_result_t decode(uint8_t *buf, uint8_t *len)
{
    uint16_t n_bits = bit_idx;

    /* ---- Buscar SS bit a bit ----
     * Probamos cada posición de inicio posible hasta encontrar
     * 5 bits que formen el SS con paridad impar válida.
     * Así no dependemos de que los bits basura sean múltiplo de 5. */
    uint16_t ss_bit = 0xFFFF;

    for (uint16_t i = 0; i + BITS_PER_CHAR <= n_bits; i++)
    {
        uint8_t ch = read_char(i);
        if (ch == SS_WITH_PARITY)   /* paridad + valor coinciden con SS */
        {
            ss_bit = i;
            break;
        }
    }

    if (ss_bit == 0xFFFF)
        return MAGTEK_ERR_FORMAT;   /* no se encontró SS */

    /* ---- A partir del SS, decodificar alineado a BITS_PER_CHAR ---- */
    uint8_t chars[60];
    uint8_t n_chars = 0;
    bool    found_es = false;

    for (uint16_t bit = ss_bit;
         bit + BITS_PER_CHAR <= n_bits && n_chars < 60;
         bit += BITS_PER_CHAR)
    {
        uint8_t ch = read_char(bit);

        if (!parity_ok(ch))
            return MAGTEK_ERR_PARITY;

        chars[n_chars++] = ch & 0x0F;  /* 4 bits de datos */

        if ((ch & 0x0F) == CHAR_ES)
        {
            found_es = true;
            break;
        }
    }

    if (!found_es)
        return MAGTEK_ERR_FORMAT;

    /* ---- Leer LRC (carácter siguiente al ES) ---- */
    uint16_t lrc_bit = ss_bit + n_chars * BITS_PER_CHAR;
    if (lrc_bit + BITS_PER_CHAR > n_bits)
        return MAGTEK_ERR_FORMAT;

    uint8_t lrc_ch = read_char(lrc_bit) & 0x0F;

    /* ---- Calcular LRC: XOR de b0..b3 desde SS hasta ES inclusive ---- */
    uint8_t lrc = 0;
    for (uint8_t c = 0; c < n_chars; c++)
        lrc ^= chars[c];

    if (lrc != lrc_ch)
        return MAGTEK_ERR_LRC;

    /* ---- Copiar datos (sin SS ni ES) ---- */
    uint8_t out_len = 0;
    for (uint8_t c = 1; c < n_chars - 1; c++)  /* c=0 es SS, último es ES */
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
void magtekEnableIRQ(void)
{
    gpioIRQ(MAGTEK_PIN_CLOCK,  GPIO_IRQ_MODE_FALLING_EDGE, clock_cb);
    gpioIRQ(MAGTEK_PIN_ENABLE, GPIO_IRQ_MODE_BOTH_EDGES,   enable_cb);
}

void magtekDisableIRQ(void)
{
    gpioIRQ(MAGTEK_PIN_CLOCK,  GPIO_IRQ_MODE_DISABLE, 0);
    gpioIRQ(MAGTEK_PIN_ENABLE, GPIO_IRQ_MODE_DISABLE, 0);
}


void magtekInit(void)
{
    bit_idx = 0;
    active  = false;
    ready   = false;

    gpioMode(MAGTEK_PIN_DATA,   INPUT);
    gpioMode(MAGTEK_PIN_CLOCK,  INPUT);
    gpioMode(MAGTEK_PIN_ENABLE, INPUT);

    magtekEnableIRQ();
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
