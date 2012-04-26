#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "0x10c_vcpu.h"
#include "0x10c_isn.h"
#include "0x10c_util.h"
#include "0x10c_generator.h"

#if 1
#define dbg(f,a...) do { /* nothing */ } while(0)
#else
#define dbg(f,a...) printf(f,##a)
#endif

static x10c_word * x10c_vcpu_get_isn_arg(struct x10c_vcpu *vcpu,
		x10c_word arg_desc, x10c_word *scratch, int is_b)
{
	x10c_word ofs;

	switch(arg_desc) {
	case X10C_REG_A ... X10C_REG_J:
		/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
		return &vcpu->gr.n[arg_desc];

	case X10C_ARG_MREF_REG(A) ... X10C_ARG_MREF_REG(J):
		/* 0x08-0x0f: [register] */
		ofs = vcpu->gr.n[arg_desc - X10C_ARG_MREF_REG_BASE];
		return vcpu->ram + ofs;

	case X10C_ARG_MREF_OFS_REG(A) ... X10C_ARG_MREF_OFS_REG(J):
		/* 0x10-0x17: [next word + register] */
		ofs = vcpu->gr.n[arg_desc - X10C_ARG_MREF_OFS_REG_BASE];
		ofs += x10c_vcpu_advance_pc_word(vcpu);
		return vcpu->ram + ofs;

	case X10C_REG_PUSH_POP:
		if (is_b) {
			// this is a push
			ofs = -- vcpu->sr.sp;
			return vcpu->ram + ofs;
		} else {
			// this is a pop
			ofs = vcpu->sr.sp ++;
			return vcpu->ram + ofs;
		}

	case X10C_REG_PEEK:
		ofs = vcpu->sr.sp;
		return vcpu->ram + ofs;

	case X10C_REG_PICK:
		die("PICK not yet implemented");
		ofs = vcpu->sr.sp;
		return vcpu->ram + ofs;

	case X10C_REG_SP:
	case X10C_REG_PC:
	case X10C_REG_EX:
		return &vcpu->sr.n[arg_desc - X10C_REG_SP];

	case X10C_MREF_NEXT_WORD:
		ofs = x10c_vcpu_advance_pc_word(vcpu);
		return vcpu->ram + ofs;

	case X10C_NEXT_WORD:
		*scratch = x10c_vcpu_advance_pc_word(vcpu);
		return scratch;

	default:
		*scratch = arg_desc - X10C_ARG_LITERAL_BASE;
		return scratch;
	}
}

static int x10c_vcpu_step (struct x10c_vcpu *vcpu)
{
	int rc;
	x10c_op_t *op;
	struct x10c_isn *isn;
	x10c_word *a, *b;
	x10c_word tmp_a, tmp_b;

	op = x10c_vcpu_current_op(vcpu);

	isn = x10c_lookup_isn_for_op(op);

	dbg("# step pc=%04x %s\n", vcpu->sr.pc, isn->op_name);

	// consume this word
	vcpu->sr.pc ++;

	if (x10c_op_is_basic(op)) {
		a = x10c_vcpu_get_isn_arg(vcpu, op->b.a, &tmp_a, 0);
		b = x10c_vcpu_get_isn_arg(vcpu, op->b.b, &tmp_b, 1);
	} else {
		a = x10c_vcpu_get_isn_arg(vcpu, op->x.a, &tmp_a, 0);
		b = NULL;
	}

	if (!vcpu->skip_next_op) {
		rc = isn->ops.execute(isn, op, a, b, vcpu);
		if (rc<0)
			return rc;
	} else {
		vcpu->skip_next_op = 0;
	}


	return 0;
}

static int x10c_vcpu_run (struct x10c_vcpu *vcpu)
{
	int rc;

	vcpu->ops.dump(NULL, stdout);
	vcpu->ops.dump(vcpu, stdout);

	for(;;) {
		x10c_word old_pc;

		old_pc = vcpu->sr.pc;

		rc = vcpu->ops.step(vcpu);
		if (rc<0)
			return rc;

		vcpu->ops.dump(vcpu, stdout);

		if (old_pc == vcpu->sr.pc)
			return -EINTR;
	}
}

static void x10c_vcpu_dump(struct x10c_vcpu *vcpu, FILE *out)
{
#define DUMP_HDR "---A ---B ---C ---X ---Y ---Z ---I ---J   --PC --SP --EX  SK  OP------------- ISN------------------------------\n"
#define DUMP_FMT "%04x %04x %04x %04x %04x %04x %04x %04x   %04x %04x %04x  %s  %-15s %s\n"

	if (vcpu) {
		char hex_buf[1024];
		char asm_buf[1024];

		x10c_generate_hex(hex_buf, sizeof(hex_buf),
				x10c_vcpu_current_op(vcpu));

		x10c_generate_asm(asm_buf, sizeof(asm_buf),
				x10c_vcpu_current_op(vcpu));

		fprintf(out, DUMP_FMT,
				vcpu->gr.a, vcpu->gr.b, vcpu->gr.c,
				vcpu->gr.x, vcpu->gr.y, vcpu->gr.z,
				vcpu->gr.i, vcpu->gr.j,
				vcpu->sr.pc, vcpu->sr.sp, vcpu->sr.ex,
				vcpu->skip_next_op ? "sk" : "  ",
				hex_buf, asm_buf);
	} else
		fprintf(out, DUMP_HDR);
}

static void x10c_vcpu_delete (struct x10c_vcpu *vcpu)
{
	free(vcpu);
}

static struct x10c_vcpu_ops x10c_vcpu_ops = {
	.step   = x10c_vcpu_step,
	.run    = x10c_vcpu_run,
	.dump   = x10c_vcpu_dump,
	.delete = x10c_vcpu_delete,
};

struct x10c_vcpu * x10c_vcpu_new(void)
{
	struct x10c_vcpu *vcpu;

	vcpu = calloc(1, sizeof(*vcpu));

	vcpu->ops = x10c_vcpu_ops;

	return vcpu;
}
