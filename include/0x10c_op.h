#ifndef __included_0x10c_op_h__
#define __included_0x10c_op_h__
#include <stdint.h>
#include <stdbool.h>

#include "0x10c_def.h"

typedef union x10c_op {
	struct {
		x10c_word word[3];
	};
	struct {
		// basic encoding: bbbbbbaaaaaaoooo
		x10c_word op:4;
		x10c_word a:6;
		x10c_word b:6;
	} b;
	struct {
		// non-basic encoding: aaaaaaoooooo0000
		x10c_word zero:4;
		x10c_word op:6;
		x10c_word a:6;
	} x;
} x10c_op_t;

static inline bool x10c_op_is_basic(const x10c_op_t *op)
{
	return op->b.op != X10C_OP_NON_BASIC;
}




#endif // __included_0x10c_op_h__
