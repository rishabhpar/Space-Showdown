// SlidePot.cpp
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 3/28/2018 
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "SlidePot.h"
#include "inc/tm4c123gh6pm.h"


//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
void ADC_Init(void){ 
//*** students write this ******
    volatile unsigned long delay;
    SYSCTL_RCGCGPIO_R |= 0x08;      // 1) activate clock for Port D
    while((SYSCTL_PRGPIO_R&0x08) == 0){};
    GPIO_PORTD_DIR_R &= ~0x04;      // 2) make PD2 input
    GPIO_PORTD_AFSEL_R |= 0x04;     // 3) enable alternate fun on PD2
    GPIO_PORTD_DEN_R &= ~0x04;      // 4) disable digital I/O on PD2
    GPIO_PORTD_AMSEL_R |= 0x04;     // 5) enable analog fun on PD2
    SYSCTL_RCGCADC_R |= 0x01;       // 6) activate ADC0
    delay = SYSCTL_RCGCADC_R;       // extra time to stabilize
    delay = SYSCTL_RCGCADC_R;
    delay = SYSCTL_RCGCADC_R;
    delay = SYSCTL_RCGCADC_R;
    ADC0_PC_R = 0x01;               // 7) configure for 125K
    ADC0_SSPRI_R = 0x0123;          // 8) Seq 3 is highest priority
    ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
    ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
    ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;  // 11) Ain5 (PD2)
    ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
    ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
    ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3
		ADC0_SAC_R = 0x06;
}

//------------ADCIn------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
//*** students write this ******
	uint32_t data;
  ADC0_PSSI_R = 0x0008;    				// start ADC        
  while((ADC0_RIS_R&0x08)==0){};  // busy wait 
  data = ADC0_SSFIFO3_R&0xFFF; 		// read data
  ADC0_ISC_R = 0x0008; 						// clear flag
  return data;
}

// constructor, invoked on creation of class
// m and b are linear calibration coeffients 
SlidePot::SlidePot(uint32_t m, uint32_t b){
//*** students write this ******
// initialize all private variables
// make slope equal to m and offset equal to b
	data = distance = flag = 0;
	slope = m;
	offset = b;
}

void SlidePot::Save(uint32_t n){
//*** students write this ****
// 1) save ADC sample into private variable
// 2) calculate distance from ADC, save into private variable
// 3) set semaphore flag = 1
	data = n;
	distance = Convert(data);
	flag = 1;	
}
uint32_t SlidePot::Convert(uint32_t n){
  //*** students write this ******
  // use calibration data to convert ADC sample to distance
	return ((n*slope)/4096)+offset;
}

void SlidePot::Sync(void){
// 1) wait for semaphore flag to be nonzero
// 2) set semaphore flag to 0
		while(flag == 0){};
		flag = 0;
}

uint32_t SlidePot::ADCsample(void){ // return ADC sample value (0 to 4095)
  //*** students write this ******
  // return last calculated ADC sample
  return data; // replace this with solution
}

uint32_t SlidePot::Distance(void){  // return distance value (0 to 2000), 0.001cm
  //*** students write this ******
  // return last calculated distance in 0.001cm
  return distance; // replace this with solution
}


