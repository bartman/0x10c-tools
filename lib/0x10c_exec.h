#ifndef __included_0x10c_exec_h__
#define __included_0x10c_exec_h__

#include <unistd.h>

#include "0x10c_op.h"
struct x10c_isn;
struct x10c_vcpu;

#define X10C_ISN_HANDLER(OP) \
	int x10c_execute_##OP(const struct x10c_isn *isn, const x10c_op_t *op, \
		x10c_word *a, x10c_word *b, struct x10c_vcpu *vcpu)

/* basic OPS */

extern X10C_ISN_HANDLER(SET);
extern X10C_ISN_HANDLER(ADD);
extern X10C_ISN_HANDLER(SUB);
extern X10C_ISN_HANDLER(MUL);
extern X10C_ISN_HANDLER(MLI);
extern X10C_ISN_HANDLER(DIV);
extern X10C_ISN_HANDLER(DVI);
extern X10C_ISN_HANDLER(MOD);
extern X10C_ISN_HANDLER(MDI);
extern X10C_ISN_HANDLER(AND);
extern X10C_ISN_HANDLER(BOR);
extern X10C_ISN_HANDLER(XOR);
extern X10C_ISN_HANDLER(SHR);
extern X10C_ISN_HANDLER(ASR);
extern X10C_ISN_HANDLER(SHL);
extern X10C_ISN_HANDLER(IFB);
extern X10C_ISN_HANDLER(IFC);
extern X10C_ISN_HANDLER(IFE);
extern X10C_ISN_HANDLER(IFN);
extern X10C_ISN_HANDLER(IFG);
extern X10C_ISN_HANDLER(IFA);
extern X10C_ISN_HANDLER(IFL);
extern X10C_ISN_HANDLER(IFU);
extern X10C_ISN_HANDLER(ADX);
extern X10C_ISN_HANDLER(SBX);
extern X10C_ISN_HANDLER(STI);
extern X10C_ISN_HANDLER(STD);

/* special ops*/

extern X10C_ISN_HANDLER(JSR);
extern X10C_ISN_HANDLER(HCF);
extern X10C_ISN_HANDLER(INT);
extern X10C_ISN_HANDLER(IAG);
extern X10C_ISN_HANDLER(IAS);
extern X10C_ISN_HANDLER(RFI);
extern X10C_ISN_HANDLER(IAQ);
extern X10C_ISN_HANDLER(HWN);
extern X10C_ISN_HANDLER(HWQ);
extern X10C_ISN_HANDLER(HWI);


#endif // __included_0x10c_exec_h__
