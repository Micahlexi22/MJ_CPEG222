// ****************************************************************
// * Authors: Micah Johnson
// * CPEG222 Project 5
// * 10/15/2026
// * NucleoF446ZE CMSIS STM32F4xx example Proj5
// * Sends text to the OLED display using SPI
// ****************************************************************
#include "stm32f4xx.h"
#include "OLED.h"
#include "SysClock180MHz.h"
#include <stdint.h> // Include stdint.h for uint32_t type
#include <stdio.h> // Include stdio.h for snprintf function
#include <stdbool.h>

//Switch1
#define SW1 8 
#define SW1_Port GPIOC

//Buttons
#define LEFT_PIN 9
#define LEFT GPIOF
#define RIGHT_PIN 6
#define RIGHT GPIOE
#define UP_PIN 7
#define UP GPIOF
#define DOWN_PIN 5
#define DOWN GPIOE

void delay_ms(uint32_t ms); // Function prototype for delay function
char buffer[90];
uint32_t initialClock; // Initial system clock frequency (16 MHz)
uint32_t finalClock;   // Final system clock frequency (180 MHz)

static int last_row = 3;
static int last_col = 0;

void IOinit(void){ //Initializing Extra Credit Inputs
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN);// Enable GPIOC, GPIOE, and GPIOF clocks
    // -- Configure Switch as Input-- //
    SW1_Port->MODER &= ~(0x3 <<(SW1 * 2)); //Set switch to input mode
    SW1_Port->PUPDR &= ~(0x3 << (SW1 * 2));//Clear mode bits
    SW1_Port->PUPDR |= (0x2 << (SW1 * 2)); //Change to pull down resistor

    // -- Configure Buttons as Inputs -- //
    
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
    OLED_Print(last_row, last_col, "Micah Johnson");


    uint8_t inverted = false;

    while(1){ // Main loop

        //Extra Credit 1
        uint8_t s1_on = ((SW1_Port->IDR & (1 << SW1)) != 0);
    
        if (s1_on && (inverted == false)){
            OLED_WriteCmd(0xA7); //Black 0n White
            inverted = true;
        }else if(!s1_on && (inverted == true)){
            OLED_WriteCmd(0xA6); //Normal Display
            inverted = false;
        }

        uint8_t row = last_row;
        uint8_t col = last_col;

        //Extra Credit 2
        uint8_t btn_up = ((UP->IDR & (1<<UP_PIN))!= 0 );
        uint8_t btn_down = ((DOWN->IDR & (1<<DOWN_PIN))!= 0 );
        uint8_t btn_left = ((LEFT->IDR & (1<<LEFT_PIN))!= 0 );
        uint8_t btn_right = ((RIGHT->IDR & (1<<RIGHT_PIN))!= 0 );

        if(btn_up || btn_down || btn_left || btn_right){
            int new_row = row;
            int new_col = col;

            if(btn_up && row < 8){ //moves up
                new_row++;
            }
            if(btn_down && row > 2){ // Bc top row is row 0 , moves down
                new_row--;
            }
            if(btn_left){
                if(col + 4 < 46){
                    new_col+= 4;
                }else{
                    new_col = 45;
                }
            }
            if(btn_right){
                if(col >= 4){
                    new_col -= 4;
                }else{
                    new_col = 0;
                }
            }

            last_row = row;
            last_col = col;

            if(new_row != last_row || new_col != last_col){
                OLED_Print(last_row,last_col,"                   ");

                row = new_row;
                col = new_col;

                //Redraw 
                OLED_Print(row,col,"Micah Johnson");

                last_row = row;
                last_col = col;

            }

            delay_ms(250); // debounce delay
        }
    }
    return 0; // Program should never reach here due to infinite loop
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