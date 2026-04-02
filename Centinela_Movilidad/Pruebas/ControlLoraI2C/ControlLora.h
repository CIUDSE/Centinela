#ifndef CONTROLLORA_H
#define CONTROLLORA_H

#include <stdint.h>

struct __attribute__((packed)) ControlData {
	int16_t x;       // -155 a 155
	int16_t y;       // -155 a 155
	uint8_t brakes;  // 0 o 1
};

extern volatile ControlData rxData;
extern volatile bool newData;

void onReceive(int numBytes);

#endif // CONTROLLORA_H
