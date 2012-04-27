#ifndef __included_0x10c_generator_lib_h__
#define __included_0x10c_generator_lib_h__

#include <unistd.h>

#include "0x10c_op.h"
struct x10c_isn;

extern char * x10c_basic_generator(const struct x10c_isn *isn,
		char *buf, size_t buf_len, const x10c_op_t *op);
extern char * x10c_non_basic_generator(const struct x10c_isn *isn,
		char *buf, size_t buf_len, const x10c_op_t *op);

#endif // __included_0x10c_generator_lib_h__
