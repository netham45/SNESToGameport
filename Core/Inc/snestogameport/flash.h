#ifndef INC_SNESTOGAMEPORT_FLASH_H_
#define INC_SNESTOGAMEPORT_FLASH_H_
#include <main.h>
uint32_t* flashReadData();
void flashWriteData(uint32_t *data, uint32_t size);

#endif /* INC_SNESTOGAMEPORT_FLASH_H_ */
