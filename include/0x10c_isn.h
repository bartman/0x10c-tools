#ifndef __included_0x10c_isn_h__
#define __included_0x10c_isn_h__
#include <stdint.h>
#include <unistd.h>

#include "0x10c_def.h"
#include "0x10c_op.h"

#define X10C_OP_NAME_MAX 8
#define X10C_REG_NAME_MAX 8

struct x10c_isn;
struct x10c_vcpu;
struct x10c_parser;

struct x10c_isn_ops {
	/**
	 * parses an instruction from text to binary
	 */
	int (*parser)(struct x10c_parser *pr, const struct x10c_isn *isn,
			x10c_op_t *op, char *buf);

	/**
	 * generates a text representation of an instruction
	 */
	char * (*generator)(const struct x10c_isn *isn,
			char *buf, size_t buf_len,
			const x10c_op_t *op);

	/**
	 * execute an instruction on the vcpu
	 * @return negative on error
	 */
	int (*execute)(const struct x10c_isn *isn,
			const x10c_op_t *op,
			x10c_word *a,
			x10c_word *b,
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

struct x10c_reg {
	const char reg_name[X10C_REG_NAME_MAX];
	uint8_t    reg_num;
	uint8_t    general:1;
};

#define X10C_REGS_MAX 0x20
extern struct x10c_reg x10c_regs[X10C_REGS_MAX];


static inline struct x10c_isn * x10c_lookup_isn_for_op(const x10c_op_t *op)
{
	struct x10c_isn *isn = NULL;
	if (x10c_op_is_basic(op)) {
		if (op->b.op < X10C_OP_MAX)
			isn = &x10c_basic_isns[op->b.op];
	} else {
		if (op->x.op < X10C_XOP_MAX)
			isn = &x10c_non_basic_isns[op->x.op];
	}

	if (isn && isn->op_name[0])
		return isn;

	return NULL;
}

extern struct x10c_isn * x10c_lookup_isn_for_name(const char *name);



static inline struct x10c_reg * x10c_lookup_reg_for_num(unsigned num)
{
	struct x10c_reg *reg = NULL;
	if (num < X10C_REGS_MAX)
		reg = &x10c_regs[num];

	if (reg && reg->reg_name[0])
		return reg;

	return NULL;
}

extern struct x10c_reg * x10c_lookup_reg_for_name(const char *name);



#endif // __included_0x10c_isn_h__
