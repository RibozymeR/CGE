#ifndef _CIN_H_
#define _CIN_H_

#define STRICT
#include <windows.h>
#include <stdbool.h>

///control key constants
#define CTRL_CAPS CAPSLOCK_ON
#define CTRL_ALT (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define CTRL_CTRL (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)
#define CTRL_SHIFT SHIFT_PRESSED

///button constants
#define LEFT_BUTTON FROM_LEFT_1ST_BUTTON_PRESSED
#define RIGHT_BUTTON RIGHTMOST_BUTTON_PRESSED
#ifdef FIVE_BUTTON_MOUSE
	#define MID_L_BUTTON FROM_LEFT_2ND_BUTTON_PRESSED
	#define MID_BUTTON FROM_LEFT_3RD_BUTTON_PRESSED
	#define MID_R_BUTTON FROM_LEFT_4TH_BUTTON_PRESSED
#else
	#define MID_BUTTON FROM_LEFT_2ND_BUTTON_PRESSED
#endif

//see https://msdn.microsoft.com/de-de/library/windows/desktop/dd375731(v=vs.85).aspx
///virtual key codes not defined by windows.h
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
extern int cinGetKeyCode(char c);

///x, y, button, controls
typedef void (*MOUSE_HANDLER)(int, int, int, int);
///x, y, button, controls
typedef void (*MOUSE_MOVE_HANDLER)(int, int, int, int);
///x, y, direction, controls
typedef void (*MOUSE_WHEEL_HANDLER)(int, int, int, int);
///keydown, keycode, character, controls
typedef void (*KEY_HANDLER)(bool, int, char, int);

extern void cinSetCINOutput(FILE *output, bool close_old);
extern void cinInit(void);
extern void cinTerminate(void);

extern void cinAddMouseHandler(MOUSE_HANDLER handler);
extern void cinRemoveMouseHandler(MOUSE_HANDLER handler);

extern void cinAddMouseMoveHandler(MOUSE_MOVE_HANDLER handler);
extern void cinRemoveMouseMoveHandler(MOUSE_MOVE_HANDLER handler);

extern void cinAddMouseWheelHandler(MOUSE_WHEEL_HANDLER handler);
extern void cinRemoveMouseWheelHandler(MOUSE_WHEEL_HANDLER handler);

extern void cinAddKeyHandler(KEY_HANDLER handler);
extern void cinRemoveKeyHandler(KEY_HANDLER handler);

///gets all waiting events and sends them to the respective handlers
extern void cinPollEvents(void);

#endif