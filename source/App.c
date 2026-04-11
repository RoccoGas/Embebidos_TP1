/*************************//**
  @file     App.c
  @brief    Shift Register Testbench
 **************************/

#include "board.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
#include "magtek.h"


typedef enum {
	IDLE, ENTER_ID, VERIFY_ID, ENTER_PASSWORD, VERIFY_PASSWORD,
	ACCESS_GRANTED, ACCESS_DENIED
} app_states_enum;


app_states_enum appState = IDLE;


static char id[8];

rotary_event_t rotaryEvent = IDLE;

static bool displayCursorOn = false;

char rotarySelectChar(char current, rotary_event_t event);
void callbackToggleDisplayIdCursor(void);



void App_Init(void)
{
	timerInit();
	displayInit();
	encoderInit();
	magtekInit();

}
void App_Run(void)
{
	static int triesCounter = 0;
	timerUpdate();
	switch(appState){
	case IDLE:
		if(magtekDataReady()){
			char auxBuffer[MAGTEK_MAX_CHARS+1];
			char lengthOfData;
			magtek_result_t magtekResult = magtekGetData(auxBuffer, &lengthOfData);

			if(magtekResult == MAGTEK_OK){
				appState = VERIFY_ID;
			}
			else{
				//DISPLAY ERROR
			}

		}

		if(rotaryEvent == BUTTON_PRESS || rotaryEvent == LONG_BUTTON_PRESS ){
			appState = ENTER_ID;
		}
		break;
	case ENTER_ID:
		
		tim_id_t encoderTimer = timerGetId();
		timerStart(encoderTimer, TIMER_MS2TICKS(500), TIM_MODE_PERIODIC, &callbackToggleDisplayIdCursor);
		
		bool enteringID = true;
		char index = 0;
		char characterBeingChosen = '0';
		bool modeConfirmInput = false;


		while(enteringID){

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
					enteringID = false;

					timerStop(encoderTimer);
					timerDestroy(encoderTimer);
				}
				else{
					id[index] = characterBeingChosen;
					index++;
				}
				break;
			case LONG_BUTTON_PRESS:
				modeConfirmInput = !modeConfirmInput;
				break;
			default:
				break;
			}

		
		////////////////// MOSTRAR EN DISPLAY EL ID QUE SE ESTA INGRESANDO
			if(displayCursorOn){
				id[index] = characterBeingChosen;
			}	
			else{
				id[index] = '\0';
			}


			if(index >= 4){
				displayStr((id + index - 4)); // muestro los ultimos 4 caracteres del ID
			}
			else{
				//mostrar en el display el ID que se esta ingresando
				char displayBuffer[5] = "    ";
				for(char i = 0; i < index; i++){
					displayBuffer[4-index+i] = id[i];
				}
				displayStr(displayBuffer);
			}


		}

		break;
	case VERIFY_ID:
		
		break;
	case ENTER_PASSWORD:
		break;
	case VERIFY_PASSWORD:
		break;
	case ACCESS_GRANTED:
		break;
	case ACCESS_DENIED:

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
