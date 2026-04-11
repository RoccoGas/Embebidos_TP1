
#include "pisr.h"
#include "hardware.h"
#include "MK64F12.h"

//Data de systick esta en ARM Cortex-M4 - Generic User Guide capitulo 4.4
//info para mi :). systick es un contador de 24-bits que se decrementa cada ciclo de reloj,
// y cuando llega a 0 genera una interrupcion y se recarga con el valor del registro SYST_RVR (Reload Value Register).
//El valor del reload se calcula como (frecuencia del reloj / frecuencia de la interrupcion deseada) - 1.


/*
The SysTick counter reload and current value are not initialized by hardware. This means the
correct initialization sequence for the SysTick counter is:
1. Program reload value.
2. Clear current value.
3. Program Control and Status register.

Registros:
Current Value Register (SYST_CVR): Este registro contiene el valor actual del contador.
Se puede leer para conocer el valor restante antes de que ocurra la próxima interrupción.
También se puede escribir para reiniciar el contador a un valor específico.

Control and Status Register (SYST_CSR): Este registro se utiliza para habilitar o deshabilitar el contador,
habilitar o deshabilitar las interrupciones, y para verificar el estado del contador.

Reload Value Register (SYST_RVR): Este registro se utiliza para establecer el valor de recarga del contador.
Cuando el contador llega a cero, se recarga automáticamente con el valor almacenado en este registro.

 Calibration Value Register (SYST_CALIB): Este registro proporciona información sobre la calibración del SysTick.
*/


#define SYSTICK_PISR_CANT 8

typedef struct{
	 int count;
	 int period;
	pisr_callback_t fun; //puntero a funcion toma void devuelve void
} pisr_t;


static pisr_t pisr[SYSTICK_PISR_CANT];
static unsigned int pisrCounter = 0;

static bool SysTick_Init (void){


	SysTick->CTRL = 0x00;
	SysTick->LOAD = __CORE_CLOCK__ / SYSTICK_ISR_FREQUENCY_HZ - 1; //99.999.999L; // <= 1 ms @ 100Mhz
	SysTick->VAL  = 0x00;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	return true;

}

bool pisrRegisterCallback(pisr_callback_t fun, int period){

	static bool systickIsInitialized = false;

	if(!systickIsInitialized){
		SysTick_Init();
		systickIsInitialized = true;
	}

	if (!fun || period <= 0 ||pisrCounter >= SYSTICK_PISR_CANT){
		return false;
	}
	pisr[pisrCounter].count = 1;
	pisr[pisrCounter].period = period;
	pisr[pisrCounter].fun = fun;
	pisrCounter++;
	return true;
}

__ISR__ SysTick_Handler (void){ //hardware salta aca directamente cuanto se activa el timer, esta en un lugar sabido
	int i = 0;
	for (i = 0 ; i < pisrCounter; i++){
		if (--(pisr[i].count) <= 0)
		        {
		            pisr[i].count = pisr[i].period;
		            pisr[i].fun();
		        }
	}
}



