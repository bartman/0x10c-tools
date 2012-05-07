#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "dcpu_vcpu.h"
#include "dcpu_def.h"
#include "dcpu_hw.h"
#include "dcpu_isn.h"
#include "dcpu_util.h"
#include "dcpu_generator.h"
#include "dcpu_colours.h"

#if 1
#define dbg(f,a...) do { /* nothing */ } while(0)
#else
#define dbg(f,a...) printf(f,##a)
#endif

// ------------------------------------------------------------------------
// debug ops

#define DEFINE_DEBUGGER_WRAPPER(callback)                          \
static inline int dcpu_debugger_##callback(struct dcpu_vcpu *vcpu) \
{                                                                  \
	if (vcpu->debug_ops)                                       \
		return vcpu->debug_ops->callback(vcpu);            \
	return 0;                                                  \
}

DEFINE_DEBUGGER_WRAPPER(post_isn);
DEFINE_DEBUGGER_WRAPPER(post_int);

void dcpu_vcpu_set_debug_ops(struct dcpu_vcpu *vcpu,
		struct dcpu_vcpu_debug_ops *debug_ops)
{
	vcpu->debug_ops = debug_ops;
}


// ------------------------------------------------------------------------
// instruction decoder

static dcpu_word * dcpu_vcpu_get_isn_arg(struct dcpu_vcpu *vcpu,
		dcpu_word arg_desc, dcpu_word *scratch, int is_b)
{
	dcpu_word ofs;

	switch(arg_desc) {
	case DCPU_REG_A ... DCPU_REG_J:
		/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
		return &vcpu->st.gr.n[arg_desc];

	case DCPU_ARG_MREF_REG(A) ... DCPU_ARG_MREF_REG(J):
		/* 0x08-0x0f: [register] */
		ofs = vcpu->st.gr.n[arg_desc - DCPU_ARG_MREF_REG_BASE];
		return vcpu->ram + ofs;

	case DCPU_ARG_MREF_OFS_REG(A) ... DCPU_ARG_MREF_OFS_REG(J):
		/* 0x10-0x17: [next word + register] */
		ofs = vcpu->st.gr.n[arg_desc - DCPU_ARG_MREF_OFS_REG_BASE];
		ofs += dcpu_vcpu_advance_pc_word(vcpu);
		return vcpu->ram + ofs;

	case DCPU_REG_PUSH_POP:
		if (is_b) {
			// this is a push
			ofs = -- vcpu->st.sr.sp;
			return vcpu->ram + ofs;
		} else {
			// this is a pop
			ofs = vcpu->st.sr.sp ++;
			return vcpu->ram + ofs;
		}

	case DCPU_REG_PEEK:
		ofs = vcpu->st.sr.sp;
		return vcpu->ram + ofs;

	case DCPU_REG_PICK:
		die("PICK not yet implemented");
		ofs = vcpu->st.sr.sp;
		return vcpu->ram + ofs;

	case DCPU_REG_SP:
		return &vcpu->st.sr.sp;
	case DCPU_REG_PC:
		return &vcpu->st.sr.pc;
	case DCPU_REG_EX:
		return &vcpu->st.sr.ex;

	case DCPU_MREF_NEXT_WORD:
		ofs = dcpu_vcpu_advance_pc_word(vcpu);
		return vcpu->ram + ofs;

	case DCPU_NEXT_WORD:
		*scratch = dcpu_vcpu_advance_pc_word(vcpu);
		return scratch;

	default:
		*scratch = arg_desc - DCPU_ARG_LITERAL_BASE - 1;
		return scratch;
	}
}

#define dcpu_vcpu_debug_callback(vcpu,cb) ({                        \
		int rc = 0;                                         \
		if ( (vcpu)->debugger )                             \
		        rc = (vcpu)->debugger->cb(vcpu);            \
		rc; })

static int dcpu_vcpu_handle_interrupts(struct dcpu_vcpu *vcpu)
{
	dcpu_word msg;
	int rc;

	// do we have pending interrupts?
	if (! dcpu_fifo_get(&vcpu->interrupts, &msg))
		return 0;

	// are we masking interrupts?
	if (vcpu->st.int_mask)
		return 0;

	// do we have an interrupt handler?
	if (!vcpu->st.sr.ia)
		return 0;

	vcpu->st.int_mask = 1;

	dcpu_vcpu_push(vcpu, vcpu->st.sr.pc);
	dcpu_vcpu_push(vcpu, vcpu->st.gr.a);

	vcpu->st.sr.pc = vcpu->st.sr.ia;
	vcpu->st.gr.a = msg;

	rc = dcpu_debugger_post_int(vcpu);

	return rc;
}

static void dcpu_vcpu_poke_hardware(struct dcpu_vcpu *vcpu)
{
	struct dcpu_hw *hw;

	list_for_each_entry(hw, &vcpu->hw_list, link) {
		if (hw->ops.poke)
			hw->ops.poke(hw);
	}
}

static int dcpu_vcpu_step (struct dcpu_vcpu *vcpu)
{
	int rc;
	dcpu_op_t *op;
	struct dcpu_isn *isn;
	dcpu_word *a, *b;
	dcpu_word tmp_a, tmp_b;
	int cycles;

	op = dcpu_vcpu_current_op(vcpu);

	isn = dcpu_lookup_isn_for_op(op);
	if (!isn)
		die("not a valid instruction 0x%04x", op->word[0]);

	dbg("# step pc=%04x %s\n", vcpu->st.sr.pc, isn->op_name);

	// consume this word
	vcpu->st.sr.pc ++;

	if (dcpu_op_is_basic(op)) {
		a = dcpu_vcpu_get_isn_arg(vcpu, op->b.a, &tmp_a, 0);
		b = dcpu_vcpu_get_isn_arg(vcpu, op->b.b, &tmp_b, 1);
	} else {
		a = dcpu_vcpu_get_isn_arg(vcpu, op->x.a, &tmp_a, 0);
		b = NULL;
	}

	cycles = dcpu_op_cycles(op, isn);
	if (cycles > 0)
		vcpu->st.cycles += cycles;

	if (!vcpu->st.skipping) {
		rc = isn->ops.execute(isn, op, a, b, vcpu);
		if (rc<0)
			return rc;
		if (rc>0)
			vcpu->st.cycles += rc;

	} else if (!isn->is_conditional) {
		// this was the last one we're gonna skip
		vcpu->st.skipping = 0;
	}

	rc = dcpu_debugger_post_isn(vcpu);
	if (rc)
		return rc;

	rc = dcpu_vcpu_handle_interrupts(vcpu);
	if (rc)
		return rc;

	dcpu_vcpu_poke_hardware(vcpu);
	// TODO: may need to call debugger if hw changed stuff

	return 0;
}

static void dcpu_vcpu_delete (struct dcpu_vcpu *vcpu)
{
	free(vcpu);
}

static struct dcpu_vcpu_ops dcpu_vcpu_ops = {
	.step   = dcpu_vcpu_step,
	.delete = dcpu_vcpu_delete,
};

struct dcpu_vcpu * dcpu_vcpu_new(void)
{
	struct dcpu_vcpu *vcpu;

	vcpu = calloc(1, sizeof(*vcpu));

	dcpu_fifo_init(&vcpu->interrupts, DCPU_MAX_INT_QUEUED);

	vcpu->hz = DCPU_HZ;

	vcpu->hw_count = 0;
	list_init(&vcpu->hw_list);

	vcpu->ops = dcpu_vcpu_ops;

	return vcpu;
}

// ------------------------------------------------------------------------

// add and remove a piece of hardware
int dcpu_vcpu_register_hw(struct dcpu_vcpu *vcpu, struct dcpu_hw *hw)
{
	// uninitialized or already on the list
	if (hw->link.next)
		return -EBUSY;

	// must have a delete callback
	if (!hw->ops.delete)
		return -EINVAL;

	// must have one of poke or handle_interrupt callbacks
	if (!hw->ops.poke && !hw->ops.handle_interrupt)
		return -EINVAL;

	hw->vcpu = vcpu;
	hw->hw_id = vcpu->hw_count ++;
	list_add_tail(&hw->link, &vcpu->hw_list);

	return 0;
}

// add and remove a piece of hardware
int dcpu_vcpu_unregister_hw(struct dcpu_vcpu *vcpu, struct dcpu_hw *hw)
{
	// not attached
	if (!hw->link.next)
		return -EFAULT;

	// maybe not ours?
	if (hw->vcpu != vcpu)
		return -ENXIO;

	// detach
	vcpu->hw_count --;
	list_del(&hw->link);

	// reset id
	hw->hw_id = -1;

	// detach
	hw->vcpu = NULL;

	if (!hw->ops.delete)
		return -EFAULT;

	hw->ops.delete(hw);
	return 0;
}

// find a piece of hardware by ID
struct dcpu_hw *dcpu_vcpu_find_hw(struct dcpu_vcpu *vcpu, dcpu_word id)
{
	struct dcpu_hw *hw;

	list_for_each_entry(hw, &vcpu->hw_list, link) {
		if (hw->hw_id == id)
			return hw;
	}

	return NULL;
}


void dcpu_vcpu_copy_to_ram(struct dcpu_vcpu *vcpu,
		dcpu_word ram_dst, const void *src, dcpu_word len)
{
	uint fits;
	uint copy;

	fits = (uint)DCPU_RAM_WORDS - ram_dst;

	// copy what fits before the end of ram
	copy = len < fits ? len : fits;

	memcpy(dcpu_vcpu_ram(vcpu) + ram_dst,
			src, sizeof(dcpu_word) * copy);

	// if overflow, copy rest to start of ram
	if (copy < len)
		memcpy(dcpu_vcpu_ram(vcpu),
				src + sizeof(dcpu_word) * copy,
				len - copy);
}

void dcpu_vcpu_copy_from_ram(const struct dcpu_vcpu *vcpu,
		void *dst, dcpu_word ram_src, dcpu_word len)
{
	uint fits;
	uint copy;

	fits = (uint)DCPU_RAM_WORDS - ram_src;

	// copy what fits before the end of ram
	copy = len < fits ? len : fits;

	memcpy(dst, dcpu_vcpu_ram(vcpu) + ram_src,
			sizeof(dcpu_word) * copy);

	// if overflow, copy rest from start of ram
	if (copy < len)
		memcpy(dst + sizeof(dcpu_word) * copy,
				dcpu_vcpu_ram(vcpu),
				len - copy);
}

