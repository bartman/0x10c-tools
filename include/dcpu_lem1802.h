#ifndef __included_dcpu_lem1802_h__
#define __included_dcpu_lem1802_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "dcpu_hw.h"
#include "dcpu_fifo.h"

struct dcpu_vcpu;
struct dcpu_lem1802;

#define DCPU_LEM1802_VENDOR_ID 0x1c6c8b36 // NYA_ELEKTRISKA
#define DCPU_LEM1802_DEVICE_ID 0x7349f615
#define DCPU_LEM1802_VERSION   0x1802

#define DCPU_LEM1802_CMD_MEM_MAP_SCREEN     0  // maps 386 words from B as video RAM
#define DCPU_LEM1802_CMD_MEM_MAP_FONT       1  // maps 256 words from B as font RAM
#define DCPU_LEM1802_CMD_MEM_MAP_PALETTE    2  // maps 16 words from B as palette RAM
#define DCPU_LEM1802_CMD_SET_BORDER_COLOR   3  // sets border to B & 0xF
#define DCPU_LEM1802_CMD_MEM_DUMP_FONT      4  // copies 386 words of font data to [B]
#define DCPU_LEM1802_CMD_MEM_DUMP_PALETTE   5  // copies 16 words of palette data to [B]

// attributes of the LEM1802
#define DCPU_LEM1802_SCREEN_COLS 32
#define DCPU_LEM1802_SCREEN_LINES 12
#define DCPU_LEM1802_VIDEO_RAM_SIZE \
	(DCPU_LEM1802_SCREEN_COLS * DCPU_LEM1802_SCREEN_LINES)
#define DCPU_LEM1802_FONT_RAM_SIZE 256
#define DCPU_LEM1802_PALETTE_RAM_SIZE 16

// create a new lem1802 and return it
extern struct dcpu_lem1802 * dcpu_lem1802_new(void);

// create a new lem1802 and register it with a vcpu
// returns the hw_id or negative on error
extern int dcpu_add_lem1802(struct dcpu_vcpu *vcpu);



#endif // __included_dcpu_lem1802_h__
