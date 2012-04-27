#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "0x10c_op.h"
#include "0x10c_isn.h"
#include "0x10c_vcpu.h"
#include "0x10c_parser.h"
#include "0x10c_util.h"

int main(int argc, char *argv[])
{
	const char *filename;
	struct x10c_parser *pr;
	//struct x10c_parsed_line *pl;
	struct x10c_vcpu *vcpu;

	filename = argv[1];
	if (!filename)
		die("need a filename");

	vcpu = x10c_vcpu_new();
	
	pr = x10c_parser_new(filename, vcpu->ram, X10C_RAM_WORDS);

	pr->ops.parse_file(pr, filename);

	pr->ops.dump(pr, stdout);

	pr->ops.delete(pr);

	vcpu->ops.run(vcpu);

	return 0;
}

