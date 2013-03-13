#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>
#include "ospfs.h"

int main(int argc, char** argv)
{
	int nswrites_to_crash = atoi(argv[1]);
	int fd = open("./test/donotremove", O_RDONLY);
	int status = ioctl(fd, OSPFSIOCRASH, nswrites_to_crash);
	close(fd);
	exit(status);
}
