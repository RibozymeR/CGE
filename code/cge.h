#ifndef _CGE_H_
#define _CGE_H_

#define STRICT
#include <windows.h>
#include <stdbool.h>

extern const int C_RED, C_GREEN, C_BLUE, C_DARK, C_KEEP, C_WHITE, C_SILVER, C_GRAY, C_DARK;
extern const int MODE_FILL, MODE_HALF, MODE_EMPTY, MODE_DUST, MODE_DENSE, MODE_LOW, MODE_HIGH, MODE_KEEP;

typedef struct{
	/**fragment's coordinates, array of foreground and background color, pointer to stencil buffer value*/
	void (*fragment)(int, int, char[], bool *);
}FRAGMENT_SHADER;

typedef struct{
	int width, height;
	CHAR_INFO *pixels;
}TEXTURE;

extern int WIDTH, HEIGHT;

extern void cgeSetCGEOutput(FILE *output, bool close_old);
extern void cgeInit(int width, int height, char *title, char fg, char bg);
extern void cgeTerminate(void);

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

extern TEXTURE *cgeLoadTextureFile(char *file);
extern TEXTURE *cgeLoadTexture(char pixels[], int width, int height, bool smallpixels);
extern void cgeSaveTexture(char *file, TEXTURE *texture);
extern void cgeDrawTexture(TEXTURE *texture, int x, int y);
extern void cgeDrawTexturePart(TEXTURE *texture, int tx, int ty, int width, int height, int x, int y);
extern TEXTURE *cgeStoreTexture(int x, int y, int width, int height);
extern void cgeDeleteTexture(TEXTURE *texture);

extern void cgeBindFragmentShader(FRAGMENT_SHADER *shader);
extern void cgeApplyShader(FRAGMENT_SHADER *shader);

extern void cgeSwapBuffers(void);
extern void cgeClear(void);
extern void cgePushState(void);
extern void cgePopState(void);

#endif
