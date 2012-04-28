#ifndef __included_dcpu_isn_h__
#define __included_dcpu_isn_h__
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "dcpu_def.h"
#include "dcpu_op.h"

#define DCPU_OP_NAME_MAX 8
#define DCPU_REG_NAME_MAX 8

struct dcpu_isn;
struct dcpu_vcpu;
struct dcpu_parser;

struct dcpu_isn_ops {
	/**
	 * parses an instruction from text to binary
	 */
	int (*parser)(struct dcpu_parser *pr, const struct dcpu_isn *isn,
			dcpu_op_t *op, char *buf);

	/**
	 * generates a text representation of an instruction
	 */
	char * (*generator)(const struct dcpu_isn *isn,
			char *buf, size_t buf_len,
			const dcpu_op_t *op);

	/**
	 * execute an instruction on the vcpu
	 * @return negative on error
	 */
	int (*execute)(const struct dcpu_isn *isn,
			const dcpu_op_t *op,
			dcpu_word *a,
			dcpu_word *b,
			struct dcpu_vcpu *vcpu);
};

struct dcpu_isn {
	const char op_name[DCPU_OP_NAME_MAX];
	uint8_t    op_code;
	uint8_t    ext_op_code;
	uint8_t    cycles;
	uint8_t    is_conditional:1;

	struct dcpu_isn_ops ops;
};

extern struct dcpu_isn dcpu_basic_isns[DCPU_OP_MAX];
extern struct dcpu_isn dcpu_non_basic_isns[DCPU_XOP_MAX];

struct dcpu_reg {
	const char reg_name[DCPU_REG_NAME_MAX];
	uint8_t    reg_num;
	uint8_t    general:1;
};

#define DCPU_REGS_MAX 0x20
extern struct dcpu_reg dcpu_regs[DCPU_REGS_MAX];


static inline struct dcpu_isn * dcpu_lookup_isn_for_op(const dcpu_op_t *op)
{
	struct dcpu_isn *isn = NULL;
	if (dcpu_op_is_basic(op)) {
		if (op->b.op < DCPU_OP_MAX)
			isn = &dcpu_basic_isns[op->b.op];
	} else {
		if (op->x.op < DCPU_XOP_MAX)
			isn = &dcpu_non_basic_isns[op->x.op];
	}

	if (isn && isn->op_name[0])
		return isn;

	return NULL;
}

extern struct dcpu_isn * dcpu_lookup_isn_for_name(const char *name);



static inline struct dcpu_reg * dcpu_lookup_reg_for_num(unsigned num)
{
	struct dcpu_reg *reg = NULL;
	if (num < DCPU_REGS_MAX)
		reg = &dcpu_regs[num];

	if (reg && reg->reg_name[0])
		return reg;

	return NULL;
}

extern struct dcpu_reg * dcpu_lookup_reg_for_name(const char *name);

/* return cycle cost of a given op
 * isn can be NULL, or provided if known,
 * returns negative value for bad opcodes */
static inline int dcpu_op_cycles(const dcpu_op_t *op,
		const struct dcpu_isn *isn)
{
	unsigned cycles;

	if (!isn) {
		isn = dcpu_lookup_isn_for_op(op);
		if (!isn)
			return -EINVAL;
	}

	if (dcpu_op_is_basic(op)) {
		cycles = dcpu_op_arg_cycles(op->b.a)
		       + dcpu_op_arg_cycles(op->b.b);
	} else {
		cycles = dcpu_op_arg_cycles(op->x.a);
	}

	return cycles + isn->cycles;
}





#endif // __included_dcpu_isn_h__
