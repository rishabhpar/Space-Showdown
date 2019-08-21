// main.cpp
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10 in C++

// Last Modified: 4/19/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "SlidePot.h"
#include "Images.h"
#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "player.h"
#include <stdlib.h>
#include "Sound.h"
#include "DAC.h"

SlidePot my(1896,48);

extern "C" void DisableInterrupts(void);
extern "C" void EnableInterrupts(void);
extern "C" void SysTick_Handler(void);
extern "C" void TIMER3A_Handler(void);

player player1(0*FIX,159*FIX,rships, 1);
player player2(110*FIX, 159*FIX, bships, 0);

volatile uint32_t flag;							// semaphore to sync moveing ships/bullets and drawing them
uint32_t shoot_index = 0;						// index to traverse shoot sound
bool explosion_music = false;				// flag to start explosion sound when a player dies (0 xp)
uint32_t eindex = 0;								// index to traverse explosion sound

void background(void){
  flag = 1; // semaphore
	player1.Move(player2.x, player2.y);
	player2.Move(player1.x, player1.y);
	for (int i=0; i<3; i++){								// move bullets
		if(player1.bullets[i].exists){				// if the bullets exist
			player1.bullets[i].moveBullet();
			if(abs((player1.bullets[i].x + 4*16) - (player2.x + 8*16)) < 8*FIX && abs((player1.bullets[i].y - 4*16) - (player2.y - 8*16)) < 8*FIX){			// bullet collision with other ship
					player2.lives-=player2.vulnerability;				// decrease xp
					player1.bullets[i].killBullet();						// clear bullet
			}
		}
  }
	for (int i=0; i<3; i++){								// move bullets
		if(player2.bullets[i].exists){				// if the bullets exist
			player2.bullets[i].moveBullet();		
			if(abs((player2.bullets[i].x + 4*16) - (player1.x + 8*16)) < 8*FIX && abs((player2.bullets[i].y - 4*16) - (player1.y - 8*16)) < 8*FIX){			// bullet collision with other ship
					player1.lives-=player1.vulnerability;				// decrease xp
					player2.bullets[i].killBullet();						// clear bullet
			}
		}
  }
}

void clock(void){
	if((GPIO_PORTE_DATA_R&0x2) == 0x2)
		player1.Rotate();
	if((GPIO_PORTE_DATA_R&0x8) == 0x8)
		player2.Rotate();
	if((GPIO_PORTE_DATA_R&0x1) == 0x1)
		player1.Shoot();
  if((GPIO_PORTE_DATA_R&0x4) == 0x4)
		player2.Shoot();
}

void SysTick_Init(unsigned long period){
  //*** students write this ******
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2          
  NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
  // enable interrupts after all initialization is finished
}

void Port_Init(){
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x10;	 // Turn clock on PortE
  delay = 100;				      	 // Wait
  GPIO_PORTE_DIR_R &= ~(0x0F); // PE0-3 input
  GPIO_PORTE_DEN_R |= 0x0F;    // digital enable on PE0-3
}

void Draw(){
	if(flag == 1){
		ST7735_DrawBitmap((player1.x/FIX) ,(player1.y/FIX) ,player1.image,17,17);			// draw red ship
		ST7735_DrawBitmap((player2.x/FIX) ,(player2.y/FIX) ,player2.image,17,17);			// draw blue ship
		
		ST7735_SetCursor(0, 0);												// draw red xp
		ST7735_SetTextColor(ST7735_WHITE);
		ST7735_OutString((char*)"Red:");
		ST7735_OutUDec(player1.lives);
		if(player1.lives < 100){											// cover leading 0 with a space
			ST7735_OutString((char*)" ");
		}
		ST7735_SetCursor(11, 0);											// draw blue xp
		ST7735_SetTextColor(ST7735_WHITE);
		ST7735_OutString((char*)"Blue:");
		ST7735_OutUDec(player2.lives);
		if(player2.lives < 100){
			ST7735_OutString((char*)" ");								// cover leading 0 with a space
		}

		for (int i=0; i<3; i++)												// draw red's bullets
			if(player1.bullets[i].exists)
        ST7735_DrawBitmap(player1.bullets[i].x/FIX,player1.bullets[i].y/FIX,player1.bullets[i].image,9,9);
		for (int i=0; i<3; i++)												// draw blue's bullets
			if(player2.bullets[i].exists)
        ST7735_DrawBitmap(player2.bullets[i].x/FIX,player2.bullets[i].y/FIX,player2.bullets[i].image,9,9);
	}
}

void endGame(){
	while(explosion_music){}
	DisableInterrupts();
	ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(6, 1);
  ST7735_OutString((char*)"GAME OVER");
  ST7735_SetCursor(4, 5);
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_OutString((char*)"The winner is:");
	if (player1.lives > 0){
		ST7735_SetCursor(4, 7);
		ST7735_OutString((char*)"<-- Player 1!");
		while(1){ 																							// draw rotating red ship if red wins
			ST7735_DrawBitmap(55,135,player1.image,17,17);
			for(int i = 0; i < 3000000; i++){}
			player1.index = (player1.index + 1) % 8;
			player1.image = player1.ships[player1.index];
		}
	} else {
		ST7735_SetCursor(4, 7);
		ST7735_OutString((char*)"Player 2! -->");
		while(1){ 																							// draw rotating blue ship if blue wins
			ST7735_DrawBitmap(55,135,player2.image,17,17);	
			for(int i = 0; i < 3000000; i++){}
			player2.index = (player2.index + 1) % 8;
			player2.image = player2.ships[player2.index];
		}
	}
}

void startScreen(){
	bool flag1 = false;
	bool flag2 = false;
	while(!flag1 || !flag2){
		ST7735_DrawBitmap(0,159,start1,128,160);
		for(int i = 0; i < 3000000; i++){}
			if((GPIO_PORTE_DATA_R&0x1) == 0x1)
				flag1 = true;
			if ((GPIO_PORTE_DATA_R&0x4) == 0x4)
				flag2 = true;
		ST7735_DrawBitmap(0,159,start2,128,160);
		for(int i = 0; i < 3000000; i++){}			
	}
}
	

void Timer3_Init(unsigned long period){
  SYSCTL_RCGCTIMER_R |= 0x08;   // 0) activate TIMER3
  TIMER3_CTL_R = 0x00000000;    // 1) disable TIMER3A during setup
  TIMER3_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER3_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER3_TAILR_R = period-1;    // 4) reload value
  TIMER3_TAPR_R = 0;            // 5) bus clock resolution
  TIMER3_ICR_R = 0x00000001;    // 6) clear TIMER3A timeout flag
  TIMER3_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 51, interrupt number 35
  NVIC_EN1_R = 1<<(35-32);      // 9) enable IRQ 35 in NVIC
  TIMER3_CTL_R = 0x00000001;    // 10) enable TIMER3A
}

void TIMER3A_Handler(void){
  TIMER3_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER3A timeout
  background();                // execute user task
}
void sound(){
	if(shoot_flag){
		DAC_Out(shoot[shoot_index]/4);
		shoot_index = (shoot_index + 1);
	}
	if(shoot_index > 1802){
		shoot_flag = false;					// end sound
		shoot_index = 0;						// wrap
	}
	if(explosion_music){
		DAC_Out(explosion[eindex]/4);
		eindex++;
	}
	if(eindex > 9000){
		explosion_music = false;			// end sound
	}
}


int main(void){
	DisableInterrupts();
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
  Output_Init();
	Port_Init();
	DAC_Init();
	ADC_Init();
  Timer3_Init(1600000); // 50 Hz
  Timer1_Init(&clock,16000000); // 5 Hz
	Timer0_Init(&sound,7255);
	SysTick_Init(666800);
	startScreen();
  EnableInterrupts();
	
	
	ST7735_FillScreen(0x0000);            // set screen to black
  while(player1.lives > 0 && player2.lives > 0)
		Draw();
	explosion_music = true;
	endGame();
}

void SysTick_Handler(void){ // every 16.67 ms
   	my.Save(ADC_In());																// slide pot changes damage value
		if (my.Distance() < 667)
				player1.vulnerability = player2.vulnerability = 10;
		else if (my.Distance() >= 667 && my.Distance() < 1334)
				player1.vulnerability = player2.vulnerability = 20;
		else if (my.Distance() >= 1334)
				player1.vulnerability = player2.vulnerability = 25;
		
}



