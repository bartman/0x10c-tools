#ifndef __included_dcpu_parser_lib_h__
#define __included_dcpu_parser_lib_h__

#include <unistd.h>

#include "dcpu_op.h"
struct dcpu_isn;

extern int dcpu_basic_parser(struct dcpu_parser *, const struct dcpu_isn *isn,
		dcpu_op_t *op, char *buf);
extern int dcpu_non_basic_parser(struct dcpu_parser *, const struct dcpu_isn *isn,
		dcpu_op_t *op, char *buf);

#endif // __included_dcpu_parser_lib_h__
