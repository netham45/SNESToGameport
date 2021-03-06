#include <snestogameport/snesmain.h>
#include <snestogameport/screen.h>
#include <snestogameport/buttons.h>
#include <snestogameport/menu.h>
#include <snestogameport/snes.h>
#include <snestogameport/lcd_hd44780_i2c.h>
#include <snestogameport/eeprom.h>

//eeprom defines
uint16_t VirtAddVarTab[NB_OF_VAR] = { 0 };
uint16_t VarDataTab[NB_OF_VAR] = {0};
uint16_t VarValue = 0;

void snesMain(I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htimdelayus) {

	HAL_FLASH_Unlock();
	uint32_t status = EE_Init(); //eeprom init
	status++;
	HAL_FLASH_Lock();
	snesSetDelayuSTimer(htimdelayus); //Init delayuS timer for snes controller polling
	lcdInit(hi2c, (uint8_t) 0x27, (uint8_t) 20, (uint8_t) 4); //Init LCD
	buttonsGPIODefaultState(); //Init GPIO
	profileSelect(0); //Load profile 0
	menuInit(); //Register menu entries
	
	//Splash
	screenWriteTopLine("SNES -> Gameport");
	screenWriteBottomLine("By Netham45");
	screenClearIn(2);

	while (1) {
		uint16_t buttons = snesPoll(); //Query SNES controller

		//If processMenu returns non-zero then the menu is open and don't process anything else this loop.
		if (menuProcess(buttons))
			continue;

		bindProcess(&buttons);
		screenProcess(buttons);
		buttonsProcess(buttons);
	}
}
