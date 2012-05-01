#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <curses.h>

#include "dcpu_op.h"
#include "dcpu_isn.h"
#include "dcpu_vcpu.h"
#include "dcpu_parser.h"
#include "dcpu_util.h"
#include "dcpu_clock.h"
#include "dcpu_keyboard.h"
#include "dcpu_tracer.h"

void curses_stop(void)
{
	echo();
	endwin();
}

void curses_start(void)
{
	initscr();

	atexit(curses_stop);

	if (has_colors()) {
		start_color();
		use_default_colors();
	}
	cbreak();
	noecho();

	erase();
	refresh();
}

int main(int argc, char *argv[])
{
	const char *filename;
	struct dcpu_parser *pr;
	struct dcpu_vcpu *vcpu;
	int rc;

	filename = argv[1];
	if (!filename)
		die("need a filename");

	//curses_start();

	vcpu = dcpu_vcpu_new();

	pr = dcpu_parser_new(filename, vcpu->ram, DCPU_RAM_WORDS);

	pr->ops.parse_file(pr, filename);

	//pr->ops.dump(pr, stdout);

	pr->ops.delete(pr);

	dcpu_vcpu_set_debugger(vcpu, &tracing_debugger);
	
	dcpu_add_clock(vcpu);
	dcpu_add_keyboard(vcpu);
	dcpu_add_lem1802(vcpu);

	rc = vcpu->ops.run(vcpu);

	return rc;
}

