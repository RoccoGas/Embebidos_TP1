/*************************//**
  @file     App.c
  @brief    Shift Register Testbench
 **************************/

#include "board.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
#include "magtek.h"

#define TEST_PERIOD_MS TIMER_MS2TICKS(1000)


static int led = 0;
static rotary_event_t event = 0;

tim_id_t testTimer;

void App_Init(void)
{


}
void App_Run(void)
{

}

