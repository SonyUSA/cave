#include "loader.h"
 
void doclearstuff();
void drawtitle();
void drawmap();
void dog();
void level1();
bool canmove(struct cGlobals *caveGlobals, int nMapX, int nMapY );
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
	caveGlobals.menu = 1;
	caveGlobals.gold = 0;
	caveGlobals.row = 1;
	caveGlobals.col = 1;
	caveGlobals.level = 1;
	
	// Start at level 1 (obviously!)
	level1(&caveGlobals);
	
	// Draw Buffers and Initial Screen
	drawtitle(&caveGlobals);
	drawmap(&caveGlobals);
	flipBuffers();
	
    int err;
   
    while(1) {
		VPADRead(0, &vpad_data, 1, &err);
		
		// Quit
		if (vpad_data.btn_trigger & BUTTON_HOME) {
			doclearstuff();
			__os_snprintf(caveGlobals.endgame, 256, "Thanks for Playing!\nYour Final Gold: %d \n\n\nBy: SonyUSA", caveGlobals.gold);
			drawString(0, 0, caveGlobals.endgame);
			flipBuffers();
			unsigned int t1 = 0x10000000;
			t1 = 0x50000000;
			while(t1--) ;
			SYSLaunchMenu();
			_Exit();
		}
		// Movement
		//Down
		if (vpad_data.btn_trigger & BUTTON_DOWN) {
			if (canmove(&caveGlobals, caveGlobals.row, caveGlobals.col +1 ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.col += 1;
				drawtitle(&caveGlobals);
				drawmap(&caveGlobals);
				flipBuffers();
			}
		}
		//Up
		if (vpad_data.btn_trigger & BUTTON_UP) {
			if (canmove(&caveGlobals, caveGlobals.row, caveGlobals.col -1 ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.col -= 1;
				drawtitle(&caveGlobals);
				drawmap(&caveGlobals);
				flipBuffers();
			}
		}
		//Left
		if (vpad_data.btn_trigger & BUTTON_LEFT) {
			if (canmove(&caveGlobals, caveGlobals.row -1 , caveGlobals.col ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.row -= 1;
				drawtitle(&caveGlobals);
				drawmap(&caveGlobals);
				flipBuffers();
			}
		}
		//Right
		if (vpad_data.btn_trigger & BUTTON_RIGHT) {
			if (canmove(&caveGlobals, caveGlobals.row +1 , caveGlobals.col ) == true ) {	
				doclearstuff();
				dog(&caveGlobals);
				caveGlobals.row += 1;
				drawtitle(&caveGlobals);
				drawmap(&caveGlobals);
				flipBuffers();
			}
		}
		//Plus
		if (vpad_data.btn_trigger & BUTTON_PLUS) {

		}
		//Minus
		if (vpad_data.btn_trigger & BUTTON_MINUS) {

		}
		//Grab Stuff (A)
		if (vpad_data.btn_release & BUTTON_A) {

		}

    }

}

void doclearstuff() {
	int ii = 0;
	for (ii; ii < 2; ii++)
	{
		fillScreen(0,0,0,0);
		flipBuffers();
	}
}

//Boolean for bump mapbuff
bool canmove(struct cGlobals *caveGlobals, int nMapX, int nMapY) {
	// Matrix Pieces
	#define TILE_VOID 0
	#define TILE_WALL 1
	#define TILE_FLOOR 2
	#define TILE_WATER 3
	#define TILE_CLOSEDOOR 4
	#define TILE_OPENDOOR 5
	// Do the thing
	int nTileValue = caveGlobals->nMapArray[nMapY][nMapX];
	if ( nTileValue == TILE_FLOOR || nTileValue == TILE_WATER ) { return true; }
	return false;
}

void drawtitle(struct cGlobals *caveGlobals) {
	// Draw the title bar and the main guy
	__os_snprintf(caveGlobals->player1, 8, "@");
	__os_snprintf(caveGlobals->titlebar1, 128, "Gold: %d          HP:          == C@VE ==             Press + for Menu", caveGlobals->gold);
	__os_snprintf(caveGlobals->titlebar2, 128, "X %d / Y %d", caveGlobals->row, caveGlobals->col);
	drawString(-4, -1, caveGlobals->titlebar1);
	drawString(-4, 17, caveGlobals->titlebar2);
	drawString(caveGlobals->row, caveGlobals->col, caveGlobals->player1);
}

void dog(struct cGlobals *caveGlobals) {
	// Puts a symbol where the player used to be
	__os_snprintf(caveGlobals->doggy, 8, "d");
	drawString(caveGlobals->row, caveGlobals->col, caveGlobals->doggy);
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
	// Draw each element in the matrix
	int y;
	int x;
    for( y = 0; y < 17; y++ ) {
        for( x = 0; x < 62; x++ ) {
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
                case TILE_CLOSEDOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "+"); drawString(x, y, caveGlobals->mapbuff);
                break;
                case TILE_OPENDOOR:
                    __os_snprintf(caveGlobals->mapbuff, 2800, "/"); drawString(x, y, caveGlobals->mapbuff);
                break;	
            }
        }
    }
}

void level1(struct cGlobals *caveGlobals) {
	// Have an array! Height then width!
	// By the way, Gamepad is -4, -1 to 65, 17! and TV is -4, -1 to 101, 27!
	unsigned char lMapArray[17][62] = {
	{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 1, 2, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 1, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
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
