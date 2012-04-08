#include <stdio.h>
#include <string.h>

#include "0x10c_generator.h"
#include "0x10c_generator_lib.h"

#include "0x10c_isn.h"

#define append(fmt,args...) ({ \
		int rc = snprintf(p, e-p, fmt, ##args); \
		if (rc<0) goto bail; \
		p += rc; \
	})

char * x10c_append_arg(char *p, char *e, const char *prefix, x10c_word arg_desc,
		const x10c_word **words)
{
	x10c_word ofs;
	struct x10c_reg *reg;

	switch(arg_desc) {
	case X10C_REG_POP:
	case X10C_REG_PEEK:
	case X10C_REG_PUSH:
	case X10C_REG_SP:
	case X10C_REG_PC:
	case X10C_REG_O:
	case X10C_REG_A ... X10C_REG_J:
		/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
		reg = x10c_lookup_reg_for_num(arg_desc);
		append("%s%s", prefix, reg->reg_name);
		return p;

	case X10C_ARG_MREF_REG(A) ... X10C_ARG_MREF_REG(J):
		/* 0x08-0x0f: [register] */
		reg = x10c_lookup_reg_for_num(arg_desc
				- X10C_ARG_MREF_REG_BASE);
		append("%s[%s]", prefix, reg->reg_name);
		return p;

	case X10C_ARG_MREF_OFS_REG(A) ... X10C_ARG_MREF_OFS_REG(J):
		/* 0x10-0x17: [next word + register] */
		ofs = *((*words)++);
		reg = x10c_lookup_reg_for_num(arg_desc
				- X10C_ARG_MREF_OFS_REG_BASE);
		append("%s[0x%04x+%s]", prefix, ofs, reg->reg_name);
		return p;


	case X10C_MREF_NEXT_WORD:
		/* 0x1e: [next word] */
		ofs = *((*words)++);
		append("%s[0x%04x]", prefix, ofs);
		return p;

	case X10C_NEXT_WORD:
		/* 0x1f: next word (literal) */
		ofs = *((*words)++);
		append("%s0x%04x", prefix, ofs);
		return p;

	default:
		/* 0x20-0x3f: literal value 0x00-0x1f (literal) */
		append("%s0x%04x", prefix, arg_desc - X10C_ARG_LITERAL_BASE);
		return p;
	}

bail:
	return NULL;
}

char * x10c_basic_generator(const struct x10c_isn *isn,
		char *buf, size_t buf_len, const x10c_op_t *op)
{
	char *p = buf;
	char *e = buf + buf_len;
	const x10c_word *words = op->word + 1;

	append("%s", isn->op_name);

	p = x10c_append_arg(p, e, " ",  op->b.a, &words);
	p = x10c_append_arg(p, e, ", ", op->b.b, &words);

bail:
	return buf;
}

char * x10c_non_basic_generator(const struct x10c_isn *isn,
		char *buf, size_t buf_len, const x10c_op_t *op)
{
	char *p = buf;
	char *e = buf + buf_len;
	const x10c_word *words = op->word + 1;

	append("%s", isn->op_name);

	p = x10c_append_arg(p, e, " ", op->b.a, &words);

bail:
	return buf;
}

char * x10c_generate_line(char *buf, size_t buf_len,
		const x10c_op_t *op)
{
	struct x10c_isn *isn;

	isn = x10c_lookup_isn_for_op(op);
	if (!isn) {
		strncpy(buf, "???", buf_len);

		return buf;

	} else if (x10c_op_is_basic(op))
		return x10c_basic_generator(isn, buf, buf_len, op);
	else
		return x10c_non_basic_generator(isn, buf, buf_len, op);
}
