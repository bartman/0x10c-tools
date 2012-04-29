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

	int (*run) (struct dcpu_vcpu *vcpu);

	void (*delete) (struct dcpu_vcpu *vcpu);

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
			dcpu_word n[4];
			struct {
				dcpu_word sp, pc, ex, ia;
			};
		} sr;

		unsigned skipping:1; // skipping IFx instructions + one non-IFx
		unsigned int_mask:1; // when set interrupts don't trigger

	} st;

	DECLARE_DCPU_FIFO(interrupts, DCPU_MAX_INT_QUEUED);

	uint32_t    hz; // clock cycles / second

	dcpu_word   hw_count;
	struct list hw_list;


#define DCPU_VCPU_DEBUG_INIT      0 // initialize the debugger
#define DCPU_VCPU_DEBUG_ISN_DONE  1 // instruction finished executing
#define DCPU_VCPU_DEBUG_INTERRUPT 2 // interrupt changed PC
	int (*debug_callback)(struct dcpu_vcpu *vcpu, int what);

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

static inline void dcpu_vcpu_set_debug(struct dcpu_vcpu *vcpu,
		int (*debug_callback)(struct dcpu_vcpu *vcpu, int what))
{
	vcpu->debug_callback = debug_callback;
}

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
