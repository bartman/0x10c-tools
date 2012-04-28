#include <stdio.h>
#include <string.h>

#include "dcpu_generator.h"
#include "dcpu_generator_lib.h"
#include "dcpu_parser.h"

#include "dcpu_isn.h"

#define append(fmt,args...) ({ \
		int rc = snprintf(p, e-p, fmt, ##args); \
		if (rc<0) goto bail; \
		p += rc; \
	})

static char * dcpu_append_arg(char *p, char *e, const char *prefix,
		dcpu_word arg_desc, const dcpu_word *word)
{
	dcpu_word ofs;
	struct dcpu_reg *reg;

	switch(arg_desc) {
	case DCPU_REG_PUSH_POP:
	case DCPU_REG_PEEK:
	case DCPU_REG_PICK:
	case DCPU_REG_SP:
	case DCPU_REG_PC:
	case DCPU_REG_EX:
	case DCPU_REG_A ... DCPU_REG_J:
		/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
		reg = dcpu_lookup_reg_for_num(arg_desc);
		append("%s%s", prefix, reg->reg_name);
		return p;

	case DCPU_ARG_MREF_REG(A) ... DCPU_ARG_MREF_REG(J):
		/* 0x08-0x0f: [register] */
		reg = dcpu_lookup_reg_for_num(arg_desc
				- DCPU_ARG_MREF_REG_BASE);
		append("%s[%s]", prefix, reg->reg_name);
		return p;

	case DCPU_ARG_MREF_OFS_REG(A) ... DCPU_ARG_MREF_OFS_REG(J):
		/* 0x10-0x17: [next word + register] */
		ofs = *word;
		reg = dcpu_lookup_reg_for_num(arg_desc
				- DCPU_ARG_MREF_OFS_REG_BASE);
		append("%s[0x%04x+%s]", prefix, ofs, reg->reg_name);
		return p;


	case DCPU_MREF_NEXT_WORD:
		/* 0x1e: [next word] */
		ofs = *word;
		append("%s[0x%04x]", prefix, ofs);
		return p;

	case DCPU_NEXT_WORD:
		/* 0x1f: next word (literal) */
		ofs = *word;
		append("%s0x%04x", prefix, ofs);
		return p;

	default:
		/* 0x20-0x3f: literal value 0x00-0x1f (literal) */
		append("%s0x%04hx", prefix,
				(uint16_t)(arg_desc-DCPU_ARG_LITERAL_BASE-1));
		return p;
	}

bail:
	return NULL;
}

char * dcpu_basic_generator(const struct dcpu_isn *isn,
		char *buf, size_t buf_len, const dcpu_op_t *op)
{
	char *p = buf;
	char *e = buf + buf_len;
	const dcpu_word *words = op->word + 1;
	int a_len;

	append("%s", isn->op_name);

	a_len = dcpu_op_arg_len(op->b.a);

	p = dcpu_append_arg(p, e, " ",  op->b.b, words + a_len);
	p = dcpu_append_arg(p, e, ", ", op->b.a, words);

bail:
	return buf;
}

char * dcpu_non_basic_generator(const struct dcpu_isn *isn,
		char *buf, size_t buf_len, const dcpu_op_t *op)
{
	char *p = buf;
	char *e = buf + buf_len;
	const dcpu_word *words = op->word + 1;

	append("%s", isn->op_name);

	p = dcpu_append_arg(p, e, " ", op->x.a, words);

bail:
	return buf;
}

char * dcpu_generate_hex(char *buf, size_t buf_len,
		const dcpu_op_t *op)
{
	char *p = buf;
	char *e = buf + buf_len;
	unsigned cnt = dcpu_op_len(op);
	const dcpu_word *words = op->word;

	switch(cnt) {
	case 3:
		append("%04x ", *(words++));
	case 2:
		append("%04x ", *(words++));
	case 1:
		append("%04x ", *(words++));
	default:
		break;
	}

bail:
	return trim(buf);
}

char * dcpu_generate_asm(char *buf, size_t buf_len,
		const dcpu_op_t *op)
{
	struct dcpu_isn *isn;

	isn = dcpu_lookup_isn_for_op(op);
	if (!isn) {
		strncpy(buf, "???", buf_len);

		return buf;

	} else if (dcpu_op_is_basic(op))
		return dcpu_basic_generator(isn, buf, buf_len, op);
	else
		return dcpu_non_basic_generator(isn, buf, buf_len, op);
}
