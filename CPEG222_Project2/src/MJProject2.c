//**********************************************
// * Author: M. Johnson
// * CPEG222 Project 2, 9/21/26
// * NucleoF466ZE CMSIS 
// *
// * Project Description: 
// * Using the CPEG222 Shield added on top of the Nucleo Board, 
// * this project will sequence 8 LEDs basued on switch input and push buttons.
//**********************************************

//** Imports & Definitions **//
#include "stm32f4xx.h" // Needed for GPIO and RCC registers
#include <stdbool.h> //Included for boolean
#define V_POT_PIN 2
#define V_POT_PORT GPIOC
#define ADC_CHANNEL 12 //Channel for GPIOC pin 2
#define ADC_SAMPLES 16 // Num samples for averaging

#define S1_PIN 8
#define S1_PORT GPIOC
#define S2_PIN 9
#define S2_PORT GPIOC
#define S3_PIN 10
#define S3_PORT GPIOC
#define S4_PIN 11
#define S4_PORT GPIOC

#define LEFT_PIN 9
#define LEFT GPIOF
#define CENTER_PIN 8
#define CENTER GPIOF
#define RIGHT_PIN 6
#define RIGHT GPIOE


#define LED1_PIN 0
#define LED1 GPIOD
#define LED2_PIN 1
#define LED2 GPIOD
#define LED3_PIN 2
#define LED3 GPIOD
#define LED4_PIN 3
#define LED4 GPIOD
#define LED5_PIN 4
#define LED5 GPIOD
#define LED6_PIN 5
#define LED6 GPIOD
#define LED7_PIN 6
#define LED7 GPIOD
#define LED8_PIN 7
#define LED8 GPIOD


//** Variable Definitions **//
volatile bool paused = true; //Keeps track of whether the LED cycling is paused
volatile uint32_t last_interrupt_time = 0; //Keeps track of last interrupt time for debouncing
volatile uint16_t shift_state = 0; //Keeps track of current shift state
volatile uint32_t delay_ms; //Default until updated
uint8_t LED_pattern = 0; // Tracks Position of LEDs

const uint8_t led_pins[8] = {LED1_PIN, LED2_PIN, LED3_PIN, LED4_PIN,LED5_PIN,LED6_PIN,LED7_PIN,LED8_PIN};

void delay(volatile uint32_t ms){ // simple delay function
    while(ms--){
        volatile uint32_t count = 2047UL;
        while(count--){
            __NOP(); //Tells clock to do nothing for one clock cycle
        }
    }

}

//void SysTick_Handler(void){}

void EXTI9_5_IRQHandler(void){ //Interrupt Handler Function
    //Debounce Button Presses
    if(EXTI->PR & (1 << LEFT_PIN)){
        EXTI->PR = (1 << LEFT_PIN); //Clear pending flag
        shift_state = 1;
        paused = false;
    }
    if(EXTI->PR & (1 << CENTER_PIN)){
        EXTI->PR = (1 << CENTER_PIN); //Clear pending flag
        shift_state = 0;
        paused = true;
    }
    if(EXTI->PR & (1 << RIGHT_PIN)){
        EXTI->PR = (1 << RIGHT_PIN); //Clear pending flag 
        shift_state = 2;
        paused = false;
    }
}

uint16_t read_adc_pot(void){
    uint32_t total = 0;
    for(int i = 0;i <ADC_SAMPLES;i++){
        ADC3->CR2 |= ADC_CR2_SWSTART; //Start ADC Conversion

        uint32_t timeout = 10000;

        while(!(ADC3->SR & ADC_SR_EOC)){//Wait for conversion to complete
            timeout--; //Failsafe to prevent freezing
            if(timeout == 0){
                break;
            }
        }
       
        total += ADC3->DR; //Read ADC value and add to total
    }
    uint16_t average = total / ADC_SAMPLES; 
    return (uint16_t)average;
}

void update_leds_paused(void){

    LED_pattern = 0;//Resets to clear the LEDs

    //Reads the switches directly into bits 0-3
    if(S1_PORT->IDR & (1<< S1_PIN)) LED_pattern |= (1<<0);
    if(S2_PORT->IDR & (1<< S2_PIN)) LED_pattern |= (1<<1);
    if(S3_PORT->IDR & (1<< S3_PIN)) LED_pattern |= (1<<2);
    if(S4_PORT->IDR & (1<< S4_PIN)) LED_pattern |= (1<<3);

    update_led_output();
}

void update_led_output(){
    //uint8_t pos1 = position;
   //uint8_t pos2 = (position + 1) % 8;
    //uint8_t pos3 = (position + 2) % 8;
   // uint8_t pos4 = (position + 3) % 8;

    for(int i = 0;i<8;i++){
        if (LED_pattern & (1<<i)){
            GPIOD->BSRR = (1 << led_pins[i]); //Set Pins
        }else{
            GPIOD->BSRR = (1<<(led_pins[i] + 16)); //Clear pins
        }
    }
}

int main(void){
    // Enable clock for GPIO ports D (LEDs), F(Analog), E, & C
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN;
    RCC ->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //Enable Syscfg clock
    RCC ->APB2ENR |= RCC_APB2ENR_ADC3EN; //Enable ADC3 clock

    // Configure GPIO pins PD0-PD7 (LEDs) as output
    GPIOD->MODER &= ~(0xFFFF); // Clear mode bits 
    GPIOD->MODER |= (0x5555); // Set as General purpose output mode 

    // Configure GPIO PE6 as input
    GPIOE->MODER &= ~((0x3 << (RIGHT_PIN*2)));
    GPIOE->PUPDR &= ~((0x3 << (RIGHT_PIN*2)));
    GPIOE->PUPDR |= ((0x1 << (RIGHT_PIN*2)));

    // Configure GPIO PF8,PF9 as inputs
    GPIOF->MODER &= ~((0x3 << (LEFT_PIN*2))|(0x3 << (CENTER_PIN*2)));
    GPIOF->PUPDR &= ~((0x3 << (LEFT_PIN*2))|(0x3 << (CENTER_PIN*2)));
    GPIOF->PUPDR |= ((0x1 << (LEFT_PIN*2))|(0x1 << (CENTER_PIN*2)));


    // Set up ADC Mode and Potentiometer
    V_POT_PORT->MODER &= ~(0x3 << (V_POT_PIN * 2)); // Clear mode bits for PC2
    V_POT_PORT->MODER |= (0x3 << (V_POT_PIN * 2)); // Set as Analog mode for PC2
    ADC3 -> SQR3 &= ~(0x1F); //Clear bits
    ADC3 ->SQR3|= ADC_CHANNEL; // Write into channel 12
    ADC3->CR2 = ADC_CR2_ADON; //Enable ADC

    // Configure Interrupts
    SYSCFG->EXTICR[1] &= ~(0xF << 8); //clear EXTI6
    SYSCFG->EXTICR[1] |= (0x4 << 8); //Set EXTI6 to PortE
    SYSCFG->EXTICR[2] &= ~((0xF << 0) | (0xF << 4)); // clear EXTI8 & EXTI9
    SYSCFG->EXTICR[2] |= (0x5 << 0) | (0x5 << 4); //Set EXTI8 and EXTI9 to PortF

    // Enable EXTI lines 6, 8, 9
    EXTI->IMR |= (1 << LEFT_PIN) | (1 << CENTER_PIN) | (1 << RIGHT_PIN); //Unmask interrupts
    EXTI->RTSR &= ~((1<< LEFT_PIN) | (1 << CENTER_PIN) | (1 << RIGHT_PIN)); //Trigger on rising edge 
    EXTI->FTSR |= (1<< LEFT_PIN) | (1 << CENTER_PIN) | (1 << RIGHT_PIN); //Trigger on Falling edge 

    NVIC_SetPriority(EXTI9_5_IRQn,2);
    NVIC_EnableIRQ(EXTI9_5_IRQn); //Enable EXTI interrupt in NVIC
    //SysTick_COnfig(SystemCoreClock/10);

    // Set Switch Pins as Inputs
    S1_PORT->MODER &= ~(0x3 << (S1_PIN *2)); // clear mode bits
    S1_PORT-> MODER |= (0x0 <<(S1_PIN *2)); // Set as input mode
    S2_PORT->MODER &= ~(0x3 << (S2_PIN *2)); // clear mode bits
    S2_PORT-> MODER |= (0x0 <<(S2_PIN *2)); // Set as input mode
    S3_PORT->MODER &= ~(0x3 << (S3_PIN *2)); // clear mode bits
    S3_PORT-> MODER |= (0x0 <<(S3_PIN *2)); // Set as input mode
    S4_PORT->MODER &= ~(0x3 << (S4_PIN *2)); // clear mode bits
    S4_PORT-> MODER |= (0x0 <<(S4_PIN *2)); // Set as input mode

    //Set all LEDs off initially
    LED1 -> BSRR |= (1<<(LED1_PIN+16));
    LED2 -> BSRR |= (1<<(LED2_PIN+16));
    LED3 -> BSRR |= (1<<(LED3_PIN+16));
    LED4 -> BSRR |= (1<<(LED4_PIN+16));
    LED5 -> BSRR |= (1<<(LED5_PIN+16));
    LED6 -> BSRR |= (1<<(LED6_PIN+16));
    LED7 -> BSRR |= (1<<(LED7_PIN+16));
    LED8 -> BSRR |= (1<<(LED8_PIN+16));


    while(1){
        delay_ms = read_adc_pot(); //reads adc to find delay

        if(shift_state == 0 || paused){ //paused state
            update_leds_paused();
            continue;//Skip the rest of the while loop
        }
        //Cycle through LEDs
        
        if(shift_state == 1){ //shift left
            LED_pattern = (LED_pattern << 1) | (LED_pattern >> 7); //Loops from 8 to 1
        }else if(shift_state == 2){ //shift right
            LED_pattern = (LED_pattern >> 1) | (LED_pattern << 7); //Loops from 1 to 8
        }
        update_led_output();
        delay(delay_ms);
        
    }
    return (0); //Line will never be reached
}
