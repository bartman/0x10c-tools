#ifndef __included_dcpu_keyboard_h__
#define __included_dcpu_keyboard_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "dcpu_hw.h"
#include "dcpu_fifo.h"

struct dcpu_vcpu;

#define DCPU_KEYBOARD_VENDOR_ID 0
#define DCPU_KEYBOARD_DEVICE_ID 0x30cf7406
#define DCPU_KEYBOARD_VERSION   1

#define DCPU_KEYBOARD_CMD_CLEAR   0  // clear keyboard buffer
#define DCPU_KEYBOARD_CMD_GETKEY  1  // read key in buffer into C
#define DCPU_KEYBOARD_CMD_TESTKEY 2  // if B is pushed, set C to 1
#define DCPU_KEYBOARD_CMD_IE      3  // interrupt enable

#define DCPU_MAX_KEYS_QUEUED 16

struct dcpu_keyboard {
	struct dcpu_hw hw;

	// they fifo
	DECLARE_DCPU_FIFO(buffer, DCPU_MAX_KEYS_QUEUED);

	dcpu_word int_message;          // 0:disable, else int message

};

static inline struct dcpu_keyboard * dcpu_hw_to_keyboard(struct dcpu_hw *hw)
{
	return container_of(hw, struct dcpu_keyboard, hw);
}

// create a new keyboard and return it
extern struct dcpu_keyboard * dcpu_keyboard_new(void);

// create a new keyboard and register it with a vcpu
// returns the hw_id or negative on error
extern int dcpu_add_keyboard(struct dcpu_vcpu *vcpu);



#endif // __included_dcpu_keyboard_h__
