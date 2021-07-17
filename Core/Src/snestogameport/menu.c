#include <snestogameport/menu.h>
#include <snestogameport/screen.h>
#include <snestogameport/buttons.h>

uint8_t menuNumEntries = 0;
uint8_t menuActive = 0;

void *menuActiveSubmenuCallback = 0; //Pointer to submenu callback, 0 if not in a submenu
//void (uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
//buttons == held buttons
//buttonHoldTime == length buttons have been held in ms
//buttonsChanged == flag if the buttons changed this tick
//firstRun == bool first loop after the menu was called

uint32_t menuLastButtonsPressedTime = 0;
uint16_t menuLastButtonsPressed = 0;

//Registers a menu entry's name, help message, and callback function
void menuInitMenuEntry(char *name, char *help, void *callback) {
	strcpy(menuItems[menuNumEntries].name, name);
	strcpy(menuItems[menuNumEntries].help, help);
	menuItems[menuNumEntries].callback = callback;
	menuNumEntries++;
}

//Hide menu, clearMessage to clear the screen immediately, otherwise leave the screen alone
void menuDeactivate(uint8_t _clearMessage) {
	menuActive = 0;
	if (_clearMessage)
		screenClear();
	menuActiveSubmenuCallback = 0;
}

//Show the menu
void menuActivate() {
	menuActive = 1;
	menuActiveSubmenuCallback = 0;
}

//Main Menu variables
uint8_t mainMenuTopSelected = 1;
uint8_t mainMenuSubmenuFirstRun = 0;
int mainMenuHelpIndex = 0;
uint32_t mainMenuHelpLastTick = 0;
uint8_t mainMenuCurrentMenuIndex = 0;

//Main Menu
int menuProcess(uint16_t buttons) {

	uint8_t buttonsChanged = 0;

	if (menuLastButtonsPressed != buttons) //Check if buttons have changed, if so reset the held timer and set the changed flag
			{
		menuLastButtonsPressedTime = HAL_GetTick();
		menuLastButtonsPressed = buttons;
		buttonsChanged = 1;
	}

	uint32_t buttonsHoldTime = HAL_GetTick() - menuLastButtonsPressedTime; //How long the button has been held

	if (buttons == (BUTTON_START | BUTTON_SELECT) && buttonsHoldTime > 1000) //If the menu isn't active and START+SELECT are held alone for 1s open it
			{
		menuActivate();
	}

	if (!menuActive) {
		return 0; //  0 for continue to press buttons on the PC
	}

	screenResetClearTimer(); //Clear any timers to turn the screen off

	if (menuActiveSubmenuCallback) //Call a submenu
	{
		void (*callback)(uint16_t buttons, uint32_t buttonsHoldTime,
				uint8_t buttonsChanged, uint8_t firstRun) = menuActiveSubmenuCallback;
		callback(buttons, buttonsHoldTime, buttonsChanged,
				mainMenuSubmenuFirstRun);
		mainMenuSubmenuFirstRun = 0;
	} else //Render the menu, check the keys
	{

		//Render menu
		if (mainMenuCurrentMenuIndex == 0)
			mainMenuTopSelected = 1; //Top entry means top is selected
		if (mainMenuCurrentMenuIndex == menuNumEntries - 1)
			mainMenuTopSelected = 0; //Bottom entry means bottom is selected

		uint8_t otherOptionPosition = mainMenuCurrentMenuIndex
				+ (mainMenuTopSelected ? 1 : -1);
		struct menuEntry *currentOption = &menuItems[mainMenuCurrentMenuIndex];
		struct menuEntry *otherOption = &menuItems[otherOptionPosition];

		char currentOptionBuffer[17]; //Current menu entry pointed to by menuPos
		char otherLineBuffer[17]; //Also holds help messages

		sprintf(currentOptionBuffer, ">%s", currentOption->name);

		if (buttonsHoldTime > 2000 && buttons == 0) //Show Help
				{
			memset(otherLineBuffer, 0x20, sizeof(otherLineBuffer));

			if (!mainMenuHelpLastTick)
				mainMenuHelpLastTick = HAL_GetTick();

			uint32_t helpLastScrolled = HAL_GetTick() - mainMenuHelpLastTick;
			if (helpLastScrolled > 350) //Tick help another char every 350ms, it's slow but any faster the LCD blurs
					{
				mainMenuHelpLastTick = HAL_GetTick();
				mainMenuHelpIndex += 1;
				if (mainMenuHelpIndex > strlen(currentOption->help))
					mainMenuHelpIndex = -16;
			}

			if (mainMenuHelpIndex < 0) //If it's negative it means it's scrolling in
					{
				memset(otherLineBuffer, 0x20, 0 - mainMenuHelpIndex);
				memcpy(otherLineBuffer + (0 - mainMenuHelpIndex),
						currentOption->help, 16 + mainMenuHelpIndex);
			} else
				memcpy(otherLineBuffer, currentOption->help + mainMenuHelpIndex,
						16);
			otherLineBuffer[16] = 0;
		} else //Show the other option
		{
			strcpy(otherLineBuffer, otherOption->name);
			mainMenuHelpIndex = 0;
		}

		if (mainMenuTopSelected) {
			screenWriteTopLine(currentOptionBuffer);
			screenWriteBottomLine(otherLineBuffer);
		} else {
			screenWriteTopLine(otherLineBuffer);
			screenWriteBottomLine(currentOptionBuffer);
		}

		//End Render Menu

		//Process Buttons
		if (buttonsChanged) {
			if (buttons & BUTTON_UP) //Scroll up menu
			{
				if (mainMenuCurrentMenuIndex > 0) {
					mainMenuCurrentMenuIndex--;
					mainMenuTopSelected = 1;
				}
			} else if (buttons & BUTTON_DOWN) //Scroll down menu
			{
				if (mainMenuCurrentMenuIndex < menuNumEntries - 1) {
					mainMenuCurrentMenuIndex++;
					mainMenuTopSelected = 0;
				}
			} else if (buttons & BUTTON_A) //Select an option
			{
				menuActiveSubmenuCallback = menuItems[mainMenuCurrentMenuIndex].callback;
				mainMenuSubmenuFirstRun = 1;
			} else if (buttons & BUTTON_B) //Close menu
			{
				menuDeactivate(1);
			}
		}
	}

	return 1; //1 for don't continue
}

//Begin submenu callbacks
//Submenu callbacks basically have a render phase where they draw to the screen and a process phase where they process input.
//These aren't defined in the header.
uint16_t menuRebindKeyFirstKey = 0;
uint16_t menuRebindKeyFirstKeyReleased = 0;

void menuRebindKeys(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {

	//Process Buttons
	if (firstRun) {
		menuRebindKeyFirstKey = 0;
		menuRebindKeyFirstKeyReleased = 0;
	}
	if (!menuRebindKeyFirstKeyReleased && buttonsChanged) //If the first bind was held for 1s then released reset the ignore flag
			{
		menuRebindKeyFirstKeyReleased = 1;
	}

	if (buttonHoldTime > 1000 && buttons) //Wait for a button combo to be held for 1s before registering it
			{
		if (!menuRebindKeyFirstKey) //If the first key hasn't been chosen yet mark it and set a flag to ignore keys until the input changes
		{
			menuRebindKeyFirstKey = buttons;
			menuRebindKeyFirstKeyReleased = 0;
		} else if (menuRebindKeyFirstKeyReleased) //The first key was pressed and released, take the second and save the binding
		{
			char bottomLineBuffer[17];
			buttonsToString(bottomLineBuffer, buttons, "\xA5");
			screenWriteBottomLine(bottomLineBuffer);
			bindKey(menuRebindKeyFirstKey, buttons, 0);
			screenWriteTopLine("Binding Saved");
			screenWriteBottomLine("");
			menuDeactivate(0);
			screenClearIn(2);
			return;
		}
	}

	//Render
	char topLineBuffer[17];
	char bottomLineBuffer[17];
	if (!menuRebindKeyFirstKey) //If the first key isn't known yet
	{
		buttonsToString(topLineBuffer, buttons, "\x7F");
		strcpy(bottomLineBuffer, "Hold Input Btns");
	} else {
		buttonsToString(topLineBuffer, menuRebindKeyFirstKey, "\xA5");
		if (buttons && menuRebindKeyFirstKeyReleased) {
			buttonsToString(bottomLineBuffer, buttons, "\x7E");
		} else {
			strcpy(bottomLineBuffer, "Hold Output Btns");
		}
	}
	screenWriteTopLine(topLineBuffer);
	screenWriteBottomLine(bottomLineBuffer);
}

//View binds/set rapid fire
uint8_t menuViewEditBindsIndex = 0;
uint8_t menuViewEditBindsCyclingRapidFire = 0;

void menuViewEditBinds(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	if (firstRun) {
		menuViewEditBindsIndex = 0;
		struct rebindEntry *bind = &currentProfile[menuViewEditBindsIndex];
		if (bind->buttonsPressed == 65535 || bind->buttonsPressed == 0) //There's no binds
				{
			screenWriteTopLine("No binds to");
			screenWriteBottomLine("display");
			menuDeactivate(0);
			screenClearIn(2);
			return;
		}
	}

	//Render
	struct rebindEntry *bind = &currentProfile[menuViewEditBindsIndex];
	char topLineBuffer[17];
	char bottomLineBuffer[17];
	if (!menuViewEditBindsCyclingRapidFire) //If not showing rapid fire
	{
		buttonsToString(topLineBuffer, bind->buttonsPressed, "\x7F");
		buttonsToString(bottomLineBuffer, bind->buttonsToPress, "\x7E");
	} else {
		strcpy(topLineBuffer, "Rapid Fire:");
		if (bind->rapidFire)
			sprintf(bottomLineBuffer, "%ims", bind->rapidFire * RAPID_FIRE_BASE_TIME);
		else
			strcpy(bottomLineBuffer, "Off");
	}
	screenWriteTopLine(topLineBuffer);
	screenWriteBottomLine(bottomLineBuffer);

	//Process Buttons
	if (menuViewEditBindsCyclingRapidFire && !(buttons & BUTTON_SELECT)
			&& (buttons || buttonHoldTime > 1000)) {
		menuViewEditBindsCyclingRapidFire = 0;
	}
	if (buttonsChanged) {
		if (buttons & BUTTON_UP) //Scroll up
		{
			if (menuViewEditBindsIndex > 0)
				menuViewEditBindsIndex--;
		} else if (buttons & BUTTON_DOWN) //Scroll down
		{
			if (menuViewEditBindsIndex < bindGetBindCount() - 1) {
				menuViewEditBindsIndex++;
			}
		} else if (buttons & BUTTON_SELECT) //Cycle rapid fire
		{
			menuViewEditBindsCyclingRapidFire = 1;
			bindCycleRapidFire(&currentProfile[menuViewEditBindsIndex]);
		} else if (buttons & BUTTON_B) //Close
		{
			menuDeactivate(1);
		}
	}
}

//Clears keybinds in the current profile
void menuClearKeybinds(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	//Render
	screenWriteTopLine("Hold Start=Clear");
	screenWriteBottomLine("Press B=Cancel");

	//Process Buttons
	if (buttons == BUTTON_START && buttonHoldTime > 3000) //Start held for 3s to clear
			{
		screenWriteTopLine("Clearing Binds");
		screenWriteBottomLine("");
		bindClearAll();
		screenWriteTopLine("Binds Cleared");
		menuDeactivate(0);
		screenClearIn(2);
	}
	if (buttons & BUTTON_B) //Cancel
	{
		menuDeactivate(1);
	}
}

//Loads a profile from flash
uint16_t menuSelectProfileSelectedProfileIndex = 0;
void menuSelectProfile(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	//Render
	char topLineBuffer[17];
	char bottomLineBuffer[17];
	sprintf(topLineBuffer, "New Profile: %i", menuSelectProfileSelectedProfileIndex + 1);
	sprintf(bottomLineBuffer, "Cur Profile: %i", profileGetSelectedIndex() + 1);
	screenWriteTopLine(topLineBuffer);
	screenWriteBottomLine(bottomLineBuffer);

	//Process Buttons
	if (buttonsChanged) {
		if (buttons & BUTTON_UP && menuSelectProfileSelectedProfileIndex > 0) //Scroll Up through profiles
				{
			menuSelectProfileSelectedProfileIndex--;
		} else if (buttons & BUTTON_DOWN
				&& menuSelectProfileSelectedProfileIndex < PROFILE_COUNT - 1) //Scroll down
						{
			menuSelectProfileSelectedProfileIndex++;
		} else if (buttons & BUTTON_A) //Select
		{
			profileSelect(menuSelectProfileSelectedProfileIndex);
			menuDeactivate(0);
			screenClearIn(2);
		} else if (buttons & BUTTON_B) //Cancel
		{
			menuDeactivate(1);
		}
	}

}

//Save a profile to a slot
uint16_t menuSaveProfileSelectedProfileIndex = 0;
void menuSaveProfile(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	//Render
	char topLineBuffer[17];
	char bottomLineBuffer[17];
	sprintf(topLineBuffer, "Save Profile: %i", menuSaveProfileSelectedProfileIndex + 1);
	sprintf(bottomLineBuffer, "Cur Profile: %i", profileGetSelectedIndex() + 1);
	screenWriteTopLine(topLineBuffer);
	screenWriteBottomLine(bottomLineBuffer);

	//Process Buttons
	if (buttonsChanged) {
		if (buttons & BUTTON_UP && menuSaveProfileSelectedProfileIndex > 0) //Scroll up
				{
			menuSaveProfileSelectedProfileIndex--;
		} else if (buttons & BUTTON_DOWN
				&& menuSaveProfileSelectedProfileIndex < PROFILE_COUNT - 1) //Scroll down
						{
			menuSaveProfileSelectedProfileIndex++;
		} else if (buttons & BUTTON_A) //Select
		{
			profileSave(menuSaveProfileSelectedProfileIndex);
			menuDeactivate(0);
			screenClearIn(2);
		} else if (buttons & BUTTON_B) //Cancel
		{
			menuDeactivate(1);
		}
	}

}

//Toggle screen displaying input during normal use
void menuToggleScreenShowInput(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	screenSetShowNormalInput(!screenGetShowNormalInput());
	menuDeactivate(1);
}

//About
void menuAbout(uint16_t buttons, uint32_t buttonHoldTime,
		uint8_t buttonsChanged, uint8_t firstRun) {
	screenWriteTopLine("SNES->GamePad");
	screenWriteBottomLine("By Netham45");
	if (buttons && buttonsChanged) {
		menuDeactivate(1);
	}
}
//End Submenu Callbacks

//Init functions

//Register menu entries/callbacks
void menuInit() {
	memset(menuItems, 0, sizeof(menuItems));
	menuInitMenuEntry("Rebind Keys", "Rebind key(s) to other key(s)",
			&menuRebindKeys);
	menuInitMenuEntry("View/Edit Binds", "Scroll through binds and edit rapid fire",
			&menuViewEditBinds);
	menuInitMenuEntry("Clear Binds", "Clear all binds", &menuClearKeybinds);
	menuInitMenuEntry("Select Profile", "Select which profile you want to use",
			&menuSelectProfile);
	menuInitMenuEntry("Save Profile", "Save profile to Flash", &menuSaveProfile);
	menuInitMenuEntry("Show Input", "Toggle showing input after binds",
			&menuToggleScreenShowInput);
	menuInitMenuEntry("About", "About this device", &menuAbout);
}
