#include <snestogameport/screen.h>
/* Screen interface */
uint32_t clearMessageInTime = 0;
uint8_t screenShowNormalInput = 0;

char topLine[17];
char bottomLine[17];

void clearMessage() {
	lcdSetCursorPosition(0, 0);
	lcdPrintStr((uint8_t*) "                ", 16);
	lcdSetCursorPosition(0, 1);
	lcdPrintStr((uint8_t*) "                ", 16);
	lcdBacklight(LCD_BIT_BACKIGHT_OFF);
	topLine[0] = 0;
	bottomLine[0] = 0;
	//Update screens
}

void clearMessageIn(uint8_t seconds) {
	clearMessageInTime = HAL_GetTick() + (seconds * 1000);
}

void clearClearMessage() {
	clearMessageInTime = 0;
}

uint32_t clearMessageTime() {
	return clearMessageInTime;
}

uint8_t screenGetShowNormalInput() {
	return screenShowNormalInput;
}

void screenSetShowNormalInput(uint8_t show) {
	screenShowNormalInput = show;
}

void writeTopLine(char *data)

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

void writeBottomLine(char *data) {
	if (strcmp(bottomLine, data) != 0) {
		strcpy(bottomLine, data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		//Update screen
		lcdSetCursorPosition(0, 1);
		lcdPrintStr((uint8_t*) data, strlen(data));
		lcdPrintStr((uint8_t*) "                ", 16 - strlen(data));
	}
}

void buttonToString(char *stringBuffer, uint16_t buttons, char *prefix) {
	char buffer[32] = { 0 };
	strcpy(buffer, prefix);
	uint8_t offset = strlen(buffer);
	uint8_t written = 0;
	for (int i = 0; i < 12; i++) {
		if (buttons & (1 << i)) {
			written = 1;
			switch (i) {
			case 8:
				sprintf(buffer + offset, "A+");
				break;
			case 0:
				sprintf(buffer + offset, "B+");
				break;
			case 9:
				sprintf(buffer + offset, "X+");
				break;
			case 1:
				sprintf(buffer + offset, "Y+");
				break;
			case 4:
				sprintf(buffer + offset, "Up+");
				break;
			case 5:
				sprintf(buffer + offset, "Down+");
				break;
			case 6:
				sprintf(buffer + offset, "Left+");
				break;
			case 7:
				sprintf(buffer + offset, "Right+");
				break;
			case 10:
				sprintf(buffer + offset, "L+");
				break;
			case 11:
				sprintf(buffer + offset, "R+");
				break;
			case 3:
				sprintf(buffer + offset, "Start+");
				break;
			case 2:
				sprintf(buffer + offset, "Select+");
				break;
			}
			offset += strlen(buffer + offset);
		}
	}
	if (written)
		buffer[strlen(buffer) - 1] = 0; //Remove the final plus
	buffer[16] = 0; //Cap length at 16 chars
	memcpy(stringBuffer, buffer, 17); //Copy 16 chars + terminator
}

void processScreen(uint16_t buttons) {
	if (clearMessageInTime && (clearMessageInTime < HAL_GetTick())) {
		clearMessage();
		clearMessageInTime = 0;
	}

	//If no other message is being shown show the currently pressed keys after rebinding
	if (screenShowNormalInput) {
		if (!clearMessageInTime) {
			if (buttons) {
				char buffer[17];
				buttonToString(buffer, buttons, "\xA5");
				writeTopLine(buffer);
				writeBottomLine("");
			} else {
				clearMessage();
			}
		}
	}
}

//End Screen Interface
