#ifndef INC_SNESTOGAMEPORT_MENU_H_
#define INC_SNESTOGAMEPORT_MENU_H_
#include <main.h>
#include <snestogameport/screen.h>
#include <snestogameport/buttons.h>

#define MAX_MENU_ITEMS  32
#define MAX_MENU_NAME_LEN  16 //Menu indicator + 15 + terminator
#define MAX_MENU_HELP_LEN 64
#define MENU_ELEMENTS 4
struct menuEntry {
	char name[MAX_MENU_NAME_LEN];
	char help[MAX_MENU_HELP_LEN];
	void *callback;
} menuItems[MAX_MENU_ITEMS];

void initMenuEntry(char *name, char *help, void *callback);
void deactivateMenu(uint8_t _clearMessage);
void activateMenu();
int processMenu(uint16_t buttons);
void initMenu();

#endif /* INC_SNESTOGAMEPORT_MENU_H_ */
