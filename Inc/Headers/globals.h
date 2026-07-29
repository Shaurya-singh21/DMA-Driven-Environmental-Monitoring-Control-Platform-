#ifndef __GLOBALS_H
#define __GLOBALS_H
typedef enum{
	DMA_PROCESS = (1U << 0),
	COOLING_PROCESS = (1U << 1),
	HEATING_PROCESS = (1U << 2),
	START_COOLING = (1U << 3),
	START_HEATING = (1U << 4),
	START_SYS = (1U << 7)
} process;


typedef struct{
	float temp;
	float ldr;
	uint8_t fan;
	uint8_t fan_speed;
	uint8_t door;
	uint8_t vent;
} sys_info;

//ntc
#define A 0.001129148f
#define B 0.000234125f
#define C 0.0000000876741f
//ldr
#define LDR_Threshold 6.6f

#define Rfix 10000.0f

//you can adjust these values based on your specific requirements
#define optimum_temp_low 100.0f
#define optimum_temp_high 110.0f


#endif
