#ifndef __included_0x10c_util_h__
#define __included_0x10c_util_h__

#include <stdio.h>
#include "0x10c_def.h"

#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)

static inline void x10c_dump(FILE *out, x10c_word *data, unsigned count)
{
	int i;

	for (i=0; i< count; i++) {
		if ((i&7) == 0)
			fprintf(out, "%04x:", i);
		fprintf(out, " %04x", data[i]);
		if ((i&7) == 7)
			fprintf(out, "\n");
	}
	if ((i&7) != 0)
		fprintf(out, "\n");
}


#endif // __included_0x10c_util_h__
