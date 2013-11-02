/////////////////////////////////////////////////////////
/*
 * Currell Berry CS 2110 Hw 9.
 * "Time Tunneler"
 * 11-1-13
 *
 * Assuming you have the 2110 toolchain installed, call "make vba"
 * in the directory this file is contained to install and run.
 *
 * On my computer, the game runs really slowly on wxvba, but normally with vba.
 * Have no idea why... suggest testing with vba.
 */
/////////////////////////////////////////////////////////
#include <debugging.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mylib.h"
#include "splash.h"
#include "splashnotext.h"
#include "font2.h"

//////////////////////////////////FONT STUFF
void draw_character2(int r, int c, char ch) 
{
	draw_image_3(r, c, 6, 8, fontdata2_6x8+(6*8)*ch);
}

void draw_string2(int r, int c, char * ch)
{
	int offset = 0;
	while(*(ch+offset) != '\0') {
		draw_character2(r,c+offset*6, *(ch+offset));
		//		draw_image_3(r,c+6*offset,6,8, fontdata2_6x8+(6*8)*(ch+offset));
		offset++;
	}
}

void wrap_string2(int r, int c, char * ch)
{
	int roffset = 0;
	int x_offset = 0;
	int y_offset = 0;
	while(*(ch+roffset) != '\0') {
		if (*(ch+roffset) == '\n') {
			y_offset++;
			x_offset=0;
			roffset++;
			continue;
		} else if (x_offset >= 39) {
			y_offset++;
			x_offset=0;
		}
		draw_character2(r+y_offset*8,c+x_offset*6, *(ch+roffset));
		//		draw_image_3(r,c+6*x_offset,6,8, fontdata2_6x8+(6*8)*(ch+x_offset));
		x_offset++;
		roffset++;
	}
}

//////////////////////////////////QUICK RANDOM NUMBER GENERATOR
int __qran_seed=42;
//set seed
int sqran(int seed)
{
	int old = __qran_seed;	
	__qran_seed = seed;
	return old;
}
INLINE int rand_range(int min, int max)
{return (rand()*(max-min)>>15)+min; }

//quick random number generator
INLINE int qran()
{
	__qran_seed = 1664525*__qran_seed+1013904223;
	return (__qran_seed>>16) & 0x7FFF;
}
//////////////////////////////////

//////////////////////////////////PREPROCESSOR
#define RANDSEED     	23
#define BLOCK_W	        8
#define BLOCK_H	        8
#define NBLOCKSW        SCREEN_WIDTH/BLOCK_W
#define NBLOCKSH	    18
#define CORNER_S        1
#define CELL_S          6
#define EWRAM           0x02000000
//////////////////////////////////

void set_rect_occupied(int r, int c, int width, int height);
/////////////////////////////////ROOM STRUCT

typedef struct Room 
{
	int row;
	int col;
	bool top_intact; //these bools encode the state of the various walls
	bool right_intact;
	bool bottom_intact;
	bool left_intact;
	bool inMaze;
	COLOR color;

} Room;

Room new_Room(int row, int col) 
{
	Room room = {row,col,1,1,1,1,0,RGB(31,31,31)};
	return room;
}

//just redraws the cell...
void draw_cell(Room room) 
{
	drawRect(room.row*BLOCK_H + CORNER_S, room.col*BLOCK_W + CORNER_S, CELL_S, CELL_S, room.color); 
}

void draw_Room(Room room) 
{
	/*	if (!room.left_intact && !room.top_intact) { //clear left top corner
		drawRect(room.row*BLOCK_H, room.col*BLOCK_W, CORNER_S, CORNER_S, WHITE); 
		}

		if (!room.top_intact && !room.right_intact) { //clear right top corner
		drawRect(room.row*BLOCK_H, room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CORNER_S, WHITE); 
		}

		if (!room.right_intact && !room.bottom_intact) { //clear right bottom corner
		drawRect(room.row*BLOCK_H+CORNER_S+CELL_S, room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CORNER_S, WHITE); 
		}

		if (!room.bottom_intact && !room.left_intact) { //clear left bottom corner
		drawRect(room.row*BLOCK_H+CORNER_S+CELL_S, room.col*BLOCK_W, CORNER_S, CORNER_S, WHITE); 
		} */

	//set wall bitset.
	set_rect_occupied(room.row*BLOCK_H,room.col*BLOCK_W,CORNER_S,CORNER_S); 
	set_rect_occupied(room.row*BLOCK_H + CORNER_S + CELL_S, room.col*BLOCK_W, CORNER_S, CORNER_S);		
	set_rect_occupied(room.row*BLOCK_H,room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CORNER_S);		
	set_rect_occupied(room.row*BLOCK_H+CORNER_S+CELL_S,room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CORNER_S);		

	//draw central cell
	drawRect(room.row*BLOCK_H + CORNER_S, room.col*BLOCK_W + CORNER_S, CELL_S, CELL_S, room.color); 

	//draw walls...
	if (!room.top_intact) {

		drawRect(room.row*BLOCK_H, room.col*BLOCK_W + CORNER_S, CELL_S, CORNER_S, WHITE); 
	} else {
		set_rect_occupied(room.row*BLOCK_H, room.col*BLOCK_W+CORNER_S, CELL_S, CORNER_S);		
	}
	if (!room.right_intact) {

		drawRect(room.row*BLOCK_H+CORNER_S, room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CELL_S, WHITE); 
	} else {
		set_rect_occupied(room.row*BLOCK_H+CORNER_S, room.col*BLOCK_W+CORNER_S+CELL_S, CORNER_S, CELL_S);		
	}

	if (!room.bottom_intact) {

		drawRect(room.row*BLOCK_H+CORNER_S+CELL_S, room.col*BLOCK_W+CORNER_S, CELL_S, CORNER_S, WHITE); 
	} else {

		set_rect_occupied(room.row*BLOCK_H+CORNER_S+CELL_S, room.col*BLOCK_W+CORNER_S, CELL_S, CORNER_S);		
	}
	if (!room.left_intact) {

		drawRect(room.row*BLOCK_H+CORNER_S, room.col*BLOCK_W, CORNER_S, CELL_S, WHITE); 
	} else {

		set_rect_occupied(room.row*BLOCK_H+CORNER_S, room.col*BLOCK_W, CORNER_S, CELL_S);		
	}
}
//////////////////////////////////

//////////////////////////////////PORTAL STRUCT
typedef struct Portal
{
	Room * room1;
	Room * room2;
	COLOR color;
} Portal;


Portal portals[];
int numportals;

// returns * to room to jump to from current
// or else returns NULL 
Room * portal_jump(Room * roomp)  
{
	for (int i = 0; i < numportals; i++) {
		if (roomp == portals[i].room1) {
			return portals[i].room2;	
		} else if (roomp == portals[i].room2) {
			return portals[i].room1;
		}
	}
	//else
	return NULL;
}

//////////////////////////////////

//////////////////////////////////FORWARD FUNCTION DECLARATIONS
Room * calc_maze(int randseed, int * start_row);
int get_next_room (Room ** rooms, 
		int current_room_index, 
		Room * visit_map);

void draw_maze(Room * visit_map, int level);
void draw_player(int rowp, int colp);
bool special_rooms_contains(int roomindex);
//////////////////////////////////

/////////////////////////////////WALL BITSET
int * wallbitset; //= (int *) calloc(240*160/sizeof(int) + 1, sizeof(int));
//bitset keeping track of whether a pixel is in a wall or not.
//a little tricky to access values, etc.  requires a mask.

bool is_occupied(int ypos, int xpos) 
{
	int intindex = (ypos*SCREEN_WIDTH+xpos)/sizeof(int);
	int maskindex = (ypos*SCREEN_WIDTH+xpos)%sizeof(int);
	int mask = (0x1 << (sizeof(int) - 1 - maskindex));
	bool out = (mask == (mask & wallbitset[intindex] ));
	return out;
}

void set_occupied(int ypos, int xpos)
{
	int intindex = (ypos*SCREEN_WIDTH+xpos)/sizeof(int);
	int maskindex = (ypos*SCREEN_WIDTH+xpos)%sizeof(int);
	int mask = (0x1 << (sizeof(int) - 1 - maskindex));
	wallbitset[intindex] = mask | wallbitset[intindex];
}

void set_rect_occupied(int r, int c, int width, int height) 
{
	for (int i=r; i < r+height; i++) {
		for (int i2=c; i2 < c+width; i2++) {
			set_occupied(i,i2);
		}
	}
}

inline bool is_legal_position(int ypos, int xpos, int height, int width)
{
	//check four corners, and every other pixel along sides
	//will have to change this if I expand characters
	if (is_occupied(ypos, xpos) || is_occupied(ypos+height-1,xpos) ||
			is_occupied(ypos, xpos+width-1) || is_occupied(ypos+height-1,xpos+width-1)
			|| is_occupied(ypos,xpos+1) || is_occupied(ypos+height-1, xpos+1) ||
			is_occupied(ypos+1,xpos) || is_occupied(ypos+1, xpos+width-1)
	   ) {
		//		DEBUG_PRINTF("the 4: %d %d %d %d\n\n", is_occupied(ypos, xpos), is_occupied(ypos+height-1,xpos), is_occupied(ypos, xpos+width-1), is_occupied(ypos+height-1,xpos+width-1));
		return 0;
	}

	//otherwise
	return 1;
}

//returns direction of r2 w.r.t. r1.
//For example, if r2 is to the right of r1, returns RIGHT
//returns -1 if they aren't adjacent
enum Direction {ABOVE, RIGHT, BELOW, LEFT};
int get_rel_orientation(Room r1, Room r2) 
{
	if (r2.col == r1.col) {
		if (r2.row == r1.row+1) { //am confused as to why this works
			return BELOW; 
		} else if (r2.row == r1.row-1) {
			return ABOVE;
		} else { //not adjacent!
			return -1;
		}
	} else if (r2.row == r1.row) {
		if (r2.col == r1.col+1) {
			return RIGHT; 
		} else if (r2.col == r1.col-1) {
			return LEFT;
		} else { //not adjacent!
			return -1;
		}
	}

	return -1;
}
//////////////////////////////////

//////////////////////////////////OTHER UTILITY DECLARATIONS
typedef struct RoomVector 
{
	Room ** rooms; 
	int numRooms;
} RoomVector;
typedef struct DXDY
{
	int dx;
	int dy;
} dxdy;
typedef struct Point
{
	int x;
	int y;
} Point;

typedef struct BadGuy
{
	int prevxpos;
	int prevypos;
	int xpos;
	int ypos;
	int facing;	
	bool made_contact;
	Point contact_pos;
	int angular_sum;
} BadGuy;
void draw_BadGuy(BadGuy badguy)
{
	drawRect(badguy.prevypos,badguy.prevxpos,4,4,WHITE);
	drawRect(badguy.ypos,badguy.xpos,4,4,ORANGE);
}
BadGuy new_BadGuy(int ypos, int xpos)
{
	Point p = {0,0};
	BadGuy b = {xpos,ypos,xpos,ypos,ABOVE,0,p,0};
	return b;
}
int abs(int val) 
{ return ((val >= 0) ? val : -val); }
//////////////////////////////////

//////////////////////////////////GLOBAL VARIABLES
int cycles_top_held = 0;
int cycles_right_held = 0;
int cycles_bottom_held = 0;
int cycles_left_held = 0;
Room * special_rooms[10]; //these need to be redrawn every cycle.
int num_special_rooms = 0;
int numportals = 0;
Portal portals[3]; //up to three portals in level
BadGuy badguys[10];
int num_badguys = 0;
int total_score = 0;
int level_score = 0; //initialize to a time to count down from.

//////////////////////////////////
RoomVector get_open_rooms(int row, 
		int col, 
		Room * visit_map); 

int posmod(int n, int base) 
{
	int t = n;
	if (t<0) {
		t = base+n;
	}
	return (t % base);
}
//////////////////////////////////FUNCTIONS
/////////////////////////////////////////////GAMEPLAY

bool is_boundary_in_contact(BadGuy * b) 
{
	int trackerrow[4];
	int trackercol[4];
	if (b->facing == ABOVE) {
		trackerrow[0] = b->ypos+4-1;
		trackercol[0] = b->xpos+4;
		int n = 3;
		while (n>0) {
			trackerrow[4-n]=b->ypos+4-1-n;
			trackercol[4-n] = b->xpos+4;
			n--;
		}
	} else if (b->facing == RIGHT) {
		trackerrow[0] = b->ypos+4;
		trackercol[0] = b->xpos;
		int n = 3;
		while (n>0) {
			trackerrow[4-n]=b->ypos+4;
			trackercol[4-n] = b->xpos+n;
			n--;
		}
	} else if (b->facing == BELOW) {
		trackerrow[0] = b->ypos;
		trackercol[0] = b->xpos-1;
		int n = 3;
		while (n>0) {
			trackerrow[4-n]=b->ypos+n;
			trackercol[4-n] = b->xpos-1;
			n--;
		}

	} else if (b->facing == LEFT) {
		trackerrow[0] = b->ypos-1;
		trackercol[0] = b->xpos+4-1;
		int n = 3;
		while (n>0) {
			trackerrow[4-n]=b->ypos-1;
			trackercol[4-n] = b->xpos+4-1-n;
			n--;
		}
	}

	for (int i=0; i < 4; i++) {
		if (is_occupied(trackerrow[i],trackercol[i])) {
			return 1;
		}
	}

	return 0;
}

char * badguy_to_string(BadGuy * b)
{
	char * out = malloc(100*sizeof(char));
	sprintf(out,"xpos %d, ypos %d, prevx %d, prevy %d, made_contact %d, facing %d\
			\ncontact_x %d, contact_y %d, angular_sum %d",
			b->xpos,b->ypos,b->prevxpos,b->prevypos,b->made_contact,b->facing,
			b->contact_pos.x,b->contact_pos.y,b->angular_sum);
	return out;
}

bool is_within_num_pixels(int num, int x1, int y1, int x2, int y2)
{
	if ((abs (x1-x2) <= num ) && (abs(y1-y2) <= num)) {
		return 1;
	} else {
		return 0;
	}
}

void move_badguys(int player_y, int player_x) 
{
	for (int i = 0; i < num_badguys; i++) {
		int dx = 0;
		int dy = 0;
		BadGuy * b = &badguys[i];
		//int prevdx = b->xpos - b->prevxpos;
		//int prevdy = b->ypos - b->prevypos; //try to repeat
		b->prevxpos = b->xpos;
		b->prevypos = b->ypos;

		if(i==2) { //hug right wall. would like to have more like this,
			//but have performance trouble.
			if (!(b->made_contact)) {
				DEBUG_PRINT("making contact yadayada\n");
				if (b->contact_pos.x==0) { //continue in same direction till wall
					int dx = (b->facing == LEFT) ?  -1 : b->facing % 2; 
					int dy = (b->facing == ABOVE) ?  -1 : (b->facing+1) % 2; 
					if (!is_legal_position(b->ypos+dy,b->xpos+dx,4,4)) {

						int ndirection = posmod(b->facing-1,4);
						b->facing = ndirection;
						draw_BadGuy(*b);
						b->made_contact = 1;
						b->angular_sum=360;
						Point p = {b->xpos,b->ypos};
						b->contact_pos = p;
					} else {
						b->xpos+=dx;
						b->ypos+=dy;
						draw_BadGuy(*b);
					} 
				} else { //choose a direction and start going
					int ndirection = rand() % 4;
					b->facing = ndirection;
					b->contact_pos.x=0;

				}
			} else {
				if (b->angular_sum==0 &&
						is_within_num_pixels(2, b->xpos, b->ypos, b->contact_pos.x, 
							b->contact_pos.y)) { 
					//then we are trapped in a loop and must escape
					b->made_contact=0;
					continue;
				}

				if (!is_boundary_in_contact(b)) {
					//then turn right and continue.	
					int ndirection = (b->facing + 1)%4;
					int ndx = (ndirection == LEFT) ?  -1 : (ndirection % 2); 
					//above makes sense if you draw it out on paper... turning right.
					int ndy = (ndirection == ABOVE) ? -1 : ((ndirection+1) % 2); 
					b->ypos += ndy;
					b->xpos += ndx;
					b->angular_sum = (b->angular_sum + 90) % 360;

					char * info = badguy_to_string(b);
					DEBUG_PRINTF("b not in contact!...\n %s \n",info);
					free(info);
					b->facing = ndirection;
					draw_BadGuy(*b);
				} else {
					//must handle the case where we can proceed in the same dir,
					//and the one where we can't.	
					dx = (b->facing == LEFT) ?  -1 : b->facing % 2; 
					dy = (b->facing == ABOVE) ?  -1 : (b->facing+1) % 2; 


					if (!is_legal_position(b->ypos+dy, b->xpos+dx,4,4)) {//then we must turn left

						int ndirection = posmod(b->facing-1,4);

						char * info = badguy_to_string(b);
						DEBUG_PRINTF("turning left!...\n %s\n", info);
						free(info);
						b->facing = ndirection;
						b->angular_sum = posmod((b->angular_sum-90),360);
						draw_BadGuy(*b);
						continue;
					} else { //we can proceed in same dir.
						b->xpos +=dx;
						b->ypos +=dy; 
						char * info = badguy_to_string(b);
						DEBUG_PRINTF("proceeding!...\n %s\n",info);
						free (info);
						draw_BadGuy(*b);
					}
				}
			} 




			/*	else {
				if (!b->in_contact) {
				dx = (b->facing == LEFT) ?  -1 : b->facing % 2; 
				dy = (b->facing == ABOVE) ?  -1 : (b->facing+1) % 2; 

				if (is_occupied(b->xpos+dx, b->ypos+dy)) {
				int ndirection = (b->facing + 1)%4;
				b->facing = ndirection;
				continue;
				} else {
				b->xpos +=dx;
				b->ypos +=dy; 
				b->in_contact = 1;
				draw_BadGuy(*b);

				}

				}
				}

				int trackerrow;
				int trackercol;
				if (b->facing == ABOVE) {
				trackerrow = b->ypos+4-1;
				trackercol = b->xpos+4;
				} else if (b->facing == RIGHT) {
				trackerrow = b->ypos+4;
				trackercol = b->xpos;
				} else if (b->facing == BELOW) {
				trackerrow = b->ypos;
				trackercol = b->xpos-1;
				} else if (b->facing == LEFT) {
				trackerrow = b->ypos-1;
				trackercol = b->xpos+4-1;
				} 

				if (is_occupied(trackerrow, trackercol)) {
				dx = (b->facing == LEFT) ?  -1 : b->facing % 2; 
				dy = (b->facing == ABOVE) ?  -1 : (b->facing+1) % 2; 
				b->xpos +=dx;
				b->ypos +=dy;

				DEBUG_PRINTF("staying course dx %d, dy %d\n\n",dx,dy);
				draw_BadGuy(*b);

				continue;
				} else { //we must turn!
				bool moved = 0;
				while(!moved) {
				int ndirection = (b->facing + 1)%4;

				int ndx = (ndirection == LEFT) ?  -1 : (ndirection % 2); 
			//above makes sense if you draw it out on paper... turning right.
			int ndy = (ndirection == ABOVE) ? -1 : ((ndirection+1) % 2); 
			DEBUG_PRINTF("facing: %d, ndirection %d, ndy: %d, ndx: %d\n\n", b->facing, ndirection, ndy, ndx);
			if (is_legal_position(b->ypos+ndy,b->xpos+ndx,4,4)) {
			DEBUG_PRINTF("actually went.  facing: %d, ndirection %d, ndy: %d, ndx: %d\n\n", b->facing, ndirection, ndy, ndx);
			b->ypos += ndy;
			b->xpos += ndx;
			b->facing = ndirection;
			moved = 1;
			draw_BadGuy(*b);
			} else {
			b->facing=(ndirection+1)%4;
			}
			}
			} */
		}

		else { //these guys are in sentinel mode... will track the player

			if (player_x > b->xpos) {
				//dx = (rand() % 4) - 1;

				dx = (rand() % 2);
			} else if (player_x < b->xpos) {
				//		dx = (-(rand() % 4)) + 1;
				dx = -(rand() % 2);
			}

			if (player_y > b->ypos) {
				//dy = (rand() % 4) - 1;
				dy = (rand() % 2);
			} else if (player_y < b->ypos) {
				//	dy = (-(rand() % 4)) + 1;
				dy = -(rand() % 2);
			} 


			if (is_legal_position(b->ypos+dy,b->xpos+dx,4,4)) {
				b->ypos += dy;
				b->xpos += dx;
				draw_BadGuy(*b);
			} else if (is_legal_position(b->ypos, b->xpos+dx, 4, 4)) {
				//then just move in x direction		
				b->xpos += dx;
				draw_BadGuy(*b);
			} else if (is_legal_position(b->ypos+dy, b->xpos, 4, 4)) {
				b->ypos += dy;
				draw_BadGuy(*b);
			} else {
				draw_BadGuy(*b);
			}

		}
		//		if (abs(prevdx) > 0  || abs(prevdy) > 0) {	
		//			if (is_legal_position(b->ypos+prevdy,b->xpos+prevdx,4,4)) {
		//				b->ypos += prevdy;
		//				b->xpos += prevdx;
		//				draw_BadGuy(*b);
		//				continue;
		//			}
		//		}

		//else we will choose a new direction, trying to move towards player.


		//move the badguy
		//		dx = rand() % 4 -1;
		//		if(dx > 0) {
		//			dx--;
		//		}
		//		dy = rand() % 4 -1;
		//		if(dy > 0) {
		//			dy--;
		//		}

	}

}

bool check_badguys_collision(int player_y, int player_x)
{
	for (int i = 0; i < num_badguys; i++) {
		BadGuy b = badguys[i];
		if ((abs(b.xpos - player_x) < 4) && (abs(b.ypos - player_y) < 4)) {
			return 1;
		}
	}
	return 0;
}

void draw_badguys()
{
	for (int i = 0; i < num_badguys; i++) {
		draw_BadGuy(badguys[i]);
	}
}
/////////////////////////////////////////////

/////////////////////////////////////////////MAZE SETUP
void create_badguys(Room * visit_map)
{
	num_badguys=0;
	for (int i = 0; i < 6; i++) {
		int row = rand() % NBLOCKSH;
		int ypos = row*BLOCK_H+1;

		int col = rand() % NBLOCKSW;
		int xpos = col*BLOCK_W+1;

		BadGuy b = new_BadGuy(ypos,xpos);
		badguys[num_badguys] = b;
		num_badguys++;
	}
}
void create_portals(Room * visit_map) 
{
	for (int i = 0; i < 3; i++) {
		int i1 = rand() % (NBLOCKSH*NBLOCKSW);
		int i2 = rand() % (NBLOCKSH*NBLOCKSW);
		while(special_rooms_contains(i1) || special_rooms_contains(i2) ) {
			i1 = rand() % (NBLOCKSH*NBLOCKSW);
			i2 = rand() % (NBLOCKSH*NBLOCKSW);
		}

		COLOR color = rand(); //hmmm 

		visit_map[i1].color = color;
		visit_map[i2].color = color;

		portals[i].room1 = &visit_map[i1];
		portals[i].room2 = &visit_map[i2];
		portals[i].color = color;
		special_rooms[num_special_rooms] = &visit_map[i1];
		num_special_rooms++;
		special_rooms[num_special_rooms] = &visit_map[i2];
		num_special_rooms++;

		numportals++;
	}
}

void draw_maze(Room * visit_map, int level) 
{
	for (int i = 0; i < NBLOCKSW*NBLOCKSH; i++) {

		Room room = visit_map[i];
		/*//		DEBUG_PRINTF("i: %d.  drawing room at row: %d, col: %d.\n \
		//				top_intact %d, right_intact %d, bottom_intact %d, \
		//				left_intact %d, inMaze %d, color %d\n\n",
		//				i, room.row, room.col, room.top_intact, room.right_intact,
		//				room.left_intact, room.inMaze, room.color); */

		draw_Room(room);
		//	vid_vsync();
	}

	char levelstring[25];
	sprintf(levelstring,"level %d/3",level);
	draw_string2(19*8,0,levelstring);
}


//returns visit_map
Room * calc_maze(int randseed, int * start_row) 
{
	//split the screen up into four by six pixel blocks

	Room * visit_map = malloc(sizeof(Room)*NBLOCKSH*NBLOCKSW); 

	//go through and initialize everything to 0 
	for (int i = 0; i < NBLOCKSH; i++) {
		for (int i2 = 0; i2 < NBLOCKSW; i2++) {
			visit_map[NBLOCKSW*i+i2] = new_Room(i,i2);
		}
	}

	Room * rooms[NBLOCKSH*NBLOCKSW]; //this is a stack which will track the rooms we've visited

	srand(randseed); //set the inital rand seed to argument

	//choose row of start and end of maze
	int lstart = rand() % NBLOCKSH;
	*start_row = lstart*BLOCK_H+1;

	int rend = rand() % NBLOCKSH;

	//i will mark entries in places as visited once I've gone there
	int current_row = lstart;
	int current_col = 0;

	Room * current_room = &visit_map[current_row*NBLOCKSW+current_col];
	current_room->inMaze = 1;
	Room * prev_room = &visit_map[0*NBLOCKSW + 0];
	rooms[0] = current_room; 

	int roomindex = 0; //indexes rooms
	Room * start_room = &visit_map[lstart*NBLOCKSW+0];
	Room * final_room = &visit_map[rend*NBLOCKSW + NBLOCKSW-1];
	special_rooms[0] = start_room;
	num_special_rooms++;	
	special_rooms[1] = final_room; //mark these rooms for re-rendering
	num_special_rooms++;

	//keep on searching till we get to the end
	while (1) { 
		roomindex = get_next_room(rooms, roomindex, visit_map);

		if (roomindex < 0) {
			start_room->color = GREEN;
			final_room->color = RED;
			//			drawRect(90, 90, BLOCK_W,BLOCK_H, RGB(0,31,0)); 
			return visit_map;
		}

		prev_room = current_room;
		current_room = rooms[roomindex];
		current_room->inMaze = 1;

		int orientation = get_rel_orientation(*prev_room,*current_room);
		if (orientation == ABOVE) {
			prev_room->top_intact = 0;
			current_room->bottom_intact = 0;

		} else if (orientation == RIGHT) {
			prev_room->right_intact = 0;
			current_room->left_intact = 0;

		} else if (orientation == BELOW) {
			prev_room->bottom_intact = 0;
			current_room->top_intact = 0;

		} else if (orientation == LEFT) {
			prev_room->left_intact = 0;
			current_room->right_intact = 0;

		} else {
			drawRect(90, 90, BLOCK_W,BLOCK_H, YELLOW); 

			/*			//			DEBUG_PRINTF("before failing return prev: row %d, col %d.\n \
			//					top_intact %d, right_intact %d, bottom_intact %d, \
			//					left_intact %d, inMaze %d, color %d\n\n",
			//					prev_room->row, prev_room->col, prev_room->top_intact, 
			//					prev_room->right_intact, prev_room->left_intact, 
			//					prev_room->inMaze, prev_room->color); 
			//
			//
			//			DEBUG_PRINTF("before failing return current: row %d, col %d.\n \
			//					top_intact %d, right_intact %d, bottom_intact %d, \
			//					left_intact %d, inMaze %d, color %d\n\n",
			//					current_room->row, current_room->col, current_room->top_intact, 
			//					current_room->right_intact, current_room->left_intact, 
			//					current_room->inMaze, current_room->color); 
			//
			//we should never get here*/
			return visit_map;
		}

		//		draw_Room(*prev_room);
		//		draw_Room(*current_room);


	}

	//we should never get here
	drawRect(90, 90, BLOCK_W,BLOCK_H, RGB(0,0,31)); 
	return visit_map;
}

//returns index of next room within rooms[]
int get_next_room(Room ** rooms, 
		int current_room_index, 
		Room * visit_map)
{
	Room * current_room = (rooms[current_room_index]);
	int curr_row = current_room->row;
	int curr_col = current_room->col;


	RoomVector openRoomVec = get_open_rooms(curr_row, curr_col, visit_map);

	int numopenrooms = openRoomVec.numRooms;
	Room ** openrooms = openRoomVec.rooms;


	if (numopenrooms == 0) { //we must drop back on stack, try there

		free(openrooms);
		return (current_room_index-1);
	} else { //we will visit one of the available rooms
		//check each room

		int randn = rand() % numopenrooms;
		Room * nroom = openrooms[randn];
		visit_map[(nroom->row)*NBLOCKSW+nroom->col].inMaze = 1; //mark as visited
		rooms[current_room_index+1] = &visit_map[(nroom->row)*NBLOCKSW+nroom->col]; //pushing onto stack


		//		DEBUG_PRINTF("nroom before next_return.  row, %d, col, %d\n\n", 
		//				nroom->row,
		//				nroom->col);
		//
		//		DEBUG_PRINTF("before next_return.  row, %d, col, %d\n\n", 
		//				rooms[current_room_index+1]->row,
		//				rooms[current_room_index+1]->col);

		free(openrooms);
		return current_room_index+1;
	}

}

//gets open rooms w.r.t. room at row, col.
RoomVector get_open_rooms(int row, int col, Room visit_map[NBLOCKSH*NBLOCKSW])
{

	RoomVector openRoomVec;
	Room ** openrooms = malloc(4*sizeof(Room *));
	openRoomVec.rooms = openrooms;
	int numopenrooms = 0;

	//add all available rooms to this list. if no rooms available, drop back on stack
	if ((row > 0) && (visit_map[(row-1)*NBLOCKSW+col].inMaze == 0)) { //then we can go up
		openrooms[numopenrooms] = &visit_map[NBLOCKSW*(row-1)+col];
		numopenrooms++;
	}

	if (col < NBLOCKSW-1 && visit_map[NBLOCKSW*row+col+1].inMaze == 0) { //then we can go right
		openrooms[numopenrooms] = &visit_map[NBLOCKSW*row+col+1];
		numopenrooms++;
	}

	if (row < NBLOCKSH-1 && visit_map[NBLOCKSW*(row+1)+col].inMaze == 0) { //then we can go down 
		openrooms[numopenrooms] = &visit_map[NBLOCKSW*(row+1)+col];
		numopenrooms++;
	}

	if (col > 0 && visit_map[NBLOCKSW*row+col-1].inMaze == 0) { //then we can go left 
		openrooms[numopenrooms] = &visit_map[NBLOCKSW*row+col-1];
		numopenrooms++;
	}

	//	for (int i = 0; i < numopenrooms; i++) {
	//		if (openrooms[i]->row > 50) {
	//			DEBUG_PRINT("error, error, error!\n\n");
	//		}
	//		waitOnArrow();
	//	}


	openRoomVec.rooms = openrooms;
	openRoomVec.numRooms = numopenrooms;

	for (int i = 0; i < openRoomVec.numRooms; i++) {
		/*		DEBUG_PRINTF("room %d adjacent to row %d, col %d.  :row, %d, col, %d\n\n", 
				i,
				row,
				col,
				openRoomVec.rooms[i]->row,
				openRoomVec.rooms[i]->col); */
		//		DEBUG_PRINTF("r1: %d, r2: %d\n\n",2,i);		
	}
	return openRoomVec;
}

void knock_out_walls(Room * visit_map)  //not currently used
{
	//knock out some walls of maze
	for (int i = 0; i < 20; i++) {
		int r = rand() % (NBLOCKSH*NBLOCKSW);	
		Room * troom = &visit_map[r];
		//		troom->top_intact = 0;
		//		visit_map[r-NBLOCKSW].bottom_intact = 0;
		//start with left wall, then cycle around
		if (troom->left_intact && ((r % NBLOCKSW) > 0)) { //then there is a room to left
			troom->left_intact = 0;
			Room * roomleft = &visit_map[r-1];
			roomleft->right_intact = 0;
		} else if (troom->top_intact && (r >= NBLOCKSW)) { //then there is a room above
			troom->top_intact = 0;
			Room * roomabove = &visit_map[r-NBLOCKSW];
			roomabove->bottom_intact = 0;
		} else if (troom->right_intact && ((r % NBLOCKSW) < (NBLOCKSW-1))) { //then there is a room to right
			troom->right_intact = 0;
			Room * roomright = &visit_map[r+1];
			roomright->left_intact = 0;
		} else if (troom->bottom_intact && (r < (NBLOCKSW*(NBLOCKSH-1)))) { //then there is a room below
			troom->bottom_intact = 0;
			Room * roombelow = &visit_map[r+NBLOCKSW];
			roombelow->top_intact = 0;
		}

	}
} 

/////////////////////////////////////////////

/////////////////////////////////////////////(MORE?) RUNTIME FUNCTIONS
void display_image(const unsigned short image[38400])
{	
	DMA[3].src = image;
	DMA[3].dst = videoBuffer;
	DMA[3].cnt = (240*160) | DMA_ENABLE;
}

void clear_holds() 
{
	cycles_top_held = 0;
	cycles_right_held = 0;
	cycles_bottom_held = 0;
	cycles_left_held = 0;
}

dxdy get_reqd_mvmnt() 
{
	key_poll();

	if (0 == ((KEY_UP | KEY_RIGHT | KEY_DOWN | KEY_LEFT) & __key_curr)) {
		//then there is no change needed
		//			  DEBUG_PRINT("doing nothin'\n");
		dxdy out = {0, 0};
		return out;
	} /*else {
		DEBUG_PRINTF("hmm %d\n", (KEY_UP | KEY_RIGHT | KEY_DOWN | KEY_LEFT) & __key_curr);
		} */

	int dx = 0;
	int dy = 0;


	bool hit = 0;
	if (key_hit(KEY_UP)) {
		dy += -2;	
		hit = 1;
	} 
	if (key_hit(KEY_RIGHT)) {
		dx += 2;	
		hit = 1;
	} 
	if (key_hit(KEY_DOWN)) {
		dy += 2;		
		hit = 1;
	} 
	if (key_hit(KEY_LEFT)) {
		dx += -2;
		hit = 1;
	}

	if (hit == 1) {
		clear_holds();
		dxdy out = {dx, dy};
		return out;
	}


	if (key_is_down(KEY_UP)) {
		cycles_top_held++;		
	} else {
		cycles_top_held=0;		
	}
	if (key_is_down(KEY_RIGHT)) {
		cycles_right_held++;		
	} else {
		cycles_right_held=0;		
	}
	if (key_is_down(KEY_DOWN)) {
		cycles_bottom_held++;		
	} else {
		cycles_bottom_held=0;		
	}
	if (key_is_down(KEY_LEFT)) {
		cycles_left_held++;		
	} else {
		cycles_left_held=0;		
	}

	if (cycles_top_held > 4) {
		cycles_top_held++;		
		dy = -1;	
	} 
	if (cycles_right_held > 4) {
		cycles_right_held++;		
		dx = 1;	
	} 
	if (cycles_bottom_held > 4) {
		cycles_bottom_held++;		
		dy = 1;		
	} 
	if (cycles_left_held > 4) {
		cycles_left_held++;		
		dx = -1;
	}


	dxdy out = {dx, dy};
	return out;
}

bool special_rooms_contains(int roomindex) 
{
	for (int i = 0; i < num_special_rooms; i++) {
		if (((special_rooms[i]->row *NBLOCKSW) + special_rooms[i]->col) == roomindex) {
			return 1;
		}
	}
	return 0;
}

Room * map_from_pix_to_room(Room * visit_map, int row, int col)
{
	int roomRow = row/BLOCK_H;	
	int roomCol = col/BLOCK_W;

	return &visit_map[NBLOCKSW*roomRow + roomCol];
}

void draw_player(int rowp, int colp) 
{
	static int prevrowp = 1;
	static int prevcolp = 1;

	if (prevrowp!= rowp || prevcolp != colp) {
		drawRect(prevrowp, prevcolp, 4, 4, WHITE);
	}
	drawRect(rowp, colp, 4, 4, RGB(0,0,20));
	prevrowp = rowp;
	prevcolp = colp;
}

Room * init_level(int num, int * ypos) 
{
	wallbitset = (int *) calloc(240*160/sizeof(int) + 1, sizeof(int));
	Room * visit_map = calc_maze(num*4, ypos);
	knock_out_walls(visit_map);
	create_portals(visit_map);
	create_badguys(visit_map);
	level_score = 100;

	return visit_map;
}

bool check_for_victory(Room * final_room, int player_x, int player_y)
{
	if ((abs((final_room->col * BLOCK_W) - player_x) < 4) && (abs((final_room->row*BLOCK_H) - player_y) < 4)) {
		return 1;
	}
	return 0;
}


int main(void) 
{
	total_score = 0;
	draw_image_3(0,0,240,160,splash);

	REG_DISPCNT = 0x403; //bg2, mode 3 (bitmap)
	//test();

	//int curr_seed = RANDSEED;

	int xpos = 1;
	int ypos; //gets set below
	int level = 1;
	Room * visit_map = init_level(level, &ypos);

	//	Room * visit_map = calc_maze(curr_seed, &ypos);
	//	knock_out_walls(visit_map);
	//	create_portals(visit_map);
	//	create_badguys(visit_map);


	while(!KEY_DOWN_NOW(KEY_START)); //wait till enter pressed

	draw_image_3(0,0,240,160,splashnotext);
	draw_string2(0,0,"hi there!");
	draw_string2(2*8,0,"DIRECTIONS:");
	draw_string2(3*8,0,"The objective is to make it through");
	draw_string2(4*8,0,"each maze, starting on green and ending");
	draw_string2(5*8,0,"at red.");
	draw_string2(7*8,0,"Portals help you reach your destination");
	draw_string2(8*8,0,"Press 'B' while on a colored portal" );
	draw_string2(9*8,0,"square to jump to its counterpart");
	draw_string2(11*8,0,"Avoid orange squares -- if they catch");
	draw_string2(12*8,0,"you, you die.");
	draw_string2(18*8,0,"Press 'A' to continue.");

	/*	wrap_string2(2*8,0,
		"DIRECTIONS:\n\
		The objective is to make it through the maze, starting on green \
		and ending at red.");*/

	while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed

	draw_image_3(0,0,240,160,splashnotext);
	draw_maze(visit_map,level);

	//	dma_transfer(3, videoBuffer, maze_back, 384000 | DMA_ENABLE);


	draw_player(ypos,xpos);
	int time=0;
	char timer_s[10];
	sprintf(timer_s,"%d", level_score);
	draw_string2(19*8,35*6,timer_s);

	//EDTLOOP!
	while(1) {
		time++;
		//paint current score
		if (time==60) {
			time=0;
			level_score--;
			sprintf(timer_s,"%d  ", level_score);
			draw_string2(19*8,35*6, timer_s);
		}


		dxdy nxtreqs = get_reqd_mvmnt(); //key_poll is called in here
		int dx = nxtreqs.dx;
		int dy = nxtreqs.dy;

		if (key_hit(KEY_B) )  {
			Room * jmp_to = portal_jump(map_from_pix_to_room(visit_map,ypos,xpos));
			DEBUG_PRINT("B pressed\n\n");

			if (jmp_to != NULL) {
				ypos = (jmp_to->row)*BLOCK_H+1;		
				xpos = (jmp_to->col)*BLOCK_W+1;		
				continue;
			}
		}

		vid_vsync();

		//		dma_transfer(3, maze_back, videoBuffer, 384000 | DMA_ENABLE);
		for (int i = 0; i < num_special_rooms; i++) {
			draw_cell(*special_rooms[i]);
		}

		//while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed

		move_badguys(ypos,xpos);

		if ((dx != 0) || (dy != 0)) {
			if (is_legal_position(ypos+dy,xpos+dx,4,4)) {
				draw_player(ypos+dy,xpos+dx);
				ypos += dy;
				xpos += dx;
			} else if (is_legal_position(ypos, xpos+dx, 4, 4)) {
				//then just move in x direction		
				draw_player(ypos,xpos+dx);
				xpos += dx;
			} else if (is_legal_position(ypos+dy, xpos, 4, 4)) {
				//then just move in x direction		
				draw_player(ypos+dy,xpos);
				ypos += dy;
			} else {
				draw_player(ypos,xpos);
				DEBUG_PRINTF("illegal position! y: %d, x: %d\n\n",ypos+dy, xpos+dx);
			}
		} else {
			draw_player(ypos,xpos);
		}

		if (level_score<0 || check_badguys_collision(ypos,xpos)) {
			free(visit_map);
			free(wallbitset);
			num_badguys = 0;
			num_special_rooms = 0;
			numportals = 0;
			xpos=1;
			visit_map = init_level(1, &ypos);
			draw_image_3(0,0,240,160,splashnotext);
			char victory[20];
			sprintf(victory,"You lost at level %d...", level);
			level=1;
			total_score=0;
			draw_string2(0,0, victory);
			draw_string2(8,0,"press 'A' to try again.");
			while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed

			draw_image_3(0,0,240,160,splashnotext);
			draw_maze(visit_map,level);
			draw_player(ypos,xpos);
		} else if (check_for_victory(special_rooms[1], xpos, ypos)) {
			free(visit_map);
			free(wallbitset);
			num_badguys = 0;
			num_special_rooms = 0;
			numportals = 0;
			xpos=1;
			if (level==3) {// then you win the game
				draw_image_3(0,0,240,160,splashnotext);
				free(visit_map);
				free(wallbitset);
				num_badguys = 0;
				num_special_rooms = 0;
				numportals = 0;
				xpos=1;
				//				visit_map = init_level(1, &ypos);
				draw_image_3(0,0,240,160,splashnotext);
				char victory[20];
				total_score += level_score;
				sprintf(victory,"You beat level %d, and won the game!", level);
				level=1;
				draw_string2(0,0, victory);

				sprintf(victory,"SCORE: %d", total_score);
				draw_string2(3*8,0, victory);

				draw_string2(8,0,"press 'A' to go again.");
				while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed
				return main();
			} else {
				draw_image_3(0,0,240,160,splashnotext);
				char victory[20];
				sprintf(victory,"You beat level %d!", level);
				draw_string2(0,0, victory);
				total_score+=level_score;

				sprintf(victory,"SCORE: %d", total_score);
				draw_string2(1*8,0,"press 'A' to continue to the next level.");
				draw_string2(3*8,0, victory);

				visit_map = init_level(++level, &ypos);
				while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed
				draw_image_3(0,0,240,160,splashnotext);
				draw_maze(visit_map,level);
				draw_player(ypos,xpos);
			}
		} else if (key_hit(KEY_SELECT)) {
			free(visit_map);
			free(wallbitset);
			num_badguys = 0;
			num_special_rooms = 0;
			numportals = 0;
			xpos=1;
			//				visit_map = init_level(1, &ypos);
			draw_image_3(0,0,240,160,splashnotext);
			char victory[20];
			sprintf(victory,"resetting game...");
			level=1;
			total_score=0;
			draw_string2(0,0, victory);
			draw_string2(8,0,"press 'A' to continue.");
			while(!KEY_DOWN_NOW(KEY_A)); //wait till A pressed

			return main();

			//				draw_image_3(0,0,240,160,splashnotext);
			//				draw_maze(visit_map);
			//				draw_player(ypos,xpos);
		}
	}
	return 0;
}
/////////////////////////////////////////////

void test() 
{
	bool found = 0;

	set_occupied(159,239);
	for (int r = 0; r < SCREEN_HEIGHT; r++)  {
		for (int c = 0; c < SCREEN_WIDTH; c++) {
			if (is_occupied(r,c)) {
				DEBUG_PRINTF("failing at %d, %d \n\n",r,c);
				found = 1;
			}
		}
	}

	if (found == 0) {
		DEBUG_PRINT("none occupied!\n\n");
	}
}
//////////////////////////////////
