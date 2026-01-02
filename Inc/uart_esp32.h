#ifndef _UART_ESP32_H_
#define _UART_ESP32_H_

#include <stdint.h>

/******************************************************************************
 * Module: UART + ESP32
 * Description:
 *   - UART1 register-level driver (STM32F103C8T6)
 *   - ESP32 communication
 * Responsibility: Doan Trong My - 23CE065
 ******************************************************************************/

/* ================= PUBLIC VARIABLES ================= */
extern volatile uint8_t dataReady;
extern char rxBuffer[50];

/* ================= PUBLIC API ================= */
void UART1_Init(void);
void GPIO_LED_Init(void);

#endif
