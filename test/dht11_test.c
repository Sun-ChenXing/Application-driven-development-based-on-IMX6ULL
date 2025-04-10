#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

void dht11_test(void)
{
	int fd;
	unsigned char data[4];

	/* 2. 打开文件 */
	fd = open("/dev/scx_dht11", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/scx_dht11 \n");
		return ;
	}

	while(1)
	{
		if (read(fd, data, 4) == 4)
		{
			printf("get humidity  : %d.%d\n", data[0], data[1]);
			printf("get temprature: %d.%d\n", data[2], data[3]);
		}
		else 
		{
			printf("get humidity/temprature: -1\n");
		}
		sleep(1);

	}
	
	close(fd);
	
	return ;
}


