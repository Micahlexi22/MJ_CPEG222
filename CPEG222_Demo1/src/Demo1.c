/********************************************
 * CPEG222 DemoProgram 1, 9/2/26
 * Author: M. Johnson
 * NUCLEOF446ZE CMSIS STM32F4xx Demo1 Blink PB0
 ********************************************/

#include <stdint.h> // Required by CMSIS; configure IntelliSense to find system headers
#include "stm32f4xx.h" // Include the STM32F4xx CMSIS header file
#define LED_PIN 0 //Assuming LED is connected to GPIOB pin 0
#define LED_PORT GPIOB

void delay(volatile uint32_t count){ // simple delay function
    while(count --){ //Function will block for a while, creating a delay
    }
}
int main(void){
    //1.Enable clock for GPIO Port B
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    //2.Configure GPIOB pin 0 (LED1) as output
    LED_PORT->MODER &= ~(0x3 << (LED_PIN *2)); // clear mode bits
    LED_PORT->MODER |= (0x1 << (LED_PIN *2)); // set as general purpose output mode

    while(1){ //Main loop to continuously toggle LED
        LED_PORT->ODR ^= (1 << LED_PIN); //Toggle the LED (LD1)
        delay(1000000); //Simple delay
    
    }
    
    return 0; //This line will never be reached, but is good practice to include
}