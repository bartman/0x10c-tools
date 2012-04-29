#include <stdio.h>
#include <errno.h>
#include "dcpu_tracer.h"
#include "dcpu_vcpu.h"
#include "dcpu_generator.h"
#include "dcpu_colours.h"

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
}


int tracer_debug_callback(struct dcpu_vcpu *vcpu, int what)
{
	static struct dcpu_vcpu_state old = {0,};

	if (!vcpu)
		return -EFAULT;

	switch (what) {
	case DCPU_VCPU_DEBUG_INIT:
		dump_header(stdout);
		dump_oneline(vcpu, &old, stdout);
		break;

	case DCPU_VCPU_DEBUG_ISN_DONE:
	case DCPU_VCPU_DEBUG_INTERRUPT:
		dump_oneline(vcpu, &old, stdout);

		// if there is no hardware, and the PC didn't change, there is no hope
		if (old.sr.pc == vcpu->st.sr.pc && !vcpu->hw_count) {
			warn("PC didn't change, bailing out.");
			return -ENODEV;
		}

		break;
	}

	dcpu_vcpu_backup_state(vcpu, &old);
	return 0;
}

