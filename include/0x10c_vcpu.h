#ifndef __included_0x10c_vcpu_h__
#define __included_0x10c_vcpu_h__

#include "0x10c_def.h"
#include "0x10c_op.h"

struct x10c_vcpu {
	union {
		x10c_word n[X10C_NUM_REGS];
		struct {
			x10c_word a, b, c, x, y, z, i, j;
		};
	} gr;
	struct {
		x10c_word pc, sp;
		x10c_word o:1;
	} sr;

	x10c_word ram[X10C_RAM_WORDS];
};

#endif // __included_0x10c_vcpu_h__
