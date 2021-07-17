#include <snestogameport/buttons.h>
#include <snestogameport/flash.h>

uint32_t data[DATA_INIT_SIZE];
uint8_t currentProfileIndex = 0;

struct rebindEntry* getDataProfileOffset(uint8_t profileIndex) //RW
{
	return (struct rebindEntry*)data + (PROFILE_SIZE * profileIndex);
}

struct rebindEntry* getFlashProfileOffset(uint8_t profileIndex) //RO
{
	 return (struct rebindEntry*)flashReadData() + (PROFILE_SIZE * profileIndex);
}


void profileSave(uint8_t newProfileIndex) {
	if (newProfileIndex != currentProfileIndex) //If we're saving to a new profile
			{
		struct rebindEntry *newProfile = getDataProfileOffset(newProfileIndex); //Pointer to new profile
		memcpy(newProfile, currentProfile, PROFILE_SIZE); //Save current profile to new slot
		memcpy(currentProfile,getFlashProfileOffset(currentProfileIndex),
				PROFILE_SIZE); //Reload current profile from flash so it isn't overwritten
		currentProfile = newProfile; //Point rebind to the new profile
		currentProfileIndex = newProfileIndex; //Update selected profile number
	}

	flashWriteData(data, sizeof(data)); //Save profile
}

void profileSelect(uint8_t profile) {
	//Load profile from flash
	memcpy(getDataProfileOffset(profile), getFlashProfileOffset(currentProfileIndex), PROFILE_SIZE); //Copy data for this profile in from flash, this discards any changes
	currentProfileIndex = profile;
	currentProfile = getDataProfileOffset(currentProfileIndex);
}

uint8_t profileGetSelectedIndex() {
	return currentProfileIndex;
}

uint8_t bindGetBindCount() {
	int i = 0;
	for (i = 0; i < REBIND_COUNT; i++)
		if (currentProfile[i].buttonsPressed == 0 || currentProfile[i].buttonsPressed == 65535)
			break;
	return i;
}

void bindKey(uint16_t buttonsPressed, uint16_t buttonsToPress,
		uint8_t rapidFire) {
	uint8_t rebindPos = 0;
	for (rebindPos = 0; rebindPos < bindGetBindCount(); rebindPos++)
		if (currentProfile[rebindPos].buttonsPressed == buttonsPressed)
			break;
	currentProfile[rebindPos].buttonsPressed = buttonsPressed;
	currentProfile[rebindPos].buttonsToPress = buttonsToPress;
	currentProfile[rebindPos].rapidFire = rapidFire;
}

void bindClearAll() {
	for (int i = 0; i < REBIND_COUNT; i++) {
		currentProfile[i].buttonsPressed = 65535;
		currentProfile[i].buttonsToPress = 65535;
		currentProfile[i].rapidFire = 255;
	}
}

void bindCycleRapidFire(struct rebindEntry *entry) {
	entry->rapidFire++;
	entry->rapidFire %= 5;
}

//End Rebinds

//Set pins to default state (axis centered, no buttons pressed)
void bindGPIODefaultState() {
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
void bindProcess(uint16_t *buttons) {
	//Process button rebinds
	uint16_t realButtons = *buttons;
	uint16_t bindButtonsToPress = 0;

	for (int i = 0; i < REBIND_COUNT; i++) {
		if (currentProfile[i].buttonsPressed == 0 || currentProfile[i].buttonsPressed == 65535) //Bail at the first empty entry
				{
			break;
		}

		if ((realButtons & currentProfile[i].buttonsPressed)
				== currentProfile[i].buttonsPressed) {
			*buttons = *buttons ^ currentProfile[i].buttonsPressed; //Remove the pressed buttons from buttons
			if (currentProfile[i].rapidFire != 0) {
				uint32_t ticks = HAL_GetTick()
						/ (RAPID_FIRE_BASE_TIME * currentProfile[i].rapidFire);
				if ((ticks % 2) == 1) {
					bindButtonsToPress |= currentProfile[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
				}
			} else {
				bindButtonsToPress |= currentProfile[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
			}
		}
	}

	*buttons |= bindButtonsToPress; //Add buttonsToPress to buttons
}

void buttonsProcess(uint16_t buttons) {
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

void buttonsToString(char *stringBuffer, uint16_t buttons, char *prefix) {
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
