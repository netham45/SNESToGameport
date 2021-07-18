#include <snestogameport/flash.h>
#include <snestogameport/eeprom.h>

//Flash Read/Write

void flashReadData(uint16_t* outputBuffer, uint32_t size,uint32_t offset) {
	offset /= 2;
	HAL_FLASH_Unlock();
	for (int i = 0; i < (size / 2); i++) {
		uint32_t status = EE_ReadVariable(i+offset,outputBuffer+i);
		if (status == 1)
			*(outputBuffer+i) = 0xFFFF;
	}
	HAL_FLASH_Lock();
}

void flashWriteData(uint16_t *data, uint32_t size, uint32_t offset) {
	offset /= 2;
	HAL_FLASH_Unlock();
	for (int i = 0; i < (size / 2); i++) {
		uint32_t status = EE_WriteVariable(i+offset,*(data+i));
		status++;
	}
	HAL_FLASH_Lock();
}

// End Flash Read/Write
