#include <stdio.h>
#include "cin.h"

int cinGetKeyCode(char c){
	if(c >= '0' && c <= '9') return VK_0 + c - '0';
	if(c >= 'a' && c <= 'z') return VK_A + c - 'a';
	if(c >= 'A' && c <= 'Z') return VK_A + c - 'A';
	return c == ' ' ? VK_SPACE : (c == '.') ? VK_OEM_PERIOD : (c == ',') ? VK_OEM_COMMA : (c == '+') ? VK_OEM_PLUS : (c == '-') ? VK_OEM_MINUS : -1;
}

static HANDLE hstdin;
static INPUT_RECORD *eventbuffer;
static int mhc = 0, mmhc = 0, mwhc = 0, khc = 0;
static MOUSE_HANDLER *mouse_handlers;
static MOUSE_MOVE_HANDLER *mousem_handlers;
static MOUSE_WHEEL_HANDLER *mousew_handlers;
static KEY_HANDLER *key_handlers;

void cinInit(){
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	
	eventbuffer = malloc(20 * sizeof(INPUT_RECORD));
	mouse_handlers = malloc(sizeof(MOUSE_HANDLER));
	mousem_handlers = malloc(sizeof(MOUSE_MOVE_HANDLER));
	mousew_handlers = malloc(sizeof(MOUSE_WHEEL_HANDLER));
	key_handlers = malloc(sizeof(KEY_HANDLER));
}

void cinTerminate(void){
	free(mouse_handlers);
	free(mousem_handlers);
	free(mousew_handlers);
	free(key_handlers);
	free(eventbuffer);
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
		ReadConsoleInput(hstdin, eventbuffer, events > 20 ? 20 : events, &eventsread);
		for(int i = 0; i < eventsread; ++i){
			if(eventbuffer[i].EventType == KEY_EVENT){
				KEY_EVENT_RECORD event = eventbuffer[i].Event.KeyEvent;
				for(register int i = 0; i < khc; ++i) (*(key_handlers[i]))((bool)event.bKeyDown, event.wVirtualKeyCode, event.uChar.AsciiChar, event.dwControlKeyState);
			}
			else if(eventbuffer[i].EventType == MOUSE_EVENT){
				MOUSE_EVENT_RECORD event = eventbuffer[i].Event.MouseEvent;
				register int type = event.dwEventFlags;
				if(type == 0 || type & DOUBLE_CLICK){
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mhc; ++i) (*(mouse_handlers[i]))(pos.X, pos.Y, event.dwButtonState, event.dwControlKeyState);
				}
				else if(type & MOUSE_MOVED){
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mmhc; ++i) (*(mousem_handlers[i]))(pos.X, pos.Y, event.dwButtonState, event.dwControlKeyState);
				}
				else if(type & MOUSE_WHEELED){
					COORD pos = event.dwMousePosition;
					for(register int i = 0; i < mwhc; ++i) (*(mousew_handlers[i]))(pos.X, pos.Y, (signed short int)(event.dwButtonState >> 16), event.dwControlKeyState);
				}
			}
		}
	}
}
