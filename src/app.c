#include <stdio.h>
#if defined(GD32F10x)
#include "gd32f10x.h"
#elif defined(GD32F1x0)
#include "gd32f1x0.h"
#elif defined(GD32F3x0)
#include <gd32f3x0.h>
#elif defined(GD32F30x)
#include "gd32f30x.h"
#elif defined(GD32F40x)
#include "gd32f4xx.h"
#elif defined(GD32E10X)
#include "gd32e10x.h"
#endif


void delay_1ms(uint32_t count);
extern void initialise_monitor_handles(void);

#pragma GCC optimize("Ofast,unroll-loops")
uint32_t test_flash_speed(uint32_t addr) {
    register uint32_t ticks_before asm("r3") = SysTick->VAL;
    volatile uint32_t* pFlash = (uint32_t*) addr;
    // do 32 reads and average results
    // is unrolled by compiler into individual LDR instructions
    #pragma GCC unroll 32
    for(int i=0; i < 32; i++) {
        register volatile uint32_t dummy asm("r4") = *pFlash++; // do read
    }
    register uint32_t ticks_after asm("r5") = SysTick->VAL;
    return (ticks_before - ticks_after)/32; /* SYSTICK is a DOWN-COUNTING TIMER! */
}

int main(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        /* capture error */
        while (1) {}
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);

    initialise_monitor_handles();
    
    // disable systick interrupt
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    /*SysTick_CTRL_TICKINT_Msk   |*/
                    SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */


    #define FLASH_START 0x8000000UL
    #define FLASH_SIZE (128UL * 1024UL) /* 128K */
    #define FLASH_END  (FLASH_START + FLASH_SIZE)
    #define FLASH_READ_WIDTH 32
    for(uint32_t addr = FLASH_START; addr < FLASH_END - FLASH_READ_WIDTH;  addr += 16*1024UL) {
        uint32_t ticks_taken = test_flash_speed(addr);
        printf("%dK: %u ticks\n", (unsigned)(addr - FLASH_START) / 1024, (unsigned)ticks_taken);
    }
    uint32_t ticks_taken = test_flash_speed(FLASH_END - FLASH_READ_WIDTH);
    printf("%dK: %u ticks\n", (unsigned)(FLASH_END - FLASH_START) / 1024, (unsigned)ticks_taken);


    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

    while(1) {
        delay_1ms(1000);
        printf("Test over\n");
    }
}

volatile static uint32_t delay = 0;

void delay_1ms(uint32_t count)
{
    delay = count;

    while (0U != delay)
    {
    }
}

void SysTick_Handler(void)
{
    if (0U != delay)
    {
        delay--;
    }
}