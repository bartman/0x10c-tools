#ifndef __included_0x10c_isn_h__
#define __included_0x10c_isn_h__
#include <stdint.h>
#include <unistd.h>

#include "0x10c_def.h"
#include "0x10c_op.h"

#define X10C_OP_NAME_MAX 16

struct x10c_isn;
struct x10c_vcpu;

struct x10c_isn_ops {
	/**
	 * parses an instruction from text to binary
	 */
	int (*parser)(const struct x10c_isn *isn,
			x10c_op_t *op,
			const char *buf, size_t buf_len);

	/**
	 * generates a text representation of an instruction
	 */
	char * (*generator)(const struct x10c_isn *isn,
			char *buf, size_t buf_len,
			const x10c_op_t *op);

	/**
	 * execute an instruction on the vcpu
	 */
	int (*execute)(const struct x10c_isn *isn,
			const x10c_op_t *op,
			struct x10c_vcpu *vcpu);
};

struct x10c_isn {
	const char op_name[X10C_OP_NAME_MAX];
	uint8_t    op_code;
	uint8_t    ext_op_code;
	uint8_t    num_args;

	struct x10c_isn_ops ops;
};

extern struct x10c_isn x10c_basic_isns[X10C_OP_MAX];
extern struct x10c_isn x10c_non_basic_isns[X10C_XOP_MAX];

static inline struct x10c_isn * x10c_lookup_isn_for_op(const x10c_op_t *op)
{
	if (x10c_op_is_basic(op)) {
		if (op->b.op < X10C_OP_MAX)
			return &x10c_basic_isns[op->b.op];
	} else {
		if (op->x.op < X10C_XOP_MAX)
			return &x10c_non_basic_isns[op->b.op];
	}
	return NULL;
}


#endif // __included_0x10c_isn_h__
