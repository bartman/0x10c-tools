#ifndef __included_dcpu_vcpu_h__
#define __included_dcpu_vcpu_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "dcpu_op.h"
#include "dcpu_fifo.h"

struct dcpu_vcpu;
struct dcpu_vcpu_state;

struct dcpu_vcpu_ops {

	int (*step) (struct dcpu_vcpu *vcpu);

	int (*run) (struct dcpu_vcpu *vcpu);

	void (*dump_oneline)(struct dcpu_vcpu *vcpu,
			const struct dcpu_vcpu_state *, FILE *out);

	void (*delete) (struct dcpu_vcpu *vcpu);

};

struct dcpu_vcpu {
	struct dcpu_vcpu_state {
		union {
			dcpu_word n[DCPU_NUM_REGS];
			struct {
				dcpu_word a, b, c, x, y, z, i, j;
			};
		} gr;
		union {
			dcpu_word n[4];
			struct {
				dcpu_word sp, pc, ex, ia;
			};
		} sr;

		unsigned skipping:1; // skipping IFx instructions + one non-IFx
		unsigned int_mask:1; // when set interrupts don't trigger

	} st;

	DECLARE_DCPU_FIFO(interrupts, DCPU_MAX_INT_QUEUED);

	struct dcpu_vcpu_ops ops;

	dcpu_word ram[DCPU_RAM_WORDS];
};

// accept an interrupt
static inline void dcpu_vcpu_accept_interrupt(struct dcpu_vcpu *vcpu,
		dcpu_word msg)
{
	dcpu_fifo_put(&vcpu->interrupts, msg);
}

// return current pc word, advancing to the next one
static inline dcpu_word dcpu_vcpu_advance_pc_word(struct dcpu_vcpu *vcpu)
{
	return vcpu->ram[ vcpu->st.sr.pc ++ ];
}

// return the op code pointer of the current instruction
static inline dcpu_op_t * dcpu_vcpu_current_op(struct dcpu_vcpu *vcpu)
{
	return (void*)&vcpu->ram[ vcpu->st.sr.pc ];
}

// copy state from current to previous
static inline void dcpu_vcpu_backup_state(const struct dcpu_vcpu *vcpu,
		struct dcpu_vcpu_state *pst)
{
	*pst = vcpu->st;
}

static inline void dcpu_vcpu_push(struct dcpu_vcpu *vcpu, dcpu_word word)
{
	vcpu->ram [ --vcpu->st.sr.sp ] = word;
}

static inline dcpu_word dcpu_vcpu_pop(struct dcpu_vcpu *vcpu)
{
	return vcpu->ram [ vcpu->st.sr.sp++ ];
}

extern struct dcpu_vcpu * dcpu_vcpu_new(void);


#endif // __included_dcpu_vcpu_h__
