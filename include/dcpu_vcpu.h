#ifndef __included_dcpu_vcpu_h__
#define __included_dcpu_vcpu_h__

#include <stdio.h>

#include "dcpu_def.h"
#include "dcpu_op.h"
#include "dcpu_fifo.h"
#include "list.h"

struct dcpu_vcpu;
struct dcpu_vcpu_state;
struct dcpu_hw;

struct dcpu_vcpu_ops {

	int (*step) (struct dcpu_vcpu *vcpu);

	void (*delete) (struct dcpu_vcpu *vcpu);

};

struct dcpu_vcpu_debug_ops {
	// instruction executed
	int (*post_isn)(struct dcpu_vcpu *);

	// interrupt changed PC
	int (*post_int)(struct dcpu_vcpu *);

	// halted
	int (*halt)(struct dcpu_vcpu *);
};

struct dcpu_vcpu {
	struct dcpu_vcpu_state {
		dcpu_cycles_t cycles;

		union {
			dcpu_word n[DCPU_NUM_REGS];
			struct {
				dcpu_word a, b, c, x, y, z, i, j;
			};
		} gr;
		union {
			dcpu_word n[DCPU_NUM_SREGS];
			struct {
				dcpu_word pc, sp, ex, ia;
			};
		} sr;

		dcpu_word hcf_code;

		unsigned skipping:1; // skipping IFx instructions + one non-IFx
		unsigned int_mask:1; // when set interrupts don't trigger
		unsigned halted:1;   // set when HCF was issued, hcf_code is set

	} st;

	DECLARE_DCPU_FIFO(interrupts, DCPU_MAX_INT_QUEUED);

	uint32_t    hz; // clock cycles / second

	dcpu_word   hw_count;
	struct list hw_list;

	struct dcpu_vcpu_debug_ops *debug_ops;

	struct dcpu_vcpu_ops ops;

	dcpu_word ram[DCPU_RAM_WORDS];
};

// create a new vcpu
extern struct dcpu_vcpu * dcpu_vcpu_new(void);

#define dcpu_vcpu_cycles(vcpu)  ((vcpu)->st.cycles)
#define dcpu_vcpu_gr(vcpu)      (&(vcpu)->st.gr)
#define dcpu_vcpu_sr(vcpu)      (&(vcpu)->st.sr)
#define dcpu_vcpu_ram(vcpu)     (&(vcpu)->ram)

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

// assign a debug ops, or NULL to reset
extern void dcpu_vcpu_set_debug_ops(struct dcpu_vcpu *vcpu,
		struct dcpu_vcpu_debug_ops *dbg);

// add and remove a piece of hardware (return 0 on success)
extern int dcpu_vcpu_register_hw(struct dcpu_vcpu *vcpu, struct dcpu_hw *hw);
extern int dcpu_vcpu_unregister_hw(struct dcpu_vcpu *vcpu, struct dcpu_hw *hw);

// find a piece of hardware by ID
extern struct dcpu_hw *dcpu_vcpu_find_hw(struct dcpu_vcpu *vcpu, dcpu_word id);

// copy to/from ram with wraparound
extern void dcpu_vcpu_copy_to_ram(struct dcpu_vcpu *vcpu,
		dcpu_word ram_dst, const void *src, dcpu_word len);
extern void dcpu_vcpu_copy_from_ram(const struct dcpu_vcpu *vcpu,
		void *dst, dcpu_word ram_src, dcpu_word len);


#endif // __included_dcpu_vcpu_h__
