#ifndef _CIN_H_
#define _CIN_H_

#define STRICT
#include <windows.h>
#include <stdbool.h>

extern const int CTRL_CAPS, CTRL_ALT, CTRL_CTRL, CTRL_SHIFT;
extern const int LEFT_BUTTON, MID_BUTTON, RIGHT_BUTTON;

//see https://msdn.microsoft.com/de-de/library/windows/desktop/dd375731(v=vs.85).aspx
extern const int VK_0, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9;
extern const int VK_A, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J, VK_K, VK_L, VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z;
extern int cinGetKeyCode(char c);

/**x, y, button, controls*/
typedef void (*MOUSE_HANDLER)(int, int, int, int);
/**x, y, button, controls*/
typedef void (*MOUSE_MOVE_HANDLER)(int, int, int, int);
/**x, y, direction, controls*/
typedef void (*MOUSE_WHEEL_HANDLER)(int, int, int, int);
/**keydown, keycode, character, controls*/
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
extern void cinPollEvents(void);

#endif
