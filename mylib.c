/* Currell Berry.  CS 2110.  GBA utility functions etc...*/
#include<stdio.h>
#include<string.h>
#include "mylib.h"


#define REG_VCOUNT *(volatile u16*)0x04000006
u16 * videoBuffer = (u16*) 0x6000000;
u16 __key_prev;
u16 __key_curr;


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
void drawRect(int r, int c, int width, int height, COLOR color) 
{

	DMA[3].cnt = 0;

	while (height--) {
		DMA[3].src = &color;
		DMA[3].dst = videoBuffer + (r+height)*240 + c;
		DMA[3].cnt = (width) | DMA_ENABLE | DMA_SOURCE_FIXED;

	}

	//	for (int i=r; i < r+height; i++) {
	//		for (int i2=c; i2 < c+width; i2++) {
	//			setPixel(i,i2,color);
	//		}
	//	}
}

void drawHollowRect(int r, int c, int width, int height, COLOR color) 
{
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

void clearScreen() 
{
	u16 black = BLACK;
	DMA[3].src = &black;
	DMA[3].dst = videoBuffer;
	DMA[3].cnt = (240*160) | DMA_ENABLE | DMA_SOURCE_FIXED;
}

/* draw_image_3
 * * A function that will draw an arbitrary sized image
 * * onto the screen (with DMA).
 * * @param r row to draw the image
 * * @param c column to draw the image
 * * @param width width of the image
 * * @param height height of the image
 * * @param image Pointer to the first element of the image.
 * */
void draw_image_3(int r, int c, int width, int height, const u16* image)
{
	while (height--) {
		DMA[3].src = image+width*height;
		DMA[3].dst = videoBuffer + 240*(r+height) + c;
		DMA[3].cnt = (width) | DMA_ENABLE;
	}
}

/***************************** DEBUGGING *****************************/
void pixelDebug(int row, int num) 
{
	int output_col = 0;
	int bits[32];
	int mask =0x1;
	for (int i = 0; i < 32; i++) {
		bits[31 - i] = (mask<<i == (num & (mask<<i))); //((num) & (mask<<i));
	}

	for (int i = 0; i < 32; i++) {
		if (bits[i] == 1) {
			setPixel(row, output_col+2*i,WHITE);
		} else {
			setPixel(row, output_col+2*i,RGB(5,5,5));
		}
	}
}
/*********************************************************************/
