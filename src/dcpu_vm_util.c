#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "dcpu_vm_util.h"
#include "dcpu_util.h"

void assert_file_is_legible(const char *file_type, const char *file_name)
{
	struct stat st;
	int rc = stat(file_name, &st);
	if (rc < 0)
		die("%s: %s: %s",
				file_name, file_type,
				strerror(errno));
}
