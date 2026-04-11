

#include "encoder.h"
#include "gpio.h"
#include "timer.h"

//este driver lo pense como allegro maso, te devuelve el evento.

// Encoder board PINM
#define PIN_RCHA PORTNUM2PIN(PB,23) //CAMBIARRRRRR CON LOS PINESSSSS
#define PIN_RCHB PORTNUM2PIN(PB,9)
#define PIN_RSWITCH PORTNUM2PIN(PA,2)

#define PIN_ACTIVE 0
#define PIN_INACTIVE 1

#define ENCODER_POLL_FREQUENCY TIMER_MS2TICKS(1)

#define DEBOUNCE_THRESHOLD TIMER_MS2TICKS(20)
#define LONG_PRESS_THRESHOLD TIMER_MS2TICKS(200)



static rotary_event_t event = IDLE;

static void rotary_event_detector(void){
	static bool lastChannelAState = true;
	bool currentChannelAState = gpioRead(PIN_RCHA);

	if((lastChannelAState == true) && (currentChannelAState == false)){
		event = gpioRead(PIN_RCHB)? LEFT_TURN: RIGHT_TURN;
	}

	lastChannelAState = currentChannelAState;

	///////////////////

	static bool lastButtonState = false;
	bool currentButtonState = gpioRead(PIN_RSWITCH);

	static tim_tick_t switchPressTime;
	static tim_tick_t buttonTimeHeld;

	if((lastButtonState == true) && (currentButtonState == false)){ //apreto
		switchPressTime = timerGetGlobalTicks();
	}
	else if((lastButtonState == false) && (currentButtonState == true)){
		buttonTimeHeld = timerGetGlobalTicks() - switchPressTime;
		event = IDLE;

		if(buttonTimeHeld < DEBOUNCE_THRESHOLD ){ // no paso el debounce
			event = IDLE;
		}
		else if(((buttonTimeHeld >= DEBOUNCE_THRESHOLD) && (buttonTimeHeld <= LONG_PRESS_THRESHOLD))){
			event = BUTTON_PRESS;
		}
		else if (buttonTimeHeld > LONG_PRESS_THRESHOLD){
			event = LONG_BUTTON_PRESS;
		}
	}
	lastButtonState = currentButtonState;
}

rotary_event_t encoderGetState(void){
	rotary_event_t e = event;
	event = IDLE; // para no devolver muchas veces el mismo evento
	return e;
}

void encoderInit(void){

	static bool encoderInitialized = false;
	if(encoderInitialized == false){

		gpioMode(PIN_RCHA, INPUT_PULLUP);
		gpioMode(PIN_RCHB, INPUT_PULLUP);
		gpioMode(PIN_RSWITCH , INPUT_PULLUP);

		timerInit();

		static tim_id_t encoderTimerID;
		encoderTimerID = timerGetId();

		timerStart(encoderTimerID, ENCODER_POLL_FREQUENCY, TIM_MODE_PERIODIC, &rotary_event_detector);
		encoderInitialized = true;
	}
}



