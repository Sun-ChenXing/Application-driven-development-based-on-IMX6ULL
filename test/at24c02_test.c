#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#define IOC_AT24C02_READ  100
#define IOC_AT24C02_WRITE 101

void at24c02_test(void)
{
	int fd;
	unsigned char buf[10];

	/* 2. 打开文件 */
	fd = open("/dev/scx_at24c02", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/scx_at24c02\n");
		return ;
	}

	buf[0] = 0x05;
	buf[1] = 0x03;
	buf[2] = 0x10;
	buf[3] = 0x20;
	buf[4] = 0x30;
	ioctl(fd, IOC_AT24C02_WRITE, buf);

	sleep(1);

	buf[0] = 0x05;
	buf[1] = 0x03;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	ioctl(fd, IOC_AT24C02_READ, buf);

	printf("0x05 data:%d \n", buf[2]);
	printf("0x06 data:%d \n", buf[3]);
	printf("0x07 data:%d \n", buf[4]);

	while(1)
	{
		sleep(1);
	}

	return ;
}


