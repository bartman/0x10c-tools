#include <stdio.h>
#include <errno.h>
#include <curses.h>

#include "dcpu_curses.h"
#include "dcpu_vcpu.h"
#include "dcpu_generator.h"
#include "dcpu_colours.h"

static void curses_stop(void)
{
	echo();
	endwin();
}

static void curses_start(void)
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



#define DUMP_HDR "TIME(d)  ---A ---B ---C ---X ---Y ---Z ---I ---J   --PC --SP --EX --IA  SK  OP------------- ISN------------------------------\n"
#define Ft  RSTCLR     "%04x" CLR(B,BLACK) "(" CLR(N,GREEN) "%d" CLR(B,BLACK) ")"
#define Fr  "%s"       "%04x" RSTCLR
#define Fsk CLR(B,RED) "%s"   RSTCLR
#define DUMP_FMT Ft"  "Fr" "Fr" "Fr" "Fr" "Fr" "Fr" "Fr" "Fr"   "Fr" "Fr" "Fr" "Fr"  "Fsk RSTCLR "  %-15s %s\n"

static void dump_header(FILE *out)
{
	fprintf(out, DUMP_HDR);
}

/* this function dumps registers using ANSI colour codes */
static void dump_oneline(struct dcpu_vcpu *vcpu,
		const struct dcpu_vcpu_state *old, FILE *out)
{
	char hex_buf[1024];
	char asm_buf[1024];

	dcpu_generate_hex(hex_buf, sizeof(hex_buf),
			dcpu_vcpu_current_op(vcpu));

	dcpu_generate_asm(asm_buf, sizeof(asm_buf),
			dcpu_vcpu_current_op(vcpu));

	// macro to colourize a register
#define Dr(reg) \
	/* colour first */                          \
	(!old)                    ? CLR(N,WHITE) :  \
	(vcpu->st.reg < old->reg) ? CLR(B,RED) :    \
	(vcpu->st.reg > old->reg) ? CLR(B,GREEN) :  \
				    CLR(N,WHITE),   \
	/* then the register value */               \
	vcpu->st.reg

	// another macro to colourize PC and IA, when they match
#define Ddup(reg, reg2) \
	/* colour first */                          \
	(vcpu->st.reg2 && vcpu->st.reg == vcpu->st.reg2) \
	                          ? CLR(B,YELLOW) : \
	(!old)                    ? CLR(N,WHITE) :  \
	(vcpu->st.reg < old->reg) ? CLR(B,RED) :    \
	(vcpu->st.reg > old->reg) ? CLR(B,GREEN) :  \
				    CLR(N,WHITE),   \
	/* then the register value */               \
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
			Ddup(sr.pc, sr.ia),
			Dr(sr.sp),
			Dr(sr.ex),
			Ddup(sr.ia, sr.pc),
			vcpu->st.skipping ? "sk" : "  ",
			hex_buf, asm_buf);
}


static struct dcpu_vcpu_state curses_debugger_state = {0,};

// first call before program starts executing
static int dcpu_curses_start(struct dcpu_vcpu *vcpu)
{
	curses_start();
	dump_header(stdout);
	dump_oneline(vcpu, &curses_debugger_state, stdout);
	dcpu_vcpu_backup_state(vcpu, &curses_debugger_state);

	return 0;
}

// handle change
static int dcpu_curses_anything(struct dcpu_vcpu *vcpu)
{
	dump_oneline(vcpu, &curses_debugger_state, stdout);

	// if there is no hardware, and the PC didn't change, there is no hope
	if (curses_debugger_state.sr.pc == vcpu->st.sr.pc && !vcpu->hw_count) {
		warn("PC didn't change, bailing out.");
		return -ENODEV;
	}

	dcpu_vcpu_backup_state(vcpu, &curses_debugger_state);

	return 0;
}

// vcpu execution halted
static int dcpu_curses_halt(struct dcpu_vcpu *vcpu)
{
	dump_header(stdout);
	return 0;
}

// all done
static int dcpu_curses_exit(struct dcpu_vcpu *vcpu)
{
	// ignore
	return 0;
}


struct dcpu_debugger curses_debugger = {
	.start = dcpu_curses_start,
	.post_isn = dcpu_curses_anything,
	.post_int = dcpu_curses_anything,
	.halt = dcpu_curses_halt,
	.exit = dcpu_curses_exit,
};

