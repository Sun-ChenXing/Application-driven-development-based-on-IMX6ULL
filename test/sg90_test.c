#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sg90_test(void)
{
	int fd;
	int flag=0;
	unsigned char angle=0;

	/* 2. 打开文件 */
	fd = open("/dev/scx_sg90", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/scx_dht11 \n");
		return ;
	}

	while(1)
	{
		if (write(fd, &angle, 1) == 1)
		{

			if(flag==0) angle +=1;
			if(flag==1) angle -=1;
			if(angle==180) flag=1;
			if(angle==0) flag=0;
		}
		usleep(10000);
		if(angle%20 == 0) printf("angle :%d \n", angle);
	}
	
	close(fd);
	
	return ;
}


