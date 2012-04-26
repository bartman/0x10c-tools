#ifndef __included_0x10c_op_h__
#define __included_0x10c_op_h__
#include <stdint.h>
#include <stdbool.h>

#include "0x10c_def.h"

typedef union x10c_op {
	struct {
		x10c_word word[X10C_MAX_OP_LEN];
	};
	struct {
		// basic encoding: aaaaaabbbbbooooo
		x10c_word op:5;
		x10c_word b:5;
		x10c_word a:6;
	} b;
	struct {
		// non-basic encoding: aaaaaaooooo00000
		x10c_word zero:5;
		x10c_word op:5;
		x10c_word a:6;
	} x;
} x10c_op_t;

static inline bool x10c_op_is_basic(const x10c_op_t *op)
{
	return op->b.op != X10C_OP_NON_BASIC;
}

static inline unsigned x10c_op_arg_len(unsigned arg_desc)
{
	switch(arg_desc) {
	default:
		return 0;

	case X10C_ARG_MREF_OFS_REG(A) ... X10C_ARG_MREF_OFS_REG(J):
	case X10C_MREF_NEXT_WORD:
	case X10C_NEXT_WORD:
		return 1;
	}
}

static inline unsigned x10c_op_len(const x10c_op_t *op)
{
	if (x10c_op_is_basic(op)) {
		return 1 + x10c_op_arg_len(op->b.a)
			 + x10c_op_arg_len(op->b.b);
	} else {
		return 1 + x10c_op_arg_len(op->x.a);
	}
}




#endif // __included_0x10c_op_h__
