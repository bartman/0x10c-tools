#include "dcpu_exec.h"

#include "dcpu_op.h"
#include "dcpu_isn.h"
#include "dcpu_vcpu.h"
#include "dcpu_util.h"
#include "dcpu_fifo.h"
#include "dcpu_hw.h"

DCPU_ISN_HANDLER(SET)
{
	uint16_t A = *a;

	*b = *a;

	return 0;
}

// sets b to b+a, sets EX to 0x0001 if there's an overflow, 0x0 otherwise
DCPU_ISN_HANDLER(ADD)
{
	uint32_t A = *a;
	uint32_t B = *b;
	uint32_t sum = B + A;

	*b = sum;
	vcpu->st.sr.ex = sum >> 16;

	return 0;
}

// sets b to b-a, sets EX to 0xffff if there's an underflow, 0x0 otherwise
DCPU_ISN_HANDLER(SUB)
{
	uint32_t A = *a;
	uint32_t B = *b;
	uint32_t diff = B - A;

	*b = diff;
	vcpu->st.sr.ex = diff >> 16;

	return 0;
}

// sets b to b*a, sets EX to ((b*a)>>16)&0xffff (treats b, a as unsigned)
DCPU_ISN_HANDLER(MUL)
{
	uint32_t A = *a;
	uint32_t B = *b;
	uint32_t prod = B * A;

	*b = prod;
	vcpu->st.sr.ex = prod >> 16;

	return 0;
}

// like MUL, but treat b, a as signed
DCPU_ISN_HANDLER(MLI)
{
	int32_t A = *a;
	int32_t B = *b;
	uint32_t prod = B * A;

	*b = prod;
	vcpu->st.sr.ex = prod >> 16;

	return 0;
}

// sets b to b/a, sets EX to ((b<<16)/a)&0xffff. if a==0, sets b and EX to 0 instead. (treats b, a as unsigned)
DCPU_ISN_HANDLER(DIV)
{
	uint32_t A = *a;
	uint32_t B = *b;

	if (A) {
		*b = B / A;
		vcpu->st.sr.ex = (B<<16) / A;

	} else {
		*b = 0;
		vcpu->st.sr.ex = 0;
	}

	return 0;
}

// like DIV, but treat b, a as signed. Rounds towards 0
DCPU_ISN_HANDLER(DVI)
{
	int32_t A = *a;
	int32_t B = *b;

	if (A) {
		*b = B / A;
		vcpu->st.sr.ex = (B<<16) / A;

	} else {
		*b = 0;
		vcpu->st.sr.ex = 0;
	}

	return 0;
}

// sets b to b%a. if a==0, sets b to 0 instead.
DCPU_ISN_HANDLER(MOD)
{
	uint32_t A = *a;
	uint32_t B = *b;

	if (A) {
		*b = B % A;

	} else {
		*b = 0;
	}

	return 0;
}

// like MOD, but treat b, a as signed. Rounds towards 0
DCPU_ISN_HANDLER(MDI)
{
	int32_t A = *a;
	int32_t B = *b;

	if (A) {
		*b = B % A;

	} else {
		*b = 0;
	}

	return 0;
}

DCPU_ISN_HANDLER(AND)
{
	*b &= *a;

	return 0;
}

DCPU_ISN_HANDLER(BOR)
{
	*b |= *a;

	return 0;
}

DCPU_ISN_HANDLER(XOR)
{
	*b ^= *a;

	return 0;
}

// sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff (logical shift)
DCPU_ISN_HANDLER(SHR)
{
	uint16_t A = *a;
	uint32_t B = *b;

	*b = B >> A;
	vcpu->st.sr.ex = (B<<16) >> A;

	return 0;
}

// sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff (arithmetic shift) (treats b as signed)
DCPU_ISN_HANDLER(ASR)
{
	uint16_t A = *a;
	int32_t B = *b;

	*b = B >> A;
	vcpu->st.sr.ex = (B<<16) >> A;

	return 0;
}

// sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff
DCPU_ISN_HANDLER(SHL)
{
	uint16_t A = *a;
	uint32_t B = *b;

	*b = B << A;
	vcpu->st.sr.ex = B>>16;

	return 0;
}

// performs next instruction only if (b&a)!=0
DCPU_ISN_HANDLER(IFB)
{
	vcpu->st.skipping = !( *b & *a );

	return 0;
}

// performs next instruction only if (b&a)==0
DCPU_ISN_HANDLER(IFC)
{
	vcpu->st.skipping = !!( *b & *a );

	return 0;
}

// performs next instruction only if b==a
DCPU_ISN_HANDLER(IFE)
{
	vcpu->st.skipping = !( *b == *a );

	return 0;
}

// performs next instruction only if b!=a
DCPU_ISN_HANDLER(IFN)
{
	vcpu->st.skipping = !( *b != *a );

	return 0;
}

// performs next instruction only if b>a
DCPU_ISN_HANDLER(IFG)
{
	vcpu->st.skipping = !( *b > *a );

	return 0;
}

// performs next instruction only if b>a (signed)
DCPU_ISN_HANDLER(IFA)
{
	vcpu->st.skipping = !( (int16_t)*b > (int16_t)*a );

	return 0;
}

// performs next instruction only if b<a
DCPU_ISN_HANDLER(IFL)
{
	vcpu->st.skipping = !( *b < *a );

	return 0;
}

// performs next instruction only if b<a (signed)
DCPU_ISN_HANDLER(IFU)
{
	vcpu->st.skipping = !( (int16_t)*b < (int16_t)*a );

	return 0;
}

// sets b to b+a+EX, sets EX to 0x0001 if there is an overflow, 0x0 otherwise
DCPU_ISN_HANDLER(ADX)
{
	uint32_t A = *a;
	uint32_t B = *b;
	uint32_t EX = vcpu->st.sr.ex;
	uint32_t sum = B + A + EX;

	*b = sum;
	vcpu->st.sr.ex = (sum >> 16) ? 1 : 0;

	return 0;
}

// sets b to b-a+EX, sets EX to 0xFFFF if there is an underflow, 0x0 otherwise
DCPU_ISN_HANDLER(SBX)
{
	uint32_t A = *a;
	uint32_t B = *b;
	uint32_t EX = vcpu->st.sr.ex;
	uint32_t diff = B - A + EX;

	*b = diff;
	vcpu->st.sr.ex = (diff >> 16) ? 0xFFFF : 0;

	return 0;
}

// sets b to a, then increases I and J by 1
DCPU_ISN_HANDLER(STI)
{
	*b = *a;

	vcpu->st.gr.i ++;
	vcpu->st.gr.j ++;

	return 0;
}

// sets b to a, then decreases I and J by 1
DCPU_ISN_HANDLER(STD)
{
	*b = *a;

	vcpu->st.gr.i --;
	vcpu->st.gr.j --;

	return 0;
}


// pushes the address of the next instruction to the stack, then sets PC to a
DCPU_ISN_HANDLER(JSR)
{
	/* push address of next instruction onto the stack */
	dcpu_vcpu_push(vcpu, vcpu->st.sr.pc);

	/* set the program counter to a */
	vcpu->st.sr.pc = *a;

	return 0;
}

DCPU_ISN_HANDLER(HCF)
{
	die("halt and catch fire, now");
}

// triggers a software interrupt with message a
DCPU_ISN_HANDLER(INT)
{
	dcpu_vcpu_accept_interrupt(vcpu, *a);

	return 0;
}

// sets a to IA
DCPU_ISN_HANDLER(IAG)
{
	*a = vcpu->st.sr.ia;
}

// sets IA to a
DCPU_ISN_HANDLER(IAS)
{
	vcpu->st.sr.ia = *a;
}

// disables interrupt queueing, pops A from the stack, then pops PC from the stack
DCPU_ISN_HANDLER(RFI)
{
	vcpu->st.int_mask = 0;

	*a             = dcpu_vcpu_pop(vcpu);
	vcpu->st.sr.pc = dcpu_vcpu_pop(vcpu);

	return 0;
}

// if a is nonzero, interrupts will be added to the queue instead of triggered.
// if a is zero, interrupts will be triggered as normal again
DCPU_ISN_HANDLER(IAQ)
{
	vcpu->st.int_mask = !! *a;

	return 0;
}

// sets a to number of connected hardware devices
DCPU_ISN_HANDLER(HWN)
{
	die("no HWN");
}

// sets A, B, C, X, Y registers to information about hardware a
// A+(B<<16) is a 32 bit word identifying the hardware id
// C is the hardware version
// X+(Y<<16) is a 32 bit word identifying the manufacturer
DCPU_ISN_HANDLER(HWQ)
{
	dcpu_word hw_id;
	struct dcpu_hw *hw;
	int rc;

	hw_id = *a;
	hw = dcpu_vcpu_find_hw(vcpu, hw_id);
	if (hw) {
		dcpu_vcpu_gr(vcpu,a) = hw->hw_device_id;
		dcpu_vcpu_gr(vcpu,b) = hw->hw_device_id >> 16;
		dcpu_vcpu_gr(vcpu,c) = hw->hw_version;
		dcpu_vcpu_gr(vcpu,x) = hw->hw_vendor_id;
		dcpu_vcpu_gr(vcpu,y) = hw->hw_vendor_id >> 16;
	}

	return 0;
}

// sends an interrupt to hardware a
DCPU_ISN_HANDLER(HWI)
{
	dcpu_word hw_id;
	struct dcpu_hw *hw;
	int rc;

	hw_id = *a;
	hw = dcpu_vcpu_find_hw(vcpu, hw_id);
	if (hw)
		return hw->ops.handle_interrupt(hw);

	return 0;
}


