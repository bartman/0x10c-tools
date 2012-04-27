#include <stdint.h>
#include <string.h>

#include "0x10c_isn.h"
#include "0x10c_util.h"
#include "0x10c_exec.h"
#include "0x10c_parser_lib.h"
#include "0x10c_generator_lib.h"

/* basic operation handling */

#define X10C_BASIC_ISN_ENTRY(OP,CLK,IF) \
	[X10C_OP_##OP] = { .op_name = __stringify(OP), \
		.op_code = X10C_OP_##OP, \
		.ext_op_code = 0, \
		.cycles = CLK, \
		.is_conditional = IF, \
		.ops = { .parser = x10c_basic_parser, \
			.generator = x10c_basic_generator, \
			.execute = x10c_execute_##OP } }

struct x10c_isn x10c_basic_isns[X10C_OP_MAX] = {
	X10C_BASIC_ISN_ENTRY(SET,1,0),
	X10C_BASIC_ISN_ENTRY(ADD,2,0),
	X10C_BASIC_ISN_ENTRY(SUB,2,0),
	X10C_BASIC_ISN_ENTRY(MUL,2,0),
	X10C_BASIC_ISN_ENTRY(MLI,2,0),
	X10C_BASIC_ISN_ENTRY(DIV,3,0),
	X10C_BASIC_ISN_ENTRY(DVI,3,0),
	X10C_BASIC_ISN_ENTRY(MOD,3,0),
	X10C_BASIC_ISN_ENTRY(MDI,3,0),
	X10C_BASIC_ISN_ENTRY(AND,1,0),
	X10C_BASIC_ISN_ENTRY(BOR,1,0),
	X10C_BASIC_ISN_ENTRY(XOR,1,0),
	X10C_BASIC_ISN_ENTRY(SHR,1,0),
	X10C_BASIC_ISN_ENTRY(ASR,1,0),
	X10C_BASIC_ISN_ENTRY(SHL,1,0),
	X10C_BASIC_ISN_ENTRY(IFB,2,1),
	X10C_BASIC_ISN_ENTRY(IFC,2,1),
	X10C_BASIC_ISN_ENTRY(IFE,2,1),
	X10C_BASIC_ISN_ENTRY(IFN,2,1),
	X10C_BASIC_ISN_ENTRY(IFG,2,1),
	X10C_BASIC_ISN_ENTRY(IFA,2,1),
	X10C_BASIC_ISN_ENTRY(IFL,2,1),
	X10C_BASIC_ISN_ENTRY(IFU,2,1),
	X10C_BASIC_ISN_ENTRY(ADX,3,0),
	X10C_BASIC_ISN_ENTRY(SBX,3,0),
	X10C_BASIC_ISN_ENTRY(STI,2,0),
	X10C_BASIC_ISN_ENTRY(STD,2,0),
};

/* special operation handling */

#define X10C_SPECIAL_ISN_ENTRY(OP,CLK) \
	[X10C_XOP_##OP] = { .op_name     = __stringify(OP), \
		.op_code     = X10C_OP_NON_BASIC, \
		.ext_op_code = X10C_XOP_##OP, \
		.cycles = CLK, \
		.is_conditional = 0, \
		.ops = { .parser = x10c_non_basic_parser, \
			.generator = x10c_non_basic_generator, \
			.execute = x10c_execute_##OP } }

struct x10c_isn x10c_non_basic_isns[X10C_XOP_MAX] = {
	X10C_SPECIAL_ISN_ENTRY(JSR,3),
	X10C_SPECIAL_ISN_ENTRY(HCF,9),
	X10C_SPECIAL_ISN_ENTRY(INT,4),
	X10C_SPECIAL_ISN_ENTRY(IAG,1),
	X10C_SPECIAL_ISN_ENTRY(IAS,1),
	X10C_SPECIAL_ISN_ENTRY(RFI,3),
	X10C_SPECIAL_ISN_ENTRY(IAQ,2),
	X10C_SPECIAL_ISN_ENTRY(HWN,2),
	X10C_SPECIAL_ISN_ENTRY(HWQ,4),
	X10C_SPECIAL_ISN_ENTRY(HWI,4),
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

