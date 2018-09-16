/* $Id: sockets.c,v 1.5 2006/08/03 17:45:05 davy Exp $ */
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mv6functions.h"

int socknum=0;	//socketnum we last returned
int sockcount=0;

//structures for the linked list
struct socketstruct *boss=NULL;
struct socketstruct **acceptor=&boss;

int closesocketstruct(int sock)
{
	int retval;
	struct socketstruct *toclose=boss,
		                **prev=&boss;
	while(toclose)
	{
		if(toclose->returnedsock==sock)  //!!!!8888!!8888!!!!!!!!!!
		{
			retval=closesocket(toclose->internalsock);
			WriteToDebug("sockcount: %d\r\n",--sockcount);
			*prev=toclose->next;
			if(!*prev)
				acceptor=prev;
			HeapFree(heap,0,toclose);
			return retval;
		}
		prev=&(toclose->next);
		toclose=toclose->next;
	}
	WriteToDebug("Internal error! Unmanaged socket!\r\n");
	return SOCKET_ERROR;
}
struct socketstruct *findsocketstruct_internal(int socket)
{
	struct socketstruct *it=boss;
	while(it)
	{
		if(it->internalsock==socket)
			return it;
		it=it->next;
	}
	WriteToDebug("Internal error! Unmanaged internal socket!\r\n");
	return NULL;
}
struct socketstruct *findsocketstruct(int socket)
{
	struct socketstruct *it=boss;
	while(it)
	{
		if(it->returnedsock==socket)	//88888888888!!!88!!!!!!!!!
			return it;
		it=it->next;
	}
	//default action is now to use the regular socket functions on the unknown socket
	//WriteToDebug("Internal error! Unmanaged socket!\r\n",35);
	return NULL;
}
struct socketstruct *makenewsocketstruct(int socket,int family)
{
	struct socketstruct *newsocketstruct;
	WriteToDebug("makenewsocketstruct()\r\n",23);
	if((newsocketstruct=HeapAlloc(heap,0,sizeof(struct socketstruct)))==NULL)
	//if((newsocketstruct=GlobalAlloc(GMEM_FIXED,sizeof(struct socketstruct)))==NULL)
	{
		MessageBox(0,"mircv6.dll could not HeapAlloc()",MSGBOXTITLE,0x10);
		return NULL;
	}
	else
	{
		newsocketstruct->internalsock=socket;
		newsocketstruct->returnedsock=++socknum;
		newsocketstruct->family=family;
		newsocketstruct->next=NULL;
		newsocketstruct->eventpending=0;
		*acceptor=newsocketstruct;
		acceptor=&(newsocketstruct->next);
		WriteToDebug("sockcount: %d\r\n",++sockcount);
		return newsocketstruct;
	}
}
char* __declspec(dllexport) socketscversion(void)
{
	return "$Id: sockets.c,v 1.5 2006/08/03 17:45:05 davy Exp $";
}
