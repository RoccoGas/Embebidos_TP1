



#include "gpio.h"
#include "hardware.h"

#define GPIO_PORTS_NUM 5 // uso para la ISR no periodicas buscar q pin causo la interrupcion
#define PINS_PER_PORT 32



bool gpioMode (pin_t pin, uint8_t mode){

    GPIO_Type * gpio;
    PORT_Type * port;

	switch (PIN2PORT(pin))
    {
    case PA:
        SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; // Enable clock to PORTA
                                             // SIM es una macro ((SIM_Type *)SIM_BASE) donde SIM_BASE
                                             //es la dirección base del módulo SIM (sonde esta el clk del periferico)
        port = PORTA;
        gpio = PTA;
        break;
    case PB:
        SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; // Enable clock to PORTB
        port = PORTB;
        gpio = PTB;
        break;
    case PC:
        SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK; // Enable clock to PORTC
        port = PORTC;
        gpio = PTC;
        break;
    case PD:
        SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK; // Enable clock to PORTD
        port = PORTD;
        gpio = PTD;
        break;
    case PE:
        SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; // Enable clock to PORTE
        port = PORTE;
        gpio = PTE;
        break;
    default:
    	return false;
        break;
    }

	port->PCR[PIN2NUM(pin)] = (port->PCR[PIN2NUM(pin)] & ~PORT_PCR_MUX_MASK)
	                         | PORT_PCR_MUX(1); //connecto el mux del puerto al gpio
    switch (mode)
    {
    case INPUT:
        gpio->PDDR &= ~(1 << PIN2NUM(pin)); // Configuro el pin como entrada (PDDR = 0)
        port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_PE_MASK; // Deshabilito las resistencias pull-up/pull-down
        break;
    case OUTPUT:
        gpio->PDDR |= (1 << PIN2NUM(pin)); // Configuro el pin como salida (PDDR = 1)
        port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_PE_MASK; // Deshabilito las resistencias pull-up/pull-down
        break;
    case INPUT_PULLUP:
    	port->PCR[PIN2NUM(pin)] &= ~(PORT_PCR_PE_MASK | PORT_PCR_PS_MASK); //limpio esos
        gpio->PDDR &= ~(1 << PIN2NUM(pin)); // Configuro el pin como entrada (PDDR = 0)
        port->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK; // Habilito las resistencias pull-up/pull-down
        port->PCR[PIN2NUM(pin)] |= PORT_PCR_PS_MASK; // Configuro la resistencia como pull-up (PS = 1)
        break;
    case INPUT_PULLDOWN:
    	port->PCR[PIN2NUM(pin)] &= ~(PORT_PCR_PE_MASK | PORT_PCR_PS_MASK); //limpio esos
        gpio->PDDR &= ~(1 << PIN2NUM(pin)); // Configuro el pin como entrada (PDDR = 0)
        port->PCR[PIN2NUM(pin)] |= PORT_PCR_PE_MASK; // Habilito las resistencias pull-up/pull-down
        port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_PS_MASK; // Configuro la resistencia como pull-down (PS = 0)
         break;
    default:
    	return false;
        break;
    }
    return true;
}

bool gpioRead (pin_t pin){
    GPIO_Type * gpio;

    switch (PIN2PORT(pin))
    {
    case PA: gpio = PTA; break;
    case PB: gpio = PTB; break;
    case PC: gpio = PTC; break;
    case PD: gpio = PTD; break;
    case PE: gpio = PTE; break;
    default: return false;
    }
    if (gpio->PDIR & (1 << PIN2NUM(pin))) // Si el bit correspondiente al pin es 1
        return HIGH;
    else
        return LOW;

}


bool gpioWrite (pin_t pin, bool value){
	GPIO_Type * gpio;

    switch (PIN2PORT(pin))
    {
    case PA: gpio = PTA; break;
    case PB: gpio = PTB; break;
    case PC: gpio = PTC; break;
    case PD: gpio = PTD; break;
    case PE: gpio = PTE; break;
    default: return false;
    	break;
    }
    if (value)
            gpio->PSOR = (1 << PIN2NUM(pin));
        else
            gpio->PCOR = (1 << PIN2NUM(pin));

    return true;
}

bool gpioToggle (pin_t pin){
    GPIO_Type * gpio;

    switch (PIN2PORT(pin))
    {
    case PA: gpio = PTA; break;
    case PB: gpio = PTB; break;
    case PC: gpio = PTC; break;
    case PD: gpio = PTD; break;
    case PE: gpio = PTE; break;
    default: return false;
    }

    gpio->PTOR = (1 << PIN2NUM(pin));
    return true;
}



//////////SECCION INTERRUPCIONES (por hardware)/////////////

static gpio_irq_callback_t callbackTable[GPIO_PORTS_NUM][PINS_PER_PORT]; //aca guardo ell callback correspondiente al port x pin y



static void gpio_irq_dispatch(uint8_t portLetter){ //funcion llamada por __ISR__ PORTx_IRQHandler(void) { gpio_irq_dispatch(Px); }

	PORT_Type * port;

	switch (portLetter)
	{
	case PA: port = PORTA;  break;
	case PB: port = PORTB;  break;
	case PC: port = PORTC;  break;
	case PD: port = PORTD;  break;
	case PE: port = PORTE;  break;
	default:
		break;
	}
	for(uint8_t i = 0; i < PINS_PER_PORT; i++ ){
		if(port->PCR[i] & PORT_PCR_ISF_MASK){
			port->PCR[i]|= PORT_PCR_ISF_MASK; // bajo la flag de interrupcion

			if(callbackTable[portLetter][i] != 0){ //si tiene funcion cargada
						callbackTable[portLetter][i]();
					}
		}

	}
}

bool gpioIRQ(pin_t pin, uint8_t mode, gpio_irq_callback_t fun, uint8_t priority){

	PORT_Type * port;
	IRQn_Type irqn;

	switch (PIN2PORT(pin))
	    {
	    case PA: port = PORTA; irqn = PORTA_IRQn; break;
	    case PB: port = PORTB; irqn = PORTB_IRQn; break;
	    case PC: port = PORTC; irqn = PORTC_IRQn; break;
	    case PD: port = PORTD; irqn = PORTD_IRQn; break;
	    case PE: port = PORTE; irqn = PORTE_IRQn; break;
	    default: return false;
	    }

	if ((PIN2PORT(pin) >= GPIO_PORTS_NUM) || (PIN2NUM(pin) >= PINS_PER_PORT) || (priority > 15)) //valido
	        return false;

	if((mode != DISABLE) && (fun == 0)) //solo podes pasar null si es modo disable
		return false;

	if (mode == DISABLE){
		callbackTable[PIN2PORT(pin)][PIN2NUM(pin)] = 0; //limpio la tabla de callback de pin mandado
		port->PCR[PIN2NUM(pin)] &= ~PORT_PCR_IRQC_MASK; //apago interrupciones

		bool anyActive = false;
		for(uint8_t i = 0; i < PINS_PER_PORT; i++){
			if(callbackTable[PIN2PORT(pin)][i] != 0){
				anyActive = true;
				break;
			}
		}
		if(!anyActive){ // si ninguna del puerto esta activa apago desconecto el puerto desde el nvic
			NVIC_DisableIRQ(irqn);
		}

		return true;

	}
	port->PCR[PIN2NUM(pin)] = (port->PCR[PIN2NUM(pin)] & ~PORT_PCR_IRQC_MASK) //habilito interrupciones
								| PORT_PCR_IRQC(mode); // recordar q mode se ingresa con una macro que tiene ya los bits para ese modo

	callbackTable[PIN2PORT(pin)][PIN2NUM(pin)] = fun; // guardo la funcion en la tabla;

	NVIC_SetPriority(irqn, priority);
	NVIC_EnableIRQ(irqn);

	return true;
}

__ISR__ PORTA_IRQHandler(void) { gpio_irq_dispatch(PA); }
__ISR__ PORTB_IRQHandler(void) { gpio_irq_dispatch(PB); }
__ISR__ PORTC_IRQHandler(void) { gpio_irq_dispatch(PC); }
__ISR__ PORTD_IRQHandler(void) { gpio_irq_dispatch(PD); }
__ISR__ PORTE_IRQHandler(void) { gpio_irq_dispatch(PE); }
