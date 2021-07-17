#include <snestogameport/snes.h>
//Microsecond Sleep, only used by snes controller polling
TIM_HandleTypeDef *htimdelayus = 0;
void snesSetDelayuSTimer(TIM_HandleTypeDef *_htimdelayus) {
	htimdelayus = _htimdelayus;
	HAL_TIM_Base_Start(htimdelayus);
}

void delayuS(uint16_t us) {
	__HAL_TIM_SET_COUNTER(htimdelayus, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(htimdelayus) < us)
		;  // wait for the counter to reach the us input in the parameter
}

//End Microsecond Sleep

//Query the SNES controller
int snesPoll() {
	uint16_t buttons = 0;
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_SET); //Pulse latch to capture current button state in controller state register
	delayuS(12); // 12µs delay
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_RESET);
	delayuS(6); // 6µs delay
	for (int i = 0; i < 16; i++) {
		buttons |= ((!HAL_GPIO_ReadPin(SNES_Data_GPIO_Port, SNES_Data_Pin)) << i); //Read button and set the bitmask index, logically inverted coming in from the controller
		HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_RESET); //Pulse clock to advance button being reported
		delayuS(12);
		HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_SET);
		delayuS(12);
	}

	return buttons;
}

//End Query SNES controller
