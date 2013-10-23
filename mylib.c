/* Currell Berry.  CS 2110.  GBA utility functions etc...*/
#include<stdio.h>
#include "mylib.h"

#define REG_VCOUNT *(volatile u16*)0x04000006
u16 * videoBuffer = (u16*) 0x6000000;



void vid_vsync()
{
    while(REG_VCOUNT >= 160);   // wait till VDraw
    while(REG_VCOUNT < 160);    // wait till VBlank
}

/* below functions assume that
   REG_DISPCNT = 0x403; //bg2, mode 3 (bitmap) */

void setPixel(int r, int c, COLOR color) {
	videoBuffer[r*240+c] = color;
}

//r and c represent the top and left corners of the rectangle, respectively
void drawRect(int r, int c, int width, int height, COLOR color) {
	for (int i=r; i < r+height; i++) {
		for (int i2=c; i2 < c+width; i2++) {
			setPixel(i,i2,color);		
		}
	}
}
void drawHollowRect(int r, int c, int width, int height, COLOR color) {
	//draw clockwise starting from top left corner
	int rightcol = c+width;
	int bottomrow = r+height;
	//top
	for (int tc = c; tc < rightcol; tc++) {
		setPixel(r,tc,color);
	}
	//right
	for (int tr = r; tr < bottomrow; tr++) {
		setPixel(tr,rightcol,color);
	}
	//bottom
	for (int tc = c; tc < rightcol; tc++) {
		setPixel(bottomrow,tc,color);
	}
	//left
	for (int tr = r; tr < bottomrow; tr++) {
		setPixel(tr,rightcol,color);
	}
}
void clearScreen() {
	drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, RGB(0,0,0));
}
