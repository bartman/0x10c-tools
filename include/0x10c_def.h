#ifndef __included_0x10c_def_h__
#define __included_0x10c_def_h__
#include <stdint.h>

/* things about the CPU */

typedef uint16_t x10c_word;
#define X10C_MAX_WORD_VALUE 0x10000

#define X10C_NUM_REGS 0x8
#define X10C_RAM_WORDS 0x10000
#define X10C_MAX_OP_LEN 3

/* basic opcodes */
#define X10C_OP_NON_BASIC 0x00  /* special instruction - see below */
#define X10C_OP_SET       0x01  /* sets b to a */
#define X10C_OP_ADD       0x02  /* sets b to b+a, sets EX to 0x0001 if there's an overflow, 0x0 otherwise */
#define X10C_OP_SUB       0x03  /* sets b to b-a, sets EX to 0xffff if there's an underflow, 0x0 otherwise */
#define X10C_OP_MUL       0x04  /* sets b to b*a, sets EX to ((b*a)>>16)&0xffff (treats b, a as unsigned) */
#define X10C_OP_MLI       0x05  /* like MUL, but treat b, a as signed */
#define X10C_OP_DIV       0x06  /* sets b to b/a, sets EX to ((b<<16)/a)&0xffff. if a==0, sets b and EX to 0 instead. (treats b, a as unsigned) */
#define X10C_OP_DVI       0x07  /* like DIV, but treat b, a as signed */
#define X10C_OP_MOD       0x08  /* sets b to b%a. if a==0, sets b to 0 instead. */
#define X10C_OP_MDI       0x09  /* like MOD, but treat b, a as signed. Rounds towards 0 */
#define X10C_OP_AND       0x0a  /* sets b to b&a */
#define X10C_OP_BOR       0x0b  /* sets b to b|a */
#define X10C_OP_XOR       0x0c  /* sets b to b^a */
#define X10C_OP_SHR       0x0d  /* sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff (logical shift) */
#define X10C_OP_ASR       0x0e  /* sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff (arithmetic shift) (treats b as signed) */
#define X10C_OP_SHL       0x0f  /* sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff */
#define X10C_OP_IFB       0x10  /* performs next instruction only if (b&a)!=0 */
#define X10C_OP_IFC       0x11  /* performs next instruction only if (b&a)==0 */
#define X10C_OP_IFE       0x12  /* performs next instruction only if b==a */
#define X10C_OP_IFN       0x13  /* performs next instruction only if b!=a */
#define X10C_OP_IFG       0x14  /* performs next instruction only if b>a */
#define X10C_OP_IFA       0x15  /* performs next instruction only if b>a (signed) */
#define X10C_OP_IFL       0x16  /* performs next instruction only if b<a */
#define X10C_OP_IFU       0x17  /* performs next instruction only if b<a (signed) */
#define X10C_OP_ADX       0x1a  /* sets b to b+a+EX, sets EX to 0x0001 if there is an overflow, 0x0 otherwise */
#define X10C_OP_SBX       0x1b  /* sets b to b-a+EX, sets EX to 0xFFFF if there is an underflow, 0x0 otherwise */
#define X10C_OP_STI       0x1e  /* sets b to a, then increases I and J by 1 */
#define X10C_OP_STD       0x1f  /* sets b to a, then decreases I and J by 1 */
#define X10C_OP_MAX       0x20

/* special instruction - see below */
#define X10C_XOP_JSR      0x01  /* pushes the address of the next instruction to the stack, then sets PC to a */
#define X10C_XOP_HCF      0x07  /* use sparingly */
#define X10C_XOP_INT      0x08  /* triggers a software interrupt with message a */
#define X10C_XOP_IAG      0x09  /* sets a to IA */
#define X10C_XOP_IAS      0x0a  /* sets IA to a */
#define X10C_XOP_RFI      0x0b  /* disables interrupt queueing, pops A from the stack, then pops PC from the stack */
#define X10C_XOP_IAQ      0x0c  /* if a is nonzero, interrupts will be added to the queue instead of triggered. if a is zero, interrupts will be triggered as normal again */
#define X10C_XOP_HWN      0x10  /* sets a to number of connected hardware devices */
#define X10C_XOP_HWQ      0x11  /* sets A, B, C, X, Y registers to information about hardware a A+(B<<16) is a 32 bit word identifying the hardware id C is the hardware version X+(Y<<16) is a 32 bit word identifying the manufacturer */
#define X10C_XOP_HWI      0x12  /* sends an interrupt to hardware a */

#define X10C_XOP_MAX      0x20

/* opcode arguments */

/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
#define X10C_REG_A 0x0
#define X10C_REG_B 0x1
#define X10C_REG_C 0x2
#define X10C_REG_X 0x3
#define X10C_REG_Y 0x4
#define X10C_REG_Z 0x5
#define X10C_REG_I 0x6
#define X10C_REG_J 0x7

/* 0x08-0x0f: [register] */
#define X10C_ARG_MREF_REG_BASE 0x8
#define X10C_ARG_MREF_REG(n) (X10C_ARG_MREF_REG_BASE + X10C_REG_##n)

/* 0x10-0x17: [next word + register] */
#define X10C_ARG_MREF_OFS_REG_BASE 0x10
#define X10C_ARG_MREF_OFS_REG(n) (X10C_ARG_MREF_OFS_REG_BASE + X10C_REG_##n)

#define X10C_REG_PUSH_POP   0x18 /* PUSH / [--SP]  or  POP / [SP++] */
#define X10C_REG_PEEK       0x19 /* PEEK / [SP] */
#define X10C_REG_PICK       0x1a /* PICK n / [SP + next word] */
#define X10C_REG_SP         0x1b /* SP */
#define X10C_REG_PC         0x1c /* PC */
#define X10C_REG_EX         0x1d /* EX */
#define X10C_MREF_NEXT_WORD 0x1e /* [next word] */
#define X10C_NEXT_WORD      0x1f /* next word (literal) */

/* 0x20-0x3f: literal value 0x00-0x1f (literal) */
#define X10C_ARG_LITERAL_BASE 0x20
#define X10C_ARG_LITERAL(n) (X10C_ARG_LITERAL_BASE + n)

/* are comparisons case sensitive? */
#define X10C_OP_NAME_CMP  strcasecmp
#define X10C_REG_NAME_CMP strcasecmp


#endif // __included_0x10c_def_h__
