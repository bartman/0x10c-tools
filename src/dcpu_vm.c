#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dcpu_op.h"
#include "dcpu_isn.h"
#include "dcpu_vcpu.h"
#include "dcpu_parser.h"
#include "dcpu_util.h"
#include "dcpu_clock.h"
#include "dcpu_keyboard.h"
#include "dcpu_vm_opts.h"
#include "dcpu_vm_tracer.h"
#include "dcpu_vm_curses.h"

int main(int argc, char *argv[])
{
	const char *filename;
	struct dcpu_parser *pr;
	struct dcpu_vcpu *vcpu;
	int rc;

	dcpu_vm_parse_cmdline(argc, argv);

	filename = argv[1];
	if (!filename)
		die("need a filename");

	vcpu = dcpu_vcpu_new();

	pr = dcpu_parser_new(filename, vcpu->ram, DCPU_RAM_WORDS);

	pr->ops.parse_file(pr, filename);

	//pr->ops.dump(pr, stdout);

	pr->ops.delete(pr);

	//dcpu_vcpu_set_debugger(vcpu, &tracing_debugger);
	dcpu_vcpu_set_debugger(vcpu, &curses_debugger);
	
	dcpu_add_clock(vcpu);
	dcpu_add_keyboard(vcpu);
	dcpu_add_lem1802(vcpu);

	rc = vcpu->ops.run(vcpu);

	return rc;
}

