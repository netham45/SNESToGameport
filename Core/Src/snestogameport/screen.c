#include <snestogameport/screen.h>
#include <snestogameport/buttons.h>
#include <snestogameport/lcd_hd44780_i2c.h>
/* Screen interface */
uint32_t clearMessageInTime = 0;
uint8_t screenShowNormalInput = 0;

char topLine[17];
char bottomLine[17];

void screenClear() {
	lcdSetCursorPosition(0, 0);
	lcdPrintStr((uint8_t*) "                ", 16);
	lcdSetCursorPosition(0, 1);
	lcdPrintStr((uint8_t*) "                ", 16);
	lcdBacklight(LCD_BIT_BACKIGHT_OFF);
	topLine[0] = 0;
	bottomLine[0] = 0;
	//Update screens
}

void screenClearIn(uint8_t seconds) {
	clearMessageInTime = HAL_GetTick() + (seconds * 1000);
}

void screenResetClearTimer() {
	clearMessageInTime = 0;
}

uint32_t screenGetClearMessageTime() {
	return clearMessageInTime;
}

uint8_t screenGetShowNormalInput() {
	return screenShowNormalInput;
}

void screenSetShowNormalInput(uint8_t show) {
	screenShowNormalInput = show;
}

void screenWriteTopLine(char *data)

{
	if (strcmp(topLine, data) != 0) {
		strcpy(topLine, data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		//Update screen
		lcdSetCursorPosition(0, 0);
		lcdPrintStr((uint8_t*) data, strlen(data));
		lcdPrintStr((uint8_t*) "                ", 16 - strlen(data));
	}

}

void screenWriteBottomLine(char *data) {
	if (strcmp(bottomLine, data) != 0) {
		strcpy(bottomLine, data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		//Update screen
		lcdSetCursorPosition(0, 1);
		lcdPrintStr((uint8_t*) data, strlen(data));
		lcdPrintStr((uint8_t*) "                ", 16 - strlen(data));
	}
}

void screenProcess(uint16_t buttons) {
	if (clearMessageInTime && (clearMessageInTime < HAL_GetTick())) {
		screenClear();
		clearMessageInTime = 0;
	}

	//If no other message is being shown show the currently pressed keys after rebinding
	if (screenShowNormalInput) {
		if (!clearMessageInTime) {
			if (buttons) {
				char buffer[17];
				buttonsToString(buffer, buttons, "\xA5");
				screenWriteTopLine(buffer);
				screenWriteBottomLine("");
			} else {
				screenClear();
			}
		}
	}
}

//End Screen Interface
