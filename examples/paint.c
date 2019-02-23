#include <stdio.h>
#include <stdbool.h>
#include "cge.h"
#include "cin.h"

#define sumsgninc(a, b) ((a) - (b) + ((a) < (b) ? -1 : 1))

char colors0[] = {C_WHITE, C_SILVER, C_RED, C_RED | C_GREEN, C_GREEN, C_GREEN | C_BLUE, C_BLUE, C_BLUE | C_RED};
char colors1[] = {C_BLACK, C_GRAY, 0, 0, 0, 0, 0, 0};
int fillers[] = {MODE_DUST, MODE_HALF, MODE_DENSE, MODE_FILL};

bool running;
int mode = 0, state = 0, px, py;
char color;

void mousing(int x, int y, int button, int controls){
	if(button == LEFT_BUTTON){
		if(y < 2){
			if(x >= 7 && x < 23){
				x = (x - 7) >> 1;
				register int c = y ? colors1[x] : colors0[x];
				if(controls & CTRL_SHIFT) cgeSetColor(C_KEEP, c);
				else cgeSetColor(c, C_KEEP);
			}
			else if(x >= 36 && x <= 46 && !(x & 1)){
				x = (x - 36) >> 1;
				if(!y) mode = x;
				else if(x >= 4) cgeFillModeCustom(0xd0 + 3 * x);
				else cgeFillMode(fillers[x]);
			}
		}
		else if(y == 2) return;
		else switch(mode){
			case 0:	cgePoint(x, y);	break;
			case 1: case 2: case 3:
				if(state){
					mode == 1 ? cgeLine(x, y, px, py) : (mode == 2 ? cgeDrawRect(px, py, sumsgninc(x, px), sumsgninc(y, py)) : cgeDrawEllipse(px, py, x, y));
				}
				else{
					px = x;
					py = y;
				}
				state = 1 - state;
				break;
			case 4:	cgeFlood(x, y);	break;
			case 5:	cgeWriteChar(' ', x, y);	break;
		}
	}
}

void keying(bool keydown, int keycode, char character, int controls){
	if(!keydown) return;
	if(keycode == VK_ESCAPE) running = false;
	if(keycode == VK_C) cgeClearRect(0, 3, 80, 40);
}

int main(void){
	for(int i = 2; i < 8; ++i) colors1[i] = colors0[i] | C_DARK;
	
	cgeInit(80, 43, "CGE Paint", C_BLACK, C_WHITE);
	cinInit();
	cinAddMouseHandler(mousing);
	cinAddMouseMoveHandler(mousing);
	cinAddKeyHandler(keying);
	
	cgePushState();
	//gui background
	cgeSetColor(C_SILVER, C_KEEP);
	cgeFillRect(0, 0, 80, 3);
	//color choice
	cgeSetColor(C_BLACK, C_SILVER);
	cgeWriteString("Pen:", 4, 2, 0);
	for(int i = 0; i < 8; ++i){
		cgeSetColor(colors0[i], C_KEEP);
		cgePoint(2 * i + 7, 0);		cgePoint(2 * i + 8, 0);
		cgeSetColor(colors1[i], C_KEEP);
		cgePoint(2 * i + 7, 1);		cgePoint(2 * i + 8, 1);
	}
	//tool rack + mode selection
	cgeSetColor(C_BLACK, C_KEEP);
	cgeWriteString("Tool: \xad - \xdb O \xe8", 15, 30, 0);
	cgeWriteString("Mode: \xb0 \xb1 \xb2 \xdb \xdc \xdf", 17, 30, 1);
	cgeSetColor(C_WHITE, C_KEEP);
	cgeWriteChar(0xFE, 46, 0);
	cgeSwapBuffers();
	cgePopState();
	
	cgeEnableStencil(1);
	cgeClearStencil(0, 0, 80, 3);
	
	running = true;
	while(running){
		cinPollEvents();
		cgeSwapBuffers();
	}
	
	cgeTerminate();
	cinTerminate();
}