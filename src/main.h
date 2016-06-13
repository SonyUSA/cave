#ifndef LOADER_H
#define LOADER_H

int Menu_Main();

struct cGlobals { 
	// Important Stuff
	int food;
	int row;
	int col;
	int dogrow;
	int dogcol;
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
