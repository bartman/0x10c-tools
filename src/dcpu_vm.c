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

static struct dcpu_vcpu *dcpu_vm_create_vcpu(void)
{
	struct dcpu_vcpu *vcpu;

	vcpu = dcpu_vcpu_new();

	if (dcpu_vm_opts.assembly_file) {
		struct dcpu_parser *pr;

		pr = dcpu_parser_new(dcpu_vm_opts.assembly_file,
				vcpu->ram, DCPU_RAM_WORDS);
		pr->ops.parse_file(pr, dcpu_vm_opts.assembly_file);
		//pr->ops.dump(pr, stdout);
		pr->ops.delete(pr);

	} else if (dcpu_vm_opts.binary_file) {

	} else if (dcpu_vm_opts.snapshot_file) {

	}

	// add some hardware
	dcpu_add_clock(vcpu);
	dcpu_add_keyboard(vcpu);
	dcpu_add_lem1802(vcpu);

	// add debugger
	dcpu_vcpu_set_debugger(vcpu, dcpu_vm_opts.debugger);

	return vcpu;
}


int main(int argc, char *argv[])
{
	const char *filename;
	struct dcpu_parser *pr;
	struct dcpu_vcpu *vcpu;
	int rc;

	dcpu_vm_parse_cmdline(argc, argv);

	vcpu = dcpu_vm_create_vcpu();

	rc = vcpu->ops.run(vcpu);

	return rc;
}

