#ifndef INC_SNESTOGAMEPORT_BUTTONS_H_
#define INC_SNESTOGAMEPORT_BUTTONS_H_
#include <main.h>
#include <snestogameport/flash.h>

#define BUTTON_A (1<<8)
#define BUTTON_B (1<<0)
#define BUTTON_X (1<<9)
#define BUTTON_Y (1<<1)
#define BUTTON_UP (1<<4)
#define BUTTON_DOWN (1<<5)
#define BUTTON_LEFT (1<<6)
#define BUTTON_RIGHT (1<<7)
#define BUTTON_L (1<<10)
#define BUTTON_R (1<<11)
#define BUTTON_START (1<<3)
#define BUTTON_SELECT (1<<2)

#define REBIND_COUNT 100
#define PROFILE_COUNT 15
#define RAPID_FIRE_BASE_TIME 150 //ms
#define PROFILE_SIZE ( sizeof(struct rebindEntry) * REBIND_COUNT )

struct rebindEntry {
	uint16_t buttonsPressed;
	uint16_t buttonsToPress;
	uint8_t rapidFire;
} *rebind;

void saveProfileNum(uint8_t newProfileNumber);
void selectProfile(uint8_t profile);
uint8_t getSelectedProfile();
uint8_t getBindCount();
void bindKey(uint16_t buttonsPressed, uint16_t buttonsToPress,
		uint8_t rapidFire);
void clearBinds();
void cycleRapidFire(struct rebindEntry *entry);
void gpioDefaultState();
void processRebinds(uint16_t *buttons);
void processButtons(uint16_t buttons);

#endif /* INC_SNESTOGAMEPORT_BUTTONS_H_ */
