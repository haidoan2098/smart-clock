#ifndef _I2C_DS3231_H_
#define _I2C_DS3231_H_

#include <stdint.h>

/******************************************************************************
 * Module: I2C + DS3231
 * Description:
 *   - I2C1 register-level driver (STM32F103C8T6)
 *   - DS3231 RTC communication
 * Responsibility: Doan Phu Hai - 23CE021
 ******************************************************************************/

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} RTC_Time;

/* ================= PUBLIC API ================= */

void I2C1_Init(void);
void TIM2_DelayInit(void);
void delay_ms(uint32_t ms);

void DS3231_SetTime(uint8_t h, uint8_t m, uint8_t s);
void DS3231_GetTime(RTC_Time *t);

#endif

