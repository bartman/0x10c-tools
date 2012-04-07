#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "0x10c_op.h"
#include "0x10c_isn.h"
#include "0x10c_vcpu.h"
#include "0x10c_parser.h"

int main(int argc, char *argv[])
{
	int rc;
	x10c_op_t op;
	char buf[1024];
	FILE *f;

	f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "%s: %s\n",
				argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ( fgets(buf, sizeof(buf), f) ) {


		memset(&op, 0, sizeof(op));
		rc = x10c_parse_line(&op, buf, strlen(buf));
		switch(rc) {
		case 1:
			printf("%-20s ; %04x\n", buf,
					op.word[0]);
			break;
		case 2:
			printf("%-20s ; %04x %04x\n", buf,
					op.word[0], op.word[1]);
			break;
		case 3:
			printf("%-20s ; %04x %04x %04x\n", buf,
					op.word[0], op.word[1], op.word[2]);
			break;
		default:
			printf("%-20s ; rc=%d\n", buf, rc);
			return rc;
		}
	}

	return 0;
}

