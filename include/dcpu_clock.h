#ifndef __included_dcpu_clock_h__
#define __included_dcpu_clock_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "dcpu_hw.h"

struct dcpu_vcpu;

#define DCPU_CLOCK_VENDOR_ID 0
#define DCPU_CLOCK_DEVICE_ID 0x12d0b402
#define DCPU_CLOCK_VERSION   1

#define DCPU_CLOCK_CMD_CONF  0  // configure
#define DCPU_CLOCK_CMD_READ  1  // read counter
#define DCPU_CLOCK_CMD_IE    2  // interrupt enable

struct dcpu_clock {
	struct dcpu_hw hw;

	uint32_t cycles_per_tick;       // number of cycles between interrupts
	dcpu_word int_message;          // 0:disable, else int message
	dcpu_word ticks;                // how many times it fired

	dcpu_cycles_t last_conf_cycles; // when was the clock configured
	dcpu_cycles_t last_tick_cycles; // next cycle count to get an interrupt

};

static inline struct dcpu_clock * dcpu_hw_to_clock(struct dcpu_hw *hw)
{
	return container_of(hw, struct dcpu_clock, hw);
}

// create a new clock and return it
extern struct dcpu_clock * dcpu_clock_new(void);

// create a new clock and register it with a vcpu
// returns the hw_id or negative on error
extern int dcpu_add_clock(struct dcpu_vcpu *vcpu);



#endif // __included_dcpu_clock_h__
