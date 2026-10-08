//**********************************************
// * Author: M. Johnson
// * CPEG222 Project 4, 10/2/26
// * NucleoF466ZE CMSIS 
//**********************************************

// ** Imports ** //
#include "IO.h"
#include "pinports.h"

// ** Constants & Variables ** //
int frequency = 440;
int samples = 100;
int ticks; //Used to set ARR for Tim6
volatile uint8_t sine_index = 0; //Used to index into the sine LUT
volatile uint16_t num = 100;
uint16_t state = 0; //Keeps track of current state
uint32_t note_start_time = 0;
uint8_t note_index = 0; //Keeps track of current note in the birthday tune

const unsigned char SSDdigits[] = {
    //SSD is common anode, so 0 = on, 1 = off.

    0b1000000, //0
    0b1111001, //1
    0b0100100, //2
    0b0110000, //3
    0b0011001, //4
    0b0010010, //5
    0b0000010, //6
    0b1111000, //7
    0b0000000, //8
    0b0010000, //9
    0b1111111 //blank or off
};

const uint16_t birthday_tune[] = {
    262, 262, 294, 262, 349, 330, //Happy Birthday To You
    262, 262, 294, 262, 392, 349, //Happy Birthday To You
    262, 262, 523, 440, 349, 330, 294, //Happy Birthday Dear [Name]
    466, 466, 440, 349, 392, 349 //Happy Birthday To You
};

//Durations in ms, (150ms = eighth note,300ms = quarter note, 600ms = half note, 1200ms = whole note)
const uint16_t note_lengths[] = {
    300, 300, 600, 600, 600, 1200, //Happy Birthday To You
    300, 300, 600, 600, 600, 1200, //Happy Birthday To You
    300, 300, 600, 600, 600, 600, 1200, //Happy Birthday Dear [Name]
    300, 300, 600, 600, 600, 1200 //Happy Birthday To You
};

const uint16_t sine_lut[100] = { //Array of Sine Wave values
2048, 2110, 2173, 2235, 2296, 2357, 2416, 2474, 2530, 2584,
2636, 2685, 2732, 2777, 2818, 2857, 2892, 2924, 2953, 2978,
2999, 3016, 3030, 3040, 3046, 3048, 3046, 3040, 3030, 3016,
2999, 2978, 2953, 2924, 2892, 2857, 2818, 2777, 2732, 2685,
2636, 2584, 2530, 2474, 2416, 2357, 2296, 2235, 2173, 2110,
2048, 1985, 1922, 1860, 1799, 1738, 1679, 1621, 1565, 1511,
1459, 1410, 1363, 1318, 1277, 1238, 1203, 1171, 1142, 1117,
1096, 1079, 1065, 1055, 1049, 1048, 1049, 1055, 1065, 1079,
1096, 1117, 1142, 1171, 1203, 1238, 1277, 1318, 1363, 1410,
1459, 1511, 1565, 1621, 1679, 1738, 1799, 1860, 1922, 1985
};

uint16_t total_notes = sizeof(birthday_tune) / sizeof(birthday_tune[0]); //Total number of notes in the birthday tune
GPIO_TypeDef* SSD_PORTS[] = {SEG_G,SEG_F,SEG_E,SEG_D,SEG_C,SEG_B,SEG_A};
uint16_t SSD_PINS[] = {1 << G,1 << F,1 << E, 1<< D,1 << C,1 << B, 1 << A}; // Bit masks Pins for MUX
GPIO_TypeDef* DIGIT_PORTS[] = {Digit1,Digit2,Digit3,Digit4};
uint16_t DIGIT_PINS[] = {1 << Digit1_PIN,1 << Digit2_PIN, 1<< Digit3_PIN, 1<< Digit4_PIN};

// ** Function Definitions ** //

void SysTick_Handler(void){
    ticks++;
}

void rcc_enable(void){ //Function to enable RCC for GPIO, TIM5, TIM6
    // -- Enable Clock Registers -- //
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN|  RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN | RCC_AHB1ENR_GPIOGEN);
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //Enable Syscfg clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN; //Enable TIM5 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; //Enable TIM6 clock 

    RCC->APB2ENR |= RCC_APB2ENR_ADC3EN; // Enable ADC
    RCC->APB1ENR |= RCC_APB1ENR_DACEN; // Enable DAC clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; //Enable DMA1 clock
     
    // -- Configure DMA for DAC -- //
    GPIOA->MODER &= ~(3U << (4*2)); // Clear bits
    GPIOA->MODER |= (3U << (4*2)); //Set PA4 to analog mode for DAC output

    DMA1_Stream5->CR &= ~DMA_SxCR_EN; // Disable stream before configuration
    while (DMA1_Stream5->CR & DMA_SxCR_EN); // Wait until the stream is disabled
    DMA1_Stream5->CR &= ~(DMA_SxCR_CHSEL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE | DMA_SxCR_DIR);
    DMA1_Stream5->CR |= (7U << DMA_SxCR_CHSEL_Pos);// Set Channel 7 (Bits [27:25] = 111b)
    DMA1_Stream5->CR |= (0x1 << DMA_SxCR_MSIZE_Pos) | (0x1 << DMA_SxCR_PSIZE_Pos);
    DMA1_Stream5->CR |= DMA_SxCR_MINC; // Enable Memory Increment Mode
    DMA1_Stream5->CR |= (0x1 << DMA_SxCR_DIR_Pos); // Direction: Memory to Peripheral (01b)
    DMA1_Stream5->CR |= DMA_SxCR_CIRC; // Enable Circular Mode
    DMA1_Stream5->NDTR = samples; // Set Transfer Count and Addresses
    DMA1_Stream5->PAR = (uint32_t)&(DAC->DHR12R1); // DAC1 12-bit Right-Aligned Data Register
    DMA1_Stream5->M0AR = (uint32_t)sine_lut;
    DMA1_Stream5->CR |= DMA_SxCR_EN; // Enable DMA Stream

    // -- Configure DAC channel 1 -- //
    DAC->CR &= ~(7U << 3); // Select TIM6 TRGO as Trigger Source (TSEL = 000b)
    DAC->CR |= DAC_CR_TEN1; // Enable Triggering for Channel 1
    DAC->CR |= DAC_CR_DMAEN1; // Enable DMA Request Generation
    DAC->CR |= DAC_CR_EN1; // Enable DAC channel 1

    // -- Configure ADC3 and Potentiometer -- //
    GPIOF->MODER &= ~(3U << (3*2));
    GPIOF->MODER |= (3U <<(3*2));

    V_POT->MODER &= ~(0x3 << (V_POT_PIN * 2)); // Clear mode bits for PC2
    V_POT->MODER |= (0x3 << (V_POT_PIN * 2)); // Set as Analog mode for PC2

    ADC3 -> SQR3 &= ~(0x1F); //Clear bits
    ADC3 ->SQR3|= ADC_CHANNEL; // Write into channel 12
    ADC3->CR2 = ADC_CR2_ADON; //Enable ADC

    for(volatile int i = 0; i<1000; i++);//Short delay for ADC stabilization
    ADC3->CR2 |= ADC_CR2_SWSTART; //Start Conversions

    // -- Configure SysTick for 1ms interrupts -- //
    SysTick_Config(SystemCoreClock/1000); 

    // -- Configure TIM5 -- //
    TIM5->PSC = 15-1; //Prescaler
    TIM5->ARR = 2500-1;
    TIM5->EGR |= TIM_EGR_UG;
    TIM5->CNT = 0; //Reset counter
    TIM5->DIER |= TIM_DIER_UIE; //Enable update interrupt
    TIM5->SR &= ~TIM_SR_UIF; // Clear pending interrupts

    // -- Enable TIM5 -- //
    NVIC_SetPriority(TIM5_IRQn,1);
    NVIC_EnableIRQ(TIM5_IRQn);
    TIM5->CR1 |= TIM_CR1_CEN; //Enable TIM5

    // -- Configure TIM6 -- //
    TIM6->PSC = 0; //Prescaler
    TIM6->ARR = (SystemCoreClock / (samples * frequency)) - 1; //Calculate ARR for TIM6 based on desired frequency
    TIM6->EGR |= TIM_EGR_UG; //Update registers
    TIM6->CNT = 0; //Reset counter
    TIM6->CR2 &= ~TIM_CR2_MMS; //Select Update Event as Trigger Output (MMS = 000)
    TIM6->CR2 |= (0x2 << TIM_CR2_MMS_Pos);
    TIM6->CR1 |= TIM_CR1_CEN; //Enable TIM6

    // -- Enable TIM6 -- //
    TIM6->DIER |= TIM_DIER_UIE; //Enable update interrupt
    TIM6->SR &= ~TIM_SR_UIF; // Clear pending interrupts

}

uint16_t read_adc_pot(void){ //Function to read ADC value from potentiometer

    //uint32_t total = 0;
    ADC3->SR &= ~ADC_SR_EOC;
      // Manually trigger a fresh conversion
    ADC3->CR2 |= ADC_CR2_SWSTART; 

    uint32_t timeout = 50000;

    // Wait precisely until the End of Conversion (EOC) flag sets high
    while(!(ADC3->SR & ADC_SR_EOC)){
        timeout--;
        if(timeout == 0) {
            return 0;
        }
    }

    // If it successfully converted, return it. Otherwise, fall back safely.
    return (uint16_t)ADC3->DR; 
}

uint16_t adc_to_freq(uint16_t adc_value){ //Function to convert ADC value to frequency
    return 100 + ((uint32_t)adc_value) * 1900 / 4095; //Map ADC value to frequency range of 100Hz to 1000Hz
}
void TIM5_IRQHandler(void){ //Interrupt handler for TIM5
    if (TIM5->SR & TIM_SR_UIF){
        TIM5->SR &= ~TIM_SR_UIF; // Clear update interrupt 

        refresh(num); // Refresh the numbers on the SSD

        static uint8_t time_div = 0;
        time_div++;
    }
}
void EXTI9_5_IRQHandler(void){ //Interrupt handler for EXTI9_5
    static uint32_t last_interrupt_time = 0;
    uint32_t current_time = ticks;

    uint32_t pending = EXTI->PR;
    
    if(current_time - last_interrupt_time < 150){ //Debounce time of 150ms
        EXTI->PR = pending & ((1 << UP_PIN)); //clear pending flags
        return; // Ignore this interrupt if it occurred within the debounce time
    }
    if(EXTI->PR & (1 << UP_PIN)){ //If "UP" button is pressed
        EXTI->PR = (1 << UP_PIN); //Clear any pending flags
        last_interrupt_time = current_time;
        state = 2; //Go straight to "Happy Birthday" state
        note_index = 0;
        note_start_time = current_time;
        num = birthday_tune[0]; // Start at first note of birthday song
        update_frequency(num);
        AUDIO_SD->BSRR = (1 << Audio_SD_PIN); //Automatically turn on Audio SD, whether S1 is on or off
    }
} 

void IO_init(void){ //Function to initialize GPIO for SSD, Buttons, and DAC
    // -- Configure SSD Reigisters as Outputs -- //
    GPIOE->MODER &= ~((0x3 << (Digit1_PIN * 2)) | (0x3 << (Digit2_PIN * 2))); // Leftmost
    GPIOE->MODER |= ((0x1 << (Digit1_PIN * 2)) | (0x1 << (Digit2_PIN * 2)));
    GPIOB->MODER &= ~((0x3 << (Digit3_PIN * 2)) | (0x3 << (Digit4_PIN * 2)));
    GPIOB->MODER |= ((0x1 << (Digit3_PIN * 2))| (0x1 << (Digit4_PIN * 2))); // Rightmost

    // -- Configure SSD Segment Registers as Outputs -- //
    GPIOB->MODER &= ~((0x3 << (G * 2)));
    GPIOB->MODER |= ((0x1 << (G * 2)));

    GPIOE->MODER  &= ~((0x3 << (E * 2)));
    GPIOE->MODER |= ((0x1 << (E* 2)));

    GPIOF->MODER &= ~((0x3 << (B * 2)) | (0x3 << (C * 2)) | (0x3 << (F * 2)) | (0x3 << (DP_PIN * 2)));
    GPIOF->MODER |= ((0x1 << (B * 2))| (0x1 << (C * 2))| (0x1 << (F * 2))| (0x1 << (DP_PIN * 2)));

    GPIOG->MODER &= ~((0x3 << (A * 2)) | (0x3 << (D * 2)));
    GPIOG->MODER |= ((0x1 << (A * 2))| (0x1 << (D * 2)));

    // -- Configure Switch as Input-- //
    S1->MODER &= ~((0x3 <<(S1_PIN * 2))); //Set switch to input mode
    S1->PUPDR &= ~((0x3 << (S1_PIN *2)));//Clear mode bits
    S1->PUPDR |= ((0x1 << (S1_PIN *2)));

    //-- Configure Buttons as Inputs --//
    GPIOF->MODER &= ~((0x3 << UP_PIN*2)); //Clear Mode Bits
    GPIOF->PUPDR &= ~((0x3 << (UP_PIN*2))); //Clear Mode Bits
    GPIOF->PUPDR |= ((0x1 << UP_PIN*2));

    // -- Configure EXTI Line for Button -- //
    SYSCFG->EXTICR[1] &= ~(0xFUL << 12); //Clear EXTI7
    SYSCFG->EXTICR[1] |= (0x5UL << 12); //Set EXTI7 to PortF.

    // -- Enable EXTI Line -- //
    EXTI->IMR |= (1UL << UP_PIN); //Unmask interrupt
    EXTI->FTSR |= (1UL << UP_PIN); // Falling Edge Trigger
    EXTI->RTSR &= ~(1UL << UP_PIN); // Turn off Rising Edge Trigger

    NVIC_SetPriority(EXTI9_5_IRQn,2);
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // -- Enable Audio SD  -- //
    AUDIO_SD->MODER &= ~(3U << (Audio_SD_PIN *2)); //Clear Mode Bits
    AUDIO_SD->MODER |= (1U << (Audio_SD_PIN *2)); //Set as output
    AUDIO_SD->BSRR =(1U << Audio_SD_PIN);
}

void refresh(uint16_t num){  //Function to refresh SSD
    static uint8_t current_digit = 0;

    for(int i = 0;i < 4 ; i++){
        DIGIT_PORTS[i]->BSRR = (DIGIT_PINS[i]); // Turn off all digits to prevent ghosting
    }

    uint8_t digit_val = 0;
    bool blank = false;

    uint8_t d1 = (num / 1000) % 10; // Thousands place
    uint8_t d2 = (num / 100) % 10; // Hundreds place
    uint8_t d3 = (num / 10) % 10; // Tens place
    uint8_t d4 = num % 10; // Ones

    // -- Blanking appropriate digits depending on the number -- //
    if(current_digit == 0){
        digit_val = d1;
        if(d1 == 0){
            blank = true;
        }
    }else if(current_digit == 1){
        digit_val = d2;
        if(d1 == 0 && d2 == 0){
            blank = true;
        }
    }else if(current_digit == 2){
        digit_val = d3;
        if(d1 == 0 && d2 == 0 && d3 == 0){
            blank = true;
        }
    }else if(current_digit == 3){
        digit_val = d4;
    }
   
    unsigned char pattern = (!blank) ? SSDdigits[digit_val] : SSDdigits[10];

    for(int i = 0; i < 7 ; i++){
        //0 -> ON, 1-> OFF
        if((pattern >> (6-i)) & 1){ //Bit Masking
            SSD_PORTS[i]->BSRR = SSD_PINS[i]; //Turn OFF LED segment goign HIGH
        }else{
            SSD_PORTS[i]->BSRR = (uint32_t)SSD_PINS[i] << 16; //Turn ON LED segment going LOW
        }
    }

    DP_PORT->BSRR = (1 << DP_PIN); // Turn off decimal point to prevent ghosting

    //Turn ON active digit common pin
    if(!blank || current_digit == 3){ // If not blank or not the first digit
        DIGIT_PORTS[current_digit]->BSRR = (DIGIT_PINS[current_digit] << 16); // Turn on active digit
    }

    //Next display digit
    current_digit = (current_digit + 1) % 4;
}

void update_frequency(uint16_t new_frequency){
    if(new_frequency == 0){
        return;
    }
    uint32_t goal_ticks = (SystemCoreClock / (new_frequency * samples) - 1); //Calculate ticks
    TIM6-> ARR = goal_ticks;
    TIM6->EGR |= TIM_EGR_UG; // Generate update event
}

void State_Machine(void){
    bool s1_on = (S1 ->IDR & (1<< S1_PIN)) != 0;
    static uint16_t last_state = 99; //Just an initial start to prevent freezing

    if (!(s1_on)){ //If the switch is not on and if the state is not "Happy birthday", state = 0;
        if(state != 2){
            state = 0;
        }
        
    }else{
        if(state == 0){//If switch is on but currently in state 0, move to state 1. Audio turns on.
            state = 1;
        }
    }

    if(state != last_state){
        if(state == 0){
            AUDIO_SD->BSRR = (1 << (Audio_SD_PIN + 16)); //Set Pin Low
            DAC->CR &= ~DAC_CR_EN1;
        }else{
            AUDIO_SD->BSRR = (1 << (Audio_SD_PIN)); //Set Pin High
            DAC->CR |= DAC_CR_EN1;
            DMA1_Stream5->CR |= DMA_SxCR_EN;
        }
        last_state = state;
    }

    if(state == 0){ //sound off
        uint16_t current_freq = adc_to_freq(read_adc_pot());
        num = current_freq;
        TIM6->ARR = 0xFFFF;
        
    }else if(state == 1){ // sound on
        uint16_t current_freq = adc_to_freq(read_adc_pot());
        num = current_freq;

        static uint16_t last_freq = 0;
        if(abs((int)current_freq - (int)last_freq) > 15){ // If frequency has changed significantly
            update_frequency(current_freq);
            last_freq = current_freq;
        }

    }else if(state == 2){ //Happy Birthday
        uint32_t elapsed_time = ticks - note_start_time;
        uint16_t current_note_len = birthday_tune[note_index];
        /*uint16_t gap = 0; //40 ms of silence to separate notes

        if(elapsed_time >= (current_note_len  - gap)){
            AUDIO_SD->BSRR = (1 << (Audio_SD_PIN+16));
        }else{
            AUDIO_SD->BSRR = (1 << Audio_SD_PIN);;
        }*/

        if(elapsed_time >= (current_note_len)){ // If note duration has elapsed
            note_index ++; // Move to next note

            if(note_index >= total_notes){ // If last note has finished
                note_index = 0;
                if(s1_on){
                    state = 1;
                    uint16_t current_freq = adc_to_freq(read_adc_pot());
                    num = current_freq;
                    update_frequency(num);

                }else{
                    state = 0;
                    DMA1_Stream5->CR &= ~DMA_SxCR_EN; // Turn off ADC
                }
            }else{
                note_start_time = ticks; // Reset note start time
                num = birthday_tune[note_index];
                update_frequency(num); // Update TIM6
                AUDIO_SD->BSRR = (1 << Audio_SD_PIN);

            }
        }
    }
}


