#ifndef __included_0x10c_parser_lib_h__
#define __included_0x10c_parser_lib_h__

#include <unistd.h>

#include "0x10c_op.h"
struct x10c_isn;

extern int x10c_basic_parser(const struct x10c_isn *isn,
		x10c_op_t *op, const char *buf, size_t buf_len);
extern int x10c_non_basic_parser(const struct x10c_isn *isn,
		x10c_op_t *op, const char *buf, size_t buf_len);

#endif // __included_0x10c_parser_lib_h__
