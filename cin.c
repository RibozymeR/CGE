#include <stdio.h>
#include "cin.h"

const int VK_0 = 0x30;
const int VK_1 = 0x31;
const int VK_2 = 0x32;
const int VK_3 = 0x33;
const int VK_4 = 0x34;
const int VK_5 = 0x35;
const int VK_6 = 0x36;
const int VK_7 = 0x37;
const int VK_8 = 0x38;
const int VK_9 = 0x39;
const int VK_A = 0x41;
const int VK_B = 0x42;
const int VK_C = 0x43;
const int VK_D = 0x44;
const int VK_E = 0x45;
const int VK_F = 0x46;
const int VK_G = 0x47;
const int VK_H = 0x48;
const int VK_I = 0x49;
const int VK_J = 0x4A;
const int VK_K = 0x4B;
const int VK_L = 0x4C;
const int VK_M = 0x4D;
const int VK_N = 0x4E;
const int VK_O = 0x4F;
const int VK_P = 0x50;
const int VK_Q = 0x51;
const int VK_R = 0x52;
const int VK_S = 0x53;
const int VK_T = 0x54;
const int VK_U = 0x55;
const int VK_V = 0x56;
const int VK_W = 0x57;
const int VK_X = 0x58;
const int VK_Y = 0x59;
const int VK_Z = 0x5A;

const int CTRL_CAPS = CAPSLOCK_ON;
const int CTRL_ALT = LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED;
const int CTRL_CTRL = LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED;
const int CTRL_SHIFT = SHIFT_PRESSED;

const int LEFT_BUTTON	= FROM_LEFT_1ST_BUTTON_PRESSED;
const int MID_BUTTON	= FROM_LEFT_2ND_BUTTON_PRESSED;
const int RIGHT_BUTTON	= RIGHTMOST_BUTTON_PRESSED;

int cinGetKeyCode(char c){
	if(c >= '0' && c <= '9') return VK_0 + c - '0';
	if(c >= 'a' && c <= 'z') return VK_A + c - 'a';
	if(c >= 'A' && c <= 'Z') return VK_A + c - 'A';
	return c == ' ' ? VK_SPACE : (c == '.') ? VK_OEM_PERIOD : (c == ',') ? VK_OEM_COMMA : (c == '+') ? VK_OEM_PLUS : (c == '-') ? VK_OEM_MINUS : -1;
}

static HANDLE hstdin;
static FILE *errstream = NULL;
static INPUT_RECORD *eventbuffer;
static int mhc = 0, mmhc = 0, mwhc = 0, khc = 0;
static MOUSE_HANDLER *mouse_handlers;
static MOUSE_MOVE_HANDLER *mousem_handlers;
static MOUSE_WHEEL_HANDLER *mousew_handlers;
static KEY_HANDLER *key_handlers;

static void err(char *c, int i){
	fprintf(errstream, c, i);
	fflush(errstream);
}

void cinSetCINOutput(FILE *newerr, bool close_old){
	if(close_old && errstream != NULL) fclose(errstream);
	errstream = newerr;
	err("changed CIN Error output\n", 0);
}

void cinInit(){
	err("initiating CIN...\n", 0);
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	
	eventbuffer = malloc(20 * sizeof(INPUT_RECORD));
	mouse_handlers = malloc(sizeof(MOUSE_HANDLER));
	mousem_handlers = malloc(sizeof(MOUSE_MOVE_HANDLER));
	mousew_handlers = malloc(sizeof(MOUSE_WHEEL_HANDLER));
	key_handlers = malloc(sizeof(KEY_HANDLER));
	err("allocated CIN memory\n", 0);
}

void cinTerminate(void){
	free(mouse_handlers);
	free(mousem_handlers);
	free(mousew_handlers);
	free(key_handlers);
	free(eventbuffer);
	err("terminated CIN\n", 0);
}

void cinAddMouseHandler(MOUSE_HANDLER handler){
	if(handler == NULL) return;
	mouse_handlers[mhc++] = handler;
	realloc(mouse_handlers, mhc * sizeof(MOUSE_HANDLER));
}

void cinRemoveMouseHandler(MOUSE_HANDLER handler){
	for(int i = 0; i < mhc; ++i) if(mouse_handlers[i] == handler) mouse_handlers[i] = mouse_handlers[--mhc];
}

void cinAddMouseMoveHandler(MOUSE_MOVE_HANDLER handler){
	if(handler == NULL) return;
	mousem_handlers[mmhc++] = handler;
	realloc(mousem_handlers, mmhc * sizeof(MOUSE_MOVE_HANDLER));
}

void cinRemoveMouseMoveHandler(MOUSE_MOVE_HANDLER handler){
	for(int i = 0; i < mmhc; ++i) if(mousem_handlers[i] == handler) mousem_handlers[i] = mousem_handlers[--mmhc];
}

void cinAddMouseWheelHandler(MOUSE_WHEEL_HANDLER handler){
	if(handler == NULL) return;
	mousew_handlers[mwhc++] = handler;
	realloc(mousew_handlers, mwhc * sizeof(MOUSE_WHEEL_HANDLER));
}

void cinRemoveMouseWheelHandler(MOUSE_WHEEL_HANDLER handler){
	for(int i = 0; i < mwhc; ++i) if(mousew_handlers[i] == handler) mousew_handlers[i] = mousew_handlers[--mwhc];
}

void cinAddKeyHandler(KEY_HANDLER handler){
	if(handler == NULL) return;
	key_handlers[khc++] = handler;
	realloc(key_handlers, mhc * sizeof(KEY_HANDLER));
}

void cinRemoveKeyHandler(KEY_HANDLER handler){
	for(int i = 0; i < khc; ++i) if(key_handlers[i] == handler) key_handlers[i] = key_handlers[--khc];
}

void cinPollEvents(void){
	DWORD events = 0;
	GetNumberOfConsoleInputEvents(hstdin, &events);
	if(events > 0){
		DWORD eventsread = 0;
		err("number of events: %i, ", events);
		ReadConsoleInput(hstdin, eventbuffer, events > 20 ? 20 : events, &eventsread);
		err("read %i\n", eventsread);
		for(int i = 0; i < eventsread; ++i){
			if(eventbuffer[i].EventType == KEY_EVENT){
				err("\tkey event\n", 0);
				KEY_EVENT_RECORD event = eventbuffer[i].Event.KeyEvent;
				for(register int i = 0; i < khc; ++i) (*(key_handlers[i]))((bool)event.bKeyDown, event.wVirtualKeyCode, event.uChar.AsciiChar, event.dwControlKeyState);
			}
			else if(eventbuffer[i].EventType == MOUSE_EVENT){
				MOUSE_EVENT_RECORD event = eventbuffer[i].Event.MouseEvent;
				register int type = event.dwEventFlags;
				if(type == 0 || type & DOUBLE_CLICK){
					err("\tmouse event\n", 0);
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mhc; ++i) (*(mouse_handlers[i]))(pos.X, pos.Y, event.dwButtonState, event.dwControlKeyState);
				}
				else if(type & MOUSE_MOVED){
					err("\tmouse move event\n", 0);
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mmhc; ++i) (*(mousem_handlers[i]))(pos.X, pos.Y, event.dwButtonState, event.dwControlKeyState);
				}
				else if(type & MOUSE_WHEELED){
					err("\tmouse wheel event\n", 0);
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mwhc; ++i) (*(mousew_handlers[i]))(pos.X, pos.Y, (signed short int)(event.dwButtonState >> 16), event.dwControlKeyState);
				}
			}
		}
	}
}
