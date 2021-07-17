#include <snestogameport/buttons.h>

uint32_t data[PROFILE_SIZE * PROFILE_COUNT];
uint8_t selectedProfile = 0;

void saveProfileNum(uint8_t newProfileNumber) {
	if (newProfileNumber != selectedProfile) //If we're saving to a new profile
			{
		struct rebindEntry *newProfile = (struct rebindEntry*) (data
				+ (PROFILE_SIZE * newProfileNumber)); //Pointer to new profile
		memcpy(newProfile, rebind, PROFILE_SIZE); //Save current profile to new slot
		memcpy(rebind, Flash_ReadData() + (PROFILE_SIZE * selectedProfile),
				PROFILE_SIZE); //Reload current profile from flash so it isn't overwritten
		rebind = newProfile; //Point rebind to the new profile
		selectedProfile = newProfileNumber; //Update selected profile number
	}
	Flash_WriteData(data, sizeof(data)); //Save profile
}

void selectProfile(uint8_t profile) {
	//Load profile from flash
	memcpy(data, Flash_ReadData(), sizeof(data)); //Copy data in from flash, this discards any changes
	selectedProfile = profile;
	rebind = (struct rebindEntry*) (data + (PROFILE_SIZE * selectedProfile));
}

uint8_t getSelectedProfile() {
	return selectedProfile;
}

uint8_t getBindCount() {
	int i = 0;
	for (i = 0; i < REBIND_COUNT; i++)
		if (rebind[i].buttonsPressed == 0 || rebind[i].buttonsPressed == 65535)
			break;
	return i;
}

void bindKey(uint16_t buttonsPressed, uint16_t buttonsToPress,
		uint8_t rapidFire) {
	uint8_t rebindPos = 0;
	for (rebindPos = 0; rebindPos < getBindCount(); rebindPos++)
		if (rebind[rebindPos].buttonsPressed == buttonsPressed)
			break;
	rebind[rebindPos].buttonsPressed = buttonsPressed;
	rebind[rebindPos].buttonsToPress = buttonsToPress;
	rebind[rebindPos].rapidFire = rapidFire;
}

void clearBinds() {
	for (int i = 0; i < REBIND_COUNT; i++) {
		rebind[i].buttonsPressed = 65535;
		rebind[i].buttonsToPress = 65535;
		rebind[i].rapidFire = 255;
	}
}

void cycleRapidFire(struct rebindEntry *entry) {
	entry->rapidFire++;
	entry->rapidFire %= 5;
}

//End Rebinds

//Set pins to default state (axis centered, no buttons pressed)
void gpioDefaultState() {
	HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Button1_GPIO_Port, Button1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Button2_GPIO_Port, Button2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Button3_GPIO_Port, Button3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Button4_GPIO_Port, Button4_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_SET);
}

//Main Loop Processing Functions
void processRebinds(uint16_t *buttons) {
	//Process button rebinds
	uint16_t realButtons = *buttons;
	uint16_t bindButtonsToPress = 0;

	for (int i = 0; i < REBIND_COUNT; i++) {
		if (rebind[i].buttonsPressed == 0 || rebind[i].buttonsPressed == 65535) //Bail at the first empty entry
				{
			break;
		}

		if ((realButtons & rebind[i].buttonsPressed)
				== rebind[i].buttonsPressed) {
			*buttons = *buttons ^ rebind[i].buttonsPressed; //Remove the pressed buttons from buttons
			if (rebind[i].rapidFire != 0) {
				uint32_t ticks = HAL_GetTick()
						/ (RAPID_FIRE_BASE_TIME * rebind[i].rapidFire);
				if ((ticks % 2) == 1) {
					bindButtonsToPress |= rebind[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
				}
			} else {
				bindButtonsToPress |= rebind[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
			}
		}
	}

	*buttons |= bindButtonsToPress; //Add buttonsToPress to buttons
}

void processButtons(uint16_t buttons) {
	//4 Buttons
	if (buttons & (1)) // B button
		HAL_GPIO_WritePin(Button1_GPIO_Port, Button1_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(Button1_GPIO_Port, Button1_Pin, GPIO_PIN_SET);
	if (buttons & (1 << 1)) // Y button
		HAL_GPIO_WritePin(Button2_GPIO_Port, Button2_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(Button2_GPIO_Port, Button2_Pin, GPIO_PIN_SET);
	if (buttons & (1 << 8)) // A button
		HAL_GPIO_WritePin(Button3_GPIO_Port, Button3_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(Button3_GPIO_Port, Button3_Pin, GPIO_PIN_SET);
	if (buttons & (1 << 9)) // X button
		HAL_GPIO_WritePin(Button4_GPIO_Port, Button4_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(Button4_GPIO_Port, Button4_Pin, GPIO_PIN_SET);

	//4 Axis
	////X2
	if (buttons & (1 << 2)) // Select button
			{

		HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_RESET);
	} else if (buttons & (1 << 3)) // Start Button
			{

		HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_SET);

	} else // X2 Centered
	{
		HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_SET);
	}

	////Y1
	if (buttons & (1 << 4)) // Up button
			{
		HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_RESET);
	} else if (buttons & (1 << 5)) // Down Button
			{
		HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_SET);
	} else // Y1 Centered
	{
		HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_SET);
	}

	////X1
	if (buttons & (1 << 6)) // Left button
			{
		HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_RESET);
	} else if (buttons & (1 << 7)) // Right Button
			{
		HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_SET);
	} else // X1 Centered
	{
		HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_SET);
	}

	////Y2
	if (buttons & (1 << 10)) // Left Shoulder button
			{
		HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_RESET);
	} else if (buttons & (1 << 11)) // Right Shoulder Button
			{
		HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_SET);
	} else // Y2 Centered
	{
		HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_SET);
	}
}
