# SMART CLOCK

## Description
This project implements a smart real-time clock system using **STM32F103C8T6**, **ESP32**, **DS3231**, and an **OLED** display.

The **ESP32** connects to the Internet to synchronize time from an **NTP server** and transmits the formatted time string `"HH:MM:SS"` to the **STM32** via **UART**. The STM32 receives and processes the data, stores the time in the **DS3231 RTC** module through **I2C**, and continuously reads the current time for display on the **OLED** screen using **SPI**.

All peripherals on the STM32 are implemented using **register-level programming**, providing a deeper understanding of low-level hardware control and communication protocols.

## Authors
- Doan Phu Hai ([@haidoan2098](https://github.com/haidoan2098))
- Doan Trong My ([@mnbpasd1](https://github.com/mnbpasd1))
- Nguyen Minh Anh Tu ([@tus0810](https://github.com/tus0810))

