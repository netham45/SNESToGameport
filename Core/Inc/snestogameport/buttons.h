#ifndef INC_SNESTOGAMEPORT_BUTTONS_H_
#define INC_SNESTOGAMEPORT_BUTTONS_H_
#include <main.h>

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

#define REBIND_COUNT 128
#define PROFILE_COUNT 16 //REBIND_COUNT * PROFILE_COUNT needs to be divisible by 2
#define RAPID_FIRE_BASE_TIME 150 //ms
#define PROFILE_SIZE ( sizeof(struct rebindEntry) * REBIND_COUNT )
#define DATA_SIZE (PROFILE_SIZE * PROFILE_COUNT)
#define DATA_INIT_SIZE (DATA_SIZE / 2)

struct rebindEntry {
	uint16_t buttonsPressed;
	uint16_t buttonsToPress;
	uint8_t rapidFire;
} *currentProfile;

void profileSave(uint8_t newProfileNumber);
void profileSelect(uint8_t profile);
uint8_t profileGetSelectedIndex();
uint8_t bindGetBindCount();
void bindKey(uint16_t buttonsPressed, uint16_t buttonsToPress,
		uint8_t rapidFire);
void bindClearAll();
void bindCycleRapidFire(struct rebindEntry *entry);
void buttonsGPIODefaultState();
void bindProcess(uint16_t *buttons);
void buttonsProcess(uint16_t buttons);
void buttonsToString(char *stringBuffer, uint16_t buttons, char *prefix);

#endif /* INC_SNESTOGAMEPORT_BUTTONS_H_ */
