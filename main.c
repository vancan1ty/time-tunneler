#include <debugging.h>
#include <stdlib.h>
#include <stdio.h>
#include "mylib.h"

/********************GLOBAL DECLARATIONS********************/
#define  NBLOCKSW  40
#define  NBLOCKSH  26

typedef struct Room {
     int row;
     int col;
} Room;

typedef struct RoomVector {
     Room * rooms; 
     int numRooms;
} RoomVector;

#define RANDSEED 7

void drawMaze(int randseed);
RoomVector * get_open_rooms(int row, int col, short visit_map[NBLOCKSH][NBLOCKSW]); 
int get_next_room (Room rooms[], int current_room_index, short visit_map[NBLOCKSH][NBLOCKSW], Room * final_room); 
/************************************************************/

/********************************* FUNCTIONS *********************************/
int main(void) 
{
     DEBUG_PRINT("hi there"); //this does not work...

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

void drawMaze(int randseed) 
{
//split the screen up into four by six pixel blocks
     int block_w = 6;
     int block_h = 6;

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
     drawRect(block_h*lstart,0,block_w,block_h,RGB(0,31,0)); //green rect for start
     drawRect(block_h*rend, (NBLOCKSW-1)*block_w, block_w,block_h, RGB(31,0,0)); //red rect for finish 
  

     //i will mark entries in places as visited once I've gone there
     int current_row = lstart;
     int current_col = 0;
     visit_map[current_row][current_col] = 1;

     Room current_room = {current_row, current_col};
     rooms[0] = current_room; 
     int roomindex = 0;
 
     Room final_room = {rend, NBLOCKSW-1};

     int bcolor = 0;
     while (!((current_room.row == final_room.row) && (current_room.col == final_room.col))) { //keep on searching till we get to the end
	  drawRect(0, 0, block_w,block_h, RGB(31,31,bcolor)); //status indicator since I can't get printf working ;)
	  bcolor = (bcolor + 1)%31;
	  roomindex = get_next_room(rooms, roomindex, visit_map, &final_room);
	  current_room = rooms[roomindex];
	  drawRect(current_room.row*block_h, current_room.col*block_w, block_w,block_h, RGB(31,31,31)); 
	  
	  if(roomindex<0) {
	       drawRect(0, 0, block_w,block_h, RGB(0,0,31)); //status indicator since I can't get printf working ;)
	       break;
	  }

	  vid_vsync();
     }

     drawRect(block_h*rend, (NBLOCKSW-1)*block_w, block_w,block_h, RGB(31,0,0)); //red rect for finish 
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
	  Room rnext = {row-1,col};
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (col < NBLOCKSW-1 && visit_map[row][col+1] == 0) { //then we can go right
	  Room rnext = {row,col+1};
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (row < NBLOCKSH-1 && visit_map[row+1][col] == 0) { //then we can go down 
	  Room rnext = {row+1,col};
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }

     if (col > 0 && visit_map[row][col-1] == 0) { //then we can go left 
	  Room rnext = {row,col-1};
	  openrooms[numopenrooms] = rnext;
	  numopenrooms++;
     }


     openRoomVec->rooms = openrooms;
     openRoomVec->numRooms = numopenrooms;
     return openRoomVec;
}
/******************************************************************************/
