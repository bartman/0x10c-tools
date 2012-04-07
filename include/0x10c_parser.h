#ifndef __included_0x10c_parser_h__
#define __included_0x10c_parser_h__

#include <unistd.h>

#include "0x10c_op.h"

extern int x10c_parse_line(x10c_op_t *op, const char *buf, size_t buf_len);

#endif // __included_0x10c_parser_h__
