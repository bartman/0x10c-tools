#include <stdint.h>
#include <string.h>

#include "dcpu_isn.h"
#include "dcpu_util.h"
#include "dcpu_exec.h"
#include "dcpu_parser_lib.h"
#include "dcpu_generator_lib.h"

/* basic operation handling */

#define DCPU_BASIC_ISN_ENTRY(OP,CLK,IF) \
	[DCPU_OP_##OP] = { .op_name = __stringify(OP), \
		.op_code = DCPU_OP_##OP, \
		.ext_op_code = 0, \
		.cycles = CLK, \
		.is_conditional = IF, \
		.ops = { .parser = dcpu_basic_parser, \
			.generator = dcpu_basic_generator, \
			.execute = dcpu_execute_##OP } }

struct dcpu_isn dcpu_basic_isns[DCPU_OP_MAX] = {
	DCPU_BASIC_ISN_ENTRY(SET,1,0),
	DCPU_BASIC_ISN_ENTRY(ADD,2,0),
	DCPU_BASIC_ISN_ENTRY(SUB,2,0),
	DCPU_BASIC_ISN_ENTRY(MUL,2,0),
	DCPU_BASIC_ISN_ENTRY(MLI,2,0),
	DCPU_BASIC_ISN_ENTRY(DIV,3,0),
	DCPU_BASIC_ISN_ENTRY(DVI,3,0),
	DCPU_BASIC_ISN_ENTRY(MOD,3,0),
	DCPU_BASIC_ISN_ENTRY(MDI,3,0),
	DCPU_BASIC_ISN_ENTRY(AND,1,0),
	DCPU_BASIC_ISN_ENTRY(BOR,1,0),
	DCPU_BASIC_ISN_ENTRY(XOR,1,0),
	DCPU_BASIC_ISN_ENTRY(SHR,1,0),
	DCPU_BASIC_ISN_ENTRY(ASR,1,0),
	DCPU_BASIC_ISN_ENTRY(SHL,1,0),
	DCPU_BASIC_ISN_ENTRY(IFB,2,1),
	DCPU_BASIC_ISN_ENTRY(IFC,2,1),
	DCPU_BASIC_ISN_ENTRY(IFE,2,1),
	DCPU_BASIC_ISN_ENTRY(IFN,2,1),
	DCPU_BASIC_ISN_ENTRY(IFG,2,1),
	DCPU_BASIC_ISN_ENTRY(IFA,2,1),
	DCPU_BASIC_ISN_ENTRY(IFL,2,1),
	DCPU_BASIC_ISN_ENTRY(IFU,2,1),
	DCPU_BASIC_ISN_ENTRY(ADX,3,0),
	DCPU_BASIC_ISN_ENTRY(SBX,3,0),
	DCPU_BASIC_ISN_ENTRY(STI,2,0),
	DCPU_BASIC_ISN_ENTRY(STD,2,0),
};

/* special operation handling */

#define DCPU_SPECIAL_ISN_ENTRY(OP,CLK) \
	[DCPU_XOP_##OP] = { .op_name     = __stringify(OP), \
		.op_code     = DCPU_OP_NON_BASIC, \
		.ext_op_code = DCPU_XOP_##OP, \
		.cycles = CLK, \
		.is_conditional = 0, \
		.ops = { .parser = dcpu_non_basic_parser, \
			.generator = dcpu_non_basic_generator, \
			.execute = dcpu_execute_##OP } }

struct dcpu_isn dcpu_non_basic_isns[DCPU_XOP_MAX] = {
	DCPU_SPECIAL_ISN_ENTRY(JSR,3),
	DCPU_SPECIAL_ISN_ENTRY(HCF,9),
	DCPU_SPECIAL_ISN_ENTRY(INT,4),
	DCPU_SPECIAL_ISN_ENTRY(IAG,1),
	DCPU_SPECIAL_ISN_ENTRY(IAS,1),
	DCPU_SPECIAL_ISN_ENTRY(RFI,3),
	DCPU_SPECIAL_ISN_ENTRY(IAQ,2),
	DCPU_SPECIAL_ISN_ENTRY(HWN,2),
	DCPU_SPECIAL_ISN_ENTRY(HWQ,4),
	DCPU_SPECIAL_ISN_ENTRY(HWI,4),
};

struct dcpu_isn * dcpu_lookup_isn_for_name(const char *name)
{
	int i;
	struct dcpu_isn *isn;

	for (i=0; i<DCPU_OP_MAX; i++) {
		isn = &dcpu_basic_isns[i];
		if (isn->op_name && !DCPU_OP_NAME_CMP(isn->op_name, name))
			return isn;
	}

	for (i=0; i<DCPU_XOP_MAX; i++) {
		isn = &dcpu_non_basic_isns[i];
		if (isn->op_name && !DCPU_OP_NAME_CMP(isn->op_name, name))
			return isn;
	}

	return NULL;
}

/* registers */

#define DCPU_REG_ENTRY(REG,GP) \
	[DCPU_REG_##REG] = { __stringify(REG), DCPU_REG_##REG, GP }

struct dcpu_reg dcpu_regs[DCPU_REGS_MAX] = {
	DCPU_REG_ENTRY(A,1),
	DCPU_REG_ENTRY(B,1),
	DCPU_REG_ENTRY(C,1),
	DCPU_REG_ENTRY(X,1),
	DCPU_REG_ENTRY(Y,1),
	DCPU_REG_ENTRY(Z,1),
	DCPU_REG_ENTRY(I,1),
	DCPU_REG_ENTRY(J,1),
	DCPU_REG_ENTRY(PUSH_POP,0),
	DCPU_REG_ENTRY(PEEK,0),
	DCPU_REG_ENTRY(PICK,0),
	DCPU_REG_ENTRY(SP,0),
	DCPU_REG_ENTRY(PC,0),
	DCPU_REG_ENTRY(EX,0),
};

struct dcpu_reg * dcpu_lookup_reg_for_name(const char *name)
{
	int i;
	struct dcpu_reg *reg;

	for (i=0; i<DCPU_REGS_MAX; i++) {
		reg = & dcpu_regs[i];
		if (reg->reg_name && !DCPU_REG_NAME_CMP(reg->reg_name, name))
			return reg;
	}

	return NULL;
}

