#include "0x10c_exec.h"

#include "0x10c_op.h"
#include "0x10c_isn.h"
#include "0x10c_vcpu.h"

int x10c_execute_SET(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *b;

	return 0;
}

int x10c_execute_ADD(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	uint32_t sum = (uint32_t)*a + (uint32_t)*b;

	*a = sum;
	vcpu->sr.ex = sum >> 16;

	return 0;
}

int x10c_execute_SUB(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	uint32_t diff = (uint32_t)*a - (uint32_t)*b;

	*a = diff;
	vcpu->sr.ex = diff >> 16;

	return 0;
}

int x10c_execute_MUL(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	uint32_t prod = (uint32_t)*a + (uint32_t)*b;

	*a = prod;
	vcpu->sr.ex = prod >> 16;

	return 0;
}

int x10c_execute_DIV(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	if (*b) {
		uint32_t A = *a;
		*a = A / *b;
		vcpu->sr.ex = (A<<16) / *b;

	} else {
		*a = 0;
		vcpu->sr.ex = 0;
	}

	return 0;
}

int x10c_execute_MOD(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	if (*b) {
		uint32_t A = *a;
		*a = A % *b;

	} else {
		*a = 0;
	}

	return 0;
}

int x10c_execute_SHL(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *a << *b;

	return 0;
}

int x10c_execute_SHR(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *a >> *b;

	return 0;
}

int x10c_execute_AND(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *a & *b;

	return 0;
}

int x10c_execute_BOR(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *a | *b;

	return 0;
}

int x10c_execute_XOR(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	*a = *a ^ *b;

	return 0;
}

int x10c_execute_IFE(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	vcpu->skip_next_op = !( *a == *b );

	return 0;
}

int x10c_execute_IFN(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	vcpu->skip_next_op = !( *a != *b );

	return 0;
}

int x10c_execute_IFG(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	vcpu->skip_next_op = !( *a > *b );

	return 0;
}

int x10c_execute_IFB(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	vcpu->skip_next_op = !( *a & *b );

	return 0;
}

int x10c_execute_JSR(const struct x10c_isn *isn, const x10c_op_t *op,
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)
{
	/* push address of next instruction onto the stack */
	vcpu->ram [ --vcpu->sr.sp ] = vcpu->sr.pc;

	/* set the program counter to a */
	vcpu->sr.pc = *a;

	return 0;
}

