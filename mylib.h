/****** Currell Berry CS 2110 HW8 header file ******/

#ifndef MYLIBH
#define MYLIBH

typedef unsigned short u16;

/****************GLOBAL DECLARATIONS*************/
extern u16 * videoBuffer; 
extern u16 __key_prev;
extern u16 __key_curr;
/************************************************/

/****************MEMORY MAPPINGS...**************/
#define MEM_IO      	0x04000000
#define MEM_VRAM    	0x06000000
#define REG_DISPCNT     *((u16*)(MEM_IO))
/************************************************/


/*****************COLOR HANDLING CODE********************/
typedef u16 COLOR;
//below macro takes r,g,and b values for a COLOR, each ranging
//from 0 to 31, and converts them to bgr format for use with
//gba code...
#define RGB(r,b,g) ( ((g)<<10)|((b)<<5)|((r)<<0) )
/********************************************************/

/*****************IO CONSTANTS**********************/
#define SCREEN_HEIGHT	160 
#define SCREEN_WIDTH	240

#define REG_KEYINPUT	*((u16*) 0x04000130)
#define KEY_A		0x0001
#define KEY_B		0x0002
#define KEY_SELECT	0x0004
#define KEY_START	0x0008
#define KEY_RIGHT	0x0010
#define KEY_LEFT	0x0020
#define KEY_UP		0x0040
#define KEY_DOWN	0x0080
#define KEY_R		0x0100
#define KEY_L		0x0200
#define KEY_MASK	0x03FF
#define IS_KEY_DOWN(key)  ((key) == (~(REG_KEYINPUT) & (key)))
/***************************************************/

typedef int bool;

/** these functions assume  you're in mode 3, background 2 */

void vid_vsync();
void setPixel(int r, int c, COLOR color);
void drawRect(int r, int c, int width, int height, COLOR color);
void drawHollowRect(int r, int c, int width, int height, COLOR color);
void clearScreen(); 

#endif
