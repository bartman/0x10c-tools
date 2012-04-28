#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "dcpu_op.h"
#include "dcpu_isn.h"
#include "dcpu_vcpu.h"
#include "dcpu_parser.h"
#include "dcpu_util.h"
#include "dcpu_clock.h"

int main(int argc, char *argv[])
{
	const char *filename;
	struct dcpu_parser *pr;
	//struct dcpu_parsed_line *pl;
	struct dcpu_vcpu *vcpu;

	filename = argv[1];
	if (!filename)
		die("need a filename");

	vcpu = dcpu_vcpu_new();
	
	pr = dcpu_parser_new(filename, vcpu->ram, DCPU_RAM_WORDS);

	pr->ops.parse_file(pr, filename);

	pr->ops.dump(pr, stdout);

	pr->ops.delete(pr);

	dcpu_add_clock(vcpu);

	return vcpu->ops.run(vcpu);
}

