#ifndef __included_dcpu_generator_lib_h__
#define __included_dcpu_generator_lib_h__

#include <unistd.h>

#include "dcpu_op.h"
struct dcpu_isn;

extern char * dcpu_basic_generator(const struct dcpu_isn *isn,
		char *buf, size_t buf_len, const dcpu_op_t *op);
extern char * dcpu_non_basic_generator(const struct dcpu_isn *isn,
		char *buf, size_t buf_len, const dcpu_op_t *op);

#endif // __included_dcpu_generator_lib_h__
