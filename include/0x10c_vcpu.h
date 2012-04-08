#ifndef __included_0x10c_vcpu_h__
#define __included_0x10c_vcpu_h__

#include <stdio.h>

#include "0x10c_def.h"
#include "0x10c_op.h"

struct x10c_vcpu;

struct x10c_vcpu_ops {

	int (*step) (struct x10c_vcpu *vcpu);

	int (*run) (struct x10c_vcpu *vcpu);

	void (*dump)(struct x10c_vcpu *vcpu, FILE *out);

	void (*delete) (struct x10c_vcpu *vcpu);

};

struct x10c_vcpu {
	union {
		x10c_word n[X10C_NUM_REGS];
		struct {
			x10c_word a, b, c, x, y, z, i, j;
		};
	} gr;
	union {
		x10c_word n[3];
		struct {
			x10c_word sp, pc, o;
		};
	} sr;

	unsigned skip_next_op:1;

	struct x10c_vcpu_ops ops;

	x10c_word ram[X10C_RAM_WORDS];
};

// return current pc word, advancing to the next one
static inline x10c_word x10c_vcpu_advance_pc_word(struct x10c_vcpu *vcpu)
{
	return vcpu->ram[ vcpu->sr.pc ++ ];
}

// return the op code pointer of the current instruction
static inline x10c_op_t * x10c_vcpu_current_op(struct x10c_vcpu *vcpu)
{
	return (void*)&vcpu->ram[ vcpu->sr.pc ];
}

extern struct x10c_vcpu * x10c_vcpu_new(void);


#endif // __included_0x10c_vcpu_h__
