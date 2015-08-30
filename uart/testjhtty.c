#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/* firstdrvtest on
  * firstdrvtest off
  */
int main(void)
{
	int fd;
	char buf[10];
	unsigned char c = 0;
	
	fd = open("/dev/jhtty", O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
	}
	
	printf("press mode :\n");
	printf("0:Send 0xAA :\n");
	printf("1:Send 0xAA 0x55 :\n");
	printf("2:Send string [1234567] :\n");
	printf("r:read data :\n");
	printf("e:to exit :\n");

	while(1)
	{
		printf("please input dat:");
		c = getchar();
		memset(buf, 0, sizeof(buf));
		if(c == '0')
		{
			//printf("high level\n");
			buf[0] = 0xAA;
			write(fd, buf, 1);
		}
		else if(c == '1')
		{
			printf("low level\n");
			buf[0] = 0xAA;
			buf[1] = 0x55;
			write(fd, buf, 2);
		}
		else if(c == '2')
		{
			printf("low level\n");
			memcpy(buf, "1234567", 7);
			write(fd, buf, strlen(buf)-1);
		}
		else if(c == 'r')
		{
			int count;
			printf("read readdy\n");
			count = read(fd, buf, 1);
			printf("read data count[%d], buf[0]=0x%02x\n", count, buf[0]);
			
		}
		else if(c == 'e')
		{
			break;
		}
		else if(c == 'h')
		{
			printf("press mode :\n");
			printf("u:get high level\n");
			printf("d:get low level\n");
			printf("e:to exit\n");
		}
		
		//sleep(1);
	}
	close(fd);
	return 0;
}
