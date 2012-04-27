#ifndef __included_0x10c_vcpu_h__
#define __included_0x10c_vcpu_h__

#include <stdio.h>

#include "0x10c_def.h"
#include "0x10c_op.h"

struct x10c_vcpu;
struct x10c_vcpu_state;

struct x10c_vcpu_ops {

	int (*step) (struct x10c_vcpu *vcpu);

	int (*run) (struct x10c_vcpu *vcpu);

	void (*dump_oneline)(struct x10c_vcpu *vcpu,
			const struct x10c_vcpu_state *, FILE *out);

	void (*delete) (struct x10c_vcpu *vcpu);

};

struct x10c_vcpu {
	struct x10c_vcpu_state {
		union {
			x10c_word n[X10C_NUM_REGS];
			struct {
				x10c_word a, b, c, x, y, z, i, j;
			};
		} gr;
		union {
			x10c_word n[4];
			struct {
				x10c_word sp, pc, ex, ia;
			};
		} sr;

		unsigned skipping:1; // skipping IFx instructions + one non-IFx
	} st;

	struct x10c_vcpu_ops ops;

	x10c_word ram[X10C_RAM_WORDS];
};

// return current pc word, advancing to the next one
static inline x10c_word x10c_vcpu_advance_pc_word(struct x10c_vcpu *vcpu)
{
	return vcpu->ram[ vcpu->st.sr.pc ++ ];
}

// return the op code pointer of the current instruction
static inline x10c_op_t * x10c_vcpu_current_op(struct x10c_vcpu *vcpu)
{
	return (void*)&vcpu->ram[ vcpu->st.sr.pc ];
}

// copy state from current to previous
static inline void x10c_vcpu_backup_state(const struct x10c_vcpu *vcpu,
		struct x10c_vcpu_state *pst)
{
	*pst = vcpu->st;
}

extern struct x10c_vcpu * x10c_vcpu_new(void);


#endif // __included_0x10c_vcpu_h__
