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

static int x10c_parser_arg(struct x10c_parser *pr, const struct x10c_isn *isn,
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

	} else if ( isdigit(*p) ) {
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
	} else {
		/* reference */
		long ofs;

		dbg("  # reference %s\n", p);

		ofs = pr->ops.find_label(pr, p);
		if (ofs == -1)
			pr->ops.mark_unresolved(pr, p,
					pr->ram_used + *next_word);

		val = 0x1f;
		op->word[*next_word] = ofs;
		(*next_word) ++;
	}

	shift = 4 + (arg_num * 6);

	op->word[0] |= val << shift;

	return 0;
}

int x10c_basic_parser(struct x10c_parser *pr, const struct x10c_isn *isn,
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

	rc = x10c_parser_arg(pr, isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	w = find_word(&p);
	if (!w)
		return -EINVAL;

	dbg("# b = %s\n", w);

	rc = x10c_parser_arg(pr, isn, op, &next_word, w, 1);
	if (rc<0)
		return rc;

	dbg("# rc=%d [ %04x %04x %04x ]\n", next_word,
			op->word[0], op->word[1], op->word[2]);

	return next_word;
}
int x10c_non_basic_parser(struct x10c_parser *pr, const struct x10c_isn *isn,
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

	rc = x10c_parser_arg(pr, isn, op, &next_word, w, 0);
	if (rc<0)
		return rc;

	return next_word;
}

// ----- line parser code -----

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

static void x10c_parsed_line_delete(struct x10c_parsed_line *pl)
{
	free(pl);
}

static struct x10c_parsed_line * x10c_parser_parse_line (struct x10c_parser *pr,
		const char *buf, size_t buf_len)
{
	int rc;
	struct x10c_parsed_line *pl;
	x10c_op_t *op;
	char *w;

	op = (void*)(pr->ram + pr->ram_used);

	pl = x10c_parsed_line_new(op, pr->file, pr->line, buf, buf_len);

	dbg("# %s\n", pl->p);

	if (pr->ram_used + X10C_MAX_OP_LEN > pr->ram_words)
		goto failed_no_memory;

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

		pr->ops.add_label (pr, pl->label);

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

	rc = pl->isn->ops.parser(pr, pl->isn, pl->op, pl->p);
	if (rc >= 0) {
		pl->word_count = rc;

		list_add_tail(&pl->link, &pr->parsed_lines);

		pr->ram_used += rc;

	} else
		pl->error = rc;

	return pl;

failed_no_isn:
	pl->error = -ENOENT;
	return pl;

failed_no_memory:
	pl->error = -ENOMEM;
	return pl;

failed_parsing:
	pl->error = -EINVAL;
	return pl;

skip_blank:
	return pl;
}


// ----- parser code -----

static int x10c_parser_parse_file(struct x10c_parser *pr, FILE *in)
{
	char buf[1024];

	while ( fgets(buf, sizeof(buf), in) ) {

		char *str = trim(buf);
		struct x10c_parsed_line *pl;
		x10c_op_t *op;

		pr->line ++;

		pl = pr->ops.parse_line(pr, str, strlen(str));

		op = pl->op;

		printf("%-80s ; %s%-10s", str,
				pl->label ? ":" : " ",
				pl->label ?: "");

		switch(pl->word_count) {
		case 0:
			printf("\n");
			break;
		case 1:
			printf("%04x\n",
					op->word[0]);
			break;
		case 2:
			printf("%04x %04x\n",
					op->word[0], op->word[1]);
			break;
		case 3:
			printf("%04x %04x %04x\n",
					op->word[0], op->word[1], op->word[2]);
			break;
		default:
			printf("empty, rc=%d\n", pl->error);
			return pl->error;
		}
	}

	return 0;
}

static void x10c_parser_set_context (struct x10c_parser *pr,
		const char *file, unsigned line)
{
	pr->file = file;
	pr->line = line;
}

struct x10c_parser_label {
	struct list link;
	const char *name;
	x10c_word ofs;
};

static void x10c_parser_add_label (struct x10c_parser *pr, const char *name)
{
	struct x10c_parser_label *lab;

	lab = calloc(1, sizeof(*lab));

	lab->name = name;
	lab->ofs = pr->ram_used;

	list_add_tail(&lab->link, &pr->labels);
}

static long x10c_parser_find_label (struct x10c_parser *pr,
		const char *name)
{
	struct x10c_parser_label *lab;

	list_for_each_entry(lab, &pr->labels, link) {
		if (!strcmp(name, lab->name))
			return lab->ofs;
	}

	return -1;
}

struct x10c_parser_unresolved {
	struct list link;
	const char *name;
	x10c_word ofs;
};

static void x10c_parser_mark_unresolved(struct x10c_parser *pr,
		const char *name, x10c_word ofs)
{
	struct x10c_parser_unresolved *ur;

	ur = calloc(1, sizeof(*ur));

	ur->name = name;
	ur->ofs = ofs;

	list_add_tail(&ur->link, &pr->unresolved);
}

static int x10c_parser_finalize (struct x10c_parser *pr)
{
	struct x10c_parser_unresolved *ur, *ur_next;

	list_for_each_entry_safe(ur, ur_next, &pr->unresolved, link) {

		long val = pr->ops.find_label(pr, ur->name);
		if (val == -1) {
			fprintf(stderr, "couldn't finalize '%s' at %04x\n",
					ur->name, ur->ofs);
			return -ENOENT;
		}

		pr->ram[ur->ofs] = val;

		list_del(&ur->link);
		free(ur);
	}

	return 0;
}

static void x10c_parser_dump(struct x10c_parser *pr, FILE *out)
{
	int i;
	x10c_word *w = pr->ram;

	for (i=0; i< pr->ram_used; i++) {
		if ((i&7) == 0)
			fprintf(out, "%04x:", i);
		fprintf(out, " %04x", w[i]);
		if ((i&7) == 7)
			fprintf(out, "\n");
	}
	if ((i&7) != 0)
		fprintf(out, "\n");

	return;
}

static void x10c_parser_delete(struct x10c_parser *pr)
{
	struct x10c_parsed_line *pl, *pl_next;
	struct x10c_parser_label *lab, *lab_next;
	struct x10c_parser_unresolved *ur, *ur_next;

	list_for_each_entry_safe(pl, pl_next, &pr->parsed_lines, link) {
		list_del(&pl->link);
		x10c_parsed_line_delete(pl);
	}

	list_for_each_entry_safe(lab, lab_next, &pr->labels, link) {
		list_del(&lab->link);
		free(lab);
	}

	list_for_each_entry_safe(ur, ur_next, &pr->unresolved, link) {
		list_del(&ur->link);
		free(ur);
	}

	free(pr);
}

static struct x10c_parser_ops x10c_parser_ops = {
	.parse_file  = x10c_parser_parse_file,
	.set_context = x10c_parser_set_context,
	.parse_line  = x10c_parser_parse_line,
	.add_label   = x10c_parser_add_label,
	.find_label  = x10c_parser_find_label,
	.mark_unresolved = x10c_parser_mark_unresolved,
	.finalize    = x10c_parser_finalize,
	.dump        = x10c_parser_dump,
	.delete      = x10c_parser_delete,
};

struct x10c_parser * x10c_parser_new(const char *file,
		x10c_word *ram, unsigned ram_words)
{
	struct x10c_parser *pr;

	pr = calloc(1, sizeof(*pr));

	list_init(&pr->parsed_lines);
	list_init(&pr->labels);
	list_init(&pr->unresolved);

	pr->ram_used = 0;

	if (ram) {
		pr->ram = ram;
		pr->ram_words = ram_words;
		pr->ram_allocated = 0;
	} else {
		pr->ram_words = 0x10000;
		pr->ram = calloc(pr->ram_words, sizeof(x10c_word));
		pr->ram_allocated = 1;
	}

	pr->ops = x10c_parser_ops;

	return pr;
}

