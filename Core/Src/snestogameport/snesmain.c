#include <snestogameport/snesmain.h>

void snesmain(I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htimdelayus) {
	//Splash
	writeTopLine("SNES -> GamePort");
	writeBottomLine("By Netham45");
	clearMessageIn(2);

	setDelayuSTimer(htimdelayus); //Init delayuS timer for snes controller polling
	lcdInit(hi2c, (uint8_t) 0x27, (uint8_t) 20, (uint8_t) 4); //Init LCD
	gpioDefaultState(); //Init GPIO
	selectProfile(0); //Load profile 0
	initMenu(); //Register menu entries

	while (1) {
		uint16_t buttons = pollSNES(); //Query SNES controller

		//If processMenu returns non-zero then the menu is open and don't process anything else this loop.
		if (processMenu(buttons))
			continue;

		processRebinds(&buttons);
		processScreen(buttons);
		processButtons(buttons);
	}
}
