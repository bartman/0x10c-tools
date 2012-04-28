#ifndef __included_dcpu_util_h__
#define __included_dcpu_util_h__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "dcpu_def.h"
#include "dcpu_colours.h"

#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)

static inline void dcpu_dump(FILE *out, dcpu_word *data, unsigned count)
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

#define warn(fmt,a...) ({ \
		fflush(stdout); \
		fprintf(stderr, CLR(B,YELLOW) "WARN: " fmt RSTCLR "\n", ##a); \
		})
#define error(fmt,a...) ({ \
		fflush(stdout); \
		fprintf(stderr, CLR(B,RED) "ERROR: " fmt RSTCLR "\n", ##a); \
		})
#define die(fmt,a...) ({ \
		error(fmt, ##a); \
		exit(EXIT_FAILURE); \
		})


#endif // __included_dcpu_util_h__
