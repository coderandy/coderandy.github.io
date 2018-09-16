#ifndef MIRCV6_H
#define MIRCV6_H

#include "base.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#define MIRCV6HVERSION "$Id: mircv6.h,v 1.6 2006/08/03 17:45:05 davy Exp $"
#define NEWVERSION		"mIRC %s [v6]\x01\x0A"
#define	NEWVERSIONLEN	22
#define TARGETDLL		"wsock32.dll"

#define getlong(x)		*((unsigned long*)((unsigned long)x))

struct LOADINFO
{
	unsigned long version;
	HWND handle;
	char mKeep;
};

//in mircv6.c
char *mircv6cversion(void);
extern HMODULE funcdll;
extern void (*WriteToDebug)(const char *fmt,...);

//in utils.c
char *utilscversion(void);
extern char bufferke[];
char *tostring(struct in6_addr *addr);
unsigned long hashfunction(char *value);
char *findstring(char *datapool,int datalen,char *tofind);

//in binarypatcher.c
char *binarypatchercversion(void);
void unpatchimages(void);
int patchimage(char *modulename,unsigned long *datasection,unsigned long *datalen);

#endif
