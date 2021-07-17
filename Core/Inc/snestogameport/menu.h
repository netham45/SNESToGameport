#ifndef INC_SNESTOGAMEPORT_MENU_H_
#define INC_SNESTOGAMEPORT_MENU_H_
#include <main.h>

#define MAX_MENU_ITEMS  32
#define MAX_MENU_NAME_LEN  16 //Menu indicator + 15 + terminator
#define MAX_MENU_HELP_LEN 64
#define MENU_ELEMENTS 4
struct menuEntry {
	char name[MAX_MENU_NAME_LEN];
	char help[MAX_MENU_HELP_LEN];
	void *callback;
} menuItems[MAX_MENU_ITEMS];

void menuInitMenuEntry(char *name, char *help, void *callback);
void menuDeactivate(uint8_t _clearMessage);
void menuActivate();
int menuProcess(uint16_t buttons);
void menuInit();

#endif /* INC_SNESTOGAMEPORT_MENU_H_ */
