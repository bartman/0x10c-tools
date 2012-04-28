#ifndef __included_dcpu_hw_h__
#define __included_dcpu_hw_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "list.h"

struct dcpu_vcpu;
struct dcpu_hw;

struct dcpu_hw_ops {
	// if non-null vcpu will poke this hardware after it executes every
	// instruction
	int (*poke) (struct dcpu_hw *hw);

	// when a hardware interrupt is generated for this hardware,
	// vcpu call this function
	int (*handle_interrupt) (struct dcpu_hw *hw);

	// delete the hardware
	// called by vcpu->ops.delete() and dcpu_vcpu_unregister_hw()
	void (*delete) (struct dcpu_hw *hw);
};

struct dcpu_hw {
	struct list link;

	uint16_t hw_id;
	uint16_t hw_version;

	uint32_t hw_vendor_id;
	uint32_t hw_device_id;

	struct dcpu_vcpu *vcpu;

	struct dcpu_hw_ops ops;
};

static inline void dcpu_hw_init(struct dcpu_hw *hw)
{
	// nothing interesting really
	hw->link = (struct list){0,0};
	hw->hw_id = -1;
	hw->hw_version = -1;
	hw->hw_vendor_id = -1;
	hw->hw_device_id = -1;
	hw->vcpu = NULL;
}

#endif // __included_dcpu_hw_h__
