#ifndef LOADER_H
#define LOADER_H

#include "../../../libwiiu/src/coreinit.h"
#include "../../../libwiiu/src/vpad.h"
#include "../../../libwiiu/src/types.h"
#include "../../../libwiiu/src/draw.h"

struct cGlobals { 
	// Important Stuff
	int menu; 
	int gold;
	int row;
	int col;
	int level;
	int dogsteps;
	int dogalive;
	int mysteps;
	int maxhealth;
	int curhealth;
	// Unlockables
	// Some Buffers
	char info1[256];
	char titlebar1[128];
	char titlebar2[128];
	char titlebar3[128];
	char stats[256];
	char map[256];
	char endgame[256];
	char player1[8];
	char doggy[8];
	char dogstat[64];
	char mystat[64];
	char mapbuff[2800];
	// Array
	unsigned char nMapArray[17][62];

};

void _start();

#endif /* LOADER_H */
