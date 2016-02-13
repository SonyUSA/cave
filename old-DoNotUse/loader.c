#include "loader.h"
 
void doclearstuff();
void drawtitle();
void drawmap();
void dog();
void drawstuff();
void level1();
bool canmove(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool isclosedoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool isopendoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool istrap(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool ishtrap(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool ishdoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
void *memcpy(void* dst, const void* src, uint32_t size);

void _start()
{
    /****************************>            Fix Stack            <****************************/
    //Load a good stack
    asm(
        "lis %r1, 0x1ab5 ;"
        "ori %r1, %r1, 0xd138 ;"
        );
    /****************************>           Get Handles           <****************************/
    //Get a handle to coreinit.rpl
    unsigned int coreinit_handle, vpad_handle, sysapp_handle;
    OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
    OSDynLoad_Acquire("vpad.rpl", &vpad_handle);
    OSDynLoad_Acquire("sysapp.rpl", &sysapp_handle);
   
    // STUFF
    VPADData vpad_data;
    int(*VPADRead)(int controller, VPADData *buffer, unsigned int num, int *err);
    OSDynLoad_FindExport(vpad_handle, 0, "VPADRead", &VPADRead);
   
    // Sysapp stuff
    int(*SYSLaunchMenu)();
    OSDynLoad_FindExport(sysapp_handle, 0, "SYSLaunchMenu", &SYSLaunchMenu);
	
	// please dont break stuff...
	int(*SYSLaunchTitle) (int bit1, int bit2);
	OSDynLoad_FindExport(sysapp_handle, 0, "SYSLaunchTitle", &SYSLaunchTitle);
	int(*_Exit)();
	OSDynLoad_FindExport(coreinit_handle, 0, "_Exit", &_Exit);
   
    /****************************>       External Prototypes       <****************************/
    //OSScreen functions
    void(*OSScreenInit)();
    unsigned int(*OSScreenGetBufferSizeEx)(unsigned int bufferNum);
    unsigned int(*OSScreenSetBufferEx)(unsigned int bufferNum, void * addr);
    //OS Memory functions
	void*(*memset)(void * dest, uint32_t value, uint32_t bytes);
    void*(*OSAllocFromSystem)(uint32_t size, int align);
    void(*OSFreeToSystem)(void *ptr);
    //IM functions
    int(*IM_Open)();
    int(*IM_Close)(int fd);
    int(*IM_SetDeviceState)(int fd, void *mem, int state, int a, int b);
    /****************************>             Exports             <****************************/
    //OSScreen functions
    OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenInit", &OSScreenInit);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenGetBufferSizeEx", &OSScreenGetBufferSizeEx);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenSetBufferEx", &OSScreenSetBufferEx);
    //OS Memory functions
    OSDynLoad_FindExport(coreinit_handle, 0, "memset", &memset);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSAllocFromSystem", &OSAllocFromSystem);
    OSDynLoad_FindExport(coreinit_handle, 0, "OSFreeToSystem", &OSFreeToSystem);
    //IM functions
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_Open", &IM_Open);
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_Close", &IM_Close);
    OSDynLoad_FindExport(coreinit_handle, 0, "IM_SetDeviceState", &IM_SetDeviceState);
    /****************************>          Initial Setup          <****************************/
    //Restart system to get lib access
    int fd = IM_Open();
    void *mem = OSAllocFromSystem(0x100, 64);
    memset(mem, 0, 0x100);
    //set restart flag to force quit browser
    IM_SetDeviceState(fd, mem, 3, 0, 0);
    IM_Close(fd);
    OSFreeToSystem(mem);
    //wait a bit for browser end
    unsigned int t1 = 0x15000000;
    while(t1--) ;
    //Call the Screen initilzation function.
    OSScreenInit();
    //Grab the buffer size for each screen (TV and gamepad)
    int buf0_size = OSScreenGetBufferSizeEx(0);
    int buf1_size = OSScreenGetBufferSizeEx(1);
    //Set the buffer area.
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)0xF4000000 + buf0_size);
    //Clear both framebuffers.
    doclearstuff();

	// Define struct for global variables!
	struct cGlobals caveGlobals;
	
	// Variables n stuff!
	caveGlobals.food = 0;
	caveGlobals.row = 1;
	caveGlobals.col = 1;
	caveGlobals.level = 1;
	caveGlobals.dogsteps = 0;
	caveGlobals.dogalive = 1;
	caveGlobals.mysteps = 0;
	caveGlobals.maxhealth = 10;
	caveGlobals.curhealth = 10;
	
	// Start at level 1 (obviously!)
	level1(&caveGlobals);
	
	// Draw Buffers and Initial Screen
	__os_snprintf(caveGlobals.mystat, 64, " ");
	doclearstuff();
	drawstuff(&caveGlobals);
	flipBuffers();
	
    int err;
   
    while(1) {
		VPADRead(0, &vpad_data, 1, &err);
		
		// Quit
		if (vpad_data.btn_trigger & BUTTON_HOME) {
			doclearstuff();
			__os_snprintf(caveGlobals.endgame, 256, "Thanks for Playing!\nYour Final Level: %d \n\n\nBy: SonyUSA", caveGlobals.level);
			drawString(0, 0, caveGlobals.endgame);
			flipBuffers();
			t1 = 0x50000000;
			while(t1--) ;
			SYSLaunchMenu();
			_Exit();
		}
		//Grab Stuff (A)
		if (vpad_data.btn_trigger & BUTTON_A) {
			if (caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] == 8) {
				doclearstuff();
				__os_snprintf(caveGlobals.mystat, 64, "Got it!");
				drawString(25, 17, caveGlobals.mystat);
				caveGlobals.food += 1;
				caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 2;
				drawstuff(&caveGlobals);
				flipBuffers();
			}
			if (caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] == 11) {
				doclearstuff();
				__os_snprintf(caveGlobals.mystat, 64, "*Gulp!*");
				drawString(25, 17, caveGlobals.mystat);
				caveGlobals.curhealth += 5;
				caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 2;
				//Make sure we don't go over health limit
				if (caveGlobals.curhealth > caveGlobals.maxhealth) { caveGlobals.curhealth = caveGlobals.maxhealth; }
				drawstuff(&caveGlobals);
				dog(&caveGlobals);
				flipBuffers();				
			}
		}
		//Search for Hidden Traps and Doors
		if (vpad_data.btn_trigger & BUTTON_Y) {			
			doclearstuff();
			drawstuff(&caveGlobals);
			dog(&caveGlobals);
			//Ask the player which way to search
			__os_snprintf(caveGlobals.mystat, 64, "Search Which Way?");
			drawString(22, 17, caveGlobals.mystat);
			flipBuffers();
			//Lets use a while loop so players cant just hold down search while they are walking! Cheating gits!
			while(2) {
				VPADRead(0, &vpad_data, 1, &err);
				// Search Up
				if (vpad_data.btn_release & BUTTON_UP) {
					// Traps
					if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "It's a trap!");
						drawString(25, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col -1][caveGlobals.row] = 6;
						drawstuff(&caveGlobals);
						flipBuffers();
						break;
					}
					// Doors
					if (ishdoor(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "A Secret Door!");
						drawString(22, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col -1][caveGlobals.row] = 4;
						drawstuff(&caveGlobals);
						flipBuffers();
						break;
					}					
					// If nothing is found...
					doclearstuff();
					drawstuff(&caveGlobals);
					__os_snprintf(caveGlobals.mystat, 64, "Nothing There!");
					drawString(23, 17, caveGlobals.mystat);
					flipBuffers();
					break;
				}
				// Search Down
				if (vpad_data.btn_release & BUTTON_DOWN) {
					// Traps
					if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "It's a trap!");
						drawString(25, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col +1][caveGlobals.row] = 6;
						drawstuff(&caveGlobals);						
						flipBuffers();
						break;
					}
					// Doors
					if (ishdoor(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "A Secret Door!");
						drawString(22, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col +1][caveGlobals.row] = 4;
						drawstuff(&caveGlobals);						
						flipBuffers();
						break;
					}					
					// If nothing is found...
					doclearstuff();
					drawstuff(&caveGlobals);
					__os_snprintf(caveGlobals.mystat, 64, "Nothing There!");
					drawString(23, 17, caveGlobals.mystat);
					flipBuffers();
					break;
				}
				// Search Right
				if (vpad_data.btn_release & BUTTON_RIGHT) {
					// Traps
					if (ishtrap(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "It's a trap!");
						drawString(25, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row +1] = 6;
						drawstuff(&caveGlobals);
						flipBuffers();
						break;
					}
					// Doors
					if (ishdoor(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "A Secret Door!");
						drawString(22, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row +1] = 4;
						drawstuff(&caveGlobals);						
						flipBuffers();
						break;
					}					
					// If nothing is found...
					doclearstuff();
					drawstuff(&caveGlobals);
					__os_snprintf(caveGlobals.mystat, 64, "Nothing There!");
					drawString(23, 17, caveGlobals.mystat);
					flipBuffers();
					break;
				}
				// Search Left
				if (vpad_data.btn_release & BUTTON_LEFT) {
					// Traps
					if (ishtrap(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "It's a trap!");
						drawString(25, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row -1] = 6;
						drawstuff(&caveGlobals);						
						flipBuffers();
						break;
					}
					// Doors
					if (ishdoor(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {
						doclearstuff();
						__os_snprintf(caveGlobals.mystat, 64, "A Secret Door!");
						drawString(22, 17, caveGlobals.mystat);
						caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row -1] = 4;
						drawstuff(&caveGlobals);
						flipBuffers();
						break;
					}					
					// If nothing is found...
					doclearstuff();
					drawstuff(&caveGlobals);
					__os_snprintf(caveGlobals.mystat, 64, "Nothing There!");
					drawString(23, 17, caveGlobals.mystat);
					flipBuffers();
					break;
				}
			}
		}
		//Open and Close Doors (X + Direction)
		if (vpad_data.btn_hold & BUTTON_X) {
			if (vpad_data.btn_trigger & BUTTON_DOWN) {
				if (isclosedoor(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col +1][caveGlobals.row] = 5;
					flipBuffers();
				}
				else if (isopendoor(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col +1][caveGlobals.row] = 4;
					flipBuffers();
				}
			}
			if (vpad_data.btn_trigger & BUTTON_UP) {
				if (isclosedoor(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col -1][caveGlobals.row] = 5;
					flipBuffers();
				}
				else if (isopendoor(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col -1][caveGlobals.row] = 4;
					flipBuffers();
				}
			}			
			if (vpad_data.btn_trigger & BUTTON_LEFT) {
				if (isclosedoor(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row -1] = 5;
					flipBuffers();
				}
				else if (isopendoor(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row -1] = 4;
					flipBuffers();
				}
			}
			if (vpad_data.btn_trigger & BUTTON_RIGHT) {
				if (isclosedoor(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row +1] = 5;
					flipBuffers();
				}
				else if (isopendoor(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {
					doclearstuff();
					drawstuff(&caveGlobals);
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row +1] = 4;
					flipBuffers();
				}
			}
		}
		// Movement
		//Down
		if (vpad_data.btn_trigger & BUTTON_DOWN) {
			if (canmove(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.col += 1;
				drawstuff(&caveGlobals);
				if (istrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
				}
				if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 6;
				}				
				flipBuffers();
			}
		}
		//Up
		if (vpad_data.btn_trigger & BUTTON_UP) {
			if (canmove(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.col -= 1;
				drawstuff(&caveGlobals);
				if (istrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
				}
				if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 6;
				}				
				flipBuffers();
			}
		}
		//Left
		if (vpad_data.btn_trigger & BUTTON_LEFT) {
			if (canmove(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.row -= 1;
				drawstuff(&caveGlobals);
				if (istrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
				}
				if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 6;
				}
				flipBuffers();
			}
		}
		//Right
		if (vpad_data.btn_trigger & BUTTON_RIGHT) {
			if (canmove(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.row += 1;
				drawstuff(&caveGlobals);
				if (istrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
				}
				if (ishtrap(&caveGlobals, caveGlobals.row, caveGlobals.col ) == true ) {
					__os_snprintf(caveGlobals.mystat, 64, "Ouch!");
					drawString(25, 17, caveGlobals.mystat);
					caveGlobals.curhealth -= 1;
					caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 6;
				}
				flipBuffers();
			}
		}
		//Feed the doggy
		if (vpad_data.btn_trigger & BUTTON_PLUS) {
			if (caveGlobals.dogalive == 1) {
				if (caveGlobals.food >= 1) {
					doclearstuff();
					__os_snprintf(caveGlobals.mystat, 64, "*crunch* Woof!");
					drawString(24, 17, caveGlobals.mystat);
					caveGlobals.food -= 1;
					caveGlobals.dogsteps -= 60;
					//Make sure we don't go negative in dog health
					if (caveGlobals.dogsteps <= 0) { caveGlobals.dogsteps = 0;}
					drawstuff(&caveGlobals);
					dog(&caveGlobals);
					flipBuffers();
				}
			}
		
		}
		// Check if the player is dead
		if(caveGlobals.curhealth == 0) {
			doclearstuff();
			__os_snprintf(caveGlobals.endgame, 256, "You're Dead!\nNow how will you get iosu? :/ \n\nThanks for Playing! \n\n\nBy: SonyUSA");
			drawString(0, 0, caveGlobals.endgame);
			flipBuffers();
			t1 = 0x80000000;
			while(t1--) ;
			SYSLaunchMenu();
			_Exit();
		}
    }

}

void drawstuff(struct cGlobals *caveGlobals) {
	drawtitle(caveGlobals);
	drawmap(caveGlobals);
}
void doclearstuff() {
	int ii = 0;
	for (ii; ii < 2; ii++)
	{
		fillScreen(0,0,0,0);
		//Oops, don't do this anymore!
		//flipBuffers();
	}
}

//Boolean for bump map
bool canmove(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	// Matrix Pieces
	#define TILE_VOID 0
	#define TILE_WALL 1
	#define TILE_FLOOR 2
	#define TILE_WATER 3
	#define TILE_CLOSEDOOR 4
	#define TILE_OPENDOOR 5
	#define TILE_TRAP 6
	#define TILE_HTRAP 7
	#define TILE_FOOD 8
	#define TILE_STAIRS 9
	#define TILE_SDOOR 10
	#define TILE_POTION 11
	// Do the thing
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == TILE_FLOOR || nTileValue == TILE_OPENDOOR || nTileValue == TILE_WATER || nTileValue == TILE_TRAP || nTileValue == TILE_HTRAP || nTileValue == TILE_FOOD || nTileValue == TILE_STAIRS || nTileValue == TILE_POTION ) { return true; }
	return false;
}

//Boolean for doors
bool isclosedoor(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == 4 ) { return true; }
	return false;
}
bool isopendoor(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == 5 ) { return true; }
	return false;
}

//Boolean for traps
bool istrap(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == 6 ) { return true; }
	return false;
}
bool ishtrap(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == 7 ) { return true; }
	return false;
}

//Boolean for hidden doors
bool ishdoor(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == 10 ) { return true; }
	return false;
}

// Draw the title bar and the main guy
void drawtitle(struct cGlobals *caveGlobals) {
	__os_snprintf(caveGlobals->player1, 8, "@");
	__os_snprintf(caveGlobals->titlebar1, 128, "Food:       HP:   /           == C@VE ==              Feed Dog With + ");
	__os_snprintf(caveGlobals->titlebar3, 128, "      %d         %d  %d ", caveGlobals->food, caveGlobals->curhealth, caveGlobals->maxhealth );
	__os_snprintf(caveGlobals->titlebar2, 128, "X %d / Y %d", caveGlobals->row, caveGlobals->col);
	__os_snprintf(caveGlobals->mystat, 64, " ");
	drawString(-4, -1, caveGlobals->titlebar1);
	drawString(-4, -1, caveGlobals->titlebar3);
	drawString(54, 17, caveGlobals->titlebar2);
	drawString(caveGlobals->row, caveGlobals->col, caveGlobals->player1);
	drawString(25, 17, caveGlobals->mystat);
}

void dog(struct cGlobals *caveGlobals) {
	// Figure out the players last position and put the dog there
	caveGlobals->dogrow = caveGlobals->row;
	caveGlobals->dogcol = caveGlobals->col;
	// Increment his "health"
	caveGlobals->dogsteps += 1;
	// Health levels
	if (caveGlobals->dogsteps <= 0) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Cheater!");
	}
	if (caveGlobals->dogsteps > 0 && caveGlobals->dogsteps <= 60) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Full");
	}
	if (caveGlobals->dogsteps > 61 && caveGlobals->dogsteps <= 120) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Peckish");
	}
	if (caveGlobals->dogsteps > 121 && caveGlobals->dogsteps <= 180) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Hungry");
	}
	if (caveGlobals->dogsteps > 181 && caveGlobals->dogsteps <= 240) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Starving");
	}
	if (caveGlobals->dogsteps > 241 && caveGlobals->dogsteps <= 300) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Desperate");
	}
	if (caveGlobals->dogsteps > 301) {
		__os_snprintf(caveGlobals->dogstat, 64, "Dog: Dead");
		caveGlobals->dogalive = 0;
	}	
	// Puts a symbol where the player used to be (or not if dead) print his status
	if (caveGlobals->dogalive == 1 ) {
		__os_snprintf(caveGlobals->doggy, 8, "d");
		drawString(caveGlobals->dogrow, caveGlobals->dogcol, caveGlobals->doggy);
	}
	drawString(-4, 17, caveGlobals->dogstat);
}

void *memset(void *ptr, int value, uint32_t count) {
	uint8_t *bytes = (uint8_t*)ptr;
	for (uint32_t i = 0; i < count; i++) bytes[i] = (uint8_t)value;
	return ptr;
}

void *memcpy(void* dst, const void* src, uint32_t size)
{
    uint32_t i;
    for (i = 0; i < size; i++)
        ((uint8_t*) dst)[i] = ((const uint8_t*) src)[i];
    return dst;
}

void drawmap(struct cGlobals *caveGlobals) {
	// Matrix Pieces
	#define TILE_VOID 0
	#define TILE_WALL 1
	#define TILE_FLOOR 2
	#define TILE_WATER 3
	#define TILE_CLOSEDOOR 4
	#define TILE_OPENDOOR 5
	#define TILE_TRAP 6
	#define TILE_HTRAP 7
	#define TILE_FOOD 8
	#define TILE_STAIRS 9
	#define TILE_SDOOR 10
	#define TILE_POTION 11
	// Draw each element in the matrix using a viewscope of 4 tiles around the player
	int y;
	int x;
	// Make sure we don't look outside the matrix on accident...
	int yy = caveGlobals->col-4;
	if (yy < 0) { yy = 0;}
	int xx = caveGlobals->row-4;
	if (xx < 0) { xx = 0;}
	// Now draw everything!
    for( y = yy; y < caveGlobals->col+5 && y < 17; y++ ) {
        for( x = xx; x < caveGlobals->row+5 && x < 62; x++ ) {
            switch ( caveGlobals->nMapArray[y][x] ) {
                case TILE_VOID:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "."); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_WALL:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "#"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_FLOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, " "); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_WATER:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "~"); drawString(x, y, caveGlobals->mapbuff);
                break;				
                case TILE_CLOSEDOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "+"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_OPENDOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "/"); drawString(x, y, caveGlobals->mapbuff);
                break;
		        case TILE_TRAP:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "*"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_HTRAP:
                    __os_snprintf(caveGlobals->mapbuff, 2800, " "); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_FOOD:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "F"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_STAIRS:
                    __os_snprintf(caveGlobals->mapbuff, 2800, ">"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_SDOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "#"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_POTION:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "p"); drawString(x, y, caveGlobals->mapbuff);
                break;				
			}
        }
    }
}

void level1(struct cGlobals *caveGlobals) {
	// Have an array! Height then width!
	// By the way, Gamepad is -4, -1 to 65, 17! and TV is -4, -1 to 101, 27!
	unsigned char lMapArray[17][62] = {
	{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 4, 2, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 8, 2, 1, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 2, 1, 0, 0, 0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 5, 2, 2, 2, 4, 2, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 2, 1, 0, 0, 0, 1, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 1, 0, 0, 0, 1, 2, 1, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 1, 0, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 1, 0, 1, 1, 1, 1, 1 },
	{ 1, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 1, 0, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 1 },
	{ 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 1, 0, 1, 8, 1, 1, 1, 1, 1, 4, 1, 1, 1, 0, 1, 2, 9, 2, 1 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 1, 0, 1, 2,10, 2, 2, 2, 2, 2, 2, 1, 0, 0, 1, 2, 2, 2, 1 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 1, 0, 1, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 1 },
	{ 1, 1, 1, 2, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 1, 0, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 1, 0, 0, 0, 1, 2, 1, 2,11, 2, 2, 4, 2, 2, 2, 2, 2, 2, 1 },
	{ 0, 0, 1, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 1 },
	{ 0, 0, 1, 2, 1, 0, 0, 0, 0, 1, 2, 2, 6, 8, 6, 2, 2, 6, 2, 2, 1, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 1 },
	{ 0, 0, 1, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 6, 2, 2, 6, 2, 2, 2, 1, 0, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 2, 2, 2, 2, 1, 2, 2, 2, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 2, 6, 2, 2, 2, 1, 0, 1, 2, 2, 2, 4, 2, 2, 2, 2, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }
	};
	// Fill our global array
	int i;
	int j;
	for( i = 0; i < 17; i++) {
		for( j = 0; j < 62; j++) {
			caveGlobals->nMapArray[i][j]=lMapArray[i][j];
		}
	}
	// Put the player where she belongs!
	caveGlobals->row = 3;
	caveGlobals->col = 3;
}
