/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "lcd_hd44780_i2c.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

TIM_HandleTypeDef htim1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */

/* Screen interface */
uint32_t clearMessageInTime = 0;

char topLine[17];
char bottomLine[17];

void clearMessage()
{
	lcdSetCursorPosition(0, 0);
	lcdPrintStr((uint8_t*)"                ", 16);
	lcdSetCursorPosition(0, 1);
	lcdPrintStr((uint8_t*)"                ", 16);
	lcdBacklight(LCD_BIT_BACKIGHT_OFF);
	topLine[0] = 0;
	bottomLine[0] = 0;
	//Update screens
}

void clearMessageIn(uint8_t seconds)
{
	clearMessageInTime = HAL_GetTick() + (seconds * 1000);
}

void clearClearMessage()
{
	clearMessageInTime = 0;
}

void writeTopLine(char* data)
{
	if (strcmp(topLine,data) != 0)
	{
		strcpy(topLine,data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		//Update screen
		lcdSetCursorPosition(0, 0);
		lcdPrintStr((uint8_t*)data, strlen(data));
		lcdPrintStr((uint8_t*)"                ", 16-strlen(data));
	}

}

void writeBottomLine(char* data)
{
	if (strcmp(bottomLine,data) != 0)
	{
		strcpy(bottomLine,data);
		lcdBacklight(LCD_BIT_BACKIGHT_ON);
		//Update screen
		lcdSetCursorPosition(0, 1);
		lcdPrintStr((uint8_t*)data, strlen(data));
		lcdPrintStr((uint8_t*)"                ", 16-strlen(data));
	}
}

void buttonToString(char* stringBuffer, uint16_t buttons, char* prefix)
{
	char buffer[32] = {0};
	strcpy(buffer,prefix);
	uint8_t offset = strlen(buffer);
	uint8_t written = 0;
	for (int i=0;i<12;i++)
	{
		if (buttons & (1<<i))
		{
			written = 1;
			switch(i)
			{
				case 8:
					sprintf(buffer+offset,"A+");
					break;
				case 0:
					sprintf(buffer+offset,"B+");
					break;
				case 9:
					sprintf(buffer+offset,"X+");
					break;
				case 1:
					sprintf(buffer+offset,"Y+");
					break;
				case 4:
					sprintf(buffer+offset,"Up+");
					break;
				case 5:
					sprintf(buffer+offset,"Down+");
					break;
				case 6:
					sprintf(buffer+offset,"Left+");
					break;
				case 7:
					sprintf(buffer+offset,"Right+");
					break;
				case 10:
					sprintf(buffer+offset,"L+");
					break;
				case 11:
					sprintf(buffer+offset,"R+");
					break;
				case 3:
					sprintf(buffer+offset,"Start+");
					break;
				case 2:
					sprintf(buffer+offset,"Select+");
					break;
			}
			offset += strlen(buffer+offset);
		}
	}
	if (written)
		buffer[strlen(buffer)-1] = 0; //Remove the final plus
	buffer[16] = 0; //Cap length at 16 chars
	memcpy(stringBuffer,buffer,17); //Copy 16 chars + terminator
}


//End Screen Interface

//Microsecond Sleep

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}

//End Microsecond Sleep

//Flash Read/Write

uint32_t* Flash_ReadData(){
	uint32_t address = 0x08020000;
	return (uint32_t*)address;
}

void Flash_WriteData(uint32_t* data,uint32_t size){
	//TODO: Find working wear leveling library
	uint32_t address = 0x08020000; // Sector 5 start address
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_5,VOLTAGE_RANGE_1);
	for (int i=0;i<(size/4);i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,address+(i*4),*(data+i));
	}
	HAL_FLASH_Lock();
}

// End Flash Read/Write


//Start Rebinds
struct rebindEntry
{
	uint16_t buttonsPressed;
	uint16_t buttonsToPress;
	uint8_t rapidFire;
}  *rebind;


#define REBIND_COUNT 100
#define PROFILE_COUNT 15
#define RAPID_FIRE_BASE_TIME 150 //ms
#define PROFILE_SIZE ( sizeof(struct rebindEntry) * REBIND_COUNT )


uint32_t data[PROFILE_SIZE * PROFILE_COUNT];
uint8_t dataInitialized = 0;
uint8_t selectedProfile = 0;

void saveProfileNum(uint8_t newProfileNumber)
{
	struct rebindEntry* newProfile = (struct rebindEntry*)(data + (PROFILE_SIZE * newProfileNumber));
	memcpy(rebind,newProfile,PROFILE_SIZE);
	memcpy(data + (PROFILE_SIZE * selectedProfile),rebind,PROFILE_SIZE);
	rebind = newProfile;
	selectedProfile = newProfileNumber;
	Flash_WriteData(data,sizeof(data));
}

void selectProfile(uint8_t profile)
{
	if (!dataInitialized || 1)
	{
		memcpy(data,Flash_ReadData(),sizeof(data));
		dataInitialized = 1;
	}
	selectedProfile = profile;
	rebind = (struct rebindEntry*)(data + (PROFILE_SIZE * selectedProfile));
}

uint8_t getBindCount()
{
	for (int i=0;i<REBIND_COUNT;i++)
		if (! (rebind[i].buttonsPressed != 0 && rebind[i].buttonsPressed != 65535))
			return i;
	return 0;
}

void bindKey(uint16_t buttonsPressed, uint16_t buttonsToPress, uint8_t rapidFire)
{
    uint8_t rebindPos = 0;
    for (rebindPos = 0;rebindPos<getBindCount();rebindPos++)
    	if (rebind[rebindPos].buttonsPressed == buttonsPressed)
    		break;
    rebind[rebindPos].buttonsPressed = buttonsPressed;
    rebind[rebindPos].buttonsToPress = buttonsToPress;
    rebind[rebindPos].rapidFire = rapidFire;
}

void clearBinds()
{
	for (int i=0;i<REBIND_COUNT;i++)
	{
		rebind[i].buttonsPressed = 65535;
		rebind[i].buttonsToPress = 65535;
		rebind[i].rapidFire = 255;
	}
}

void cycleRapidFire(struct rebindEntry *entry)
{
	entry->rapidFire++;
	entry->rapidFire%=5;
}

//End Rebinds



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//Query the SNES controller
int pollSNES()
{
	uint16_t retval = 0;
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_SET);
	delay_us(12); // 12µs delay
	HAL_GPIO_WritePin(SNES_Latch_GPIO_Port, SNES_Latch_Pin, GPIO_PIN_RESET);
	delay_us(6); // 6µs delay
	for (int i=0;i<16;i++)
	{
	  retval |= (HAL_GPIO_ReadPin(SNES_Data_GPIO_Port, SNES_Data_Pin) << i);
	  HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_RESET);
	  delay_us(12);
	  HAL_GPIO_WritePin(SNES_Clock_GPIO_Port, SNES_Clock_Pin, GPIO_PIN_SET);
	  delay_us(12);
	}
	retval = ~retval;

 	return retval;
}

//End Query SNES controller


//Begin Menu
//Button Defines
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

#define MAX_MENU_ITEMS  32
#define MAX_MENU_NAME_LEN  16 //Menu indicator + 15 + terminator
#define MAX_MENU_HELP_LEN 64
#define MENU_ELEMENTS 4

uint8_t menuEntries = 0;

uint8_t menuPos = 0;
uint8_t menuActive = 0;

void* submenu = 0; //Pointer to submenu callback, 0 if not in a submenu
//void (uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
//buttons == held buttons
//buttonHoldTime == length buttons have been held in ms
//buttonsChanged == flag if the buttons changed this tick
//firstRun == bool first loop after the menu was called

uint32_t menuLastButtonsPressedTime = 0;
uint16_t menuLastButtonsPressed = 0;


struct menuEntry
{
	char name[MAX_MENU_NAME_LEN];
	char help[MAX_MENU_HELP_LEN];
	void* callback;
} menuItems[MAX_MENU_ITEMS];

//Registers a menu entry's name, help message, and callback function
void initMenuEntry(char* name, char* help, void* callback)
{
	strcpy(menuItems[menuEntries].name,name);
	strcpy(menuItems[menuEntries].help,help);
	menuItems[menuEntries].callback = callback;
	menuEntries++;
}

//Hide menu, clearMessage to clear the screen immediately, otherwise leave the screen alone
void deactivateMenu(uint8_t _clearMessage)
{
	menuActive = 0;
	if (_clearMessage)
		clearMessage();
	submenu = 0;
}

//Show the menu
void activateMenu()
{
	menuActive = 1;
}

//Main Menu variables
uint8_t mainMenuTopSelected = 1;
uint8_t mainMenuSubmenuFirstRun = 0;
int mainMenuHelpPos = 0;
uint32_t mainMenuHelpLastTick = 0;

//Main Menu
int processMenu(uint16_t buttons)
{

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
		activateMenu();
	}

	if (!menuActive)
	{
		return 0; //  0 for continue to press buttons on the PC
	}

	clearClearMessage(); //Clear any timers to turn the screen off

	if (submenu) //Call a submenu
	{
		void (*callback)(uint16_t buttons, uint32_t buttonsHoldTime, uint8_t buttonsChanged,uint8_t firstRun) = submenu;
		callback(buttons,buttonsHoldTime,buttonsChanged,mainMenuSubmenuFirstRun);
		mainMenuSubmenuFirstRun = 0;
	}
	else //Render the menu, check the keys
	{

		//Render menu
		if (menuPos == 0)
			mainMenuTopSelected = 1; //Top entry means top is selected
		if (menuPos == menuEntries - 1)
			mainMenuTopSelected = 0; //Bottom entry means bottom is selected


		uint8_t otherOptionPosition = menuPos+(mainMenuTopSelected?1:-1);
		struct menuEntry *currentOption = &menuItems[menuPos];
		struct menuEntry *otherOption = &menuItems[otherOptionPosition];

		char currentOptionBuffer[17]; //Current menu entry pointed to by menuPos
		char otherOptionBuffer[17]; //Also holds help messages

		sprintf(currentOptionBuffer,">%s",currentOption->name);

		if (buttonsHoldTime > 2000 && buttons == 0) //Show Help
		{
			memset(otherOptionBuffer,0x20,sizeof(otherOptionBuffer));

			if (!mainMenuHelpLastTick)
				mainMenuHelpLastTick = HAL_GetTick();

			uint32_t helpLastScrolled = HAL_GetTick() - mainMenuHelpLastTick;
			if (helpLastScrolled > 350) //Tick help another char every 350ms, it's slow but any faster the LCD blurs
			{
				mainMenuHelpLastTick = HAL_GetTick();
				mainMenuHelpPos += 1;
				if (mainMenuHelpPos > strlen(currentOption->help))
					mainMenuHelpPos = -16;
			}

			if (mainMenuHelpPos < 0) //If it's negative it means it's scrolling in
			{
				memset(otherOptionBuffer,0x20,0-mainMenuHelpPos);
				memcpy(otherOptionBuffer+(0-mainMenuHelpPos),currentOption->help,16+mainMenuHelpPos);
			}
			else
				memcpy(otherOptionBuffer,currentOption->help + mainMenuHelpPos,16);
			otherOptionBuffer[16] = 0;
		}
		else //Show the other option
		{
			strcpy(otherOptionBuffer,otherOption->name);
			mainMenuHelpPos = 0;
		}

		if (mainMenuTopSelected)
		{
			writeTopLine(currentOptionBuffer);
			writeBottomLine(otherOptionBuffer);
		}
		else
		{
			writeTopLine(otherOptionBuffer);
			writeBottomLine(currentOptionBuffer);
		}

        //End Render Menu


		//Process Buttons
		if (buttonsChanged)
		{
			if (buttons & BUTTON_UP) //Scroll up menu
			{
				if (menuPos > 0)
				{
					menuPos--;
					mainMenuTopSelected = 1;
				}
			}
			else if (buttons & BUTTON_DOWN) //Scroll down menu
			{
				if (menuPos < menuEntries - 1)
				{
					menuPos++;
					mainMenuTopSelected = 0;
				}
			}
			else if (buttons & BUTTON_A) //Select an option
			{
				submenu = menuItems[menuPos].callback;
				mainMenuSubmenuFirstRun = 1;
			}
			else if (buttons & BUTTON_B) //Close menu
			{
				deactivateMenu(1);
			}
		}
	}

	return 1; //1 for don't continue
}

//Begin submenu callbacks
//Submenu callbacks basically have a render phase where they draw to the screen and a process phase where they process input.
uint16_t menuRebindKeyFirstKey = 0;
uint16_t menuRebindKeyFirstKeyReleased = 0;

void menuRebindKeys(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{

	//Process Buttons
	if (firstRun)
	{
		menuRebindKeyFirstKey = 0;
		menuRebindKeyFirstKeyReleased = 0;
	}
	if (!menuRebindKeyFirstKeyReleased && buttonsChanged) //If the first bind was held for 1s then released reset the ignore flag
	{
		menuRebindKeyFirstKeyReleased = 1;
	}

	if ( buttonHoldTime > 1000 && buttons) //Wait for a button combo to be held for 1s before registering it
	{
		if (!menuRebindKeyFirstKey) //If the first key hasn't been chosen yet mark it and set a flag to ignore keys until the input changes
		{
			menuRebindKeyFirstKey = buttons;
			menuRebindKeyFirstKeyReleased = 0;
		}
		else if (menuRebindKeyFirstKeyReleased) //The first key was pressed and released, take the second and save the binding
		{
			char message2[17];
			buttonToString(message2,buttons,"\xA5");
			writeBottomLine(message2);
			bindKey(menuRebindKeyFirstKey,buttons,0);
			writeTopLine("Binding Saved");
			writeBottomLine("");
			deactivateMenu(0);
			clearMessageIn(2);
			return;
		}
	}

	//Render
	char message[17];
	char message2[17];
	if (!menuRebindKeyFirstKey) //If the first key isn't known yet
	{
		buttonToString(message,buttons,"\x7F");
		strcpy(message2,"Hold Input Btns");
	}
	else
	{
		buttonToString(message,menuRebindKeyFirstKey,"\xA5");
		if (buttons && menuRebindKeyFirstKeyReleased)
		{
			buttonToString(message2,buttons,"\x7E");
		}
		else
		{
			strcpy(message2,"Hold Output Btns");
		}
	}
	writeTopLine(message);
	writeBottomLine(message2);
}

uint8_t menuViewEditBindsPos = 0;
uint8_t menuViewEditBindsCyclingRapidFire = 0;

//View binds/set rapid fire
void menuViewEditBinds(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{
	if (firstRun)
	{
		menuViewEditBindsPos = 0;
		struct rebindEntry *bind = &rebind[menuViewEditBindsPos];
		if (bind->buttonsPressed == 65535 || bind->buttonsPressed == 0) //There's no binds
		{
			writeTopLine("No binds to");
			writeBottomLine("display");
			deactivateMenu(0);
			clearMessageIn(2);
			return;
		}
	}

	//Render
	struct rebindEntry *bind = &rebind[menuViewEditBindsPos];
	char message[17];
	char message2[17];
	if (!menuViewEditBindsCyclingRapidFire) //If not showing rapid fire
	{
		buttonToString(message,bind->buttonsPressed,"\x7F");
		buttonToString(message2,bind->buttonsToPress,"\x7E");
	}
	else
	{
		strcpy(message, "Rapid Fire:");
		if (bind->rapidFire)
			sprintf(message2, "%ims", bind->rapidFire * RAPID_FIRE_BASE_TIME);
		else
			strcpy(message2,"Off");
	}
	writeTopLine(message);
	writeBottomLine(message2);

	//Process Buttons
	if (menuViewEditBindsCyclingRapidFire && ! ( buttons & BUTTON_SELECT ) && (buttons || buttonHoldTime > 1000))
	{
		menuViewEditBindsCyclingRapidFire = 0;
	}
	if (buttonsChanged)
	{
		if (buttons & BUTTON_UP) //Scroll up
		{
			if (menuViewEditBindsPos > 0)
				menuViewEditBindsPos--;
		}
		else if (buttons & BUTTON_DOWN) //Scroll down
		{
			if (menuViewEditBindsPos < getBindCount() - 1)
			{
				menuViewEditBindsPos++;
			}
		}
		else if (buttons & BUTTON_SELECT) //Cycle rapid fire
		{
			menuViewEditBindsCyclingRapidFire = 1;
			cycleRapidFire(&rebind[menuViewEditBindsPos]);
		}
		else if (buttons & BUTTON_B) //Close
		{
			deactivateMenu(1);
		}
}
}

//Clears keybinds in the current profile
void menuClearKeybinds(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{
    //Render
	writeTopLine("Hold Start=Clear");
	writeBottomLine("Press B=Cancel");

	//Process Buttons
	if (buttons == BUTTON_START && buttonHoldTime > 3000) //Start held for 3s to clear
	{
		writeTopLine("Clearing Binds");
		writeBottomLine("");
		clearBinds();
		writeTopLine("Binds Cleared");
		deactivateMenu(0);
		clearMessageIn(2);
	}
	if (buttons & BUTTON_B) //Cancel
	{
		deactivateMenu(1);
	}
}

uint16_t menuSelectProfileSelectedProfile = 0;
//Loads a profile from flash
void menuSelectProfile(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{
	//Render
	char message[17];
	char message2[17];
	sprintf(message,"New Profile: %i", menuSelectProfileSelectedProfile+1);
	sprintf(message2,"Cur Profile: %i", selectedProfile+1);
	writeTopLine(message);
	writeBottomLine(message2);

	//Process Buttons
	if (buttonsChanged)
	{
		if (buttons & BUTTON_UP && menuSelectProfileSelectedProfile > 0) //Scroll Up through profiles
		{
			menuSelectProfileSelectedProfile--;
		}
		else if (buttons & BUTTON_DOWN && menuSelectProfileSelectedProfile < PROFILE_COUNT - 1) //Scroll down
		{
			menuSelectProfileSelectedProfile++;
		}
		else if (buttons & BUTTON_A) //Select
		{
			selectProfile(menuSelectProfileSelectedProfile);
			deactivateMenu(0);
			clearMessageIn(2);
		}
		else if (buttons & BUTTON_B) //Cancel
		{
			deactivateMenu(1);
		}
	}

}

uint16_t menuSaveProfileSelectedProfile = 0;
//Save a profile to a slot
void menuSaveProfile(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{
	//Render
	char message[17];
	char message2[17];
	sprintf(message,"Save Profile: %i", menuSaveProfileSelectedProfile+1);
	sprintf(message2,"Cur Profile: %i", selectedProfile+1);
	writeTopLine(message);
	writeBottomLine(message2);

	//Process Buttons
	if (buttonsChanged)
	{
		if (buttons & BUTTON_UP && menuSaveProfileSelectedProfile > 0) //Scroll up
		{
			menuSaveProfileSelectedProfile--;
		}
		else if (buttons & BUTTON_DOWN && menuSaveProfileSelectedProfile < PROFILE_COUNT - 1) //Scroll down
		{
			menuSaveProfileSelectedProfile++;
		}
		else if (buttons & BUTTON_A) //Select
		{
			saveProfileNum(menuSaveProfileSelectedProfile);
			deactivateMenu(0);
			clearMessageIn(2);
		}
		else if (buttons & BUTTON_B) //Cancel
		{
			deactivateMenu(1);
		}
	}

}

//About
void menuAbout(uint16_t buttons, uint32_t buttonHoldTime, uint8_t buttonsChanged, uint8_t firstRun)
{
	writeTopLine("SNES->GamePad");
	writeBottomLine("By Netham45");
	if (buttons && buttonsChanged)
	{
		deactivateMenu(1);
	}
}
//End Submenu Callbacks

//Register menu entries/callbacks
void initMenu()
{
	memset(menuItems,0,sizeof(menuItems));
	initMenuEntry("Rebind Keys","Rebind key(s) to other key(s)",&menuRebindKeys);
	initMenuEntry("View/Edit Binds","Scroll through binds and edit rapid fire",&menuViewEditBinds);
	initMenuEntry("Clear Binds","Clear all binds",&menuClearKeybinds);
	initMenuEntry("Select Profile","Select which profile you want to use",&menuSelectProfile);
	initMenuEntry("Save Profile","Save profile to Flash",&menuSaveProfile);
	initMenuEntry("About","About this device",&menuAbout);
}
//End Menu

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff - 1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LEDpin_Pin|X2_Max_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SNES_Latch_Pin|SNES_Clock_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, X1_Center_Pin|X1_Max_Pin|Y1_Center_Pin|Button1_Pin
                          |Button2_Pin|Button3_Pin|Button4_Pin|X2_Center_Pin
                          |Y1_Max_Pin|Y2_Center_Pin|Y2_Max_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LEDpin_Pin X2_Max_Pin */
  GPIO_InitStruct.Pin = LEDpin_Pin|X2_Max_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SNES_Latch_Pin SNES_Clock_Pin */
  GPIO_InitStruct.Pin = SNES_Latch_Pin|SNES_Clock_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SNES_Data_Pin */
  GPIO_InitStruct.Pin = SNES_Data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SNES_Data_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : X1_Center_Pin X1_Max_Pin Y1_Center_Pin Button1_Pin
                           Button2_Pin Button3_Pin Button4_Pin X2_Center_Pin
                           Y1_Max_Pin Y2_Center_Pin Y2_Max_Pin */
  GPIO_InitStruct.Pin = X1_Center_Pin|X1_Max_Pin|Y1_Center_Pin|Button1_Pin
                          |Button2_Pin|Button3_Pin|Button4_Pin|X2_Center_Pin
                          |Y1_Max_Pin|Y2_Center_Pin|Y2_Max_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	//TODO: Init function
	lcdInit(&hi2c1, (uint8_t)0x27, (uint8_t)20, (uint8_t)4);

	HAL_TIM_Base_Start(&htim1);
	writeTopLine("SNES -> GamePort");
	writeBottomLine("By Netham45");
	  clearMessageIn(2);
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


	  selectProfile(0); //Load profile 0
	  initMenu(); //Register menu entries

	  while (1)
	  {

		  //TODO: Move everything here to separate process functions

		  //Check if screen needs cleared TODO: Move to screenProcess() function
		  if (clearMessageInTime && (clearMessageInTime < HAL_GetTick()))
		  {
			  clearMessage();
			  clearMessageInTime = 0;
		  }
		  uint16_t buttons = pollSNES(); //Query SNES controller
		  uint16_t realButtons = buttons;

		  //If processMenu returns non-negative then don't process buttons this loop.
		  if (processMenu(buttons))
			  continue;

		  //Process button rebinds
		  uint16_t bindButtonsToPress = 0;

		  for (int i=0;i<REBIND_COUNT;i++)
		  {
			  if (rebind[i].buttonsPressed == 0 || rebind[i].buttonsPressed == 65535) //Bail at the first empty entry
			  {
				  break;
			  }
			  //TODO: if (rebind[i].buttonsPressed == 8192) continue

			  if ((realButtons & rebind[i].buttonsPressed) == rebind[i].buttonsPressed)
			  {
				  buttons = buttons ^ rebind[i].buttonsPressed; //Remove the pressed buttons from buttons
				  if (rebind[i].rapidFire != 0)
				  {
					  uint32_t ticks = HAL_GetTick() / (RAPID_FIRE_BASE_TIME * rebind[i].rapidFire);
					  if ((ticks % 2) == 1)
					  {
						  bindButtonsToPress |= rebind[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
					  }
				  }
				  else
				  {
					  bindButtonsToPress |= rebind[i].buttonsToPress; //Store the buttons to press in another variable so they don't get cleared by other binds
				  }
			  }
		  }

		  buttons |= bindButtonsToPress; //Add buttonsToPress to buttons

		  //If no other message is being shown show the currently pressed keys after rebinding
		  //TODO: Add menu option to toggle
		  if (!clearMessageInTime)
		  {
			  if (buttons)
			  {
				  char buffer[17];
				  buttonToString(buffer,buttons,"\xA5");
				  writeTopLine(buffer);
				  writeBottomLine("");
			  }
			  else
			  {
				  clearMessage();
			  }
		  }



		  //Process Button Presses
		  //TODO: Move to a processButtons() function
		  if (buttons & (1)) // B button
			  HAL_GPIO_WritePin(Button1_GPIO_Port,Button1_Pin, GPIO_PIN_RESET);
		  else
			  HAL_GPIO_WritePin(Button1_GPIO_Port,Button1_Pin, GPIO_PIN_SET);
		  if (buttons & (1<<1)) // Y button
			  HAL_GPIO_WritePin(Button2_GPIO_Port,Button2_Pin, GPIO_PIN_RESET);
		  else
			  HAL_GPIO_WritePin(Button2_GPIO_Port,Button2_Pin, GPIO_PIN_SET);
		  if (buttons & (1<<8)) // A button
			  HAL_GPIO_WritePin(Button3_GPIO_Port,Button3_Pin, GPIO_PIN_RESET);
		  else
			  HAL_GPIO_WritePin(Button3_GPIO_Port,Button3_Pin, GPIO_PIN_SET);
		  if (buttons & (1<<9)) // X button
			  HAL_GPIO_WritePin(Button4_GPIO_Port,Button4_Pin, GPIO_PIN_RESET);
		  else
			  HAL_GPIO_WritePin(Button4_GPIO_Port,Button4_Pin, GPIO_PIN_SET);

		  //Axis
		  ////X2
		  if (buttons & (1<<2)) // Select button
		  {

				HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_RESET);
		  }
		  else if (buttons & (1<<3)) // Start Button
		  {

					HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_SET);
					HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_SET);

		  }
		  else // X2 Centered
		  {
				HAL_GPIO_WritePin(X2_Center_GPIO_Port, X2_Center_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(X2_Max_GPIO_Port, X2_Max_Pin, GPIO_PIN_SET);
		  }

		  ////Y1
		  if (buttons & (1<<4)) // Up button
		  {
				HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_RESET);
		  }
		  else if (buttons & (1<<5)) // Down Button
		  {
				HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_SET);
		  }
		  else // Y1 Centered
		  {
				HAL_GPIO_WritePin(Y1_Center_GPIO_Port, Y1_Center_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(Y1_Max_GPIO_Port, Y1_Max_Pin, GPIO_PIN_SET);
		  }

		  ////X1
		  if (buttons & (1<<6)) // Left button
		  {
				HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_RESET);
		  }
		  else if (buttons & (1<<7)) // Right Button
		  {
				HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_SET);
		  }
		  else // X1 Centered
		  {
				HAL_GPIO_WritePin(X1_Center_GPIO_Port, X1_Center_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(X1_Max_GPIO_Port, X1_Max_Pin, GPIO_PIN_SET);
		  }

		  ////Y2
		  if (buttons & (1<<10)) // Left Shoulder button
		  {
				HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_RESET);
		  }
		  else if (buttons & (1<<11)) // Right Shoulder Button
		  {
				HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_SET);
		  }
		  else // Y2 Centered
		  {
				HAL_GPIO_WritePin(Y2_Center_GPIO_Port, Y2_Center_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(Y2_Max_GPIO_Port, Y2_Max_Pin, GPIO_PIN_SET);
		  }
	  }
  /* USER CODE END 5 */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
