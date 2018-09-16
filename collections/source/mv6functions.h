#ifndef MV6FUNCTIONS_H
#define MV6FUNCTIONS_H

#include "base.h"
#include "windows.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#define ADDRLISTLEN		8

struct socketstruct
{
	int returnedsock;	//we told mirc this is his socket
	int internalsock;	//but in fact, this is the socket
	int family;
	//event async crap
	HWND eventhandler;	//mirc wants a message on this window in case of an event
	unsigned int eventmsg;	//and wants it to be this message
	long eventlevent;		//and what its all about
	int eventpending;
	struct socketstruct *next;
};

//in utils.c
char *utilscversion(void);
extern char bufferke[];
void opendebug(void);
void WriteToDebug(char *fmt,...);
char *tostring(struct in6_addr *addr);
unsigned long hashfunction(char *value);
char *findstring(char *datapool,int datalen,char *tofind);

//in sockets.c
char *socketscversion(void);
int closesocketstruct(int sock);
struct socketstruct *findsocketstruct(int socket);
struct socketstruct *findsocketstruct_internal(int socket);
struct socketstruct *makenewsocketstruct(int socket,int family);

//in faked.c
char *fakedcversion(void);
extern HANDLE heap;
int initfaked(void);

//in magic.c
void initmagic(void);
int ismagic(struct in_addr *in);
int findmagicindex(struct in_addr *in);
struct addrinfo *getmagicaddrinfo(int index);
struct addrinfo *findmagicaddrinfo(struct in_addr *in);
unsigned long getmagiclong(unsigned long hash);
void addmagic(struct addrinfo *info,unsigned char *retval);

#endif
