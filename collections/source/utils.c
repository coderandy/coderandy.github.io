/* $Id: utils.c,v 1.4 2005/10/09 15:53:42 davy Exp $ */
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mircv6.h"

char *findstring(char *datapool,int datalen,char *tofind)
{
	int m=lstrlen(tofind);
	int i=0;
	int p;
	for(p=0;p<datalen;p++)
	{
		if(datapool[p]==tofind[i])
		{
			i++;
			if(i==m)
				return datapool+p-m+1;
		}
		else
			i=0;
	}
	return NULL;
}
char *utilscversion(void)
{
	return "$Id: utils.c,v 1.4 2005/10/09 15:53:42 davy Exp $";
}
