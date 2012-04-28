#ifndef __included_dcpu_generator_h__
#define __included_dcpu_generator_h__

#include <unistd.h>

#include "dcpu_op.h"

extern char * dcpu_generate_asm(char *buf, size_t buf_len,
		const dcpu_op_t *op);
extern char * dcpu_generate_hex(char *buf, size_t buf_len,
		const dcpu_op_t *op);

#endif // __included_dcpu_generator_h__
