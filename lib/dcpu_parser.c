#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "0x10c_parser.h"
#include "0x10c_parser_lib.h"
#include "0x10c_isn.h"
#include "0x10c_util.h"

#if 1
#define dbg(f,a...) do { /* nothing */ } while(0)
#else
#define dbg(f,a...) printf(f,##a)
#endif


// ----- line parser code -----

static struct x10c_parsed_line * x10c_parsed_line_new(void)
{
	struct x10c_parsed_line *pl;

	pl = calloc(1, sizeof(*pl));

	return pl;
}

static void x10c_parsed_line_delete(struct x10c_parsed_line *pl)
{
	free((char*)pl->file);
	free(pl);
}

// ----- parser code -----

static int x10c_parser_parse_file(struct x10c_parser *pr, const char *filename)
{
	int rc;
	char buf[4096];
	char *str;
	int tmp;
	char *tok_file, *tok_line, *tok_ofs, *tok_isn;
	FILE *in;
	struct x10c_parsed_line *pl;

	if (!filename)
		return -EINVAL;

	rc = snprintf(buf, sizeof(buf), "dcpu-asm -i -o - %s", filename);
	if (rc<0)
		die("could not generate command parsing string");

	in = popen(buf, "r");
	if (!in)
		die("popen(%s): %s", filename, strerror(errno));

	while ( fgets(buf, sizeof(buf), in) ) {

		pl = x10c_parsed_line_new();

		str = buf;
		tok_file = strsep(&str, "\t");
		tok_line = strsep(&str, "\t");
		tok_ofs = strsep(&str, "\t");
		tok_isn = strsep(&str, "\t");

		if (!tok_file || !tok_line || !tok_ofs || !tok_isn)
			die("failed to parse intermediate format");

		pl->file = strdup(tok_file);
		pl->line = strtoul(tok_line, NULL, 0);
		pl->word_offset = strtoul(tok_ofs, NULL, 0);

		pl->word_count = sscanf(tok_isn,
				"0x%04hx 0x%04hx 0x%04hx %c",
				&pl->op.word[0], &pl->op.word[1],
				&pl->op.word[2], &tmp);

		if (pl->word_count > 3) {
			if (tmp != '\n' && tmp != '\r')
				die("found garbage in intermediate format");
			pl->word_count = 3;
		}

		if (pl->word_count > (pr->ram_words - pr->ram_used))
			die("overflowed memory");

		memcpy(pr->ram + pr->ram_used, pl->op.word,
				pl->word_count * sizeof(x10c_word));

		pr->ram_used += pl->word_count;

		list_add_tail(&pl->link, &pr->parsed_lines);
	}

	rc = 0;
bail:

	pclose(in);

	return 0;
}

static int x10c_parser_parse_block(struct x10c_parser *pr, const char *block)
{
	// not implemented yet
	return -EIO;
}

int x10c_basic_parser(struct x10c_parser *pr, const struct x10c_isn *isn,
		x10c_op_t *op, char *buf)
{
	// not implemented yet
	return -EIO;
}

int x10c_non_basic_parser(struct x10c_parser *pr, const struct x10c_isn *isn,
		x10c_op_t *op, char *buf)
{
	// not implemented yet
	return -EIO;
}

#if 0
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
#endif

static void x10c_parser_dump(struct x10c_parser *pr, FILE *out)
{
	x10c_dump(out, pr->ram, pr->ram_used);

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

#if 0
	list_for_each_entry_safe(lab, lab_next, &pr->labels, link) {
		list_del(&lab->link);
		free(lab);
	}
#endif

	free(pr);
}

static struct x10c_parser_ops x10c_parser_ops = {
	.parse_file  = x10c_parser_parse_file,
	.parse_block = x10c_parser_parse_block,
	.dump        = x10c_parser_dump,
	.delete      = x10c_parser_delete,
};

struct x10c_parser * x10c_parser_new(const char *file,
		x10c_word *ram, unsigned ram_words)
{
	struct x10c_parser *pr;
	int rc;

	pr = calloc(1, sizeof(*pr));

	list_init(&pr->parsed_lines);
	//list_init(&pr->labels);

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

