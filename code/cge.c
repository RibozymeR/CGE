/*Console Graphics engine
    Copyleft (C) 2017 RibozymeR

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <math.h>
#include "cge.h"

#define exch(a, b) {register int h = a; a = b; b = h;}

const int C_RED = 4, C_GREEN = 2, C_BLUE = 1, C_DARK = 8;
const int C_WHITE = 7, C_SILVER = 15, C_GRAY = 0, C_BLACK = 8;
const int C_KEEP = -1;

const int MODE_FILL = 1, MODE_HIGH = 7, MODE_LOW = 8, MODE_EMPTY = 3;
const int MODE_DUST = 4, MODE_HALF = 2, MODE_DENSE = 5;
const int MODE_KEEP = -1;

static const COORD ZERO = {0, 0};
static const char *MAGIC = "CGET";

int WIDTH, HEIGHT;
static int SCRSIZE;
static FILE *errstream = NULL;
static HANDLE hstdout;
static SMALL_RECT bounds = {0, 0, 0, 0};
static COORD size;
static CHAR_INFO *screen;
static bool stencil_enabled = false, *stencil = NULL;
static int color;
static char filler;
static int stacksize = 0;
static int *statestack;
static FRAGMENT_SHADER *fragment_shader = NULL;

static void err(char *c, int i){
	fprintf(errstream, c, i);
	fflush(errstream);
}

void cgeSetCGEOutput(FILE *newerr, bool close_old){
	if(close_old && errstream != NULL) fclose(errstream);
	errstream = newerr;
	err("changed CGE Error output\n", 0);
}

void cgeInit(int width, int height, char *title, char fg, char bg){
	WIDTH = width;
	HEIGHT = height;
	SCRSIZE = WIDTH * HEIGHT;
	bounds.Right = WIDTH - 1;
	bounds.Bottom = HEIGHT - 1;
	size.X = WIDTH;
	size.Y = HEIGHT;
	
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SetConsoleTitle(title);
	SetConsoleWindowInfo(hstdout, 1, &bounds);
	CONSOLE_CURSOR_INFO ci = {2, false};
	SetConsoleCursorInfo(hstdout, &ci);
	SetConsoleScreenBufferSize(hstdout, size);
	err("initiated console\n", 0);
	
	statestack = malloc(3 * sizeof(int));
	
	cgeSetColor(fg, bg);
	cgeFillMode(MODE_FILL);
	screen = (CHAR_INFO *)malloc(SCRSIZE * sizeof(CHAR_INFO));
	stencil = (bool *)malloc(SCRSIZE);
	for(int i = 0; i < SCRSIZE; ++i) stencil[i] = true;
	cgeClear();
	cgeSwapBuffers();
}

void cgeTerminate(void){
	free(statestack);
	free(screen);
	if(stencil != NULL) free(stencil);
	err("terminated CGE\n", 0);
}

void cgeSetColor(char foreground, char background){
	if(foreground == C_KEEP) color = (background ^ 8) << 4 | (color & 0xF);
	else if(background == C_KEEP) color = (color & 0xF0) | (foreground ^ 8);
	else color = (background << 4 | foreground) ^ 0x88;
}

void cgeFillMode(char mode){
	switch(mode){
		case 1:	filler = 0xDB; break;
		case 2:	filler = 0xB1; break;
		case 3:	filler = ' ';  break;
		case 4:	filler = 0xB0; break;
		case 5: filler = 0xB2; break;
		case 6:	filler = 0;    break;
		case 7: filler = 0xDF; break;
		case 8: filler = 0xDC; break;
		default: filler = ' '; break;
	}
}

void cgeFillModeCustom(char c){
	filler = c;
}

void cgeWriteChar(char c, int x, int y){
	int pos = x + y * WIDTH;
	screen[pos].Char.AsciiChar = c;
	screen[pos].Attributes = color;
}

void cgeWriteString(char *str, int len, int x, int y){
	int pos = x + y * WIDTH;
	for(int i = 0; i < len; ++i){
		register char c = str[i];
		if(c){
			screen[pos].Char.AsciiChar = c;
			screen[pos++].Attributes = color;
		}
	}
}

//\0 -> don't fill
//\\ -> backslash
//\n -> newline
//\q -> exit
//\c(x) -> foreground = x
void cgeWriteStringF(char *str, int len, int x, int y){
	int pos = x + y * WIDTH;
	register bool ctrlmode = false;
	for(int i = 0; i < len; ++i){
		register char c = str[i];
		if(!c) continue;
		if(ctrlmode){
			if(c == '\\'){
				screen[pos].Char.AsciiChar = '\\';
				screen[pos++].Attributes = color;
			}
			else if(c == 'n') pos = x + (++y) * WIDTH;
			else if(c == 'q') return;
			else if(c == 'c') cgeSetColor(str[++i], C_KEEP);
			ctrlmode = false;
		}
		else{
			if(c == '\\') ctrlmode = true;
			else{
				screen[pos].Char.AsciiChar = c;
				screen[pos++].Attributes = color;
			}
		}
	}
}

static void cgePoint0(int pos){
	bool paint = true;
	if(pos < 0 || pos >= SCRSIZE) return;
	register int pcolor = color;
	if(fragment_shader != NULL){
		paint = stencil[pos];
		char cparts[] = {(pcolor & 15) ^ 8, (pcolor >> 4) ^ 8};
		(*fragment_shader->fragment)(pos % WIDTH, pos / WIDTH, cparts, &paint);
		pcolor = (cparts[1] << 4 | cparts[0]) ^ 0x88;
	}
	if(stencil_enabled && !paint) return;
	if(filler) screen[pos].Char.AsciiChar = filler;
	screen[pos].Attributes = pcolor;
}

void cgePoint(int x, int y){
	//if(x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
	cgePoint0(x + y * WIDTH);
}

void cgeLine(int x0, int y0, int x1, int y1){
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	while(1){
		cgePoint(x0, y0);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 > dy){
			err += dy;
			x0 += sx;
		}
		if(e2 < dx){
			err += dx;
			y0 += sy;
		}
	}
}

void cgeDrawTriangle(int x0, int y0, int x1, int y1, int x2, int y2){
	cgeLine(x0, y0, x1, y1);
	cgeLine(x1, y1, x2, y2);
	cgeLine(x2, y2, x0, y0);
}

void cgeDrawRect(int x, int y, int width, int height){
	if(width < 0) x -= (width = -width);
	if(height < 0) y -= (height = -height);
	for(int tx = x; tx < x + width; ++tx){
		cgePoint(tx, y);
		cgePoint(tx, y + height - 1);
	}
	for(int ty = y + 1; ty < y + height - 1; ++ty){
		cgePoint(x, ty);
		cgePoint(x + width - 1, ty);
	}
}

void cgeClearRect(int x, int y, int width, int height){
	char last = filler;
	filler = ' ';
	cgeFillRect(x, y, width, height);
	filler = last;
}

void cgeFillRect(int x, int y, int width, int height){
	if(width < 0) x -= (width = -width);
	if(height < 0) y -= (height = -height);
	register int incr = WIDTH - width, c = 0;
	for(register int i = x + y * WIDTH; i < x + width + (y + height - 1) * WIDTH; ++i){
		cgePoint0(i);
		if(++c == width){
			c = 0;
			i += incr;
		}
	}
}

static void cgeDrawEllipse0(int xm, int ym, int a, int b){
	int dx = 0, dy = b;
	long a2 = a * a, b2 = b * b;
	long err = b2 - (2 * b - 1) * a2, e2;
	do{
		cgePoint(xm + dx, ym + dy);
		cgePoint(xm + dx, ym - dy);
		cgePoint(xm - dx, ym + dy);
		cgePoint(xm - dx, ym - dy);
		e2 = 2 * err;
		if(e2 < (2 * dx + 1) * b2){
			++dx;
			err += (2 * dx + 1) * b2;
		}
		if(e2 > -(2 * dy - 1) * a2){
			--dy;
			err -= (2 * dy - 1) * a2;
		}
	}while(dy >= 0);
	while(dx++ < a){
		cgePoint(xm + dx, ym);
		cgePoint(xm - dx, ym);
	}
}

void cgeDrawEllipse(int x0, int y0, int x1, int y1){
	if(x0 > x1) exch(x0, x1);
	if(y0 > y1) exch(y0, y1);
	cgeDrawEllipse0((x0 + x1) >> 1, (y0 + y1) >> 1, (x1 - x0) >> 1, (y1 - y0) >> 1);
}

#define equscrgnd(a) (screen[a].Char.AsciiChar == ground->Char.AsciiChar && screen[a].Attributes == ground->Attributes)
bool *searched;
static void cgeFloodPos(int x, int y, CHAR_INFO *ground){
	int pos = x + y * WIDTH;
	searched[pos] = true;
	if(y > 0 && !searched[pos - WIDTH] && equscrgnd(pos - WIDTH)) cgeFloodPos(x, y - 1, ground);
	if(y < HEIGHT - 1 && !searched[pos + WIDTH] && equscrgnd(pos + WIDTH)) cgeFloodPos(x, y + 1, ground);
	if(x > 0 && !searched[pos - 1] && equscrgnd(pos - 1)) cgeFloodPos(x - 1, y, ground);
	if(x < WIDTH - 1 && !searched[pos + 1] && equscrgnd(pos + 1)) cgeFloodPos(x + 1, y, ground);
	cgePoint0(pos);
}
#undef equscrgnd

void cgeFlood(int x, int y){
	err("starting flood from %i, ", x);
	err("%i (transformed)\n", y);
	searched = (bool *)calloc(SCRSIZE, sizeof(bool));
	cgeFloodPos(x, y, &screen[x + y * WIDTH]);
	free(searched);
}

void cgeEnableStencil(bool enable){
	stencil_enabled = enable;
}

static void fillStencilRect(int x, int y, int width, int height, bool state){
	if(width < 0) x -= (width = -width);
	if(height < 0) y -= (height = -height);
	register int incr = WIDTH - width, c = 0;
	for(register int i = x + y * WIDTH; i < x + width + (y + height - 1) * WIDTH; ++i){
		stencil[i] = state;
		if(++c == width){
			c = 0;
			i += incr;
		}
	}
}

void cgeStencil(int x, int y, int width, int height){
	fillStencilRect(x, y, width, height, true);
}

void cgeClearStencil(int x, int y, int width, int height){
	fillStencilRect(x, y, width, height, false);
}

TEXTURE *cgeLoadTextureFile(char *file){
	err("loading texture\n", 0);
	FILE *stream = fopen(file, "rb");
	if(stream == NULL){
		err("NULL!!", 0);
		return NULL;
	}
	void *magic = malloc(4);
	fread(magic, 4, 1, stream);
	if(memcmp(magic, MAGIC, 4)){
		fclose(stream);
		err("could not validate file\n", 0);
		return NULL;
	}
	free(magic);
	err("validated file via magic number\n", 0);
	TEXTURE *texture = malloc(sizeof(TEXTURE));
	fread((void *)&texture->width, sizeof(int), 1, stream);
	fread((void *)&texture->height, sizeof(int), 1, stream);
	err("read width %i ", texture->width);
	err("and height %i\n", texture->height);
	register int size = texture->width * texture->height;
	texture->pixels = malloc(size * sizeof(CHAR_INFO));
	err("loading pixels\n", 0);
	for(int i = 0; i < size; ++i){
		texture->pixels[i].Char.AsciiChar = fgetc(stream);
		texture->pixels[i].Attributes = fgetc(stream);
	}
	fclose(stream);
	err("loaded texture\n", 0);
	return texture;
}

TEXTURE *cgeLoadTexture(char pixels[], int width, int height, bool smallpixels){
	TEXTURE *texture = malloc(sizeof(TEXTURE));
	texture->width = width;
	if(smallpixels){
		if(height & 1) return NULL;
		height >>= 1;
		for(int y = 0; y < height; y += 2){
			for(int x = 0; x < width; ++x){
				register int pos = x + width * y;
				register int tpos = x + ((pos - x) >> 1);
				texture->pixels[tpos].Char.AsciiChar = 0xDC;
				texture->pixels[tpos].Attributes = pixels[pos] << 4 | (pixels[pos + width] & 0xF);
			}
		}
	}
	else{
		texture->height = height;
		texture->pixels = malloc(width * height);
		for(int pos = 0; pos < width * height; ++pos){
			texture->pixels[pos].Char.AsciiChar = 0xDB;
			texture->pixels[pos].Attributes = 0xF0 | pixels[pos];
		}
	}
	return texture;
}

void cgeSaveTexture(char *file, TEXTURE *texture){
	err("saving texture with size %i, ", texture->width);
	err("%i\n", texture->height);
	FILE *stream = fopen(file, "wb");
	if(stream == NULL) return;
	fwrite(MAGIC, 4, 1, stream);
	fwrite((const void *)&texture->width, sizeof(int), 1, stream);
	fwrite((const void *)&texture->height, sizeof(int), 1, stream);
	for(int i = 0; i < texture->width * texture->height; ++i){
		fputc(texture->pixels[i].Char.AsciiChar, stream);
		fputc(texture->pixels[i].Attributes, stream);
	}
	fclose(stream);
	err("saved texture\n", 0);
}

void cgeDrawTexture(TEXTURE *texture, int x0, int y0){
	err("starting to draw texture\n", 0);
	for(register int x = 0; x < texture->width; ++x){
		register int scrx = x0 + x;
		if(scrx < 0 || scrx >= WIDTH) continue;
		for(register int y = 0; y < texture->height; ++y){
			register int scry = y0 + y;
			register int pos = scrx + scry * WIDTH;
			if(scry < 0 || scry >= HEIGHT || (stencil_enabled && !stencil[pos])) continue;
			screen[pos] = texture->pixels[x + y * texture->width];
		}
	}
	err("drew texture\n", 0);
}

void cgeDrawTexturePart(TEXTURE *texture, int tx, int ty, int width, int height, int x0, int y0){
	if(width > texture->width || height > texture->height){
		err("part is too big", 0);
		return;
	}
	err("starting to draw texture\n", 0);
	for(register int x = tx; x < width; ++x){
		register int scrx = x0 + x;
		if(scrx < 0 || scrx >= WIDTH) continue;
		for(register int y = ty; y < height; ++y){
			register int scry = y0 + y;
			register int pos = scrx + scry * WIDTH;
			if(scry < 0 || scry >= HEIGHT || (stencil_enabled && !stencil[pos])) continue;
			screen[pos] = texture->pixels[x + y * texture->width];
		}
	}
	err("drew texture\n", 0);
}

TEXTURE *cgeStoreTexture(int x, int y, int width, int height){
	err("storing texture from %i, ", x);
	err("%i\n", y);
	TEXTURE *texture = malloc(sizeof(TEXTURE));
	texture->width = width;
	texture->height = height;
	texture->pixels = malloc(width * height * sizeof(CHAR_INFO));
	for(register int tx = 0; tx < width; ++tx){
		register int scrx = x + tx;
		if(scrx < 0 || scrx >= WIDTH) continue;
		for(register int ty = 0; ty < height; ++ty){
			register int scry = y + ty;
			register int pos = scrx + scry * WIDTH;
			if(scry < 0 || scry >= HEIGHT || (stencil_enabled && !stencil[pos])) continue;
			texture->pixels[tx + ty * width] = screen[pos];
		}
	}
	err("stored texture\n", 0);
	return texture;
}

void cgeDeleteTexture(TEXTURE *texture){
	err("freeing pixels\n", 0);
	free(texture->pixels);
	err("freeing texture struct\n", 0);
	free(texture);
}

void cgeBindFragmentShader(FRAGMENT_SHADER *shader){
	fragment_shader = shader;
}

void cgeApplyShader(FRAGMENT_SHADER *shader){
	if(shader == NULL) return;
	int pos = 0;
	char cparts[2];
	bool paint;
	for(int y = 0; y < HEIGHT; ++y){
		for(int x = 0; x < WIDTH; ++x){
			register int color = screen[pos].Attributes;
			cparts[0] = (color & 15) ^ 8;
			cparts[1] = (color >> 4) ^ 8;
			paint = stencil[pos];
			(*shader->fragment)(x, y, cparts, &paint);
			if(paint) screen[pos].Attributes = (cparts[1] << 4 | cparts[0]) ^ 0x88;
			++pos;
		}
	}
}

void cgeSwapBuffers(void){
	WriteConsoleOutputA(hstdout, screen, size, ZERO, &bounds);
}

void cgeClear(void){
	for(int i = 0; i < SCRSIZE; ++i){
		screen[i].Char.AsciiChar = ' ';
		screen[i].Attributes = color;
	}
}

void cgePushState(void){
	statestack[stacksize++] = color;
	statestack[stacksize++] = filler;
	statestack[stacksize++] = stencil_enabled;
	realloc(statestack, stacksize * sizeof(int));
}

void cgePopState(void){
	stencil_enabled = statestack[--stacksize];
	filler = statestack[--stacksize];
	color = statestack[--stacksize];
	realloc(statestack, stacksize * sizeof(int));
}
