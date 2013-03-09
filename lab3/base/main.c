#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include "ospfs.h"

int main()
{
	int fd = open("/tmp/cs111/lab3-thief/test/hello.txt", O_RDWR);
	printf("ioctl call: %d\n", ioctl(fd, OSPFSIOCRASH, 100));
	close(fd);
}
