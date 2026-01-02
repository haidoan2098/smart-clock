#ifndef _SPI_OLED_H_
#define _SPI_OLED_H_

#include <stdint.h>

/******************************************************************************
 * Module: SPI + OLED
 * Description:
 *   - SPI register-level driver (STM32F103C8T6)
 *   - OLED communication
 * Responsibility: Nguyen Minh Anh Tu
 ******************************************************************************/

/* ================= PUBLIC API ================= */

void OLED_RCC_Init(void);
void OLED_GPIO_Init(void);
void OLED_SPI_Init(void);
void OLED_Init(void);

void OLED_Clear(void);
void OLED_SetPos(uint8_t x, uint8_t y);
void OLED_ShowChar(uint8_t x, uint8_t y, char chr);
void OLED_ShowString(uint8_t x, uint8_t y, char *str);

#endif
