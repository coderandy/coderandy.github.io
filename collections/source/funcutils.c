/* $Id: funcutils.c,v 1.2 2006/08/03 08:59:59 davy Exp $ */
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mv6functions.h"

#define HASHBASE		37

char resolvedip[40];

#ifdef DEBUGTOCONSOLE
	HANDLE mstdout;
#endif
#ifdef DEBUGTOFILE
	HANDLE debugfile;
#endif

void __declspec(dllexport) opendebug(void)
{
#ifdef DEBUGTOCONSOLE
	AllocConsole();
	mstdout=GetStdHandle(STD_OUTPUT_HANDLE);
#endif
#ifdef DEBUGTOFILE
	debugfile=CreateFile("mircv6.log",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
#endif
	WriteToDebug("mircv6 by davy hollevoet - Version "VERSION"\r\n");
}
void __declspec(dllexport) WriteToDebug(char *fmt,...)
{
	char buffer[1000];
	unsigned long written;
	va_list ap;
	va_start(ap,fmt);
	int len=wvsprintf(buffer,fmt,ap);
#ifdef DEBUGTOCONSOLE
	WriteConsole(mstdout,buffer,len,&written,0);
#endif
#ifdef DEBUGTOFILE
	WriteFile(debugfile,buffer,len,&written,NULL);
#endif
}
unsigned long hashfunction(char *value)
{
	unsigned long hash=0;
	for(;*value;value++)
		hash=hash*HASHBASE+*value;
	return hash;
}
char *tostring(struct in6_addr *addr)
{
	wsprintf(resolvedip,"%x:%x:%x:%x:%x:%x:%x:%x",htons(addr->u.Word[0]),htons(addr->u.Word[1]),htons(addr->u.Word[2]),htons(addr->u.Word[3]),htons(addr->u.Word[4]),htons(addr->u.Word[5]),htons(addr->u.Word[6]),htons(addr->u.Word[7]));
	return resolvedip;
}
char*  __declspec(dllexport) funcutilscversion(void)
{
	return "$Id: funcutils.c,v 1.2 2006/08/03 08:59:59 davy Exp $";
}
