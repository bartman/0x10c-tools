#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "0x10c_op.h"
#include "0x10c_isn.h"
#include "0x10c_vcpu.h"
#include "0x10c_parser.h"

int main(int argc, char *argv[])
{
	const char *filename;
	struct x10c_parser *pr;
	//struct x10c_parsed_line *pl;
	struct x10c_vcpu vcpu;
	FILE *f;

	filename = argv[1];

	pr = x10c_parser_new(filename, vcpu.ram, X10C_RAM_WORDS);
	
	f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "%s: %s\n",
				filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	pr->ops.parse_file(pr, f);

	pr->ops.dump(pr, stdout);

	pr->ops.delete(pr);

	return 0;
}

