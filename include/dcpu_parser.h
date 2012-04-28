#ifndef __included_dcpu_parser_h__
#define __included_dcpu_parser_h__

#include <unistd.h>
#include <ctype.h>
#include "list.h"

#include "dcpu_op.h"

struct dcpu_parsed_line;
struct dcpu_parser;
struct lua_State;

struct dcpu_parsed_line {
	struct list link;

	const char *file;
	unsigned line;

	unsigned word_offset;
	unsigned word_count;

	dcpu_op_t op;

	//const char *label;
};

struct dcpu_parser_ops {
	int (*parse_file)(struct dcpu_parser *pr, const char *filename);

	int (*parse_block)(struct dcpu_parser *pr, const char *block);

	void (*dump)(struct dcpu_parser *pr, FILE *out);

	void (*delete)(struct dcpu_parser *pr);
};

struct dcpu_parser {
	struct list parsed_lines;   // list of struct dcpu_parsed_line
	//struct list labels;         // list of struct dcpu_parser_label

	dcpu_word *ram;
	unsigned ram_words;
	unsigned ram_used;
	unsigned ram_allocated:1;   // is free() needed at cleanup

	const char *file;
	unsigned line;

	struct dcpu_parser_ops ops;
};

extern struct dcpu_parser * dcpu_parser_new(const char *name,
		dcpu_word *ram, unsigned ram_words);


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


#endif // __included_dcpu_parser_h__
