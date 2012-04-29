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

/* this function dumps registers using ANSI colour codes */
static void dump_oneline(struct dcpu_vcpu *vcpu,
		const struct dcpu_vcpu_state *old, FILE *out)
{
#define DUMP_HDR "TIME(d)  ---A ---B ---C ---X ---Y ---Z ---I ---J   --PC --SP --EX --IA  SK  OP------------- ISN------------------------------\n"
#define Ft  RSTCLR     "%04x" CLR(B,BLACK) "(" CLR(N,GREEN) "%d" CLR(B,BLACK) ")"
#define Fr  "%s"       "%04x" RSTCLR
#define Fsk CLR(B,RED) "%s"   RSTCLR
#define DUMP_FMT Ft"  "Fr" "Fr" "Fr" "Fr" "Fr" "Fr" "Fr" "Fr"   "Fr" "Fr" "Fr" "Fr"  "Fsk RSTCLR "  %-15s %s\n"

	if (vcpu) {
		char hex_buf[1024];
		char asm_buf[1024];

		dcpu_generate_hex(hex_buf, sizeof(hex_buf),
				dcpu_vcpu_current_op(vcpu));

		dcpu_generate_asm(asm_buf, sizeof(asm_buf),
				dcpu_vcpu_current_op(vcpu));

#define Dr(reg) \
		(!old)                    ? CLR(N,WHITE) : \
		(vcpu->st.reg < old->reg) ? CLR(B,RED) : \
		(vcpu->st.reg > old->reg) ? CLR(B,GREEN) : \
		                            CLR(N,WHITE), \
		vcpu->st.reg

		fprintf(out, DUMP_FMT,
				(dcpu_word)vcpu->st.cycles,
				old ? vcpu->st.cycles - old->cycles : 0,
				Dr(gr.a),
				Dr(gr.b),
				Dr(gr.c),
				Dr(gr.x),
				Dr(gr.y),
				Dr(gr.z),
				Dr(gr.i),
				Dr(gr.j),
				Dr(sr.pc),
				Dr(sr.sp),
				Dr(sr.ex),
				Dr(sr.ia),
				vcpu->st.skipping ? "sk" : "  ",
				hex_buf, asm_buf);
	} else
		fprintf(out, DUMP_HDR);
}


int debug_callback(struct dcpu_vcpu *vcpu)
{
	static struct dcpu_vcpu_state old;

	dump_oneline(vcpu, &old, stdout);

	// if there is no hardware, and the PC didn't change, there is no hope
	if (old.sr.pc == vcpu->st.sr.pc && !vcpu->hw_count) {
		warn("PC didn't change, bailing out.");
		return -ENODEV;
	}

	dcpu_vcpu_backup_state(vcpu, &old);

	return 0;
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

	dcpu_vcpu_set_debug(vcpu, debug_callback);
	dump_oneline(NULL, NULL, stdout);
	dump_oneline(vcpu, NULL, stdout);
	
	dcpu_add_clock(vcpu);
	dcpu_add_keyboard(vcpu);
	dcpu_add_lem1802(vcpu);

	rc = vcpu->ops.run(vcpu);

	return rc;
}

