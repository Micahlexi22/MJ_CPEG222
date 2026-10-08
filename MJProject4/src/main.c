//**********************************************
// * Author: M. Johnson
// * CPEG222 Project 4, 10/2/26
// * NucleoF466ZE CMSIS 
// *
// * Project Description: 
// * Using the CPEG222 Shield added on top of the Nucleo Board, 
// * board plays a sound at a specific frequency depending on the value read from the potentiometer.
// * As the potentiometer value changes, the frequency value is shown on the SSD.
// * The switch determines if sound is on or off (S = 1 -> ON, S = 0 -> OFF), but the frequency still changes.
// * BONUS: When the "UP" button is pressed, "Happy Birthday" plays.
//**********************************************

// ** Imports & Helper Files **/
#include "IO.h"
#include "pinports.h"

// ** Main Functions ** //
int main(void){
    rcc_enable(); //Enable RCC for GPIOs, TIM5,TIM6
    IO_init(); //Initialize GPIO for SSD, Buttons, and DAC

    while(1){
        State_Machine(); //Main function determining what is active. Everything else handled by interrupts.
    }

    return 0; //Will never be reached
}

