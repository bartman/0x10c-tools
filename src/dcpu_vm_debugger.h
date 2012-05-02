#ifndef __included_dcpu_vm_debugger_h__
#define __included_dcpu_vm_debugger_h__

#include "dcpu_def.h"

struct dcpu_vcpu;

struct dcpu_vm_debugger {

	void (*add_vcpu)(struct dcpu_vcpu *);

	int (*start)(void);
};

#endif // __included_dcpu_vm_debugger_h__
