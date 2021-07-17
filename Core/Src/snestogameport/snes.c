#include <snestogameport/snes.h>
//Microsecond Sleep
TIM_HandleTypeDef *htimdelayus = 0;
void setDelayuSTimer(TIM_HandleTypeDef *_htimdelayus) {
	htimdelayus = _htimdelayus;
}

uint8_t tim1_init = 0;
void delayuS(uint16_t us) {
	if (!tim1_init) {
		HAL_TIM_Base_Start(htimdelayus);
		tim1_init = 1;
	}
	__HAL_TIM_SET_COUNTER(htimdelayus, 0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(htimdelayus) < us)
		;  // wait for the counter to reach the us input in the parameter
}

//End Microsecond Sleep

//Query the SNES controller
int pollSNES() {
	uint16_t buttons = 0;
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_SET);
	delayuS(12); // 12µs delay
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_RESET);
	delayuS(6); // 6µs delay
	for (int i = 0; i < 16; i++) {
		buttons |= (HAL_GPIO_ReadPin(SNES_Data_GPIO_Port, SNES_Data_Pin) << i);
		HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_RESET);
		delayuS(12);
		HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_SET);
		delayuS(12);
	}
	buttons = ~buttons;

	return buttons;
}

//End Query SNES controller
