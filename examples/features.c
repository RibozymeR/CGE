#include <stdio.h>
#include "cge.h"
#include "cin.h"

#define sumsgninc(a, b) (((a) - (b)) + ((a) < (b) ? -1 : 1))

bool running;
int dx = 0, state = 0, px, py;

void frag(int x, int y, char colors[], bool *stencil){
	if(*stencil) colors[0] &= ~C_RED & ~C_BLUE;
	colors[1] &= ~C_RED & ~C_BLUE;
	*stencil = true;
}
FRAGMENT_SHADER shader = {&frag};

void mousing(int x, int y, int button, int controls){
	if(button == LEFT_BUTTON) cgePoint(x + dx, y);
	else if(button == MID_BUTTON){
		cgePushState();
		cgeFillMode(MODE_HALF);
		cgeFlood(x, y);
		cgePopState();
	}
	else if(button == RIGHT_BUTTON){
		if(state) cgeFillRect(px, py, sumsgninc(x, px), sumsgninc(y, py));
		else{ px = x; py = y; }
		state = ~state;
	}
}

void wheeling(int x, int y, int direction, int controls){
	dx += direction > 0 ? 1 : -1;
}

void keying(bool keydown, int keycode, char character, int controls){
	if(keycode == VK_ESCAPE) running = false;
	else if(keycode == VK_C) cgeClear();
	else if(keycode == VK_S){
		TEXTURE *texture = cgeStoreTexture(0, 0, 80, 25);
		if(texture == NULL) return;
		cgeSaveTexture("texture.cge", texture);
		cgeDeleteTexture(texture);
	}
	else if(keycode == VK_L){
		TEXTURE *texture = cgeLoadTextureFile("texture.cge");
		if(texture == NULL) return;
		cgeDrawTexture(texture, 0, 0);
		cgeDeleteTexture(texture);
	}
	else if(keycode == VK_1){
		cgeEnableStencil(1);
		cgeClearStencil(0, 0, 40, 25);
	}
	else if(keycode == VK_2){
		cgeEnableStencil(0);
	}
	else if(keycode == VK_5){
		cgeBindFragmentShader(&shader);
	}
	else if(keycode == VK_6){
		cgeBindFragmentShader(NULL);
	}
	else if(keycode == VK_Y){
		cgeApplyShader(&shader);
	}
}

int main(void){
	freopen("err.txt", "w", stderr);
	cgeSetCGEOutput(stderr, false);
	cinSetCINOutput(stderr, false);
	cgeInit(80, 25, "Test", C_RED | C_DARK, C_GREEN | C_BLUE);
	cinInit();
	cinAddMouseHandler(mousing);
	cinAddMouseMoveHandler(mousing);
	cinAddMouseWheelHandler(wheeling);
	cinAddKeyHandler(keying);
	cgeWriteString("Draw lines!", 11, 0, 0);
	cgeSwapBuffers();
	running = true;
	while(running){
		cinPollEvents();
		cgeSwapBuffers();
	}
	fclose(stderr);
	cgeTerminate();
	cinTerminate();
}
