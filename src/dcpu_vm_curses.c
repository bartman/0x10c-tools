#include <stdio.h>
#include <errno.h>
#include <curses.h>
#include <string.h>
#include <stdarg.h>

#include "dcpu_vm_curses.h"
#include "dcpu_vcpu.h"
#include "dcpu_util.h"
#include "dcpu_generator.h"
#include "dcpu_colours.h"

#include "uput.h"

#define CTRL(x) ((x) & 0x1f)

static void curses_stop(void)
{
	erase();
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
#define Ft  RSTCLR     "%04x" CLR(B,BLACK) "(" CLR(N,GREEN) "%ld" CLR(B,BLACK) ")"
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

static struct dcpu_vcpu_debug_ops curses_ops = {
	.post_isn = dcpu_curses_anything,
	.post_int = dcpu_curses_anything,
};

// ------------------------------------------------------------------------
// debugger interface

static struct dcpu_vcpu *curses_vcpu = NULL;

static void curses_add_vcpu(struct dcpu_vcpu *vcpu)
{
	if (curses_vcpu)
		die("curses debugger only supports a single dcpu");

	dcpu_vcpu_set_debug_ops(vcpu, &curses_ops);

	curses_vcpu = vcpu;
}

#include "dcpu_lem1802.h"
#define LEM1802_SCREEN_COLS   (DCPU_LEM1802_SCREEN_COLS + 4)
#define LEM1802_SCREEN_LINES  (DCPU_LEM1802_SCREEN_LINES + 4)

static void draw_rect(int height, int width, int line, int col)
{
	mvaddch(line,              col,             ACS_ULCORNER);
	mvaddch(line + height - 1, col,             ACS_LLCORNER);
	mvaddch(line,              col + width - 1, ACS_URCORNER);
	mvaddch(line + height - 1, col + width - 1, ACS_LRCORNER);

	move(line, col + 1);
	hline(ACS_HLINE, width - 2);

	move(line + height - 1, col + 1);
	hline(ACS_HLINE, width - 2);

	move(line + 1, col);
	vline(ACS_VLINE, height - 2);

	move(line + 1, col + width - 1);
	vline(ACS_VLINE, height - 2);
}

static WINDOW * new_frame(int height, int width, int line, int col)
{
	WINDOW *win;

	//win = newwin(height, width, line, col);
	win = subwin(stdscr, height - 4, width - 4, line + 2, col + 2);

	draw_rect(height, width, line, col);

	//box(win, ACS_VLINE, ACS_HLINE);

#if 1
	{
	int y;
	for (y=0; y<height; y++)
		mvwprintw(win, y, 8, " %u ", y);

	mvwprintw(win, 0, 8, "size{%u,%u} start{%u,%u}",
			height, width, line, col);
	}
#endif

	return win;
}

struct debug_state {
	WINDOW *scr;
	WINDOW *reg;
	WINDOW *mem;
	WINDOW *trc;
	WINDOW *trc_pad;
	unsigned trc_top_line;
};

static struct debug_state debug_state = {0,};


#define REGS_COLS 20
#define CMD_ROWS 3
#define TRACE_LINES 100 //1000
static void create_debugger_windows(struct debug_state *ds)
{
	erase();
	refresh();

	// screen

	ds->scr = new_frame(LEM1802_SCREEN_LINES, LEM1802_SCREEN_COLS, 0, 0);

	// regs

	ds->reg = new_frame(LEM1802_SCREEN_LINES, REGS_COLS,
			0, LEM1802_SCREEN_COLS);

	// memory

	ds->mem = new_frame(LEM1802_SCREEN_LINES, COLS - LEM1802_SCREEN_COLS - REGS_COLS,
			0, LEM1802_SCREEN_COLS + REGS_COLS);

	// trace

	ds->trc = new_frame(LINES - LEM1802_SCREEN_LINES - CMD_ROWS, COLS,
			LEM1802_SCREEN_LINES, 0);
	ds->trc_pad = newpad(TRACE_LINES, COLS);
	ds->trc_top_line = 0;

	wmove(ds->trc_pad, 0, 0);
	scrollok(ds->trc_pad, 1);

	// console

	draw_rect(CMD_ROWS, COLS,
			LINES - CMD_ROWS, 0);
	refresh();
}

static void debug_trace_append(struct debug_state *ds,
		const char *fmt, ...)
{
	int vbegy, vbegx, vmaxy, vmaxx;
	int pcury, pcurx;
	va_list ap;

	// get dimensions of the trace window
	getbegyx(ds->trc, vbegy, vbegx);
	getmaxyx(ds->trc, vmaxy, vmaxx);

	// current location in pad
	getyx(ds->trc_pad, pcury, pcurx);

	// print the new line in the pad
	va_start(ap, fmt);
	vwprintw(ds->trc_pad, fmt, ap);
	va_end(ap);

	// figure out what to show
	pcury = getcury(ds->trc_pad);
	if (pcury > vmaxy) {
		int top_line_max = TRACE_LINES - vmaxy - 2;
		if (ds->trc_top_line <= top_line_max)
			ds->trc_top_line ++;
	}


	// copy the visible part of the pad into trace window
	werase(ds->trc);
	wnoutrefresh(ds->trc);
	pnoutrefresh(ds->trc_pad, 
			/* start in pad */
			ds->trc_top_line, 0,
			/* where to place */
			vbegy, vbegx,
			vbegy+vmaxy-1, vbegx+vmaxx-1);

	doupdate();
}

static void debug_shell(struct debug_state *ds)
{
	char cmd[UPUT_MAX_LENGTH];
	// loop
	
	raw();

	for(;;) {
		int rc;
		
		*cmd = 0;
		rc = uput(LINES - CMD_ROWS + 1, 1, COLS - 2, 7, 0, cmd, false,
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"1234567890-=\\`"
				"!@#$%^&*()_+|~"
				"{}[]:;\"'<>,./?");
		switch (rc) {
		case UPUT_EXIT_ENTER:
			if (!strcasecmp(cmd, "quit")
					|| !strcasecmp(cmd, "exit"))
				goto bail;

			// this is just for quick testing...
			if (!strcasecmp(cmd, "exec"))
				execlp("dcpu-vm", "dcpu-vm", "-a", "x", NULL);

			debug_trace_append(ds, "cmd: %s\n", cmd);
			wrefresh(ds->trc);
			break;
		case UPUT_EXIT_ESCAPE:
		case UPUT_EXIT_UP_ARROW:
		case UPUT_EXIT_DOWN_ARROW:
		case UPUT_EXIT_TAB:
		case UPUT_EXIT_SHIFT_TAB:
			goto bail;
		}

	}
bail:

	noraw();
}

static int curses_start_debugger(void)
{
	int rc = 0;
	struct dcpu_vcpu *vcpu = curses_vcpu;

	if (!vcpu)
		die("need to specify dcpu to debug");

	curses_start();
#if 0
	dump_header(stdout);
	dump_oneline(vcpu, &curses_debugger_state, stdout);
#endif
	dcpu_vcpu_backup_state(vcpu, &curses_debugger_state);

	create_debugger_windows(&debug_state);
	debug_shell(&debug_state);

	while(!vcpu->st.halted) {
		rc = vcpu->ops.step(vcpu);
		if (rc<0)
			break;
	}

	dump_header(stdout);

	if (vcpu->st.halted)
		return vcpu->st.hcf_code;

	return rc;
}

struct dcpu_vm_debugger curses_debugger = {
	.add_vcpu = curses_add_vcpu,
	.start = curses_start_debugger,
};
