#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <selinux/selinux_internal.h>  //STDLIBC_INTEGER
#include <selinux/policy.h>  //STDLIBC_INTEGER
#include <stdio.h>
#include <limits.h>

int security_get_checkreqprot(void)
{
	int fd, ret, checkreqprot = 0;
	char path[PATH_MAX];
	char buf[20];

	if (!selinux_mnt) {
		errno = ENOENT;
		return -1;
	}

	snprintf(path, sizeof(path), "%s/checkreqprot", selinux_mnt);
	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -1;

	memset(buf, 0, sizeof(buf));
	ret = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (ret < 0)
		return -1;

	if (sscanf(buf, "%d", &checkreqprot) != 1)
		return -1;

	return checkreqprot;
}

hidden_def(security_get_checkreqprot);
