#ifndef INC_SNESTOGAMEPORT_FLASH_H_
#define INC_SNESTOGAMEPORT_FLASH_H_
#include <main.h>
uint32_t* Flash_ReadData();
void Flash_WriteData(uint32_t *data, uint32_t size);

#endif /* INC_SNESTOGAMEPORT_FLASH_H_ */
