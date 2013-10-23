#include <debugging.h>
#include <stdlib.h>
#include <stdio.h>
#include "mylib.h"


/******************************** PREPROCESSOR ********************************/
#define RANDSEED	7
#define NBLOCKSW	40
#define NBLOCKSH	26
#define BLOCK_W		6
#define BLOCK_H		6
#define BLACK           RGB(0,0,0)
#define GREEN           RGB(0,31,0)
#define BLUE            RGB(0,0,31)
#define WHITE           RGB(31,31,31)

/******************************************************************************/

/*********************************** STRUCT ***********************************/
typedef struct Room {
     int row;
     int col;
     bool top_intact; //these bools encode the state of the various walls
     bool right_intact;
     bool bottom_intact;
     bool left_intact;
  
} Room;

Room new_Room(int row, int col) 
{
     Room room = {row,col,1,1,1,1};
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
     drawRect(room.row*BLOCK_H + corner_s, room.col*BLOCK_W + corner_s, cell_s, cell_s, RGB(31,31,31)); 

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
     Room * rooms; 
     int numRooms;
} RoomVector;

/******************************************************************************/

/**************************** FORWARD DECLARATIONS ****************************/
void drawMaze(int randseed);
RoomVector * get_open_rooms(int row, int col, short visit_map[NBLOCKSH][NBLOCKSW]); 
int get_next_room (Room rooms[], int current_room_index, short visit_map[NBLOCKSH][NBLOCKSW], Room * final_room); 
/******************************************************************************/

/********************************* FUNCTIONS *********************************/
int main(void) 
{
     DEBUG_PRINT("hi there\n"); //this does not work...
     //DEBUG_PRINTF("%s\n\n", "hi no 2."); //this does not work...

     REG_DISPCNT = 0x403; //bg2, mode 3 (bitmap)

     /* setPixel(40, 160, RGB(31,0,0)); */
     /* setPixel(80, 160, RGB(0,31,0)); */
     /* setPixel(120, 160, RGB(0,0,31)); */

     /* drawRect(60, 60, 20, 40, RGB(5,5,5)); */

     /* drawHollowRect(20, 200, 30, 90, RGB(31,31,31)); */
 
     int curr_seed = RANDSEED;
     drawMaze(curr_seed);
  


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
	       drawMaze(curr_seed);
	  } else if (leftPushed) {
	       curr_seed--; 
	       drawMaze(curr_seed);
	  }
    
     }
    
     return 0;
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


typedef struct TwoRooms {
  int cur;
  int next;
} TwoRooms;

void drawMaze(int randseed) 
{
//split the screen up into four by six pixel blocks

     short visit_map[NBLOCKSH][NBLOCKSW]; 

     //go through and initialize everything to 0 
     for (int i = 0; i < NBLOCKSH; i++) {
	  for (int i2 = 0; i2 < NBLOCKSW; i2++) {
	       visit_map[i][i2] = 0;
	  }
     }

     Room rooms[NBLOCKSH*NBLOCKSW]; //this is a stack which will track the rooms we've visited
  
     srand(randseed); //set the inital rand seed to argument

     //choose row of start and end of maze
     int lstart = rand() % NBLOCKSH;
     int rend = rand() % NBLOCKSH;
     drawRect(BLOCK_H*lstart,0,BLOCK_W,BLOCK_H,RGB(0,31,0)); //green rect for start
     drawRect(BLOCK_H*rend, (NBLOCKSW-1)*BLOCK_W, BLOCK_W,BLOCK_H, RGB(31,0,0)); //red rect for finish 
  

     //i will mark entries in places as visited once I've gone there
     int current_row = lstart;
     int current_col = 0;
     visit_map[current_row][current_col] = 1;

     Room current_room = new_Room(current_row, current_col);
     Room prev_room = {0,0};
     rooms[0] = current_room; 
    
     int roomindex = 0;
 
     Room final_room = new_Room(rend, NBLOCKSW-1);

     int bcolor = 0;

     //keep on searching till we get to the end
     while (!((current_room.row == final_room.row) && (current_room.col == final_room.col))) { 
	  drawRect(0, 0, BLOCK_W,BLOCK_H, RGB(31,31,bcolor)); 
	  bcolor = (bcolor + 1)%31;
	  roomindex = get_next_room(rooms, roomindex, visit_map, &final_room);

	  prev_room = current_room;
	  current_room = rooms[roomindex];

	  int orientation = get_rel_orientation(prev_room,current_room);
	  if (orientation == ABOVE) {
	    prev_room.top_intact = 0;
	    current_room.bottom_intact = 0;
	  } else if (orientation == RIGHT) {
	    prev_room.right_intact = 0;
	    current_room.left_intact = 0;
	  } else if (orientation == BELOW) {
	    prev_room.bottom_intact = 0;
	    current_room.top_intact = 0;
	  } else if (orientation == LEFT) {
	    prev_room.left_intact = 0;
	    current_room.right_intact = 0;
	  } else {
	    //assert (0);
	    return;
	  }

	  // DEBUG_PRINTF("we are here!.  direction: %d\n", orientation);
	  //  waitOnArrow();

	  draw_Room(prev_room);
	  draw_Room(current_room);
	  
	  if(roomindex<=0) {
	       drawRect(0, 0, BLOCK_W,BLOCK_H, RGB(0,0,31)); //status indicator since I can't get printf working ;)
	       break;
	  }

	  vid_vsync();
     }

     drawRect(BLOCK_H*rend, (NBLOCKSW-1)*BLOCK_W, BLOCK_W,BLOCK_H, RGB(31,0,0)); //red rect for finish 
}


//returns index of next room within rooms[]
int get_next_room(Room rooms[], 
		  int current_room_index, 
		  short visit_map[NBLOCKSH][NBLOCKSW], 
		  Room * final_room) 
{
     Room * current_room = &(rooms[current_room_index]);
     int curr_row = current_room->row;
     int curr_col = current_room->col;

     RoomVector * openRoomVec = get_open_rooms(curr_row, curr_col, visit_map);

     int numopenrooms = openRoomVec->numRooms;
     Room * openrooms = openRoomVec->rooms;
     free(openRoomVec);

     int final_row = final_room->row;
     int final_col = final_room->col;
  
     //if an adjacent room equals the final room, then just return that room!
     for (int i=0; i < numopenrooms; i++) {
	  if (openrooms[i].row == final_row && openrooms[i].col == final_col) {

	       Room nroom = *final_room;
	       visit_map[nroom.row][nroom.col] = 1; //mark as visited
	       rooms[current_room_index+1] = nroom; //pushing onto stack
	       return current_room_index+1;
	  }
     }

     //REVISION: I don't want to create big rooms.  only go somewhere if that Room is NOT connected
     for (int i = 0; i < numopenrooms; i++) {
	  Room riq = openrooms[i];
	  RoomVector * other_o_vec = get_open_rooms(riq.row, riq.col, visit_map);
	  if (other_o_vec->numRooms != 3)  { //then that is not the only adjacent cell...
	       for (int i2 = i+1; i2 < numopenrooms; i2++) { //remove it from the list of options!
		    openrooms[i2-1] = openrooms[i2];  
	       }
	       numopenrooms--;
	       i--;
	  }
	  free(other_o_vec->rooms);
	  free(other_o_vec);
     }

     if (numopenrooms == 0) { //we must drop back on stack, try there
	  free(openrooms);
	  return (current_room_index-1);
     } else { //we will visit one of the available rooms
	  //check each room
	  int randn = rand() % numopenrooms;
	  Room nroom = openrooms[randn];
	  visit_map[nroom.row][nroom.col] = 1; //mark as visited
	  rooms[current_room_index+1] = nroom; //pushing onto stack
	  free(openrooms);
	  return current_room_index+1;
     }
}

RoomVector * get_open_rooms(int row, int col, short visit_map[NBLOCKSH][NBLOCKSW])
{
     RoomVector * openRoomVec = malloc(sizeof(RoomVector));
     Room * openrooms = malloc(4*sizeof(Room));
     openRoomVec->rooms = openrooms;

     int numopenrooms = 0;
     //add all available rooms to this list. if no rooms available, drop back on stack
     if ((row > 0) && (visit_map[row-1][col] == 0)) { //then we can go up
	  Room rnext = new_Room(row-1,col);
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (col < NBLOCKSW-1 && visit_map[row][col+1] == 0) { //then we can go right
	  Room rnext = new_Room(row,col+1);
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (row < NBLOCKSH-1 && visit_map[row+1][col] == 0) { //then we can go down 
	  Room rnext = new_Room(row+1,col);
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (col > 0 && visit_map[row][col-1] == 0) { //then we can go left 
	  Room rnext = new_Room(row,col-1);
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }


     openRoomVec->rooms = openrooms;
     openRoomVec->numRooms = numopenrooms;
     return openRoomVec;
}
/******************************************************************************/
