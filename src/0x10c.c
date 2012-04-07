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
	char buf[1024];
	FILE *f;
	unsigned line;
	struct x10c_parsed_line *pl;


	f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "%s: %s\n",
				argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	line = 0;
	while ( fgets(buf, sizeof(buf), f) ) {

		char *p = trim(buf);
		x10c_op_t op;

		line ++;

		pl = x10c_parse_line(&op, argv[1], line, p, strlen(p));

		printf("%-80s ; %s%-10s", p,
				pl->label ? ":" : " ",
				pl->label ?: "");

		switch(pl->word_count) {
		case 0:
			printf("\n");
			break;
		case 1:
			printf("%04x\n",
					op.word[0]);
			break;
		case 2:
			printf("%04x %04x\n",
					op.word[0], op.word[1]);
			break;
		case 3:
			printf("%04x %04x %04x\n",
					op.word[0], op.word[1], op.word[2]);
			break;
		default:
			printf("empty, rc=%d\n", pl->error);
			return pl->error;
		}


		x10c_parsed_line_free(pl);
	}

	return 0;
}

