/*
 * STM32F103C8T6 - I2C Register Level Driver
 * I2C1: PB6 (SCL), PB7 (SDA)
 * Speed: 100kHz
 * APB1: 8MHz
 */

#include <stdint.h>

/* ============================================================================
 * BASE ADDRESSES
 * ==========================================================================*/
#define RCC_BASE        0x40021000
#define GPIOB_BASE      0x40010C00
#define I2C1_BASE       0x40005400
#define TIM2_BASE       0x40000000

/* ============================================================================
 * RCC REGISTERS
 * ==========================================================================*/
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))

#define RCC_APB2ENR_IOPBEN   (1 << 3)
#define RCC_APB1ENR_I2C1EN   (1 << 21)
#define RCC_APB1ENR_TIM2EN   (1 << 0)

/* ============================================================================
 * GPIOB REGISTERS
 * ==========================================================================*/
#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00))

/* ============================================================================
 * I2C1 REGISTERS
 * ==========================================================================*/
#define I2C1_CR1        (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2        (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1       (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR         (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1        (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2        (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR        (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE      (*(volatile uint32_t *)(I2C1_BASE + 0x20))

/* ============================================================================
 * TIM2 REGISTERS
 * ==========================================================================*/
#define TIM2_CR1        (*(volatile uint32_t *)(TIM2_BASE + 0x00))
#define TIM2_CNT        (*(volatile uint32_t *)(TIM2_BASE + 0x24))
#define TIM2_PSC        (*(volatile uint32_t *)(TIM2_BASE + 0x28))
#define TIM2_ARR        (*(volatile uint32_t *)(TIM2_BASE + 0x2C))
#define TIM2_EGR        (*(volatile uint32_t *)(TIM2_BASE + 0x14))

/* ============================================================================
 * I2C BIT DEFINITIONS
 * ==========================================================================*/
#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)
#define I2C_CR1_SWRST   (1 << 15)

#define I2C_SR1_SB      (1 << 0)
#define I2C_SR1_ADDR    (1 << 1)
#define I2C_SR1_RXNE    (1 << 6)
#define I2C_SR1_TXE     (1 << 7)
#define I2C_SR1_AF      (1 << 10)

/* ============================================================================
 * DS3231 DEFINITIONS
 * ==========================================================================*/
#define DS3231_ADDR     0x68
#define DS3231_REG_TIME 0x00

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} RTC_Time;

/* ============================================================================
 * TIMER DELAY (TIM2)
 * ==========================================================================*/
void TIM2_DelayInit(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* APB1 = 8MHz → 8MHz / 8000 = 1kHz → 1ms/tick */
    TIM2_PSC = 8000 - 1;
    TIM2_ARR = 0xFFFFFFFF;
    TIM2_EGR = 1;
    TIM2_CR1 |= 1;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = TIM2_CNT;
    while ((TIM2_CNT - start) < ms);
}

/* ============================================================================
 * UTILITIES
 * ==========================================================================*/
static uint8_t BCD_to_DEC(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t DEC_to_BCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

/* ============================================================================
 * I2C LOW LEVEL
 * ==========================================================================*/
static void I2C_Start(uint8_t addr)
{
    I2C1_CR1 |= I2C_CR1_START;
    while (!(I2C1_SR1 & I2C_SR1_SB));
    I2C1_DR = addr;
    while (!(I2C1_SR1 & I2C_SR1_ADDR));
    (void)I2C1_SR1;
    (void)I2C1_SR2;
}

static void I2C_Stop(void)
{
    I2C1_CR1 |= I2C_CR1_STOP;
}

static void I2C_WriteByte(uint8_t data)
{
    while (!(I2C1_SR1 & I2C_SR1_TXE));
    I2C1_DR = data;
}

static uint8_t I2C_ReadByte_ACK(void)
{
    I2C1_CR1 |= I2C_CR1_ACK;
    while (!(I2C1_SR1 & I2C_SR1_RXNE));
    return I2C1_DR;
}

static uint8_t I2C_ReadByte_NACK(void)
{
    I2C1_CR1 &= ~I2C_CR1_ACK;
    I2C_Stop();
    while (!(I2C1_SR1 & I2C_SR1_RXNE));
    return I2C1_DR;
}

/* ============================================================================
 * I2C INIT
 * ==========================================================================*/
void I2C1_Init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* PB6, PB7: AF Open-drain, 2MHz */
    GPIOB_CRL &= ~0xFF000000;
    GPIOB_CRL |=  (0xEE << 24);

    I2C1_CR1 |= I2C_CR1_SWRST;
    I2C1_CR1 &= ~I2C_CR1_SWRST;

    I2C1_CR2   = 8;
    I2C1_CCR   = 40;
    I2C1_TRISE = 9;
    I2C1_OAR1  = (1 << 14);

    I2C1_CR1 |= I2C_CR1_ACK;
    I2C1_CR1 |= I2C_CR1_PE;
}

/* ============================================================================
 * DS3231 DRIVER
 * ==========================================================================*/
void DS3231_SetTime(uint8_t h, uint8_t m, uint8_t s)
{
    I2C_Start(DS3231_ADDR << 1);
    I2C_WriteByte(DS3231_REG_TIME);
    I2C_WriteByte(DEC_to_BCD(s) & 0x7F);
    I2C_WriteByte(DEC_to_BCD(m) & 0x7F);
    I2C_WriteByte(DEC_to_BCD(h) & 0x3F);
    I2C_Stop();
}

void DS3231_GetTime(RTC_Time *t)
{
    uint8_t sec, min, hour;

    I2C_Start(DS3231_ADDR << 1);
    I2C_WriteByte(DS3231_REG_TIME);

    I2C_Start((DS3231_ADDR << 1) | 1);

    sec  = I2C_ReadByte_ACK();
    min  = I2C_ReadByte_ACK();
    hour = I2C_ReadByte_NACK();

    t->seconds = BCD_to_DEC(sec  & 0x7F);
    t->minutes = BCD_to_DEC(min  & 0x7F);
    t->hours   = BCD_to_DEC(hour & 0x3F);
}

/* ============================================================================
 * MAIN
 * ==========================================================================*/
RTC_Time now;

int main(void)
{
    I2C1_Init();
    TIM2_DelayInit();

    delay_ms(100);

    DS3231_SetTime(10, 15, 14);

    while (1)
    {
        DS3231_GetTime(&now);
        delay_ms(1000);
    }
}
