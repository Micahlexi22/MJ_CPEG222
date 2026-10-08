//**********************************************
// * Author: M. Johnson
// * CPEG222 Project 4, 10/2/26
// * NucleoF466ZE CMSIS 
//**********************************************//

// ** Buttons ** //
#define UP_PIN 7
#define UP GPIOF

// ** Switches ** //
#define S1_PIN 8
#define S1 GPIOC

// ** SSD **//
#define Digit1_PIN 10
#define Digit1 GPIOE
#define Digit2_PIN 7
#define Digit2 GPIOE
#define Digit3_PIN 5
#define Digit3 GPIOB
#define Digit4_PIN 3
#define Digit4 GPIOB
#define DP_PIN 14
#define DP_PORT GPIOF
#define A 9
#define SEG_A GPIOG
#define B 12
#define SEG_B GPIOF
#define C 13
#define SEG_C GPIOF
#define D 14
#define SEG_D GPIOG
#define E 8
#define SEG_E GPIOE
#define F 15
#define SEG_F GPIOF
#define G 4
#define SEG_G GPIOB

// ** Other **//
//#define DAC1 4
#define DAC1_PORT GPIOA
#define V_POT_PIN 2
#define V_POT GPIOC
#define ADC_CHANNEL 12 //Channel for V_POT
#define ADC_SAMPLES 16 // Num samples for averaging
#define Audio_SD_PIN 6
#define AUDIO_SD GPIOB