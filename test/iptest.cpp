#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned int ipaton(const char *str)
{
	unsigned int ip = 0;
	int   i;

	for (i = 24; i >= 0; i -= 8)
	{
		int cur = atoi(str);
		ip |= (unsigned int)(cur & 0xFF) << i;
		if (!i)	return (ip);

		str = strchr (str, '.');
		if (!str) return (0);
		++str;
	}
	return 0;
}


void ipntoa(const unsigned int addr, char* buf)
{                                                                                                   
	sprintf(buf, "%d.%d.%d.%d", (addr >> 24), (addr >> 16) & 0x00FF, (addr >> 8) & 0x00FF, (addr & 0x00FF));
}

void ipntoa_test(const unsigned int addr, char* buf)
{
	printf("%x\n", addr);
	printf("%d.%d.%d.%d\n", (addr >> 24), (addr >> 16) &0x00FF , (addr >> 8 & 0x00FF), (addr & 0x00FF));
}


void main()
{
	unsigned int ip = 0;
	const char* ip_str = "217.107.212.132";
	char buf[24];

	ip = ipaton(ip_str);
	ipntoa(ip, buf);
	ipntoa_test(ip, buf);
	printf("%s\n", buf);
}