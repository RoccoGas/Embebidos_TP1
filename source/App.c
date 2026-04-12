/*************************//**
  @file     App.c
  @brief    Shift Register Testbench
 **************************/

#include "board.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
#include "magtek.h"
#include <string.h>


#define MAX_ID_LENGTH 8

#define MAX_PASSWORD_LENGTH 5

// Agregar variable global
static int matchedUserIndex = -1;


typedef enum {
	APP_IDLE, ENTER_ID, VERIFY_ID, ENTER_PASSWORD, VERIFY_PASSWORD,
	ACCESS_GRANTED, ACCESS_DENIED
} app_states_enum;



app_states_enum appState = APP_IDLE;


static char id[MAX_ID_LENGTH + 1];
static char password[MAX_PASSWORD_LENGTH + 1];


rotary_event_t rotaryEvent = IDLE;

static bool displayCursorOn = false;

bool volatile accessTimerStarted = false;

typedef struct {
    const char id[9];
    const char password[6];
} credential_t;

static const credential_t credentials[] = {
    { "12345678", "12345" },
    { "USER0001", "pass1" },
    { "USER0002", "abc12" },
    { "ADMIN001", "9999"  },
};

#define MAX_ID_CANT (sizeof(credentials) / sizeof(credentials[0]))

char rotarySelectChar(char current, rotary_event_t event);
void callbackToggleDisplayIdCursor(void);
void callbackAccessTimer(void);



void App_Init(void)
{
	timerInit();
	displayInit();
	encoderInit();
	magtekInit();
	gpioMode(PIN_LED_GREEN, OUTPUT);
	gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);

	gpioMode(PIN_LED_RED, OUTPUT);
	gpioWrite(PIN_LED_RED, !LED_ACTIVE);
}
void App_Run(void)
{
	static int triesCounter = 0;
	timerUpdate();
	rotaryEvent = encoderGetState();
	switch(appState){
	case APP_IDLE:
		if(magtekDataReady()){
			char auxBuffer[MAGTEK_MAX_CHARS+1];
			uint8_t lengthOfData;
			magtek_result_t magtekResult = magtekGetData(auxBuffer, &lengthOfData);


			if(magtekResult == MAGTEK_OK){
				// auxBuffer contiene el PAN limpio, ej: "12345678901234567890..."
				// los primeros 8 son el ID (sin dígito de control = sin el último del PAN)
				for(int i = 0; i < MAX_ID_LENGTH && i < lengthOfData - 1; i++){
					id[i] = auxBuffer[i];  // lengthOfData - 1 para excluir el dígito de control
				}
				id[MAX_ID_LENGTH] = '\0';
				appState = VERIFY_ID;
			}
			else{
				displayStr("Err ");
			}

		}

		if(rotaryEvent == BUTTON_PRESS || rotaryEvent == LONG_BUTTON_PRESS ){
			appState = ENTER_ID;
		}
		break;
	case ENTER_ID:


		static tim_id_t encoderTimer;
  		static bool timerStarted = false;
    	static char index = 0;
    	static char characterBeingChosen = '0';
    	static bool modeConfirmInput = false;


		if(!timerStarted){
			encoderTimer = timerGetId();
			timerStart(encoderTimer, TIMER_MS2TICKS(500), TIM_MODE_PERIODIC, &callbackToggleDisplayIdCursor);
			index = 0;
			characterBeingChosen = '0';
			modeConfirmInput = false;
			timerStarted = true;
		}





		switch(rotaryEvent){
		case LEFT_TURN:
		case RIGHT_TURN:
			if(!modeConfirmInput){
				characterBeingChosen = rotarySelectChar(characterBeingChosen, rotaryEvent);
			}
			else{
				if(rotaryEvent == LEFT_TURN){
					modeConfirmInput = false;
					if(index > 0){
						id[index] = '\0';
						index--;
					}
				}
				else if(rotaryEvent == RIGHT_TURN){
					modeConfirmInput = false;
				}

			}
			break;
		case BUTTON_PRESS:
			if(modeConfirmInput){
				appState = VERIFY_ID;

				timerStop(encoderTimer);
				timerDestroy(encoderTimer);
				timerStarted = false;
			}
			else{
				if(index < 8){
					id[index] = characterBeingChosen;
					index++;
				}
			}
			break;
		case LONG_BUTTON_PRESS:
			modeConfirmInput = !modeConfirmInput;
			break;
		default:
			break;
		}

	////////////////// MOSTRAR EN DISPLAY EL ID QUE SE ESTA INGRESANDO
		char displayBuffer[5] = {0};
		
		if(index >= 3){
			for(int i = 0; i < 4; i++){
		        displayBuffer[i] = id[index - 3 + i]; // muestro los ultimos 4 caracteres del ID
		    }
		    displayBuffer[4] = '\0';
	
		}
		else{
		    for(char i = 0; i < index; i++){
		        displayBuffer[i] = id[i];    // confirmed chars left-aligned
		    }
		    displayBuffer[index] = id[index]; // cursor char — already set to characterBeingChosen or '\0'
		}

		displayStr(displayBuffer);
		displayChar(displayCursorOn? characterBeingChosen : '\0', index >= 4 ? 3 : index);
		modeConfirmInput? displayLed(2) : displayLed(1);
		break;
	case VERIFY_ID:
		displayLed(0);
		if(triesCounter >= 3){
			appState = ACCESS_DENIED;
		}
		else{
			bool idMatch = false;
						// En VERIFY_ID, guardar el índice
			for(int i = 0; i < MAX_ID_CANT; i++){
				if(strcmp(id, credentials[i].id) == 0){
					idMatch = true;
					matchedUserIndex = i;  // <-- guardar
					break;
				}
			}
			if(idMatch){
				appState = ENTER_PASSWORD;
			}
			else{
				appState = APP_IDLE;
				triesCounter++;
				for(int i = 0; i < MAX_ID_LENGTH + 1; i++){
					id[i] = '\0';
				}
			}
		}

		break;
	case ENTER_PASSWORD:
		static tim_id_t pwEncoderTimer;
  		static bool pwTimerStarted = false;
    	static char pwIndex = 0;
    	static char pwCharacterBeingChosen = '0';
    	static bool pwModeConfirmInput = false;


		if(!pwTimerStarted){
			pwEncoderTimer = timerGetId();
			timerStart(pwEncoderTimer, TIMER_MS2TICKS(500), TIM_MODE_PERIODIC, &callbackToggleDisplayIdCursor);
			pwIndex = 0;
			pwCharacterBeingChosen = '0';
			pwModeConfirmInput = false;
			pwTimerStarted = true;
		}





		switch(rotaryEvent){
		case LEFT_TURN:
		case RIGHT_TURN:
			if(!pwModeConfirmInput){
				pwCharacterBeingChosen = rotarySelectChar(pwCharacterBeingChosen, rotaryEvent);
			}
			else{
				if(rotaryEvent == LEFT_TURN){
					pwModeConfirmInput = false;
					if(pwIndex > 0){
						password[pwIndex] = '\0';
						pwIndex--;
					}
				}
				else if(rotaryEvent == RIGHT_TURN){
					pwModeConfirmInput = false;
				}

			}
			break;
		case BUTTON_PRESS:
			if(pwModeConfirmInput){
				appState = VERIFY_PASSWORD;

				timerStop(pwEncoderTimer);
				timerDestroy(pwEncoderTimer);
				pwTimerStarted = false;
			}
			else{
				if(pwIndex < 5){
					password[pwIndex] = pwCharacterBeingChosen;
					pwIndex++;
				}
			}
			break;
		case LONG_BUTTON_PRESS:
			pwModeConfirmInput = !pwModeConfirmInput;
			break;
		default:
			break;
		}

	////////////////// MOSTRAR EN DISPLAY EL ID QUE SE ESTA INGRESANDO
		for(int i = 0; i < 4; i++){
			displayBuffer[i] = '\0'; // muestro asteriscos para el password
		}
		
		for(int i = 0; i < ((pwIndex >= 3) ? 3 : pwIndex) ; i++){
			displayBuffer[i] = '-'; // muestro los ultimos 4 caracteres del password
		}
	

		displayStr(displayBuffer);
		displayChar(displayCursorOn? pwCharacterBeingChosen : '\0', pwIndex >= 4 ? 3 : pwIndex);
		pwModeConfirmInput? displayLed(2) : displayLed(1);

		break;
	case VERIFY_PASSWORD:
	displayLed(0);
	if(triesCounter >= 3){
			appState = ACCESS_DENIED;
		}
		else{
			bool idMatch = false;
			if(strcmp(password, credentials[matchedUserIndex].password) == 0){
				idMatch = true;
			}
			if(idMatch){
				appState = ACCESS_GRANTED;
			}
			else{
				appState = ENTER_PASSWORD;
				triesCounter++;
			}
		}
		break;
	case ACCESS_GRANTED:
		gpioWrite(PIN_LED_GREEN, LED_ACTIVE);
		static tim_id_t AccessTimerGranted;
		accessTimerStarted = true;
			AccessTimerGranted = timerGetId();
			timerStart(AccessTimerGranted, TIMER_MS2TICKS(5000), TIM_MODE_SINGLESHOT, &callbackAccessTimer);
			while (accessTimerStarted)
			{
				timerUpdate();
			}
			gpioWrite(PIN_LED_GREEN, !LED_ACTIVE);
			appState = APP_IDLE;
		break;
	case ACCESS_DENIED:
		gpioWrite(PIN_LED_RED, LED_ACTIVE);
		static tim_id_t AccessTimerDenied;
		accessTimerStarted = true;
			AccessTimerDenied = timerGetId();
			timerStart(AccessTimerDenied, TIMER_MS2TICKS(5000), TIM_MODE_SINGLESHOT, &callbackAccessTimer);
			while (accessTimerStarted)
			{
				timerUpdate();
			}
			gpioWrite(PIN_LED_RED, !LED_ACTIVE);
			appState = APP_IDLE;
		triesCounter = 0;
		break;
	}

}


char rotarySelectChar(char current, rotary_event_t event)
{
    int rotation = (event == RIGHT_TURN) ? +1 : -1;
    char next = current + rotation;

    // Saltar huecos entre rangos
    if (next > '9' && next < 'A') next = (rotation > 0) ? 'A' : '9';
    if (next > 'Z' && next < 'a') next = (rotation > 0) ? 'a' : 'Z';

    // Wrap-around entre extremos
    if (next > 'z') next = '0';
    if (next < '0') next = 'z';

    return next;
}

void callbackToggleDisplayIdCursor(void){
	displayCursorOn = !displayCursorOn;
}

void callbackAccessTimer(void){
	accessTimerStarted = false;
}


