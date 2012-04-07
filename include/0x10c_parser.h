#ifndef __included_0x10c_parser_h__
#define __included_0x10c_parser_h__

#include <unistd.h>
#include <ctype.h>
#include "list.h"

#include "0x10c_op.h"

struct x10c_parsed_line;
struct x10c_parser;

struct x10c_parsed_line {
	struct list link;

	const char *file;
	unsigned line;

	char *buffer;
	char *p;

	int error;

	const struct x10c_isn *isn;
	x10c_op_t *op;
	unsigned word_count;

	const char *label;
};

struct x10c_parser_ops {
	int (*parse_file)(struct x10c_parser *pr, FILE *in);

	void (*set_context) (struct x10c_parser *pr,
			const char *file, unsigned line);

	struct x10c_parsed_line * (*parse_line) (struct x10c_parser *pr,
		const char *buf, size_t buf_len);

	void (*dump)(struct x10c_parser *pr, FILE *out);

	void (*delete)(struct x10c_parser *pr);
};

struct x10c_parser {
	struct list parsed_lines; // list of struct x10c_parsed_line

	x10c_word *ram;
	unsigned ram_words;
	unsigned ram_used;
	unsigned ram_allocated:1;

	const char *file;
	unsigned line;

	struct x10c_parser_ops ops;
};

extern struct x10c_parser * x10c_parser_new(const char *name,
		x10c_word *ram, unsigned ram_words);


// ---- parsing helpers ----

// find a non space
static inline char * find_non_space(char *p)
{
	while (*p && isspace(*p))
		p++;

	return p;
}

// find some space
static inline char * find_space(char *p)
{
	while (*p && !isspace(*p))
		p++;

	return p;
}

static inline char * find_match(char *p, const char *match)
{
	while (*p && !strchr(match, *p))
		p++;

	return p;
}

static inline char * find_no_match(char *p, const char *match)
{
	while (*p && strchr(match, *p))
		p++;

	return p;
}

// return word start, advance pp to end of word */
static inline char * find_word(char **pp)
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
static inline char * find_word_sep(char **pp, const char *sep)
{
	char *w, *e;

	w = find_non_space(*pp);
	if (!*w)
		return NULL;

	e = find_match(w, sep);
	if (w == e)
		return NULL;

	*(e++) = 0;

	*pp = find_no_match(e, sep);

	return w;
}

// trim leading and trailing space
static inline char * trim(char *p)
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


#endif // __included_0x10c_parser_h__
