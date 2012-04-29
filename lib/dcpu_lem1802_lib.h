#ifndef __included_dcpu_lem1802_lib_h__
#define __included_dcpu_lem1802_lib_h__

#include <curses.h>

#include "dcpu_def.h"
#include "dcpu_lem1802.h"

struct dcpu_lem1802 {
	struct dcpu_hw hw;

	dcpu_word video_ram_base;
	dcpu_word font_ram_base;
	dcpu_word palette_ram_base;
	dcpu_word border_color;

	WINDOW *win;
};

static inline struct dcpu_lem1802 * dcpu_hw_to_lem1802(struct dcpu_hw *hw)
{
	return container_of(hw, struct dcpu_lem1802, hw);
}

// the actual curses display
#define LEM1802_SCREEN_COLS   (DCPU_LEM1802_SCREEN_COLS + 4)
#define LEM1802_SCREEN_LINES  (DCPU_LEM1802_SCREEN_LINES + 4)

// video ram is made up of these cells...
union lem1802_video_cell {
	// ffffbbbbBccccccc
	dcpu_word word;
	struct {
		dcpu_word ch:7; // character
		dcpu_word bl:1; // blink
		dcpu_word bg:4; // background colour
		dcpu_word fg:4; // foreground colour
	};
};

// font ram is made up of these cells...
union lem1802_font_cell {
	// each font entry is defined 4x8 matrix of bits and stored as 4 octets
	struct {
		dcpu_word word[2];
	};
	struct {
		uint8_t octet[4];
	};
};

// palette ram is made up of these cells...
union lem1802_palette_cell {
	// 0000rrrrggggbbbb
	dcpu_word word;
	struct {
		dcpu_word b:4; // blue
		dcpu_word r:4; // red
		dcpu_word g:4; // green
		dcpu_word zero:4;
	};
};

#endif // __included_dcpu_lem1802_lib_h__
