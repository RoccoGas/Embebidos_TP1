

#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>
#include <stdbool.h>

enum {IDLE, LEFT_TURN, RIGHT_TURN, BUTTON_PRESS, LONG_BUTTON_PRESS};

typedef uint8_t rotary_event_t;

void encoderInit(void);

rotary_event_t encoderGetState(void);


#endif /* ENCODER_H_ */
