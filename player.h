#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "Images.h"
#include "ST7735.h"
#define FIX 16

const int8_t yTable[] = {-16,-11,0,11,16,11,0,-11};
const int8_t xTable[] = {0,11,16,11,0,-11,-16,-11};
const int8_t y2[] = {-24,-17,0,17,24,17,0,-17};
const int8_t x2[] = {0,17,24,17,0,-17,-24,-17};
bool shoot_flag = false;


class Bullet{
	public:
		int16_t x;
		int16_t y;
		int8_t vx;
		int8_t vy;
		uint8_t index;
		const unsigned short *image;
		bool exists;

	Bullet() : exists(false){};
	void setBullet(int16_t xn, int16_t yn, int8_t indexn, uint8_t color){
		x = xn;
		y = yn;
		index = indexn;
		vy = y2[index];
		vx = x2[index];
		exists = true;
		image = (color == 1) ? rbull : bbull;
	}
	void killBullet(){
		exists = false;
		ST7735_DrawBitmap(x/FIX,y/FIX,bullets,9,9);
	}
	void moveBullet(){
		if(x < -7*16 || x > 127*16 || y < -7*16 || y > 167*16)
 			exists = 0;
		x += vx;
		y += vy;
	}
};

class player{
	public:
		int32_t x;
		int32_t y;
		const unsigned short *image; // ptr->image
		int8_t lives;            //100 xp
		uint32_t index;
		int32_t vx;
		int32_t vy;
		int32_t oldx;
		int32_t oldy;
		const unsigned short **ships;
		Bullet bullets[3];
		uint8_t bCount;
		uint8_t color; 			// 1 for red, 0 for blue
		uint8_t vulnerability;

	player(int32_t x, int32_t y, const unsigned short **images, uint8_t color) : x(x), y(y), color(color){
		ships = images;
		index = 0;
		oldx = x;
		oldy = y;
		lives = 100;
		vy = yTable[0];
		vx = xTable[0];
		image = ships[0];
		bCount = 0;
		vulnerability = 10;
	}
	
	void Rotate(){
		index = (index + 1) % 8;
		image = ships[index];
		vx = xTable[index];
		vy = yTable[index];
	}

	void Move(uint16_t otherX, uint16_t otherY){
		if (((x+8*16)-(otherX+8*16)<18*FIX && ((x+8*16)-(otherX+8*16)>-18*FIX)) && ((y-8*16)-(otherY-8*16)<18*FIX && (y-8*16)-(otherY-8*16)>-18*FIX)){
			index = (index + 4) % 8;
			image = ships[index];
			vx = xTable[index];
			vy = yTable[index];
		}
		x += vx;
		if(x < 0 || x > 110*FIX)
			x = oldx;
		else
			oldx = x;
		y += vy;
		if(y < 23*FIX || y > 160*FIX)
			y = oldy;
		else 
			oldy = y;
	}
	void Shoot(){
		if(!bullets[bCount].exists){
			//bullets[bCount].setBullet(x+6*16, y-6*16, index, color);         //  the sahlil way
			shoot_flag = true;
			if (index == 0)
				bullets[bCount].setBullet((x+8*16)-4*16, y-16*16, index, color);
			else if (index == 1)
				bullets[bCount].setBullet(x+16*16, y-16*16, index, color);
			else if (index == 2)
				bullets[bCount].setBullet(x+16*16, (y-8*16)+4*16, index, color);
			else if (index == 3)
				bullets[bCount].setBullet(x+16*16, y+9*16, index, color);
			else if (index == 4)
				bullets[bCount].setBullet((x+8*16)-4*16, y+9*16, index, color);
			else if (index == 5)
				bullets[bCount].setBullet(x-7*16, y+7*16, index, color);
			else if (index == 6)
				bullets[bCount].setBullet(x-7*16, (y-9*16)+4*16, index, color);
			else if (index == 7)	
				bullets[bCount].setBullet(x-9*16, y-17*16, index, color);
			bCount = (bCount+1) %3;
		}	
	}
	
};

