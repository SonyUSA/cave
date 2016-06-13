#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"
#include "main.h"

void doclearstuff();
void drawtitle();
void drawmap();
void dog();
void drawstuff();
void changelevel();
bool canmove(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool isclosedoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool isopendoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool istrap(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool ishtrap(struct cGlobals *caveGlobals, int nMapX, int nMapY );
bool ishdoor(struct cGlobals *caveGlobals, int nMapX, int nMapY );
void *memcpy(void* dst, const void* src, uint32_t size);
void flipBuffers();
void drawString(unsigned int x, unsigned int y, void * string);

int Menu_Main()
{
    
    InitOSFunctionPointers();
    InitVPadFunctionPointers();
    memoryInitialize();
    
    VPADData vpad_data;
    VPADInit();
    OSScreenInit();
    
    int buf0_size = OSScreenGetBufferSizeEx(0);
    //int buf1_size = OSScreenGetBufferSizeEx(1);
    
    OSScreenSetBufferEx(0, (void *)0xF4000000);
    OSScreenSetBufferEx(1, (void *)0xF4000000 + buf0_size);
    
    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);
    
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
	changelevel(&caveGlobals);

	// Draw Buffers and Initial Screen
	__os_snprintf(caveGlobals.mystat, 64, " ");
	doclearstuff();
	drawstuff(&caveGlobals);
	flipBuffers();

    int err;

    while(1) {
		VPADRead(0, &vpad_data, 1, &err);

		// Quit
		if (vpad_data.btns_d & VPAD_BUTTON_HOME) {
			doclearstuff();
			__os_snprintf(caveGlobals.endgame, 256, "Thanks for Playing!\nYour Final Level: %d \n\n\nBy: SonyUSA", caveGlobals.level);
			drawString(0, 0, caveGlobals.endgame);
			flipBuffers();
			sleep(10);
			//Maybe fix for exit crash?
			doclearstuff();
			flipBuffers();
			doclearstuff();
			flipBuffers();
			
            memoryRelease();
            return EXIT_SUCCESS;
		}
		//Grab Stuff (A)
		if (vpad_data.btns_r & VPAD_BUTTON_A) {
			//Checks for Food
			if (caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] == 8) {
				doclearstuff();
				__os_snprintf(caveGlobals.mystat, 64, "Got it!");
				drawString(25, 17, caveGlobals.mystat);
				caveGlobals.food += 1;
				caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] = 2;
				drawstuff(&caveGlobals);
				flipBuffers();
			}
			//Check for Potions
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
			//Checks for Stairs
			if (caveGlobals.nMapArray[caveGlobals.col][caveGlobals.row] == 9) {
				caveGlobals.level += 1;
				doclearstuff();
				changelevel(&caveGlobals);
				drawstuff(&caveGlobals);
				flipBuffers();
			}
		}
		//Search for Hidden Traps and Doors
		if (vpad_data.btns_d & VPAD_BUTTON_Y) {
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
				if (vpad_data.btns_r & VPAD_BUTTON_UP) {
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
				if (vpad_data.btns_r & VPAD_BUTTON_DOWN) {
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
				if (vpad_data.btns_r & VPAD_BUTTON_RIGHT) {
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
				if (vpad_data.btns_r & VPAD_BUTTON_LEFT) {
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
		if (vpad_data.btns_h & VPAD_BUTTON_X) {
			if (vpad_data.btns_d & VPAD_BUTTON_DOWN) {
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
			if (vpad_data.btns_d & VPAD_BUTTON_UP) {
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
			if (vpad_data.btns_d & VPAD_BUTTON_LEFT) {
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
			if (vpad_data.btns_d & VPAD_BUTTON_RIGHT) {
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
		if (vpad_data.btns_d & VPAD_BUTTON_DOWN) {
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
		if (vpad_data.btns_d & VPAD_BUTTON_UP) {
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
		if (vpad_data.btns_d & VPAD_BUTTON_LEFT) {
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
		if (vpad_data.btns_d & VPAD_BUTTON_RIGHT) {
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
		if (vpad_data.btns_d & VPAD_BUTTON_PLUS) {
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
			sleep(10);
            doclearstuff();
            flipBuffers();
            doclearstuff();
            flipBuffers();
            memoryRelease();
            return EXIT_SUCCESS;
		}
		/*// Cheat and go to next level with Minus key
		if(vpad_data.btns_r & VPAD_BUTTON_MINUS) {
				caveGlobals.level += 1;
				doclearstuff();
				changelevel(&caveGlobals);
				drawstuff(&caveGlobals);
				flipBuffers();
		}*/
    }

}

void drawstuff(struct cGlobals *caveGlobals) {
	drawtitle(caveGlobals);
	drawmap(caveGlobals);
}
void doclearstuff() {
	OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);
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
	// X+Y for debug
	// __os_snprintf(caveGlobals->titlebar2, 128, "X %d / Y %d", caveGlobals->row, caveGlobals->col);
	__os_snprintf(caveGlobals->mystat, 64, " ");
	drawString(-4, -1, caveGlobals->titlebar1);
	drawString(-4, -1, caveGlobals->titlebar3);
	drawString(46, 17, caveGlobals->titlebar2);
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

void changelevel(struct cGlobals *caveGlobals) {
	// Have an array! Height then width! (Need to clean this up later...)
	// By the way, Gamepad is -4, -1 to 65, 17! and TV is -4, -1 to 101, 27!
	if (caveGlobals->level == 1) {
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
		// Put the player where she belongs!
		caveGlobals->row = 3;
		caveGlobals->col = 3;
		// Fill our global array
		int i;
		int j;
		for( i = 0; i < 17; i++) {
			for( j = 0; j < 62; j++) {
				caveGlobals->nMapArray[i][j]=lMapArray[i][j];
			}
		}
		// Who made it???
		__os_snprintf(caveGlobals->titlebar2, 128, "Map by: SonyUSA");
	}
	if (caveGlobals->level == 2) {
		unsigned char lMapArray[17][62] = {
		{1,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1},
		{1,6,2,8,2,6,1,0,0,0,1,7,1,0,0,0,1,8,1,0,0,0,1,7,1,0,0,1,2,8,2,2,2,2,2,1,0,0,0,0,1,2,2,2,1,0,0,1,6,1,0,0,0,0,1,2,2,2,2,7,2,1},
		{1,2,2,2,2,2,1,0,0,0,1,2,1,0,0,0,1,2,1,0,0,0,1,2,1,0,0,1,1,1,1,1,1,2,1,1,0,0,0,0,1,2,7,2,1,0,0,1,8,1,0,0,1,1,1,2,1,1,1,1,2,1},
		{1,2,1,1,1,2,1,0,0,0,1,2,1,1,1,1,1,2,1,1,1,1,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,1,2,2,2,1,0,0,1,2,1,0,0,1,7,8,2,1,0,0,1,2,1},
		{1,2,1,1,1,2,1,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,2,2,1,1,1,1,2,1,0,0,1,1,1,2,1,0,0,1,2,1},
		{1,2,1,1,1,2,1,0,0,0,1,2,1,1,1,2,2,2,2,2,2,2,2,2,1,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,1,2,1,0,0,1,2,1},
		{1,2,2,2,2,2,1,1,1,1,1,2,1,0,1,2,2,2,2,2,2,2,2,2,1,0,1,6,2,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,0,0,1,2,1},
		{1,2,1,1,1,2,4,2,2,2,2,2,1,0,1,2,2,2,6,2,6,2,2,2,1,0,1,2,2,1,0,0,1,2,1,0,1,2,1,0,0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,1,0,0,1,2,1},
		{1,2,1,8,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,0,1,8,6,1,0,1,2,2,1,0,1,2,1,0,0,0,0,0,0,0,0,1,2,1,1,1,1,1,1,2,1,0,0,1,2,1},
		{1,2,1,2,1,2,2,2,2,2,2,2,1,1,0,0,0,0,1,2,1,0,1,2,1,0,1,1,1,1,0,1,2,1,1,0,1,4,1,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,1,2,1,0,0,1,2,1},
		{1,2,2,2,2,2,1,1,1,2,7,2,1,1,0,0,0,0,1,2,1,0,1,2,1,0,0,0,0,0,0,1,2,1,0,0,1,2,1,0,0,0,1,1,1,0,0,1,2,1,0,0,0,0,1,2,1,1,1,1,2,1},
		{1,1,1,1,1,1,1,1,2,2,2,2,2,1,0,0,0,0,1,2,1,0,1,2,1,0,0,0,0,0,0,1,2,1,0,0,1,2,1,0,0,0,1,7,1,1,1,1,2,1,0,0,0,0,1,2,2,2,2,7,2,1},
		{0,0,0,0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,1,2,1,1,1,2,1,1,1,1,1,1,1,1,2,1,0,0,1,2,1,1,1,1,1,2,2,2,2,2,2,1,0,0,0,0,1,2,1,1,1,1,2,1},
		{0,0,0,0,0,0,0,1,2,2,6,2,2,1,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,1,2,2,2,2,2,2,2,1,1,1,1,1,1,0,0,0,0,1,2,1,0,0,1,2,1},
		{1,1,1,1,1,1,1,1,2,2,8,2,2,1,0,0,0,0,1,8,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,2,2,2,2,2,2,2,1,0,0,0,0,0,1,1,1,1,1,2,1,1,1,1,2,1},
		{1,2,2,2,2,2,2,2,2,2,6,2,2,1,0,0,0,0,1,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,8,2,2,2,7,1,0,0,0,0,0,1,9,2,2,2,2,2,2,2,2,2,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1}
		};
		// Put the player where she belongs!
		caveGlobals->row = 1;
		caveGlobals->col = 15;
		// Fill our global array
		int i;
		int j;
		for( i = 0; i < 17; i++) {
			for( j = 0; j < 62; j++) {
				caveGlobals->nMapArray[i][j]=lMapArray[i][j];
			}
		}
		// Who made it???
		__os_snprintf(caveGlobals->titlebar2, 128, "Map by: ScarletDreamz");
	}
	if (caveGlobals->level == 3) {
		unsigned char lMapArray[17][62] = {
		{ 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 3, 3, 0, 1, 8, 6, 2, 8, 6, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 3, 3, 0, 1, 8, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 1, 1, 0, 0, 1, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 1, 2, 2, 2, 4, 2, 2, 6, 2, 2, 2, 2, 1 },
		{ 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 0, 3, 3, 0, 1,11, 2, 2, 6, 2, 1, 2, 1, 1, 4, 1, 0, 1, 2, 1, 2, 2, 1, 0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 1, 4, 1, 1, 1, 2, 6, 2, 6, 6, 2, 1 },
		{ 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 3, 3, 0, 1, 2, 6, 2, 2, 2, 1, 2, 1, 2, 2, 1, 1, 1, 4, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 4, 1, 1, 2, 2, 2, 2, 1, 2, 2, 2, 6, 2, 2, 1 },
		{ 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 3, 3, 1, 1, 1, 2, 2, 2, 6, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 6, 2, 2, 6, 1 },
		{ 1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 4, 2, 3, 3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 0, 0, 0, 1, 2, 7, 2, 1, 2, 2, 2, 2, 1, 6, 6, 6, 6, 2, 2, 1 },
		{ 1, 2, 1, 2, 2, 2, 2, 1, 0, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1,10, 1, 2, 1, 2, 2, 2, 7, 2, 2, 1, 1, 2, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 7, 2,10, 2, 2, 2, 2, 1, 6, 2, 2, 2, 6, 2, 1 },
		{ 1, 2,10, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 7, 7, 7, 2, 1, 1, 2,10, 2, 1, 1, 1, 1, 1, 1, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 6, 2, 6, 2, 1 },
		{ 1, 2, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,11, 2, 2, 7, 2, 2, 2, 1, 2, 1, 2, 2, 2, 2, 1, 2, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 6, 2, 2, 2, 1 },
		{ 1, 2, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 7, 7, 7, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 6, 6, 6, 2, 6, 1 },
		{ 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 3, 3, 2, 2, 4, 2, 2, 7, 1, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 1, 2, 1, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 6, 2, 1 },
		{ 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 3, 3, 1, 1, 1, 2, 2, 7, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 8, 1, 2, 1, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 6, 2, 2, 6, 1 },
		{ 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 0, 3, 3, 0, 0, 1, 2, 2, 7, 1, 2, 2, 2, 2, 1, 1, 4, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 6, 6, 2, 6, 2, 2, 1 },
		{ 1, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 3, 3, 0, 0, 1, 6, 2, 2, 1, 2, 2, 2, 2, 4, 2, 2, 1, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 1, 0, 1, 9, 2, 2, 2, 2, 6, 1 },
		{ 1, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 3, 3, 0, 0, 1, 6, 2, 2,10, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
		};
		// Put the player where she belongs!
		caveGlobals->row = 5;
		caveGlobals->col = 8;
		// Fill our global array
		int i;
		int j;
		for( i = 0; i < 17; i++) {
			for( j = 0; j < 62; j++) {
				caveGlobals->nMapArray[i][j]=lMapArray[i][j];
			}
		}
		// Who made it???
		__os_snprintf(caveGlobals->titlebar2, 128, "Map by: SonyUSA");
	}
	if (caveGlobals->level == 4) {
		unsigned char lMapArray[17][62] = {
		{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{1,11,8,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,1,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0},
		{1,2,2,2,2,2,2,2,2,10,2,2,2,2,2,8,1,0,0,0,0,1,2,7,2,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0},
		{1,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,1,2,2,2,1,0,3,3,3,3,1,3,1,3,3,1,2,1,3,3,1,3,1,3,3,1,2,1,3,3,1,2,1,0,3,1,3,1,3,3,0},
		{1,2,2,2,2,2,2,2,2,1,2,8,2,2,2,2,1,0,1,1,1,1,1,4,1,1,1,1,3,3,3,1,3,1,0,3,1,2,1,3,0,1,2,1,3,0,1,1,1,3,3,1,2,1,0,3,1,2,1,3,0,0},
		{1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,4,1,1,2,2,2,2,2,2,2,2,2,1,0,3,3,1,2,1,0,0,1,2,1,0,0,1,2,1,1,0,1,2,10,3,0,1,2,1,0,0,1,2,1,3,0,0},
		{0,1,4,1,1,2,1,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,0},
		{0,1,2,1,1,2,1,1,0,0,0,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,0,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,0},
		{1,1,2,2,1,2,2,1,0,0,0,1,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,1,0,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,0,1,2,1,1,1,0},
		{1,2,2,2,7,1,2,2,1,0,0,1,2,2,2,2,1,2,2,8,2,1,1,1,1,1,1,1,0,0,1,1,2,1,1,1,1,3,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,1,0,0,1,2,2,2,1,0},
		{1,2,7,2,2,2,1,2,2,1,1,1,1,1,4,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,2,2,8,2,1,3,3,3,3,1,8,7,2,1,2,2,2,2,2,1,0,0,0,0,0,1,11,8,8,1,0},
		{1,10,1,1,1,1,1,1,2,2,2,2,8,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1,1,0,0,0,0,0,1,1,1,1,1,0},
		{0,3,3,3,3,3,3,1,1,1,1,1,1,1,2,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,2,1,0,0,0,1,1,1,1,1,1,1,1,1},
		{0,3,3,3,3,3,3,3,0,0,0,0,0,1,2,1,0,0,1,2,2,8,2,2,1,1,1,1,11,7,2,2,2,7,2,7,2,2,2,2,2,7,2,2,2,2,1,1,2,1,1,1,1,1,2,2,6,2,2,2,2,1},
		{0,3,3,3,3,3,3,3,3,1,1,1,1,1,2,1,0,0,1,2,9,2,2,2,2,2,4,2,2,7,2,7,2,2,7,2,2,7,2,2,7,2,2,7,2,2,2,4,2,2,4,2,2,2,2,6,2,2,6,2,2,1},
		{0,3,3,3,3,3,3,3,3,3,2,2,2,2,2,1,0,0,1,2,2,2,2,1,1,1,1,1,2,2,2,2,7,2,2,2,7,2,7,2,2,2,2,2,7,2,1,1,1,1,1,1,1,1,2,2,2,6,8,2,6,1},
		{0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1}
		};
		// Put the player where she belongs!
		caveGlobals->row = 2;
		caveGlobals->col = 2;
		// Fill our global array
		int i;
		int j;
		for( i = 0; i < 17; i++) {
			for( j = 0; j < 62; j++) {
				caveGlobals->nMapArray[i][j]=lMapArray[i][j];
			}
		}
		// Who made it???
		__os_snprintf(caveGlobals->titlebar2, 128, "Map by: SonyUSA");
	}
	if (caveGlobals->level == 5) {
		unsigned char lMapArray[17][62] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8,8,8,8},
		{1,2,2,2,1,2,2,2,2,7,8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8,8,8,8},
		{1,2,8,2,5,2,2,2,2,2,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,8,8,8,8},
		{1,1,4,1,1,1,1,1,1,1,10,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2},
		{0,1,2,1,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,8,10,2,2,2,2,2,2,7,2,1,0,0,0,1,7,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
		{0,1,2,1,0,0,0,0,0,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,3,3,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
		{0,1,2,1,0,0,0,0,0,1,2,1,0,0,0,0,0,0,1,6,1,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,1,3,3,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
		{0,1,2,1,0,0,0,0,0,1,2,1,0,0,0,0,0,0,1,6,1,0,0,0,0,0,0,1,2,2,1,0,1,2,2,1,0,1,3,3,3,3,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
		{0,1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,11,1,11,2,1,0,0,1,3,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
		{0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,8,7,10,1,10,2,2,7,1,0,0,1,1,2,1,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{0,1,10,1,10,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,1,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0},
		{0,1,2,1,7,1,6,1,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,10,8,2,2,2,2,1,2,1,2,2,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,0,0,0},
		{0,1,2,1,7,1,6,1,0,1,2,1,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,1,2,1,2,2,2,2,2,2,2,1,2,8,2,1,2,2,2,2,2,0,0,0},
		{0,1,2,1,7,1,6,1,0,1,2,1,0,1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,1,2,1,2,2,2,2,2,2,2,1,2,9,2,1,2,2,2,2,2,0,0,0},
		{0,1,2,1,7,1,6,1,0,1,2,1,1,1,2,2,2,7,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,2,1,1,1,1,10,1,1,1,1,2,2,2,1,2,2,2,2,2,0,0,0},
		{0,1,8,1,8,1,11,1,0,1,2,2,2,4,2,2,6,2,7,2,5,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,10,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,0,0,0},
		{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,0,0,0}
		};
		// Put the player where she belongs!
		caveGlobals->row = 1;
		caveGlobals->col = 1;
		// Fill our global array
		int i;
		int j;
		for( i = 0; i < 17; i++) {
			for( j = 0; j < 62; j++) {
				caveGlobals->nMapArray[i][j]=lMapArray[i][j];
			}
		}
		// Who made it???
		__os_snprintf(caveGlobals->titlebar2, 128, "Map by: Sivart0");
	}
}

void flipBuffers() {
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
}

void drawString(unsigned int x, unsigned int y, void * string) {
    OSScreenPutFontEx(0, x, y, string);
    OSScreenPutFontEx(1, x, y, string);
}