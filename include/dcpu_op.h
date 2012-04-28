#ifndef __included_dcpu_op_h__
#define __included_dcpu_op_h__
#include <stdint.h>
#include <stdbool.h>

#include "dcpu_def.h"

typedef union dcpu_op {
	struct {
		dcpu_word word[DCPU_MAX_OP_LEN];
	};
	struct {
		// basic encoding: aaaaaabbbbbooooo
		dcpu_word op:5;
		dcpu_word b:5;
		dcpu_word a:6;
	} b;
	struct {
		// non-basic encoding: aaaaaaooooo00000
		dcpu_word zero:5;
		dcpu_word op:5;
		dcpu_word a:6;
	} x;
} dcpu_op_t;

static inline bool dcpu_op_is_basic(const dcpu_op_t *op)
{
	return op->b.op != DCPU_OP_NON_BASIC;
}

static inline unsigned dcpu_op_arg_len(unsigned arg_desc)
{
	switch(arg_desc) {
	default:
		return 0;

	case DCPU_ARG_MREF_OFS_REG(A) ... DCPU_ARG_MREF_OFS_REG(J):
	case DCPU_REG_PICK:
	case DCPU_MREF_NEXT_WORD:
	case DCPU_NEXT_WORD:
		return 1;
	}
}

static inline unsigned dcpu_op_len(const dcpu_op_t *op)
{
	if (dcpu_op_is_basic(op)) {
		return 1 + dcpu_op_arg_len(op->b.a)
			 + dcpu_op_arg_len(op->b.b);
	} else {
		return 1 + dcpu_op_arg_len(op->x.a);
	}
}

/* return cycle cost of an argument */
static inline int dcpu_op_arg_cycles(unsigned arg_desc)
{
	switch(arg_desc) {
	default:
		return 0;

	case DCPU_ARG_MREF_OFS_REG(A) ... DCPU_ARG_MREF_OFS_REG(J):
	case DCPU_REG_PICK:
	case DCPU_MREF_NEXT_WORD:
	case DCPU_NEXT_WORD:
		return 1;
	}
}


#endif // __included_dcpu_op_h__
