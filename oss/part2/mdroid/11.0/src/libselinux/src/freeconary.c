#include <unistd.h>
#include <selinux/selinux_internal.h>  //STDLIBC_INTEGER
#include <stdlib.h>
#include <errno.h>

void freeconary(char ** con)
{
	char **ptr;

	if (!con)
		return;

	for (ptr = con; *ptr; ptr++) {
		free(*ptr);
	}
	free(con);
}

hidden_def(freeconary)
