#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <curses.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "dcpu_vm_curses.h"
#include "dcpu_vcpu.h"
#include "dcpu_util.h"
#include "dcpu_generator.h"

#include "curses_utils.h"

#include "uput.h"

#include "dcpu_lem1802.h"
#define LEM1802_SCREEN_COLS   (DCPU_LEM1802_SCREEN_COLS + 4)
#define LEM1802_SCREEN_LINES  (DCPU_LEM1802_SCREEN_LINES + 4)

#define REGS_COLS 20
#define CMD_ROWS 3
#define TRACE_LINES 100 //1000

#define CTRL(x) ((x) & 0x1f)

// ------------------------------------------------------------------------
// module-global curses debug state

struct curses_debugger_state {
	WINDOW *scr;
	WINDOW *reg;
	WINDOW *mem;
	WINDOW *trc;
	WINDOW *trc_pad;
	unsigned trc_top_line;

	struct {
		struct dcpu_vcpu *vcpu;
		struct dcpu_vcpu_state last_state;
	} vcpu[1];
};

static struct curses_debugger_state curses_debugger_state = {0,};


static void curses_stop(void)
{
	erase();
	echo();
	nocbreak();
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
	keypad(stdscr, TRUE);

	erase();
	refresh();
}

// ------------------------------------------------------------------------
// scrollable output/tracing window

// print the new line in the pad
static inline void __debug_trace_printf(struct curses_debugger_state *cds,
		const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwprintw(cds->trc_pad, fmt, ap);
	va_end(ap);
}

static inline void __debug_trace_addstr(struct curses_debugger_state *cds,
		const char *str)
{
	waddstr(cds->trc_pad, str);
}

// made changes to made visible on the screen
static void debug_trace_commit(struct curses_debugger_state *cds)
{
	int vbegy, vbegx, vmaxy, vmaxx;
	int pcury;

	// get dimensions of the trace window
	getbegyx(cds->trc, vbegy, vbegx);
	getmaxyx(cds->trc, vmaxy, vmaxx);

	// figure out what to show
	pcury = getcury(cds->trc_pad);
	if (pcury > vmaxy) {
		int top_line_max = TRACE_LINES - vmaxy - 2;
		if (cds->trc_top_line <= top_line_max)
			cds->trc_top_line ++;
	}

	// copy the visible part of the pad into trace window
	werase(cds->trc);
	wnoutrefresh(cds->trc);
	pnoutrefresh(cds->trc_pad,
			/* start in pad */
			cds->trc_top_line, 0,
			/* where to place */
			vbegy, vbegx,
			vbegy+vmaxy-1, vbegx+vmaxx-1);

	doupdate();
}

static inline void debug_trace_rewind(struct curses_debugger_state *cds)
{
	wmove(cds->trc_pad, getcury(cds->trc_pad), 0);
	wclrtoeol(cds->trc_pad);
}

// print a line, and commit
#define debug_trace_printf(cds,fmt,a...) ({ \
		__debug_trace_printf(cds, fmt, ##a); \
		debug_trace_commit(cds); \
	})

// scroll pad up/down
static void debug_trace_scroll(struct curses_debugger_state *cds, int lines)
{
	//beep();
	flash();
}




// ------------------------------------------------------------------------
// tracer

#define DUMP_HDR "TIME(d)  ---A ---B ---C ---X ---Y ---Z ---I ---J   --PC --SP --EX --IA  SK  OP------------- ISN------------------------------\n"

static void dump_header(struct curses_debugger_state *cds)
{
	debug_trace_rewind(cds);

	init_pair(54, COLOR_WHITE, COLOR_BLACK);

	wattrset(cds->trc_pad, COLOR_PAIR(54)|A_BOLD);

	debug_trace_printf(cds, DUMP_HDR);
}

/* this function dumps registers using ANSI colour codes */
static void dump_oneline(struct dcpu_vcpu *vcpu,
		const struct dcpu_vcpu_state *old,
		struct curses_debugger_state *cds)
{
	char hex_buf[1024];
	char asm_buf[1024];
	int i;

	dcpu_generate_hex(hex_buf, sizeof(hex_buf),
			dcpu_vcpu_current_op(vcpu));

	dcpu_generate_asm(asm_buf, sizeof(asm_buf),
			dcpu_vcpu_current_op(vcpu));

	init_pair(54, COLOR_WHITE, COLOR_BLACK);
	init_pair(55, COLOR_BLACK, COLOR_BLACK);
	init_pair(56, COLOR_GREEN, COLOR_BLACK);
	init_pair(57, COLOR_RED,   COLOR_BLACK);

#define Creset   wattrset(cds->trc_pad, COLOR_PAIR(54))
#define Cdark    wattrset(cds->trc_pad, COLOR_PAIR(55)|A_BOLD)
#define Cbold    wattrset(cds->trc_pad, COLOR_PAIR(54)|A_BOLD)

#define Cgreen   wattrset(cds->trc_pad, COLOR_PAIR(56)|A_BOLD)
#define Cred     wattrset(cds->trc_pad, COLOR_PAIR(57)|A_BOLD)

	debug_trace_rewind(cds);

	Cbold;  __debug_trace_printf(cds, "%04x", (dcpu_word)vcpu->st.cycles);
	Cdark;  __debug_trace_addstr(cds, "(");
	Creset; __debug_trace_printf(cds, "%lu",
			old ? vcpu->st.cycles - old->cycles : 0);
	Cdark;  __debug_trace_addstr(cds, ")");
	Creset; __debug_trace_addstr(cds, "  ");

	for (i=0; i<DCPU_NUM_REGS; i++) {
		if (!old)
			Creset;
		else if (vcpu->st.gr.n[i] < old->gr.n[i])
			Cred;
		else if (vcpu->st.gr.n[i] > old->gr.n[i])
			Cgreen;
		else
			Creset;
		
		__debug_trace_printf(cds, "%04x", vcpu->st.gr.n[i]);
		Creset;
		__debug_trace_addstr(cds, " ");
	}

	__debug_trace_addstr(cds, "  ");

	for (i=0; i<DCPU_NUM_SREGS; i++) {
		if (!old)
			Creset;
		else if (vcpu->st.sr.n[i] < old->sr.n[i])
			Cred;
		else if (vcpu->st.sr.n[i] > old->sr.n[i])
			Cgreen;
		else
			Creset;
		
		__debug_trace_printf(cds, "%04x", vcpu->st.sr.n[i]);
		Creset;
		__debug_trace_addstr(cds, " ");
	}

	if (vcpu->st.skipping) {
		__debug_trace_addstr(cds, " ");
		Cred;
		__debug_trace_addstr(cds, "sk");

		Creset;
		__debug_trace_addstr(cds, "  ");
	} else {
		__debug_trace_addstr(cds, "     ");
	}

	Cbold;
	debug_trace_printf(cds, "%-15s %s\n",
			hex_buf, asm_buf);
}


// handle change
static int dcpu_curses_anything(struct dcpu_vcpu *vcpu)
{
	struct dcpu_vcpu_state *last_state;

	last_state = &curses_debugger_state.vcpu[0].last_state;
	dump_oneline(vcpu, last_state, &curses_debugger_state);

	// if there is no hardware, and the PC didn't change, there is no hope
	if (last_state->sr.pc == vcpu->st.sr.pc && !vcpu->hw_count) {
		warn("PC didn't change, bailing out.");
		return -ENODEV;
	}

	dcpu_vcpu_backup_state(vcpu, last_state);

	return 0;
}

static int dcpu_curses_halt(struct dcpu_vcpu *vcpu)
{
	struct curses_debugger_state *cds = &curses_debugger_state;

	init_pair(53, COLOR_YELLOW, COLOR_BLACK);
	init_pair(54, COLOR_WHITE,  COLOR_BLACK);

	debug_trace_rewind(cds);

	wattrset(cds->trc_pad, COLOR_PAIR(53)|A_BOLD);
	debug_trace_printf(cds, "Execution halted.\n");
	wattrset(cds->trc_pad, COLOR_PAIR(54));

	return -EFAULT;
}

static struct dcpu_vcpu_debug_ops curses_ops = {
	.post_isn = dcpu_curses_anything,
	.post_int = dcpu_curses_anything,
	.halt     = dcpu_curses_halt,
};

// ------------------------------------------------------------------------
// debugger interface

static void curses_add_vcpu(struct dcpu_vcpu *vcpu)
{
	if (curses_debugger_state.vcpu[0].vcpu)
		die("curses debugger only supports a single dcpu");

	dcpu_vcpu_set_debug_ops(vcpu, &curses_ops);

	curses_debugger_state.vcpu[0].vcpu = vcpu;
}

static void create_debugger_windows(struct curses_debugger_state *cds)
{
	erase();
	refresh();

	// screen

	cds->scr = new_frame(LEM1802_SCREEN_LINES, LEM1802_SCREEN_COLS, 0, 0);

	// regs

	cds->reg = new_frame(LEM1802_SCREEN_LINES, REGS_COLS,
			0, LEM1802_SCREEN_COLS);

	// memory

	cds->mem = new_frame(LEM1802_SCREEN_LINES, COLS - LEM1802_SCREEN_COLS - REGS_COLS,
			0, LEM1802_SCREEN_COLS + REGS_COLS);

	// trace

	cds->trc = new_frame(LINES - LEM1802_SCREEN_LINES - CMD_ROWS, COLS,
			LEM1802_SCREEN_LINES, 0);
	cds->trc_pad = newpad(TRACE_LINES, COLS);
	cds->trc_top_line = 0;

	wmove(cds->trc_pad, 0, 0);
	scrollok(cds->trc_pad, 1);

	// console

	draw_rect(CMD_ROWS, COLS,
			LINES - CMD_ROWS, 0);
	refresh();
}

// ------------------------------------------------------------------------
// debugger commands

static int debug_shell_run(struct curses_debugger_state *cds, char *cmd)
{
	struct dcpu_vcpu_state *last_state;
	struct dcpu_vcpu *vcpu;
	int rc;

	vcpu = cds->vcpu[0].vcpu;
	last_state = &cds->vcpu[0].last_state;

	dump_header(cds);
	dump_oneline(vcpu, last_state, cds);

	while(!vcpu->st.halted) {
		rc = vcpu->ops.step(vcpu);
		if (rc<0)
			break;
	}

	dump_header(cds);

	if (vcpu->st.halted)
		return vcpu->st.hcf_code;

	return 0;
}

static int debug_shell_step(struct curses_debugger_state *cds, char *cmd)
{
	return -1;
}

static int debug_shell_load(struct curses_debugger_state *cds, char *cmd)
{
	return -1;
}

static int debug_shell_reload(struct curses_debugger_state *cds, char *cmd)
{
	return -1;
}

static int debug_shell_help(struct curses_debugger_state *cds, char *cmd)
{
	return -1;
}

static int debug_shell_reexec(struct curses_debugger_state *cds, char *cmd)
{
	execlp("dcpu-vm", "dcpu-vm", "-a", "x", NULL);
	return -1;
}

struct debug_shell_command {
	char *name;
	int (*fn)(struct curses_debugger_state *cds, char *cmd);
} debug_shell_commands[] = {
	/* main debugger commands */
	{ "run",    debug_shell_run      },
	{ "step",   debug_shell_step     },
	{ "load",   debug_shell_load     },
	{ "reload", debug_shell_reload   },
	{ "help",   debug_shell_help     },
	/* quit the debugger */
	{ "quit",   NULL },
	{ "exit",   NULL },
	/* for debug */
	{ "exec",   debug_shell_reexec   },
	/* the end */
	{ NULL, NULL }
};

static struct debug_shell_command *debug_shell_command_find(const char *_cmd)
{
	int cmdlen;
	char *cmd;
	char *tok;
	struct debug_shell_command *c;

	cmdlen = strlen(_cmd);
	if (!cmdlen)
		return NULL;

	// duplicate on the stack
	cmd = strndupa(_cmd, cmdlen);

	// consume leading white space
	while(*cmd && isspace(*cmd)) cmd++;
	
	tok = strsep(&cmd, " \t\n\r\f\v");

	for (c = debug_shell_commands; c->name; c++) {
		if (strcmp(tok, c->name))
			continue;

		return c;
	}

	return NULL;
}

static int debug_shell(struct curses_debugger_state *cds)
{
	char cmd[UPUT_MAX_LENGTH];
	struct debug_shell_command *c;
	int rc;
	
	rc = 0;
	for(;;) {
		int rc;
		
		*cmd = 0;
		rc = uput(LINES - CMD_ROWS + 1, 1, COLS - 2, 7, 0, cmd, true,
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"1234567890-=\\`"
				"!@#$%^&*()_+|~"
				"{}[]:;\"'<>,./?");
		switch (rc) {
		case UPUT_EXIT_ENTER:
			c = debug_shell_command_find(cmd);
			if (!c) {
				debug_trace_printf(cds,
					"unknown command in '%s'\n", cmd);
				break;
			}
			if (!c->fn) {
				rc = 0;
				goto bail;
			}

			rc = c->fn(cds, cmd);
			break;

		case UPUT_EXIT_UP_ARROW:
			debug_trace_scroll(cds, -1);
			break;

		case UPUT_EXIT_DOWN_ARROW:
			debug_trace_scroll(cds, 1);
			break;

		case UPUT_EXIT_ESCAPE:
		case UPUT_EXIT_TAB:
		case UPUT_EXIT_SHIFT_TAB:
			debug_trace_printf(cds, "# rc=%u\n", rc);
			break;

		default:
			break;
		}

	}
bail:

	return rc;
}

// ------------------------------------------------------------------------
// debugger FSM

static int curses_start_debugger(void)
{
	struct curses_debugger_state *cds = &curses_debugger_state;
	struct dcpu_vcpu *vcpu;
	struct dcpu_vcpu_state *last_state;

	last_state = &cds->vcpu[0].last_state;
	vcpu = cds->vcpu[0].vcpu;

	if (!vcpu)
		die("need to specify dcpu to debug");

	curses_start();
	create_debugger_windows(cds);

	dcpu_vcpu_backup_state(vcpu, last_state);

	debug_shell(cds);

	curses_stop();

	return 0;
}

struct dcpu_vm_debugger curses_debugger = {
	.add_vcpu = curses_add_vcpu,
	.start = curses_start_debugger,
};
