//includes

#include "shift.h"
#include "gpio.h"
#include "timer.h"
#include "board.h"

//pines de hardware

#define TICKS_DATA	TIMER_US2TICKS(300)
#define TICKS_CLOCK TICKS_DATA - 2
#define TICKS_LATCH	TICKS_DATA - 1

//variable globales

static uint16_t data = 0;
static uint16_t buffer = 0;

static uint8_t counter = 0;
static bool ready = 1; //ready = 1 preparado para recibir denuevo

static uint8_t bright = 10; 

// static bool enable = 0; en caso de despues querer utilizar brillo

static tim_id_t shift_id;
static tim_id_t clock_id;
static tim_id_t latch_id;
//static tim_id_t reset_id;

//prototipos para funciones locales
void shiftCallbackData (void);
void shiftClock(void);
void shiftLatch(void);

//funciones

void shiftInit (void){
	//habiilito los correspondientes pines fisicos
	gpioMode (PIN_DATA, OUTPUT);
	gpioMode (PIN_LATCH, OUTPUT);
	gpioMode (PIN_OUTPUT_ENABLE, OUTPUT);
	gpioMode (PIN_CLOCK, OUTPUT);
	gpioMode (PIN_RESET, OUTPUT);

	//valores iniciales
	gpioWrite(PIN_LATCH, 0);
	gpioWrite(PIN_OUTPUT_ENABLE, 0);
	gpioWrite(PIN_CLOCK, 0);
	gpioWrite(PIN_RESET, 1);

	shift_id = timerGetId();
	clock_id = timerGetId();
	latch_id = timerGetId();
}

void shiftWrite (bool value, uint16_t pos){
	if (value)
		buffer |= ( 1 << pos );
	else
		buffer &= ~( 1 << pos );
}

void shiftSend (void){

	//valores iniciales
	ready = 0;
	data = buffer;
	buffer = 0;
	counter = 0;

	gpioWrite(PIN_CLOCK, 0);
	gpioWrite (PIN_LATCH, 0);

	//inicio la funcion periodica a llamarse una vez finalizada envio un flag
	timerStart(shift_id, TICKS_DATA, TIM_MODE_PERIODIC, shiftCallbackData);

}

void shiftCallbackData(){

	gpioWrite(PIN_DATA, (data >> counter) & 1u );

	gpioWrite(PIN_CLOCK, 0);
	timerStart(clock_id, TICKS_CLOCK, TIM_MODE_SINGLESHOT, shiftClock);

	if(counter == bright){
		gpioWrite (PIN_OUTPUT_ENABLE, 1);
	}

	if (counter == SHIFTREGISTER_SIZE - 1){

		timerStop(shift_id);

		gpioWrite (PIN_LATCH, 0);
		gpioWrite (PIN_OUTPUT_ENABLE, 0);
		timerStart(latch_id, TICKS_LATCH, TIM_MODE_SINGLESHOT, shiftLatch);

		return;
	}

	counter += 1;
}

bool shiftIsReady(void){
	return ready;
}

void shiftOutputEnable (uint8_t brightness){
	bright = brightness;
}

//callbacks
void shiftClock(void){
	gpioWrite(PIN_CLOCK, 1);
}

void shiftLatch(void){
	gpioWrite(PIN_LATCH, 1);
	ready = 1;
}



