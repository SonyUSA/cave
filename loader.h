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
	int maph;
	int mapw;
	// Unlockables
	// Some Buffers
	char info1[256];
	char titlebar1[128];
	char titlebar2[128];
	char stats[256];
	char map[256];
	char endgame[256];
	char player1[8];
	char doggy[8];
	// Array
	int nMapArray[][];
};

void _start();

#endif /* LOADER_H */
