#ifndef __included_0x10c_generator_h__
#define __included_0x10c_generator_h__

#include <unistd.h>

#include "0x10c_op.h"

extern char * x10c_generate_line(char *buf, size_t buf_len,
		const x10c_op_t *op);

#endif // __included_0x10c_generator_h__
