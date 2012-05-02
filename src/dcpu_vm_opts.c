#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <getopt.h>

#include "dcpu_util.h"

#include "dcpu_vm_opts.h"
#include "dcpu_vm_tracer.h"
#include "dcpu_vm_curses.h"

const char *progname_full = DCPU_VM_PROGNAME;
const char *progname_base = DCPU_VM_PROGNAME;
const char *progname_dir  = ".";

struct dcpu_vm_opts dcpu_vm_opts = {
	.debugger = &curses_debugger,

};

static void dcpu_vm_help(int err)
{
	FILE *out = err ? stderr : stdout;

	fprintf(out,
		"%s [ options ]\n"
		"\n"
		"-h --help                    - this help\n"
		"\n"
		"-d --debugger                - interactive debugger mode\n"
		"-t --trace                   - trace mode (no input/output hw)\n"
		"\n"
		"-a --assemble <dasm-file>    - assemble and execute this .dasm file\n"
		"-b --binary <binary-file>    - execute this binary file\n"
		"-s --snapshot <vm-file>      - store/resume execution in this VM snapshot\n"
		"\n",
		progname_full
	       );

	exit(err);
}

static struct option long_options[] = {
	{"help",       no_argument,       0,  'h' },

	{"debugger",   no_argument,       0,  'd' },
	{"trace",      no_argument,       0,  't' },

	{"assemble",   required_argument, 0,  'a' },
	{"binary",     required_argument, 0,  'b' },
	{"snapshot",   required_argument, 0,  's' },

	{0,            0,                 0,  0 }
};


// command line parser
void dcpu_vm_parse_cmdline(int argc, char *argv[])
{
	int c;

	progname_full = argv[0];
	progname_base = basename(strdup(argv[0]));
	progname_dir  = dirname(strdup(argv[0]));

	for(;;) {
		int option_index = 0;

		c = getopt_long(argc, argv, "hdta:b:s:", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			dcpu_vm_help(0);
			break;

		case 'd':
			dcpu_vm_opts.debugger = &curses_debugger;
			break;

		case 't':
			dcpu_vm_opts.debugger = &tracing_debugger;
			break;

		case 'a':
			dcpu_vm_opts.assembly_file = optarg;
			break;

		case 'b':
			dcpu_vm_opts.binary_file = optarg;
			break;

		case 's':
			dcpu_vm_opts.snapshot_file = optarg;
			break;

		case '?':
			die("could not parse argument '%s'", argv[optind++]);

		default:
			die("unknown option '%c' (0x%0x)", c, c);
		}
	}
}
