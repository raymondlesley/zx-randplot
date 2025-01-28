#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arch/zx.h>
//#include <arch/zx/spectrum.h>

#include "zxlib/types.h"

// -- ---------------------------------------------------------------------- --

/*
// get_pixel_address
//
// get screen memory location of specified point in pixels
// NB: x_pixels interpreted at byte boundary i.e. character position
//
// from: http://www.breakintoprogram.co.uk/hardware/computers/zx-spectrum/screen-memory-layout
const char *get_pixel_address(ubyte x_pixels, ubyte y_pixels);  // declaration
const char *get_pixel_address(ubyte x_pixels, ubyte y_pixels)   // definition
{
	const char *address = (char *)0x4000;  // start with screen base
	ubyte y2y1y0 = y_pixels & 0x07;  // bottom 3 bits
	ubyte y7y6 = (y_pixels >> 3) & 0x18;  // top 2 bits shifted to bits 4, 3
	ubyte y5y4y3 = (y_pixels << 2) & 0xE0;
	ubyte x4x3x2x1x0 = (x_pixels >> 3) & 0x1F;  // top 5 bits shifted to bottom 5 bits
	//address |= ((uword)(y7y6 | y2y1y0) << 8 | (x4x3x2x1x0 | y5y4y3));
	return address + ((uword)(y7y6 | y2y1y0) << 8 | (x4x3x2x1x0 | y5y4y3));
}
*/

// get_pixel_address
//
// get screen memory location of specified point in character positions
//
// inspired by: http://www.breakintoprogram.co.uk/hardware/computers/zx-spectrum/screen-memory-layout
const char *get_char_address(ubyte x_chars, ubyte y_chars);  // declaration
const char *get_char_address(ubyte x_chars, ubyte y_chars)   // definition
{
	// const char *address = (char *)0x4000;  // start with screen base
	// const ubyte y7y6 = y_chars & 0x18;
	// const ubyte y5y4y3 = (y_chars<< 5) & 0xE0;
	// const ubyte x4x3x2x1x0 = x_chars& 0x1F;  // top 5 bits shifted to bottom 5 bits
	// //address += ((uword)(y7y6) << 8 | (x4x3x2x1x0 | y5y4y3));
	// return address + ((uword)(y7y6) << 8 | (x4x3x2x1x0 | y5y4y3));
	return zx_cxy2saddr(x_chars, y_chars);
}

static const uword *sys_chars = (uword *)0x5C36;  // 'chars' sytem variable

void print_character_at(ubyte row, ubyte col, char character);
void print_character_at(ubyte row, ubyte col, char character)
{
	const char *chars = (char *)*(sys_chars); // default = 0x3C00
	const uword character_offset = character << 3;  // 8* as each char takes 8 bytes

	char *screen_location = (char *)get_char_address(col, row);
	char *character_location = (char *)(chars + character_offset);

	for (int counter = 0; counter < 8; counter++) {
		// copy byte to screen memory
		*screen_location = *character_location;
		// and move to next pixel row
		screen_location += 0x0100;
		character_location++;
	}
}

void print_string_at(ubyte row, ubyte col, char *text);
void print_string_at(ubyte row, ubyte col, char *text)
{
	const char *chars = (char *)*(sys_chars); // default = 0x3C00

	char character = *(text++);
	uword character_offset = character << 3;  // 8* as each char takes 8 bytes

	char *screen_location = (char *)get_char_address(col, row);
	char *character_location = (char *)(chars + character_offset);

	do {
		char *pixel_row = screen_location;  // start at the top left
		for (int counter = 0; counter < 8; counter++) {
			// copy byte to screen memory
			*pixel_row = *character_location;
			// and move to next pixel row
			pixel_row += 0x0100;
			character_location++;  // next row of pixels
		}
		character = *(text++);  // next character
		character_offset = character << 3;
		character_location = (char *)(chars + character_offset);
		screen_location++;  // step to next character position
	} while (character);  // assume null-terminated
}

void printf_at(ubyte row, ubyte col, char *fmt, ...);
void printf_at(ubyte row, ubyte col, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char buffer[256];

	(void)vsprintf(buffer, fmt, args);
	print_string_at(row, col, buffer);

	va_end(args);
}

void plot(unsigned char x, unsigned char y) {
    *zx_pxy2saddr(x,y) |= zx_px2bitmask(x);
}

// according to documentation, this should work - but goes haywire!
/*
void print_at(unsigned row, unsigned col, const char *message) {
	// printf("%c%c%c%s ", 22, col-1, row-1, message);
	printf("\x16%c%c%s", col-1, row+31, message);
}
*/

/*
int myfunc(char b, unsigned char *p)
{
  #asm
  
  ld hl,2
  add hl,sp              ; skip over return address on stack
  ld e,(hl)
  inc hl
  ld d,(hl)              ; de = p
  inc hl
  ld a,(hl)              ; a = b, "char b" occupies 16 bits on stack
                         ;  but only the LSB is relevant
  
  ld hl,1                ; hl is the return parameter
  
  #endasm
}
*/

// -- ---------------------------------------------------------------------- --

int main(void);
int main(void) {
	int x=128, y=81;
	uword s = 0;
	// char buffer[10];
	int num_colours = 8;
	static const ubyte colours[8] = {INK_BLACK, INK_BLUE, INK_RED, INK_MAGENTA, INK_GREEN, INK_CYAN, INK_YELLOW, INK_WHITE};

	// myfunc(0, buffer);

	zx_cls(INK_BLACK | PAPER_WHITE);
	zx_border(INK_BLACK);
	ubyte border = INK_BLACK;

/*
	// find min and max value from rand()
	int num = 0;
	int min = 32767;
	int max = 0;
	while (s++ <= 32766) {
		int num = rand();
		if (num > max)  max = num;
		if (num < min)  min = num;
	}
	sprintf(buffer, "min=%d", min);
	print_at(0, 0, buffer);
	sprintf(buffer, "max=%d", max);
	print_at(1, 0, buffer);
*/

	s = 0;
	while (s < 65535) {
		s++;
		if (x>0 && x<256 && y>0 && y<192) {
			plot(x, y);
		}
		switch (rand() / 8192) {  // 0, 1, 2 or 3
			case 0: x++; break;
			case 1: x--; break;
			case 2: y++; break;
			case 3: y--; break;
		}
		if (1) { // }((s % 1000) == 0) {
			printf_at(22, 0, "%u ", s);
		}
		if ((s % 2) == 0) {
			border = (++border) % num_colours;
			zx_border(border);
		}
	}

	return(0);
}

// -- ---------------------------------------------------------------------- --
