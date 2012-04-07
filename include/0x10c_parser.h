#ifndef __included_0x10c_parser_h__
#define __included_0x10c_parser_h__

#include <unistd.h>
#include <ctype.h>
#include "list.h"

#include "0x10c_op.h"

struct x10c_parsed_line {
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

extern struct x10c_parsed_line * x10c_parse_line(x10c_op_t *op,
		const char *file, unsigned line,
		const char *buf, size_t buf_len);

extern void x10c_parsed_line_free(struct x10c_parsed_line *pl);

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
