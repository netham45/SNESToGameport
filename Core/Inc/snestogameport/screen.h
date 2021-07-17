#ifndef INC_SNESTOGAMEPORT_SCREEN_H_
#define INC_SNESTOGAMEPORT_SCREEN_H_
#include <main.h>
void screenClear();
void screenClearIn(uint8_t seconds);
void screenResetClearTime();
uint32_t screenGetClearMessageTime();
uint8_t screenGetShowNormalInput();
void screenSetShowNormalInput(uint8_t show);
void screenWriteTopLine(char *data);
void screenWriteBottomLine(char *data);
void screenProcess(uint16_t buttons);
#define SCREEN_WIDTH 16
#define SCREEN_CSTR_WIDTH (SCREEN_WIDTH + 1)
#endif /* INC_SNESTOGAMEPORT_SCREEN_H_ */
