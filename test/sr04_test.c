#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void sr04_test(void)
{
	int fd;
	int mm;

	int i;

	/* 1. 打开文件 */
	fd = open("/dev/scx_sr04", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/scx_sr04 \n");
		return ;
	}

	while(1)
	{
		if (read(fd, &mm, 4) == 4)
		{
			printf("get distance: %d mm\n", mm);  /* mm */
			// printf("get distance: %d mm\n", ns*340/2/1000000);  /* mm */
		}
		else
		{
			printf("get distance: -1\n");
		}
		usleep(100000);	//休眠50ms
	}
	
	close(fd);
	
	return ;
}


