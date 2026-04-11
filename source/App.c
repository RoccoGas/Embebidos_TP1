/*************************//**
  @file     App.c
  @brief    Shift Register Testbench
 **************************/

#include "board.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
#include "magtek.h"

enum {IDLE, ENTER_ID, VERIFY_ID, ENTER_PASSWORD, VERIFY_PASSWORD,
		ACCESS_GRANTED, ACCESS_DENIED }app_states_enum ;

app_states_enum appState = IDLE;


static char id[8];

rotary_event_t rotaryEvent = IDLE;


void App_Init(void)
{
	timerInit();
	displayInit();
	encoderInit();
	magtekInit();

}
void App_Run(void)
{
	timerUpdate();
	switch(appState){
	case IDLE:
		if(magtekDataReady()){
			char auxBuffer[MAGTEK_MAX_CHARS+1];
			char lengthOfData;
			magtek_result_t magtekResult = magtekGetData(auxbuffer, &lengthOfData);

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
		bool enteringID = true;
		char characterBeingChosen = '0';
		bool modeConfirmInput = false;
		while(enteringID){

			switch(rotaryEvent){
			case LEFT_TURN:

				break;
			default:
				break;
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
			break;
	}

}
