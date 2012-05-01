#ifndef __included_dcpu_debugger_h__
#define __included_dcpu_debugger_h__

#include "dcpu_def.h"

struct dcpu_vcpu;

struct dcpu_debugger {

	// first call before program starts executing
	int (*start)(struct dcpu_vcpu *);

	// instruction executed
	int (*post_isn)(struct dcpu_vcpu *);

	// interrupt changed PC
	int (*post_int)(struct dcpu_vcpu *);

	// vcpu execution halted
	int (*halt)(struct dcpu_vcpu *);

	// vcpu being destoryed
	int (*exit)(struct dcpu_vcpu *);

};

#endif
