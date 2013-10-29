#include <debugging.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mylib.h"


/******************************** PREPROCESSOR ********************************/
#define RANDSEED	23
#define NBLOCKSW	40
#define NBLOCKSH	26
#define BLOCK_W		6
#define BLOCK_H		6
#define RED             RGB(31,0,0)
#define GREEN           RGB(0,31,0)
#define BLUE            RGB(0,0,31)
#define WHITE           RGB(31,31,31)
#define BLACK           RGB(0,0,0)
#define YELLOW			RGB(31,31,0)

/******************************************************************************/

/*********************************** STRUCT ***********************************/
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

void draw_Room(Room room) 
{
	int corner_s = 1;
	int cell_s = 4;

	//draw corners black
	drawRect(room.row*BLOCK_H, room.col*BLOCK_W, corner_s, corner_s, BLACK); 
	drawRect(room.row*BLOCK_H+corner_s+ cell_s, room.col*BLOCK_W, corner_s, corner_s, BLACK); 
	drawRect(room.row*BLOCK_H, room.col*BLOCK_W+corner_s+cell_s, corner_s, corner_s, BLACK); 
	drawRect(room.row*BLOCK_H+corner_s+cell_s, room.col*BLOCK_W+corner_s+cell_s, corner_s, corner_s, BLACK); 

	//draw central cell
	drawRect(room.row*BLOCK_H + corner_s, room.col*BLOCK_W + corner_s, cell_s, cell_s, room.color); 

	//draw walls...
	if (!room.top_intact) {
		drawRect(room.row*BLOCK_H, room.col*BLOCK_W + corner_s, cell_s, corner_s, WHITE); 
	}
	if (!room.right_intact) {
		drawRect(room.row*BLOCK_H+corner_s, room.col*BLOCK_W + corner_s + cell_s, corner_s, cell_s, WHITE); 
	}
	if (!room.bottom_intact) {
		drawRect(room.row*BLOCK_H+corner_s+cell_s, room.col*BLOCK_W + corner_s, cell_s, corner_s, WHITE); 
	}
	if (!room.left_intact) {
		drawRect(room.row*BLOCK_H+corner_s, room.col*BLOCK_W, corner_s, cell_s, WHITE); 
	}

}

enum Direction {ABOVE, RIGHT, BELOW, LEFT};

//returns direction of r2 w.r.t. r1.
//For example, if r2 is to the right of r1, returns RIGHT
//returns -1 if they aren't adjacent
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

typedef struct RoomVector {
	Room ** rooms; 
	int numRooms;
} RoomVector;

/******************************************************************************/

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

/**************************** FORWARD DECLARATIONS ****************************/
Room * calcMaze(int randseed);
RoomVector get_open_rooms(int row, 
		int col, 
		Room * visit_map); 
int get_next_room (Room ** rooms, 
		int current_room_index, 
		Room * visit_map);

void drawMaze(Room * visit_map);
/******************************************************************************/

/********************************* FUNCTIONS *********************************/
int main(void) 
{
	DEBUG_PRINT("hi there\n"); //this does not work...
	//DEBUG_PRINTF("%s\n\n", "hi no 2."); //this does not work...

	REG_DISPCNT = 0x403; //bg2, mode 3 (bitmap)



	int curr_seed = RANDSEED;
	Room * visit_map = calcMaze(curr_seed);
	drawMaze(visit_map);

	while(1) {
		bool rightPushed = 0;
		bool leftPushed = 0;
		bool hasInput = 0;
		while(!hasInput){ 
			rightPushed = IS_KEY_DOWN(KEY_RIGHT); 
			leftPushed = IS_KEY_DOWN(KEY_LEFT);
			if (rightPushed || leftPushed) {
				hasInput=1; //break out of loop, do action
			}
		} 

		clearScreen();

		REG_DISPCNT = 0x403; //bg2, mode 3 (bitmap)
		if(rightPushed) { //go to next maze!
			curr_seed++;
			calcMaze(curr_seed);
		} else if (leftPushed) {
			curr_seed--; 
			calcMaze(curr_seed);
		}

	}

	return 0;
}

void drawMaze(Room * visit_map) 
{
	for (int i = 0; i < NBLOCKSW*NBLOCKSH; i++) {

		Room room = visit_map[i];
		DEBUG_PRINTF("i: %d.  drawing room at row: %d, col: %d.\n \
		             top_intact %d, right_intact %d, bottom_intact %d, \
				   left_intact %d, inMaze %d, color %d\n\n",
				   i, room.row, room.col, room.top_intact, room.right_intact,
				   room.left_intact, room.inMaze, room.color); 
		             
		draw_Room(room);
	//	vid_vsync();
	}
}

void waitOnArrow() 
{
	bool rightPushed = 0;
	bool leftPushed = 0;
	bool hasInput = 0;
	while(!hasInput){ 
		rightPushed = IS_KEY_DOWN(KEY_RIGHT); 
		leftPushed = IS_KEY_DOWN(KEY_LEFT);
		if (rightPushed || leftPushed) {
			hasInput=1; //break out of loop, do action
		}
	} 

}

typedef struct TwoRooms 
{
	int cur;
	int next;
} TwoRooms;

//returns visit_map
Room * calcMaze(int randseed) 
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


//		DEBUG_PRINTF("prev_room.  r: %d, c: %d\n\n", 
//				prev_room->row, 
//				prev_room->col);
//
//		DEBUG_PRINTF("current_room.  r: %d, c: %d\n\n", 
//				current_room->row, 
//				current_room->col);

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

			DEBUG_PRINTF("before failing return prev: row %d, col %d.\n \
					top_intact %d, right_intact %d, bottom_intact %d, \
					left_intact %d, inMaze %d, color %d\n\n",
					prev_room->row, prev_room->col, prev_room->top_intact, 
					prev_room->right_intact, prev_room->left_intact, 
					prev_room->inMaze, prev_room->color); 


			DEBUG_PRINTF("before failing return current: row %d, col %d.\n \
					top_intact %d, right_intact %d, bottom_intact %d, \
					left_intact %d, inMaze %d, color %d\n\n",
					current_room->row, current_room->col, current_room->top_intact, 
					current_room->right_intact, current_room->left_intact, 
					current_room->inMaze, current_room->color); 

			//we should never get here
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
/******************************************************************************/
