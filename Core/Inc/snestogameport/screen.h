#ifndef INC_SNESTOGAMEPORT_SCREEN_H_
#define INC_SNESTOGAMEPORT_SCREEN_H_
#include <main.h>
void screenClear();
void screenClearIn(uint8_t seconds);
void screenResetClearTimer();
uint32_t screenGetClearMessageTime();
uint8_t screenGetShowNormalInput();
void screenSetShowNormalInput(uint8_t show);
void screenWriteTopLine(char *data);
void screenWriteBottomLine(char *data);
void screenProcess(uint16_t buttons);

#endif /* INC_SNESTOGAMEPORT_SCREEN_H_ */
