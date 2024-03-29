// put implementations for functions, explain how it works
// put your names here, date
#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

#include "DAC.h"

// **************DAC_Init*********************
// Initialize 6-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	 volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x00000002;
	delay = 100;
	GPIO_PORTB_DIR_R |= 0x3F;
	GPIO_PORTB_DEN_R |= 0x3F;
	
	SYSCTL_RCGCGPIO_R |= 0x00000001;
	delay = 100;
	GPIO_PORTA_DIR_R |= 0x04;
	GPIO_PORTA_DEN_R |= 0x04;

}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data;	
}
