#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "0x10c_parser.h"
#include "0x10c_parser_lib.h"
#include "0x10c_isn.h"

// ----

// find a non space
static char * find_non_space(char *p)
{
	while (*p && isspace(*p))
		p++;

	return p;
}

// find some space
static char * find_space(char *p)
{
	while (*p && !isspace(*p))
		p++;

	return p;
}

static char * find_until_match(char *p, const char *match)
{
	while (*p && !strchr(match, *p))
		p++;

	return p;
}

static char * find_while_match(char *p, const char *match)
{
	while (*p && strchr(match, *p))
		p++;

	return p;
}

// return word start, advance pp to end of word */
static char * find_word(char **pp)
{
	char *w, *e;

	w = find_non_space(*pp);
	if (!*w)
		return NULL;

	e = find_space(w);
	if (w == e)
		return NULL;

	*(e++) = 0;

	*pp = find_non_space(e);

	return w;
}

/* like find_word, but uses a special separator */
static char * find_word_sep(char **pp, const char *sep)
{
	char *w, *e;

	w = find_non_space(*pp);
	if (!*w)
		return NULL;

	e = find_until_match(w, sep);
	if (w == e)
		return NULL;

	*(e++) = 0;

	*pp = find_while_match(e, sep);

	return w;
}

// trim leading and trailing space
static char * trim(char *p)
{
	char *e;
	e = p = find_non_space(p);

	while (*e) e++;
	if (e == p)
		return p;

	e--;
	while(e>p && isspace(*e)) e--;
	e[1] = 0;

	return p;
}

// duplicate w/o spaces at the edges
static char * trimndup(const char *src, size_t max)
{
	return trim(strndup(src, max));
}


// ----

static int x10c_parser_arg(const struct x10c_isn *isn,
			x10c_op_t *op, int *next_word, char *arg, int arg_num)
{
	char *p = arg;
	char *w, *e;
	struct x10c_reg *reg;
	unsigned ret = 0;
	unsigned val;
	unsigned shift;

	if (*p == '[') {
		/* memory reference */

		printf("  # mref\n");

	} else if (reg = x10c_lookup_reg_for_name(p)) {
		/* register */

		printf("  # reg %s/%d\n", reg->reg_name, reg->reg_num);

		val = reg->reg_num;

	} else {
		/* literal */

		unsigned long v = strtoul(p, &e, 0);
		if (v > X10C_MAX_WORD_VALUE)
			return -ERANGE;

		printf("  # literal %lu\n", v);

		if (v < 0x20) {
			val = 0x20 + v;
		} else {
			val = 0x1f;
			op->word[*next_word] = v;
			(*next_word) ++;
		}
	}

	shift = 4 + (arg_num * 6);

	op->word[0] |= val << shift;

	return 0;
}

int x10c_basic_parser(const struct x10c_isn *isn,
		x10c_op_t *op, char *buf)
{
	char *p = buf;
	char *w;
	int rc, next_word = 1;

	op->b.op = isn->op_code;

	w = find_word_sep(&p, ", \t");
	if (!w)
		return -EINVAL;

	printf("# a = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	w = find_word(&p);
	if (!w)
		return -EINVAL;

	printf("# b = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 1);
	if (rc<0)
		return rc;

	printf("# rc=%d [ %04x %04x %04x ]\n", next_word,
			op->word[0], op->word[1], op->word[2]);

	return next_word;
}
int x10c_non_basic_parser(const struct x10c_isn *isn,
		x10c_op_t *op, char *buf)
{
	char *p = buf;
	char *w;
	int rc, next_word = 1;

	op->x.zero = X10C_OP_NON_BASIC;
	op->x.op = isn->ext_op_code;

	w = find_word_sep(&p, ", \t");
	if (!w)
		return -EINVAL;

	printf("# a = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	return next_word;
}

int x10c_parse_line(x10c_op_t *op, const char *buf, size_t buf_len)
{
	int rc;
	char *buf_copy = trimndup(buf, buf_len);
	char *p = buf_copy;
	char *w;
	const struct x10c_isn *isn;

	printf("# %s\n", buf_copy);

	w = find_word(&p);
	if (!w) {
		rc = -EINVAL;
		goto bail;
	}

	isn = x10c_lookup_isn_for_name(w);
	if (!isn) {
		rc = -EINVAL;
		goto bail;
	}

	printf("# isn=%s op=%u,%u\n", isn->op_name,
			isn->op_code, isn->ext_op_code);

	rc = isn->ops.parser(isn, op, p);

bail:
	free(buf_copy);
	return rc;
}
