/***************************************************************************//**
  @file     gpio.c
  @brief    Simple GPIO Pin services, similar to Arduino
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "gpio.h"
#include "hardware.h"


/*******************************************************************************
 * PRIVATE MACROS AND CONSTANTS
 ******************************************************************************/

/* Tabla de punteros a registros PORT (PCR/ISFR) */
static PORT_Type * const port_regs[] = { PORTA, PORTB, PORTC, PORTD, PORTE };

/* Tabla de punteros a registros GPIO (PDDR/PSOR/PCOR/PTOR/PDIR) */
static GPIO_Type * const gpio_regs[] = { PTA, PTB, PTC, PTD, PTE };

/* Tabla de IRQn para cada puerto */
static const IRQn_Type port_irqn[] = {
    PORTA_IRQn, PORTB_IRQn, PORTC_IRQn, PORTD_IRQn, PORTE_IRQn
};

/* Tabla de callbacks: [puerto][pin] */
static pinIrqFun_t irq_table[5][32];

/* Pin de testeo para modo development (PTD0 por defecto) */
#define GPIO_IRQ_TEST_PIN   PORTNUM2PIN(PD, 0)


/*******************************************************************************
 * GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void gpioMode (pin_t pin, uint8_t mode)
{
    uint8_t port = PIN2PORT(pin);
    uint8_t num  = PIN2NUM(pin);

    PORT_Type *prt  = port_regs[port];
    GPIO_Type *gpio = gpio_regs[port];

    uint32_t pcr = PORT_PCR_MUX(1);

    switch (mode)
    {
        case INPUT_PULLUP:
            pcr |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
            break;
        case INPUT_PULLDOWN:
            pcr |= PORT_PCR_PE_MASK;
            break;
        case INPUT:
        case OUTPUT:
        default:
            break;
    }

    prt->PCR[num] = pcr;

    if (mode == OUTPUT)
        gpio->PDDR |=  (1UL << num);
    else
        gpio->PDDR &= ~(1UL << num);

#ifdef GPIO_DEVELOPMENT_MODE
    /* En modo development, configurar el TP como salida la primera vez */
    static bool tp_init = false;
    if (!tp_init)
    {
        tp_init = true;
        uint8_t tp_port = PIN2PORT(GPIO_IRQ_TEST_PIN);
        uint8_t tp_num  = PIN2NUM(GPIO_IRQ_TEST_PIN);
        port_regs[tp_port]->PCR[tp_num] = PORT_PCR_MUX(1);
        gpio_regs[tp_port]->PDDR |= (1UL << tp_num);
        gpio_regs[tp_port]->PCOR  = (1UL << tp_num);  /* TP en bajo */
    }
#endif // GPIO_DEVELOPMENT_MODE
}


bool gpioIRQ (pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
    uint8_t port = PIN2PORT(pin);
    uint8_t num  = PIN2NUM(pin);

    if (port >= 5 || num >= 32)
        return false;
    if (irqMode >= GPIO_IRQ_CANT_MODES)
        return false;

    /* Mapear modo a campo IRQC */
    static const uint8_t irqc_map[] = {
        0x0,    /* GPIO_IRQ_MODE_DISABLE      */
        0x9,    /* GPIO_IRQ_MODE_RISING_EDGE  */
        0xA,    /* GPIO_IRQ_MODE_FALLING_EDGE */
        0xB,    /* GPIO_IRQ_MODE_BOTH_EDGES   */
    };

    /* Read-modify-write: preservar PCR, actualizar IRQC y limpiar flag previo */
    port_regs[port]->PCR[num] =
        (port_regs[port]->PCR[num] & ~PORT_PCR_IRQC_MASK)
        | PORT_PCR_IRQC(irqc_map[irqMode])
        | PORT_PCR_ISF_MASK;    /* limpiar flag espurio antes de habilitar */

    /* Guardar callback */
    irq_table[port][num] = irqFun;

    /* Habilitar o deshabilitar la IRQ del puerto en la NVIC */
    if (irqMode == GPIO_IRQ_MODE_DISABLE)
        NVIC_DisableIRQ(port_irqn[port]);
    else
    {
        NVIC_ClearPendingIRQ(port_irqn[port]);
        NVIC_EnableIRQ(port_irqn[port]);
    }

    return true;
}


void gpioWrite (pin_t pin, bool value)
{
    uint8_t port = PIN2PORT(pin);
    uint8_t num  = PIN2NUM(pin);

    if (value)
        gpio_regs[port]->PSOR = (1UL << num);
    else
        gpio_regs[port]->PCOR = (1UL << num);
}


void gpioToggle (pin_t pin)
{
    gpio_regs[PIN2PORT(pin)]->PTOR = (1UL << PIN2NUM(pin));
}


bool gpioRead (pin_t pin)
{
    return (bool)((gpio_regs[PIN2PORT(pin)]->PDIR >> PIN2NUM(pin)) & 1UL);
}


/*******************************************************************************
 * PRIVATE: ISR genérica para un puerto
 ******************************************************************************/

static void port_irq_handler(uint8_t port)
{
#ifdef GPIO_DEVELOPMENT_MODE
    gpioWrite(GPIO_IRQ_TEST_PIN, HIGH);
#endif // GPIO_DEVELOPMENT_MODE

    PORT_Type *prt = port_regs[port];

    uint32_t isfr = prt->ISFR;  /* leer todos los flags de una sola vez */
    prt->ISFR = isfr;            /* limpiar (write-1-to-clear, registro completo) */

    for (uint8_t pin = 0; pin < 32; pin++)
    {
        if ((isfr & (1UL << pin)) && irq_table[port][pin])
            irq_table[port][pin]();
    }

#ifdef GPIO_DEVELOPMENT_MODE
    gpioWrite(GPIO_IRQ_TEST_PIN, LOW);
#endif // GPIO_DEVELOPMENT_MODE
}


/*******************************************************************************
 * IRQ HANDLERS — uno por puerto
 ******************************************************************************/

__ISR__ PORTA_IRQHandler (void) { port_irq_handler(PA); }
__ISR__ PORTB_IRQHandler (void) { port_irq_handler(PB); }
__ISR__ PORTC_IRQHandler (void) { port_irq_handler(PC); }
__ISR__ PORTD_IRQHandler (void) { port_irq_handler(PD); }
__ISR__ PORTE_IRQHandler (void) { port_irq_handler(PE); }
