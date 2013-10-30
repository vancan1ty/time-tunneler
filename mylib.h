/****** Currell Berry CS 2110 HW8 header file ******/

#ifndef MYLIBH
#define MYLIBH

#define INLINE static inline

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/****************GLOBAL DECLARATIONS*************/
extern u16 * videoBuffer; 
/************************************************/

/****************** BUTTON HANDLING *******************/

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
#define RGB(r,b,g)      ( ((g)<<10)|((b)<<5)|((r)<<0) )
#define RED             RGB(31,0,0)
#define GREEN           RGB(0,31,0)
#define BLUE            RGB(0,0,31)
#define WHITE           RGB(31,31,31)
#define BLACK           RGB(0,0,0)
#define YELLOW	        RGB(31,31,0)

/********************************************************/

/*****************IO CONSTANTS**********************/
#define SCREEN_HEIGHT	160 
#define SCREEN_WIDTH	240
/***************************************************/

/****************** BUTTON HANDLING *********************/

extern u16 __key_prev;
extern u16 __key_curr;

#define REG_KEYINPUT	  *((u16*) 0x04000130)
#define KEY_A		  0x0001
#define KEY_B		  0x0002
#define KEY_SELECT	  0x0004
#define KEY_START	  0x0008
#define KEY_RIGHT	  0x0010
#define KEY_LEFT	  0x0020
#define KEY_UP		  0x0040
#define KEY_DOWN	  0x0080
#define KEY_R		  0x0100
#define KEY_L             0x0200
#define KEY_MASK          0x03FF
#define KEY_DOWN_NOW(key)  ((key) == (~(REG_KEYINPUT) & (key)))


//Polling function
INLINE void key_poll()
{
	__key_prev= __key_curr;
	__key_curr= ~REG_KEYINPUT & KEY_MASK; //inverting REG_KEYINPUT 
					      //to deal with active low
}

// Basic state checks
INLINE u32 key_curr_state()         {   return __key_curr;          }
INLINE u32 key_prev_state()         {   return __key_prev;          }
INLINE u32 key_is_down(u32 key)     {   return  __key_curr & key;   }
INLINE u32 key_is_up(u32 key)       {   return ~__key_curr & key;   }
INLINE u32 key_was_down(u32 key)    {   return  __key_prev & key;   }
INLINE u32 key_was_up(u32 key)      {   return ~__key_prev & key;   }

//Transitional state checks
// Key is changing state.
INLINE u32 key_transit(u32 key)
{   return ( __key_curr ^  __key_prev) & key;   }

// Key is held (down now and before).
INLINE u32 key_held(u32 key)
{   return ( __key_curr &  __key_prev) & key;  }

// Key is being hit (down now, but not before).
INLINE u32 key_hit(u32 key)
{   return ( __key_curr &~ __key_prev) & key;  }

// Key is being released (up now but down before)
INLINE u32 key_released(u32 key)
{   return (~__key_curr &  __key_prev) & key;  }


/***************************************************/

/*****************DMA*******************/
#define REG_DMA3SAD *(const volatile u32*) 0x40000D4 //source address
#define REG_DMA3DAD *(volatile u32*) 0x40000D8; // destination address
#define REG_DMA3CNT *(volatile u32*) 0x40000DC; // control register

#define DMA_DESTINATION_INCREMENT (0 << 0x15) //increment after each transfer
#define DMA_DESTINATION_DECREMENT (1 << 0x15) //decrement after each transfer
#define DMA_DESTINATION_FIXED     (2 << 0x15) //destination address is fixed
#define DMA_DESTINATION_RESET     (3 << 0x15) //increment during transfer, then reset?

//below same as above, but for source
#define DMA_SOURCE_INCREMNT       (0 << 0x17) 
#define DMA_SOURCE_DECREMENT      (1 << 0x17) 
#define DMA_DEST_FIXED            (2 << 0x17) 
//^no reset for source...

#define DMA_REPEAT                (1 << 0x19)

#define DMA_16                    (0 << 0x1A)
#define DMA_32                    (1 << 0x1A)

#define DMA_NOW                   (0 << 0x1C)
#define DMA_AT_VBLANK             (1 << 0x1C)
#define DMA_AT_HBLANK             (2 << 0x1C)
#define DMA_AT_REFRESH            (3 << 0x1C)

#define DMA_IRQ                   (1 << 0x1E)
#define DMA_ENABLE                (1 << 0x1F)

#define DMA_BASEADDR              0x40000B0;

//struct DMA_CONTROLLER 
//{
//	const volatile u32 source_address;
//	volatile u32 destination_address;
//	volatile u32 control_register;
//}

typedef struct
{
	const volatile void *src;
	const volatile void *dst;
	u32                  cnt;

} DMA_CONTROLLER;

#define DMA ((volatile DMA_CONTROLLER *) 0x040000B0)

/************************************************************/

typedef char bool;

/** these functions assume  you're in mode 3, background 2 */

void vid_vsync();
void setPixel(int r, int c, COLOR color);
void drawRect(int r, int c, int width, int height, COLOR color);
void drawHollowRect(int r, int c, int width, int height, COLOR color);
void clearScreen(); 
void pixelDebug(int row, int num);

#endif
