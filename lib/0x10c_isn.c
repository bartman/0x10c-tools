#include <stdint.h>
#include <string.h>

#include "0x10c_isn.h"
#include "0x10c_util.h"
#include "0x10c_exec.h"
#include "0x10c_parser_lib.h"
#include "0x10c_generator_lib.h"

/* basic operation handling */

#define X10C_BASIC_ISN_ENTRY(OP) \
	[X10C_OP_##OP] = { __stringify(OP), X10C_OP_##OP, 0, 2, \
		.ops = { .parser = x10c_basic_parser, \
			.generator = x10c_basic_generator, \
			.execute = x10c_execute_##OP } }

struct x10c_isn x10c_basic_isns[X10C_OP_MAX] = {
	X10C_BASIC_ISN_ENTRY(SET),
	X10C_BASIC_ISN_ENTRY(ADD),
	X10C_BASIC_ISN_ENTRY(SUB),
	X10C_BASIC_ISN_ENTRY(MUL),
	X10C_BASIC_ISN_ENTRY(DIV),
	X10C_BASIC_ISN_ENTRY(MOD),
	X10C_BASIC_ISN_ENTRY(SHL),
	X10C_BASIC_ISN_ENTRY(SHR),
	X10C_BASIC_ISN_ENTRY(AND),
	X10C_BASIC_ISN_ENTRY(BOR),
	X10C_BASIC_ISN_ENTRY(XOR),
	X10C_BASIC_ISN_ENTRY(IFE),
	X10C_BASIC_ISN_ENTRY(IFN),
	X10C_BASIC_ISN_ENTRY(IFG),
	X10C_BASIC_ISN_ENTRY(IFB),
};

/* non-basic operation handling */

#define X10C_NON_BASIC_ISN_ENTRY(OP) \
	[X10C_XOP_##OP] = { .op_name     = __stringify(OP), \
		.op_code     = X10C_OP_NON_BASIC, \
		.ext_op_code = X10C_XOP_##OP, 2, \
		.ops = { .parser = x10c_non_basic_parser, \
			.generator = x10c_non_basic_generator, \
			.execute = x10c_execute_##OP } }

struct x10c_isn x10c_non_basic_isns[X10C_XOP_MAX] = {
	X10C_NON_BASIC_ISN_ENTRY(JSR),
};

struct x10c_isn * x10c_lookup_isn_for_name(const char *name)
{
	int i;
	struct x10c_isn *isn;

	for (i=0; i<X10C_OP_MAX; i++) {
		isn = &x10c_basic_isns[i];
		if (isn->op_name && !X10C_OP_NAME_CMP(isn->op_name, name))
			return isn;
	}

	for (i=0; i<X10C_XOP_MAX; i++) {
		isn = &x10c_non_basic_isns[i];
		if (isn->op_name && !X10C_OP_NAME_CMP(isn->op_name, name))
			return isn;
	}

	return NULL;
}

/* registers */

#define X10C_REG_ENTRY(REG,GP) \
	[X10C_REG_##REG] = { __stringify(REG), X10C_REG_##REG, GP }

struct x10c_reg x10c_regs[X10C_REGS_MAX] = {
	X10C_REG_ENTRY(A,1),
	X10C_REG_ENTRY(B,1),
	X10C_REG_ENTRY(C,1),
	X10C_REG_ENTRY(X,1),
	X10C_REG_ENTRY(Y,1),
	X10C_REG_ENTRY(Z,1),
	X10C_REG_ENTRY(I,1),
	X10C_REG_ENTRY(J,1),
	X10C_REG_ENTRY(PUSH_POP,0),
	X10C_REG_ENTRY(PEEK,0),
	X10C_REG_ENTRY(PICK,0),
	X10C_REG_ENTRY(SP,0),
	X10C_REG_ENTRY(PC,0),
	X10C_REG_ENTRY(EX,0),
};

struct x10c_reg * x10c_lookup_reg_for_name(const char *name)
{
	int i;
	struct x10c_reg *reg;

	for (i=0; i<X10C_REGS_MAX; i++) {
		reg = & x10c_regs[i];
		if (reg->reg_name && !X10C_REG_NAME_CMP(reg->reg_name, name))
			return reg;
	}

	return NULL;
}
