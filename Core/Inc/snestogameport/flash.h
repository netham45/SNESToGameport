#ifndef INC_SNESTOGAMEPORT_FLASH_H_
#define INC_SNESTOGAMEPORT_FLASH_H_
#include <main.h>
void flashReadData(uint16_t* outputBuffer, uint32_t size, uint32_t offset);
void flashWriteData(uint16_t *data, uint32_t size, uint32_t offset);

#endif /* INC_SNESTOGAMEPORT_FLASH_H_ */
