#include <unistd.h>
#include <selinux/selinux_internal.h>  //STDLIBC_INTEGER
#include <stdlib.h>
#include <errno.h>

void freecon(char * con)
{
	free(con);
}

hidden_def(freecon)
