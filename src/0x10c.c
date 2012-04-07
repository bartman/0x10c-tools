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
	const char *buf = "SET A, 0x30";

	rc = x10c_parse_line(&op, buf, strlen(buf));

	return rc;
}

