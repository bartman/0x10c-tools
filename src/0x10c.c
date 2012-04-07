#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "0x10c_op.h"
#include "0x10c_isn.h"
#include "0x10c_vcpu.h"
#include "0x10c_parser.h"

int main(int argc, char *argv[])
{
	int rc;
	x10c_op_t op;
	struct {
		const char buf[128];
	} tests[] = {
		{ .buf = "SET A, 0x30" },
		{ .buf = "SET [0x1000], 0x20" },
		{ .buf = "SUB A, [0x1000]" },
		{ .buf = "IFN A, 0x10" },
		{ .buf = "SET PC, crash" },
		{ .buf = "SET I, 10" },
		{ .buf = "SET A, 0x2000" },
		{ .buf = "SET [0x2000+I], [A]" },
		{ .buf = "SUB I, 1" },
		{ .buf = "IFN I, 0" },
		{ .buf = "SET PC, loop" },
		{ .buf = "SET X, 0x4" },
		{ .buf = "JSR testsub" },
		{ .buf = "SET PC, crash" },
		{ .buf = "SHL X, 4" },
		{ .buf = "SET PC, POP" },
		{ .buf = "SET PC, crash" },
		{ },
	}, *t;

	for (t=tests; *t->buf; t++) {
		memset(&op, 0, sizeof(op));
		rc = x10c_parse_line(&op, t->buf, strlen(t->buf));
		switch(rc) {
		case 1:
			printf("%-20s ; %04x\n", t->buf,
					op.word[0]);
			break;
		case 2:
			printf("%-20s ; %04x %04x\n", t->buf,
					op.word[0], op.word[1]);
			break;
		case 3:
			printf("%-20s ; %04x %04x %04x\n", t->buf,
					op.word[0], op.word[1], op.word[2]);
			break;
		default:
			printf("%-20s ; rc=%d\n", t->buf, rc);
			return rc;
		}
	}

	return 0;
}

