#ifndef __GLOBALS_H
#define __GLOBALS_H
typedef enum{
	DMA_PROCESS = (1U << 0),
	COOLING_PROCESS = (1U << 1),
	HEATING_PROCESS = (1U << 2),
	START_SYS = (1U << 7)
} process;

typedef struct{
	float temp;
	uint8_t fan;
	uint8_t door;
	uint8_t ai;
} sys_info;

//ntc
#define A 0.001129148f
#define B 0.000234125f
#define C 0.0000000876741f
//ldr
#define LDR_Threshold 8.5f

#define Rfix 10000.0f

#define optimum_temp_low 20.0f
#define optimum_temp_high 21.0f


#endif
