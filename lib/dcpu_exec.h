#ifndef __included_dcpu_exec_h__
#define __included_dcpu_exec_h__

#include <unistd.h>

#include "dcpu_op.h"
struct dcpu_isn;
struct dcpu_vcpu;

#define DCPU_ISN_HANDLER(OP) \
	int dcpu_execute_##OP(const struct dcpu_isn *isn, const dcpu_op_t *op, \
		dcpu_word *a, dcpu_word *b, struct dcpu_vcpu *vcpu)

/* basic OPS */

extern DCPU_ISN_HANDLER(SET);
extern DCPU_ISN_HANDLER(ADD);
extern DCPU_ISN_HANDLER(SUB);
extern DCPU_ISN_HANDLER(MUL);
extern DCPU_ISN_HANDLER(MLI);
extern DCPU_ISN_HANDLER(DIV);
extern DCPU_ISN_HANDLER(DVI);
extern DCPU_ISN_HANDLER(MOD);
extern DCPU_ISN_HANDLER(MDI);
extern DCPU_ISN_HANDLER(AND);
extern DCPU_ISN_HANDLER(BOR);
extern DCPU_ISN_HANDLER(XOR);
extern DCPU_ISN_HANDLER(SHR);
extern DCPU_ISN_HANDLER(ASR);
extern DCPU_ISN_HANDLER(SHL);
extern DCPU_ISN_HANDLER(IFB);
extern DCPU_ISN_HANDLER(IFC);
extern DCPU_ISN_HANDLER(IFE);
extern DCPU_ISN_HANDLER(IFN);
extern DCPU_ISN_HANDLER(IFG);
extern DCPU_ISN_HANDLER(IFA);
extern DCPU_ISN_HANDLER(IFL);
extern DCPU_ISN_HANDLER(IFU);
extern DCPU_ISN_HANDLER(ADX);
extern DCPU_ISN_HANDLER(SBX);
extern DCPU_ISN_HANDLER(STI);
extern DCPU_ISN_HANDLER(STD);

/* special ops*/

extern DCPU_ISN_HANDLER(JSR);
extern DCPU_ISN_HANDLER(HCF);
extern DCPU_ISN_HANDLER(INT);
extern DCPU_ISN_HANDLER(IAG);
extern DCPU_ISN_HANDLER(IAS);
extern DCPU_ISN_HANDLER(RFI);
extern DCPU_ISN_HANDLER(IAQ);
extern DCPU_ISN_HANDLER(HWN);
extern DCPU_ISN_HANDLER(HWQ);
extern DCPU_ISN_HANDLER(HWI);


#endif // __included_dcpu_exec_h__
