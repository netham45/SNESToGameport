#include <snestogameport/screen.h>
#include <snestogameport/buttons.h>
#include <snestogameport/lcd_hd44780_i2c.h>
/* Screen interface */
uint32_t clearMessageTime = 0;
uint8_t screenShowNormalInput = 0;

char currentTopLine[SCREEN_CSTR_WIDTH];
char currentBottomLine[SCREEN_CSTR_WIDTH];

void screenClear() {
	lcdSetCursorPosition(0, 0);
	lcdPrintStr((uint8_t*) "                ", SCREEN_WIDTH);
	lcdSetCursorPosition(0, 1);
	lcdPrintStr((uint8_t*) "                ", SCREEN_WIDTH);
	lcdBacklight(LCD_BIT_BACKIGHT_OFF);
	currentTopLine[0] = 0;
	currentBottomLine[0] = 0;
}

void screenClearIn(uint8_t seconds) {
	clearMessageTime = HAL_GetTick() + (seconds * 1000);
}

void screenResetClearTime() {
	clearMessageTime = 0;
}

uint32_t screenGetClearMessageTime() {
	return clearMessageTime;
}

uint8_t screenGetShowNormalInput() {
	return screenShowNormalInput;
}

void screenSetShowNormalInput(uint8_t show) {
	screenShowNormalInput = show;
}

void screenWriteTopLine(char *data)

{
	if (strcmp(currentTopLine, data) != 0) {
		strcpy(currentTopLine, data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		lcdSetCursorPosition(0, 0);
		lcdPrintStr((uint8_t*) data, strlen(data));
		lcdPrintStr((uint8_t*) "                ", SCREEN_WIDTH - strlen(data));
	}

}

void screenWriteBottomLine(char *data) {
	if (strcmp(currentBottomLine, data) != 0) {
		strcpy(currentBottomLine, data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		lcdSetCursorPosition(0, 1);
		lcdPrintStr((uint8_t*) data, strlen(data));
		lcdPrintStr((uint8_t*) "                ", SCREEN_WIDTH - strlen(data));
	}
}

void screenProcess(uint16_t buttons) {
	if (clearMessageTime && (clearMessageTime < HAL_GetTick())) {
		screenClear();
		clearMessageTime = 0;
	}

	//If no other message is being shown show the currently pressed keys after rebinding
	if (screenShowNormalInput) {
		if (!clearMessageTime) {
			if (buttons) {
				char buffer[SCREEN_CSTR_WIDTH];
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
