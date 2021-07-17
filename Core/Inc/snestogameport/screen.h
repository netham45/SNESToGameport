#ifndef INC_SNESTOGAMEPORT_SCREEN_H_
#define INC_SNESTOGAMEPORT_SCREEN_H_
#include <main.h>
#include <snestogameport/lcd_hd44780_i2c.h>
void clearMessage();
void clearMessageIn(uint8_t seconds);
void clearClearMessage();
uint32_t clearMessageTime();
uint8_t screenGetShowNormalInput();
void screenSetShowNormalInput(uint8_t show);
void writeTopLine(char *data);
void writeBottomLine(char *data);
void buttonToString(char *stringBuffer, uint16_t buttons, char *prefix);
void processScreen(uint16_t buttons);

#endif /* INC_SNESTOGAMEPORT_SCREEN_H_ */
