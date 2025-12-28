#include "stm32f1xx.h"

/* ================= biến toàn cục ================= */
char rxBuffer[50];
uint8_t rxIndex = 0;
volatile uint8_t dataReady = 0;

/* ================= hàm ================= */
void SystemClock_Config(void);
void UART1_Init(void);
void GPIO_LED_Init(void);
void USART1_IRQHandler(void);
void delay_ms(uint32_t ms);

/* ================= MAIN ================= */
int main(void)
{
    SystemClock_Config();
    GPIO_LED_Init();
    UART1_Init();

    // Tắt LED ban đầu
    GPIOC->BSRR = GPIO_BSRR_BS14;  // Tắt LED PC14

    while (1)
    {
        if (dataReady)
        {
            dataReady = 0;

            // Khi nhận chuỗi thành công, sáng LED PC14 trong 0.5s
            // BẬT LED PC14
            GPIOC->BSRR = GPIO_BSRR_BR14;

            // Delay 500ms
            delay_ms(500);

            // TẮT LED PC14
            GPIOC->BSRR = GPIO_BSRR_BS14;
        }
    }
}

/* ================= GPIO ================= */
void GPIO_LED_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // PC14 Output Push Pull
    GPIOC->CRH &= ~(GPIO_CRH_MODE14 | GPIO_CRH_CNF14);
    GPIOC->CRH |=  GPIO_CRH_MODE14_1;   // Output 2MHz
}

/* ================= UART ================= */
void UART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // PA9 cấu hình TX chế độ AF Push Pull
    GPIOA->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
    GPIOA->CRH |=  (GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1);

    // PA10 RX Input floating
    GPIOA->CRH &= ~(GPIO_CRH_MODE10 | GPIO_CRH_CNF10);
    GPIOA->CRH |=  GPIO_CRH_CNF10_0;

    // Baudrate 115200 (PCLK2 = 72MHz)
    USART1->BRR = 72000000 / 115200;

    // BẬT RXNE INTERRUPT
    USART1->CR1 |= USART_CR1_RXNEIE;

    USART1->CR1 |= USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;

    // BẬT NVIC cho USART1
    NVIC_EnableIRQ(USART1_IRQn);
}

/* =================  ================= */
void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_RXNE)
    {
        char c = (char)USART1->DR;

        if (c == '\n' || c == '\r')
        {
            rxBuffer[rxIndex] = '\0';
            rxIndex = 0;
            dataReady = 1;
        }
        else
        {
            if (rxIndex < sizeof(rxBuffer) - 1)
            {
                rxBuffer[rxIndex++] = c;
            }
            else
            {
                rxIndex = 0;
            }
        }
    }
}

/* ================= DELAY ================= */
void delay_ms(uint32_t ms)
{
    volatile uint32_t count = ms * 3600;
    for(volatile uint32_t i = 0; i < count; i++);
}

/* ================= CLOCK ================= */
void SystemClock_Config(void)
{
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));

    RCC->CFGR |= RCC_CFGR_PLLSRC;
    RCC->CFGR |= RCC_CFGR_PLLMULL9;

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
