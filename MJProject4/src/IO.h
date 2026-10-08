//**********************************************
// * Author: M. Johnson
// * CPEG222 Project 4, 10/2/26
// * NucleoF466ZE CMSIS 
//**********************************************

// ** Imports ** //
#include "stm32f4xx.h" // Needed for GPIO and RCC registers
#include <stdbool.h> //Included for boolean
#include <math.h>
#include <stdlib.h>


// ** Function Declarations ** //
void rcc_enable(void); //Function to enable RCC for GPIO, TIM5, TIM6
void SysTick_Handler(void); //Interrupt handler for SysTick

void TIM5_IRQHandler(void); //Interrupt handler for TIM5
void EXTI9_5_IRQHandler(void); //Interrupt handler for EXTI9_5

void IO_init(void); //Function to initialize GPIO for SSD, Buttons, and DAC
void refresh(uint16_t num); //Function to refresh SSD

uint16_t read_adc_pot(void); //Function to read ADC value from potentiometer
void update_frequency(uint16_t new_frequency); //Function to update frequency of sine wave
uint16_t adc_to_freq(uint16_t adc_value); //Function to convert ADC value to frequency

void State_Machine(void); //Function to handle state machine