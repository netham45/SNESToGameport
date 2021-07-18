#include <snestogameport/flash.h>
#include <snestogameport/eeprom.h>

//Flash Read/Write

/*
 * uint16_t EE_ReadVariable(uint16_t VirtAddress, uint16_t* Data)
uint16_t EE_WriteVariable(uint16_t VirtAddress, uint16_t Data)
 */

void flashReadData(uint16_t* outputBuffer, uint32_t size,uint32_t offset) {
	for (int i = 0; i < (size / 2); i++) {
		uint32_t status = EE_ReadVariable(i+offset,outputBuffer+i);
		status++;
	}
	//uint32_t address = 0x08020000;
	//return (uint32_t*) address;
}

void flashWriteData(uint16_t *data, uint32_t size, uint32_t offset) {
	/*
	//TODO: Find working wear leveling library
	uint32_t address = 0x08020000; // Sector 5 start address
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_5, VOLTAGE_RANGE_1);
	for (int i = 0; i < (size / 2); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 2),
				*(data + i));
	}
	HAL_FLASH_Lock();
	*/
	for (int i = 0; i < (size / 2); i++) {
		uint32_t status = EE_WriteVariable(i+offset,*(data+i));
		status++;
	}
}

// End Flash Read/Write
