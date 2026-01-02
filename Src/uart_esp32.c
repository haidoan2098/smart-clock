/******************************************************************************
 * Author: Doan Trong My - 23CE065
 ******************************************************************************/

#include "uart_esp32.h"

/* ============================ BASE ADDRESS ============================ */
#define PERIPH_BASE     0x40000000UL
#define APB2_BASE       (PERIPH_BASE + 0x10000)
#define AHB_BASE        (PERIPH_BASE + 0x20000)

/* ================================ RCC ================================ */
#define RCC_BASE        (AHB_BASE + 0x1000)
#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))

/* =============================== GPIO ================================ */
#define GPIOA_BASE      (APB2_BASE + 0x0800)
#define GPIOC_BASE      (APB2_BASE + 0x1000)

#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_BSRR      (*(volatile uint32_t *)(GPIOC_BASE + 0x10))

/* ============================== USART1 =============================== */
#define USART1_BASE     (APB2_BASE + 0x3800)

#define USART1_SR       (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR       (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR      (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1      (*(volatile uint32_t *)(USART1_BASE + 0x0C))

/* ================================ NVIC ================================ */
#define NVIC_ISER1      (*(volatile uint32_t *)0xE000E104)

/* ============================== BIT DEFINE ============================ */
#define USART_SR_RXNE    (1 << 5)
#define USART_CR1_UE     (1 << 13)
#define USART_CR1_RE     (1 << 2)
#define USART_CR1_RXNEIE (1 << 5)

/* ============================== VARIABLES ============================ */
char rxBuffer[50];
uint8_t rxIndex = 0;
volatile uint8_t dataReady = 0;

/* ============================== GPIO LED ============================= */
void GPIO_LED_Init(void)
{
    RCC_APB2ENR |= (1 << 4);          // GPIOC

    GPIOC_CRH &= ~(0xF << 24);
    GPIOC_CRH |=  (0x2 << 24);        // PC14 output 2MHz

    GPIOC_BSRR = (1 << (14 + 16));    // LED OFF
}

/* ============================== UART INIT ============================ */
void UART1_Init(void)
{
    RCC_APB2ENR |= (1 << 2);          // GPIOA
    RCC_APB2ENR |= (1 << 14);         // USART1

    /* PA9 TX */
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xB << 4);

    /* PA10 RX */
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |=  (0x4 << 8);

    USART1_BRR = 8000000 / 115200;

    USART1_CR1 |= USART_CR1_RE | USART_CR1_RXNEIE;
    USART1_CR1 |= USART_CR1_UE;

    NVIC_ISER1 |= (1 << (37 - 32));
}

/* =========================== USART1 INTERRUPT ========================= */
void USART1_IRQHandler(void)
{
    if (USART1_SR & USART_SR_RXNE)
    {
        char c = USART1_DR;

        if (c == '\n' || c == '\r')
        {
            rxBuffer[rxIndex] = '\0';
            rxIndex = 0;
            dataReady = 1;
        }
        else if (rxIndex < sizeof(rxBuffer) - 1)
        {
            rxBuffer[rxIndex++] = c;
        }
        else
        {
            rxIndex = 0;
        }
    }

    dataReady = 1;
}
