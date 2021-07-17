#include <snestogameport/flash.h>

//Flash Read/Write

uint32_t* flashReadData() {
	uint32_t address = 0x08020000;
	return (uint32_t*) address;
}

void flashWriteData(uint32_t *data, uint32_t size) {
	//TODO: Find working wear leveling library
	uint32_t address = 0x08020000; // Sector 5 start address
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_5, VOLTAGE_RANGE_1);
	for (int i = 0; i < (size / 4); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 4),
				*(data + i));
	}
	HAL_FLASH_Lock();
}

// End Flash Read/Write
