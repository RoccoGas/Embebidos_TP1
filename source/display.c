//includes

#include "display.h"
#include "shift.h"
#include "timer.h"
#include "table.h"

//Defines
//tire cualquier cosa para que compile
#define	MASC_SEGM	0xFF00
#define	POS_SEGM    8
#define	MASC_SEL    0x00C0
#define	POS_SEL    	6
#define	MASC_LED    0x0030
#define	POS_LED    	4

#define TICKS_DISPLAY TIMER_US2TICKS(2500)

#define POS_U 	0b00
#define POS_D 	0b10
#define POS_C	0b01
#define POS_M	0b11

#define D0		0b00
#define D1		0b10
#define D2		0b01
#define D3		0b11


//variables globales

static uint16_t display = 0;
static tim_id_t display_id;
static uint8_t mul = 0;
static unsigned char characters[4];

//prototipos
void displayUpdate(void);

//funciones
void displayInit(void)
{
	shiftInit();
	display_id = timerGetId();
	timerStart(display_id, TICKS_DISPLAY, TIM_MODE_PERIODIC, displayUpdate);
}

void displayChar(char num,uint8_t pos)
{
	characters[pos] = tabla7Seg[(int)num];
}

void displayStr(char str[4])
{
	for (int i=0; i < 4; i++)
	{
		characters[i] = tabla7Seg[(int)str[i]];
	}
}

void displayLed(uint8_t led)
{
	//pongo el led a encender en la palabra a enviar
	switch(led){
	case 0:
		display &= (~MASC_LED);
		display |= ((uint16_t)D0 << POS_LED);
		break;
	case 1:
		display &= (~MASC_LED);
		display |= ((uint16_t)D1 << POS_LED);
		break;
	case 2:
		display &= (~MASC_LED);
		display |= ((uint16_t)D2 << POS_LED);
		break;
	case 3:
		display &= (~MASC_LED);
		display |= ((uint16_t)D3 << POS_LED);
		break;
	default:
		break;
	}

}

void displayBrightness(uint8_t brightness){
	shiftOutputEnable(brightness);
}

void displayUpdate(void)
{
	mul = mul % 4;

	switch(mul){
	case 0:
		//pongo el character en la palabra a enviar
		display &= (~MASC_SEGM);
		display |= ((uint16_t)characters[mul] << POS_SEGM);

		//pongo la posicion en la palabra a enviar
		display &= (~MASC_SEL);
		display |= ((uint16_t) POS_U << POS_SEL);
		break;
	case 1:
		display &= (~MASC_SEGM);
		display |= ((uint16_t)characters[mul] << POS_SEGM);

		display &= (~MASC_SEL);
		display |= ((uint16_t) POS_D << POS_SEL);
		break;
	case 2:
		display &= (~MASC_SEGM);
		display |= ((uint16_t)characters[mul] << POS_SEGM);

		display &= (~MASC_SEL);
		display |= ((uint16_t) POS_C << POS_SEL);
		break;
	case 3:
		display &= (~MASC_SEGM);
		display |= ((uint16_t)characters[mul] << POS_SEGM);

		display &= (~MASC_SEL);
		display |= ((uint16_t) POS_M << POS_SEL);
		break;
	default:
		//en caso de tener un mul invalido (no deberia ocurrir)
		mul=0;
		break;
	}

	for (uint16_t i = 0; i < SHIFTREGISTER_SIZE; i++) {
		shiftWrite((display >> i) & 1u, i);
	}

	if(shiftIsReady()){
		shiftSend();
		mul += 1;
	}
}
