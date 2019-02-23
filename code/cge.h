#ifndef _CGE_H_
#define _CGE_H_

#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

///Color constants
#define C_RED 4
#define C_GREEN 2
#define C_BLUE 1
#define C_DARK 8
#define C_WHITE 7
#define C_SILVER 15
#define C_GRAY 0
#define C_BLACK 8
///Keep color value (for example to only repaint background)
#define C_KEEP (-1)

///Painting modes
#define MODE_FILL 1
#define MODE_HIGH 7
#define MODE_LOW 8
#define MODE_EMPTY 3
#define MODE_DUST 4
#define MODE_HALF 2
#define MODE_DENSE 5
#define MODE_KEEP (-1)

///Fragment shader type; function (x, y, [foreground, background], enable deletion by stencil buffer)
typedef void (*FRAGMENT_SHADER)(int, int, char[], bool *);

///Texture structure
typedef struct{
	size_t width, height;
	CHAR_INFO *pixels;
} TEXTURE;

extern void cgeInit(size_t width, size_t height, char *title, char fg, char bg);
extern void cgeTerminate(void);

extern size_t cgeGetWidth();
extern size_t cgeGetHeight();

extern void cgeSetColor(char fg, char bg);
extern void cgeFillMode(char mode);
extern void cgeFillModeCustom(char c);
extern void cgeWriteChar(char c, int x, int y);
extern void cgeWriteString(char *str, int len, int x, int y);
extern void cgeWriteStringF(char *str, int len, int x, int y);

extern void cgePoint(int x, int y);
extern void cgeLine(int x0, int y0, int x1, int y1);
extern void cgeDrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
extern void cgeDrawRect(int x, int y, int width, int height);
extern void cgeClearRect(int x, int y, int width, int height);
extern void cgeFillRect(int x, int y, int width, int height);
extern void cgeDrawEllipse(int x0, int y0, int x1, int y1);
extern void cgeFlood(int x, int y);
extern void cgeEnableStencil(bool enable);
extern void cgeStencil(int x, int y, int width, int height);
extern void cgeClearStencil(int x, int y, int width, int height);

///loads a texture from a file; returns NULL in case of error
extern TEXTURE *cgeLoadTextureFile(FILE *stream);
///reads a texture from an array
extern TEXTURE *cgeLoadTexture(char pixels[], int width, int height, bool smallpixels);

///returns non-zero in case of error
extern int cgeSaveTexture(FILE *stream, TEXTURE *texture);

///draws a texture at the specified location
extern void cgeDrawTexture(TEXTURE *texture, int x, int y);
///draws part of a texture at the specified location
extern void cgeDrawTexturePart(TEXTURE *texture, int tx, int ty, int width, int height, int x, int y);
///stores part of the screen in a texture
extern TEXTURE *cgeStoreTexture(int x, int y, int width, int height);
///deletes a texture
extern void cgeDeleteTexture(TEXTURE *texture);

///binds a fragment shader
extern void cgeBindFragmentShader(FRAGMENT_SHADER shader);
///applies a fragment shader to the screen
extern void cgeApplyShader(FRAGMENT_SHADER shader);

///makes all changes visible
extern void cgeSwapBuffers(void);
///clears everything; ignores stencil buffer
extern void cgeClear(void);
extern void cgePushState(void);
extern void cgePopState(void);

#endif