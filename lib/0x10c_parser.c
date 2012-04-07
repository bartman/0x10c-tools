#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "0x10c_parser.h"
#include "0x10c_parser_lib.h"
#include "0x10c_isn.h"

#if 1
#define dbg(f,a...) do { /* nothing */ } while(0)
#else
#define dbg(f,a...) printf(f,##a)
#endif


// ----

static int x10c_parser_arg(const struct x10c_isn *isn,
			x10c_op_t *op, int *next_word, char *arg, int arg_num)
{
	char *p = arg;
	char *a, *b, *e;
	struct x10c_reg *reg;
	unsigned long v;
	unsigned val;
	unsigned shift;

	if (*p == '[') {
		/* memory reference */
		p++;

		dbg("  # mref\n");

		e = find_match(p, "+]");
		switch (*e) {
		case '+':
			/* [reg+n] or [n+reg] */

			*e = 0;
			a = trim(p);
			b = trim(e+1);

			e = find_match(b, "]");
			if (!*e)
				return -EINVAL;
			*e = 0;

			dbg("  # a=%s\n", a);
			dbg("  # b=%s\n", b);

			if ( (reg = x10c_lookup_reg_for_name(a)) ) {
				p = b;
				dbg("    # reg %s/%d\n",
						reg->reg_name, reg->reg_num);
				val = 0x10 + reg->reg_num;

			} else if ( (reg = x10c_lookup_reg_for_name(b)) ) {
				p = a;
				dbg("    # reg %s/%d\n",
						reg->reg_name, reg->reg_num);
				val = 0x10 + reg->reg_num;

			} else {
				return -EINVAL;
			}

			v = strtoul(p, &e, 0);
			if (v > X10C_MAX_WORD_VALUE)
				return -ERANGE;

			dbg("    # literal %lu\n", v);

			op->word[*next_word] = v;
			(*next_word) ++;

			break;
		case ']':
			/* [reg] or [n] */

			*e = 0;
			a = trim(p);

			if ( (reg = x10c_lookup_reg_for_name(a)) ) {
				/* [reg] */

				dbg("    # reg %s/%d\n",
						reg->reg_name, reg->reg_num);
				val = 0x8 + reg->reg_num;

			} else {
				/* [n] */


				v = strtoul(p, &e, 0);
				if (v > X10C_MAX_WORD_VALUE)
					return -ERANGE;

				dbg("    # literal %lu\n", v);

				val = 0x1e;
				op->word[*next_word] = v;
				(*next_word) ++;
			}

			break;
		default:
			return -EINVAL;
		}

	} else if ( (reg = x10c_lookup_reg_for_name(p)) ) {
		/* register */

		dbg("  # reg %s/%d\n", reg->reg_name, reg->reg_num);

		val = reg->reg_num;

	} else {
		/* literal */

		v = strtoul(p, &e, 0);
		if (v > X10C_MAX_WORD_VALUE)
			return -ERANGE;

		dbg("  # literal %lu\n", v);

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

	dbg("# a = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	w = find_word(&p);
	if (!w)
		return -EINVAL;

	dbg("# b = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 1);
	if (rc<0)
		return rc;

	dbg("# rc=%d [ %04x %04x %04x ]\n", next_word,
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

	dbg("# a = %s\n", w);

	rc = x10c_parser_arg(isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	return next_word;
}

static struct x10c_parsed_line * x10c_parsed_line_new(x10c_op_t *op,
		const char *file, unsigned line,
		const char *buf, size_t buf_len)
{
	struct x10c_parsed_line *pl;

	pl = calloc(1, sizeof(*pl));

	pl->file = file;
	pl->line = line;

	pl->buffer = strndup(buf, buf_len);
	pl->p = trim(pl->buffer);

	op->word[0] = 0;

	pl->op = op;

	return pl;
}

void x10c_parsed_line_free(struct x10c_parsed_line *pl)
{
	free(pl);
}

struct x10c_parsed_line * x10c_parse_line(x10c_op_t *op,
		const char *file, unsigned line,
		const char *buf, size_t buf_len)
{
	int rc;
	struct x10c_parsed_line *pl;
	char *w;

	pl = x10c_parsed_line_new(op, file, line, buf, buf_len);

	dbg("# %s\n", pl->p);

	w = find_word(&pl->p);

	if (w && *w == ':') {
		// label
		if (w[1]) {
			pl->label = w+1;
		} else {
			w = find_word(&pl->p);
			if (!w)
				goto failed_parsing;
			pl->label = w;
		}

		w = find_word(&pl->p);
	}

	switch (w ? *w : 0) {
	case 0:
	case ';':
	case '#':
		// empty line, just space, or a comment
		goto skip_blank;
	case ':':
		// second label on one line not allowed
		goto failed_parsing;
	default:
		// needs more parsing
		break;
	}

	pl->isn = x10c_lookup_isn_for_name(w);
	if (!pl->isn)
		goto failed_no_isn;

	dbg("# isn=%s op=%u,%u\n", pl->isn->op_name,
			pl->isn->op_code, pl->isn->ext_op_code);

	rc = pl->isn->ops.parser(pl->isn, pl->op, pl->p);
	if (rc < 0)
		pl->error = rc;
	else
		pl->word_count = rc;

	return pl;

failed_no_isn:
	pl->error = -ENOENT;
	return pl;

failed_parsing:
	pl->error = -EINVAL;
	return pl;

skip_blank:
	return pl;
}
