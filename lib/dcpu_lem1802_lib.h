#ifndef __included_dcpu_lem1802_lib_h__
#define __included_dcpu_lem1802_lib_h__

#define DCPU_LEM1802_SCREEN_COLS 32
#define DCPU_LEM1802_SCREEN_LINES 12
#define DCPU_LEM1802_VIDEO_RAM_SIZE \
	(DCPU_LEM1802_SCREEN_COLS * DCPU_LEM1802_SCREEN_LINES)
#define DCPU_LEM1802_FONT_RAM_SIZE 256
#define DCPU_LEM1802_PALETTE_RAM_SIZE 16

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

union lem1802_font_cell {
	// each font entry is defined 4x8 matrix of bits and stored as 4 octets
	struct {
		dcpu_word word[2];
	};
	struct {
		uint8_t octet[4];
	};
};

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
