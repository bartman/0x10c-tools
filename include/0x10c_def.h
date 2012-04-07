#ifndef __included_0x10c_def_h__
#define __included_0x10c_def_h__
#include <stdint.h>

/* things about the CPU */

typedef uint16_t x10c_word;

#define X10C_NUM_REGS 0x8
#define X10C_RAM_WORDS 0x10000
#define X10C_MAX_OP_LEN 3

/* basic opcodes */
#define X10C_OP_NON_BASIC 0x0   /* non-basic instruction - see below */
#define X10C_OP_SET       0x1   /* SET a, b - sets a to b */
#define X10C_OP_ADD       0x2   /* ADD a, b - sets a to a+b, sets O to 0x0001 if there's an overflow, 0x0 otherwise */
#define X10C_OP_SUB       0x3   /* SUB a, b - sets a to a-b, sets O to 0xffff if there's an underflow, 0x0 otherwise */
#define X10C_OP_MUL       0x4   /* MUL a, b - sets a to a*b, sets O to ((a*b)>>16)&0xffff */
#define X10C_OP_DIV       0x5   /* DIV a, b - sets a to a/b, sets O to ((a<<16)/b)&0xffff. if b==0, sets a and O to 0 instead. */
#define X10C_OP_MOD       0x6   /* MOD a, b - sets a to a%b. if b==0, sets a to 0 instead. */
#define X10C_OP_SHL       0x7   /* SHL a, b - sets a to a<<b, sets O to ((a<<b)>>16)&0xffff */
#define X10C_OP_SHR       0x8   /* SHR a, b - sets a to a>>b, sets O to ((a<<16)>>b)&0xffff */
#define X10C_OP_AND       0x9   /* AND a, b - sets a to a&b */
#define X10C_OP_BOR       0xb   /* BOR a, b - sets a to a|b */
#define X10C_OP_XOR       0xb   /* XOR a, b - sets a to a^b */
#define X10C_OP_IFE       0xc   /* IFE a, b - performs next instruction only if a==b */
#define X10C_OP_IFN       0xd   /* IFN a, b - performs next instruction only if a!=b */
#define X10C_OP_IFG       0xe   /* IFG a, b - performs next instruction only if a>b */
#define X10C_OP_IFB       0xf   /* IFB a, b - performs next instruction only if (a&b)!=0 */
#define X10C_OP_MAX       0x10

/* non-basic opcodes */
#define X10C_XOP_JSR      0x01  /* JSR a - pushes the address of the next instruction to the stack, then sets PC to a */
#define X10C_XOP_MAX      0x02

/* opcode arguments */

/* 0x00-0x07: register (A, B, C, X, Y, Z, I or J, in that order) */
#define X10C_ARG_REG_A 0x0
#define X10C_ARG_REG_B 0x1
#define X10C_ARG_REG_C 0x2
#define X10C_ARG_REG_X 0x3
#define X10C_ARG_REG_Y 0x4
#define X10C_ARG_REG_Z 0x5
#define X10C_ARG_REG_I 0x6
#define X10C_ARG_REG_J 0x7

/* 0x08-0x0f: [register] */
#define X10C_ARG_MREF_REG_BASE 0x8
#define X10C_ARG_MREF_REG(n) (X10C_ARG_MREF_REG_BASE + X10C_ARG_REG_##n)

/* 0x10-0x17: [next word + register] */
#define X10C_ARG_MREF_OFS_REG_BASE 0x10
#define X10C_ARG_MREF_OFS_REG(n) (X10C_ARG_MREF_OFS_REG_BASE + X10C_ARG_REG_##n)

#define X10C_ARG_REG_POP        0x18 /* POP / [SP++] */
#define X10C_ARG_REG_PEEK       0x19 /* PEEK / [SP] */
#define X10C_ARG_REG_PUSH       0x1a /* PUSH / [--SP] */
#define X10C_ARG_REG_SP         0x1b /* SP */
#define X10C_ARG_REG_PC         0x1c /* PC */
#define X10C_ARG_REG_O          0x1d /* O */
#define X10C_ARG_MREF_NEXT_WORD 0x1e /* [next word] */
#define X10C_ARG_NEXT_WORD      0x1f /* next word (literal) */

/* 0x20-0x3f: literal value 0x00-0x1f (literal) */
#define X10C_ARG_LITERAL_BASE 0x20
#define X10C_ARG_LITERAL(n) (X10C_ARG_LITERAL_BASE + n)




#endif // __included_0x10c_def_h__
