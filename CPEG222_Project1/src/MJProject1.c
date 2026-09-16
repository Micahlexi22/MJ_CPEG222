//**********************************************
// * Author: M. Johnson
// * CPEG222 Project1, 9/2/26
// * NucleoF466ZE CMSIS Sequence Nucleo LEDs with USR BTN
// *
// * Project Description: 
// * This project uses a user button on the Nucleo board to switch between the 3 LEDs.
// * The user starts with the red LED on, and then flips to red, green, and blue with each button press.
//**********************************************

//** Imports **//
#include "stm32f4xx.h" // Needed for GPIO and RCC registers
#include <stdbool.h> //Included for boolean

#define Button_pin 13 //USR Button
#define Button_port GPIOC
#define Green_LED_pin 0 //Green LED
#define Green_LED_port GPIOB
#define Blue_LED_pin 7 //Blue LED
#define Blue_LED_port GPIOB
#define Red_LED_pin 14 //Red LED
#define Red_LED_port GPIOB

//**Global Variables **//
int LED_state = 0; //0 = red, 1 = green, 2= blue
bool ButtonLock = false; //Lock to prevent multiple state changes on a single button press

void delay(volatile uint32_t count){ // simple delay function
    while(count --){ //Function will block for a while, creating a delay
    }
}

int main(void){
    //1. Enable clock for GPIO ports B and C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    //2. Configure GPIOC pin 13 (USR BTN)
    Button_port-> MODER &= ~(0x3 <<(Button_pin *2)); // clear mode bits
    Button_port-> MODER |= (0x0 <<(Button_pin *2)); // Set as input mode

    //3. Configure GPIOB pins (LED1) as output
    Green_LED_port->MODER &= ~(0x3 << (Green_LED_pin * 2)); // clear mode bits
    Green_LED_port->MODER |= (0x1 << (Green_LED_pin * 2)); // set as general purpose output mode
    Red_LED_port->MODER &= ~(0x3 << (Red_LED_pin *2)); // clear mode bits
    Red_LED_port->MODER |= (0x1 << (Red_LED_pin *2)); // set as general purpose output mode
    Blue_LED_port->MODER &= ~(0x3 << (Blue_LED_pin *2)); // clear mode bits
    Blue_LED_port->MODER |= (0x1 << (Blue_LED_pin *2)); // set as general purpose output mode

    //4. Turn on red LED to start
    Red_LED_port->BSRR |= (1<<Red_LED_pin); //Turn on Red LED

    //5. Continuous Main Loop to toggle state changes
    while(1) {
        delay(125000); //Delay to debounce button press
        if((Button_port ->IDR & (1 << Button_pin)) && !ButtonLock) //Check if button is pressed and not locked))
        {
            if(LED_state == 0){ //Initial State with Red LED
                Green_LED_port ->BSRR |= (1<< Green_LED_pin); // Turn on Green LED
                Red_LED_port->BSRR |= (1<< (Red_LED_pin + 16)); //Turn off Red LED
                LED_state = 1; //Changes to next state
            }else if(LED_state == 1){
                Blue_LED_port -> BSRR |= (1<<Blue_LED_pin); // Turn on Blue LED
                Green_LED_port -> BSRR |= (1<<(Green_LED_pin+16)); // Turn off Green LED
                LED_state = 2; //Changes to next state

            }else if (LED_state == 2){
                LED_state = 0; //Changes to next/initial state
                Red_LED_port -> BSRR |= (1<<Red_LED_pin); // Turn on Red LED
                Blue_LED_port -> BSRR |= (1<<(Blue_LED_pin+16)); // Turn off Blue LED
                
            }
            ButtonLock = true; // Lock button to prevent unexpected state changes
        }else if(!(Button_port ->IDR & (1 << Button_pin))){ // Check if button is released
            ButtonLock = false; // Unlock botton for next state change
        }
    }

    delay(125000); //Delay to debounce button press
    return 0; //Required for main function, will never be reached

}