#ifndef __included_0x10c_exec_h__
#define __included_0x10c_exec_h__

#include <unistd.h>

#include "0x10c_op.h"
struct x10c_isn;
struct x10c_vcpu;

extern int x10c_execute_SET(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_ADD(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_SUB(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_MUL(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_DIV(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_MOD(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_SHL(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_SHR(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_AND(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_BOR(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_XOR(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_IFE(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_IFN(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_IFG(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_IFB(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);
extern int x10c_execute_JSR(const struct x10c_isn *isn, const x10c_op_t *op,
		struct x10c_vcpu *vcpu);

#endif // __included_0x10c_exec_h__
