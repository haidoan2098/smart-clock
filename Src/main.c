#include <stdint.h>
#include <stdio.h>

#include "uart_esp32.h"
#include "i2c_ds3231.h"
#include "spi_oled.h"

RTC_Time rtc_time;

/* Parse chuỗi "HH:MM:SS" */
void parse_time_string(char *str, uint8_t *h, uint8_t *m, uint8_t *s)
{
    *h = (str[0] - '0') * 10 + (str[1] - '0');
    *m = (str[3] - '0') * 10 + (str[4] - '0');
    *s = (str[6] - '0') * 10 + (str[7] - '0');
}

/* Hiển thị thời gian lên OLED */
void OLED_ShowTime(RTC_Time *t)
{
    char buf[16];

    /* HH:MM:SS */
    sprintf(buf, "%02d:%02d:%02d", t->hours, t->minutes, t->seconds);

    OLED_Clear();
    OLED_ShowString(32, 5, buf);
}

int main(void)
{
    uint8_t hour, minute, second;

    /* ===== Init all modules ===== */
    UART1_Init();

    I2C1_Init();
    TIM2_DelayInit();

    OLED_RCC_Init();
    OLED_GPIO_Init();
    OLED_SPI_Init();
    OLED_Init();
    OLED_Clear();

    DS3231_SetTime(5, 1, 10);

    while (1)
    {
        /* ===== Có dữ liệu từ ESP32 ===== */
        if (dataReady)
        {
            dataReady = 0;

            parse_time_string(rxBuffer, &hour, &minute, &second);

            DS3231_SetTime(hour, minute, second);

            DS3231_GetTime(&rtc_time);

            OLED_ShowTime(&rtc_time);
        }

        /* Có thể refresh lại thời gian mỗi 1s */
        DS3231_GetTime(&rtc_time);
        OLED_ShowTime(&rtc_time);
        delay_ms(1000);
    }
}
