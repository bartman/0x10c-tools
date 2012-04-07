#include <stdint.h>

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
	[X10C_XOP_##OP] = { __stringify(OP), X10C_XOP_##OP, 0, 2, \
		.ops = { .parser = x10c_non_basic_parser, \
			.generator = x10c_non_basic_generator, \
			.execute = x10c_execute_##OP } }

struct x10c_isn x10c_non_basic_isns[X10C_XOP_MAX] = {
	X10C_NON_BASIC_ISN_ENTRY(JSR),
};
