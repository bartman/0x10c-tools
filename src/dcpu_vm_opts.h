#ifndef __included_dcpu_vm_opts_h__
#define __included_dcpu_vm_opts_h__

#define DCPU_VM_PROGNAME "dcpu-vm"

struct dcpu_debugger;

extern const char *progname_full;
extern const char *progname_base;
extern const char *progname_dir;

struct dcpu_vm_opts {
	// which debugger to use
	struct dcpu_debugger *debugger;

	// what to execute
	const char *assembly_file;
	const char *binary_file;
	const char *snapshot_file;
};

// options are global
extern struct dcpu_vm_opts dcpu_vm_opts;

// command line parser
extern void dcpu_vm_parse_cmdline(int argc, char *argv[]);

#endif // __included_dcpu_vm_opts_h__
