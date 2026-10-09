// ****************************************************************
// * Authors: T. LUM and R. MARTIN
// * CPEG222 Demo Program5, 9/28/2026
// * NucleoF446ZE CMSIS STM32F4xx example Proj5
// * Sends text to the OLED display using SPI
// ****************************************************************
#include "stm32f4xx.h"
#include "OLED.h"
#include "SysClock180MHz.h"
#include <stdint.h> // Include stdint.h for uint32_t type
#include <stdio.h> // Include stdio.h for snprintf function
#include <stdbool.h>

#define SW1 8
#define SW1_Port GPIOC

void delay_ms(uint32_t ms); // Function prototype for delay function
char buffer[90];
uint32_t initialClock; // Initial system clock frequency (16 MHz)
uint32_t finalClock;   // Final system clock frequency (180 MHz)

void IOinit(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;// Enable GPIOC clock
    // -- Configure Switch as Input-- //
    SW1_Port->MODER &= ~(0x3 <<(SW1 * 2)); //Set switch to input mode
    SW1_Port->PUPDR &= ~(0x3 << (SW1 * 2));//Clear mode bits
    SW1_Port->PUPDR |= (0x2 << (SW1 * 2)); //Change to pull down resistor
}





int main(void){
    SystemCoreClockUpdate(); // CMSIS function to update SystemCoreClock variable
    initialClock = SystemCoreClock; // Store the initial clock frequency (16 MHz)
    SystemClock_Config(); // Configure the system clock to 180 MHz
    SystemCoreClockUpdate(); // Update SystemCoreClock variable after clock change
    finalClock = SystemCoreClock; // Store the final clock frequency (180 MHz)

    IOinit();

    OLED_Init();
    SH1106_RenderFullScreenLogo();
    delay_ms(4000); // Delay for 4 seconds to display the logo
    OLED_Clear();
    OLED_Print(0, 0, "CPEG222-F26 OLED Test");
    snprintf(buffer, sizeof(buffer), "Initial Clock: %lu MHz  ", initialClock / 1000000U);               
    OLED_Print(5, 0, buffer);
    snprintf(buffer, sizeof(buffer), "Final Clock: %lu MHz  ", finalClock / 1000000U);
    OLED_Print(6, 0, buffer);
    delay_ms(8000); // Delay for 8 seconds to display the clock information
    OLED_Clear();
    OLED_Print(0, 0, "CPEG222-F26 OLED Test");
    OLED_Print(1, 0, "October 16th, 2026");
    OLED_Print(3, 0, "Micah Johnson");

    uint8_t s1_on = ((SW1_Port->IDR & (1 << SW1)) != 0);
    uint8_t inverted = false;

    while(1){
        // Main loop
        if (s1_on && (inverted == false)){
            OLED_WriteCmd(0xA7); //Black 0n White
            inverted = true;
        }else if(!s1_on && (inverted == true)){
            OLED_WriteCmd(0xA6); //Normal Display
            inverted = false;
        }
    }
    // Program should never reach here due to infinite loop
    return 0;
}

void delay_ms(uint32_t ms){
    if(SystemCoreClock == 16000000U){ // If the system clock is 16 MHz
        for(uint32_t i = 0; i < ms * 2000; i++){
            __NOP();
        }
    } else if(SystemCoreClock == 180000000U){ // If the system clock is 180 MHz
        for(uint32_t i = 0; i < ms * 22500; i++){
            __NOP();
        }
    }
}