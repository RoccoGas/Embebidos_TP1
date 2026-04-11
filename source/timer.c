

#include "pisr.h"
#include "timer.h"

typedef struct{
	tim_tick_t deadline;
	tim_tick_t period;
	tim_callback_t fun;
	uint8_t mode;
	bool isRunning;
	bool isTaken;
	bool isExpired;
} timer_t;

static timer_t timers[TIMERS_MAX_CANT];
static volatile tim_tick_t globalCount = 0;

static void timer_isr_tick(void){
	globalCount++;
}

void timerInit(void){
	static bool timerInitialized = false;
	if(!timerInitialized){
		pisrRegisterCallback(&timer_isr_tick, 1); //esto seria el "clock global", cada vez q dispara systick incremento el global
		timerInitialized = true;
	}
}

tim_id_t timerGetId(void){ //te devuelve un slop libre en el array de timers
	for(int i = 0; i < TIMERS_MAX_CANT; i++){ //ID es simplemente su posicion en el array
		if(!timers[i].isTaken){
			timers[i].isTaken = true;
			return (tim_id_t)i ;
		}
	}
	return TIMER_INVALID_ID;
}


bool timerStart(tim_id_t id, tim_tick_t ticks, uint8_t mode, tim_callback_t callback){
	timers[id].period = ticks; //guardo por si es periodico
	timers[id].deadline = globalCount + ticks;
	timers[id].isRunning = true;
	timers[id].mode = mode;
	timers[id].fun = callback;
	timers[id].isExpired = false;
	return true;
}

void timerStop(tim_id_t id){
	timers[id].isRunning = false;
}

bool peekTimerExpired(tim_id_t id){
	return timers[id].isExpired;
}

bool timerExpired(tim_id_t id){
	if(timers[id].isExpired){
		timers[id].isExpired = false;
		return true;
	}
	else{
		return false;
	}
}

void timerDestroy(tim_id_t id){
	timers[id].isTaken = false;
	timers[id].isRunning = false;
	timers[id].fun = 0;
}

void timerUpdate(void){
	for(int i = 0; i < TIMERS_MAX_CANT; i++){
		if((globalCount >= timers[i].deadline) && timers[i].isRunning){
			if(timers[i].mode == TIM_MODE_SINGLESHOT){
				timers[i].isRunning = false;
			}
			else{
				timers[i].deadline = globalCount + timers[i].period;
			}
			timers[i].isExpired = true;
			if(timers[i].fun != 0){
				timers[i].fun();
			}
		}
	}
}

tim_tick_t timerGetGlobalTicks(void){
	return globalCount;
}







