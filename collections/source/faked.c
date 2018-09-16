/* $Id: faked.c,v 1.6 2006/08/03 17:45:05 davy Exp $ */
#include "mv6functions.h"

#define EVENTMESSAGE	0x1337

HANDLE heap=NULL;
HWND eventhandler=NULL;

HINSTANCE myinst;

int identsocket=INVALID_SOCKET; //the returnedsock that is
struct socketstruct *ipv6ident=NULL;

void stopipv6identd(void)
{
	identsocket=INVALID_SOCKET;
	closesocketstruct(ipv6ident->returnedsock);
	ipv6ident=NULL;
}

int __stdcall DllMain(HINSTANCE hinstDLL,unsigned long fdwReason,void *lpvReserved)
{
	myinst=hinstDLL;
	return 1;
}

int __stdcall evhandlerproc(HWND hwndDlg,unsigned int uMsg,WPARAM wParam,LPARAM lParam)
{
	struct socketstruct *target;
	if(uMsg==EVENTMESSAGE)
	{
		target=findsocketstruct_internal(wParam);
		if(target)									//!!!888888888!!!88888888888!!!
		{
			//WriteToDebug("We got message 0x%x\r\n",target->eventmsg);

			//if there is a event on the ipv6 identd socket, the only thing we can do is
			//signal it to mirc with the ipv4 identd socket stuff
			//to know which socket we should try in newaccept(), we mark eventpending in ipv6ident
			if(target==ipv6ident)
				ipv6ident->eventpending=1;
			else if(target->returnedsock==identsocket)
				ipv6ident->eventpending=0;
			//else do nothing

			PostMessage(target->eventhandler,target->eventmsg,target->returnedsock,lParam);
		}
	}
	return 0;
}

int __declspec(dllexport) initfaked(void)
{
	heap=GetProcessHeap();
	initmagic();
	eventhandler=CreateDialog(myinst,MAKEINTRESOURCE(100),NULL,evhandlerproc);
	if(!eventhandler)
	{
		char buffer[1000];
		wsprintf(buffer,"Could not create dummy dialog\nerror %d\n",GetLastError());
		WriteToDebug(buffer);
		MessageBox(0,buffer,MSGBOXTITLE,0x10);
	}
	return (heap&&eventhandler);
}
unsigned long __stdcall __declspec(dllexport) newinet_addr(const char *cp)
{
	unsigned long lhash=hashfunction((char*)cp);
	WriteToDebug("inet_addr(%s)\r\n",cp);

	lhash=getmagiclong(lhash);
	//lhash can never be 0 and be magic indeed
	return lhash?lhash:inet_addr(cp);
}
int __stdcall __declspec(dllexport) newlisten(int socket,int backlog)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("listen()\r\n");
	if(target)
	{
		if(socket==identsocket)
		{
			WriteToDebug("also listen()ing on ipv6 (identd)\r\n");
			if(listen(ipv6ident->internalsock,backlog))
			{
				WriteToDebug("listen() for ipv6 ident failed: error %d\r\n",WSAGetLastError());
				stopipv6identd();
			}
		}
		return listen(target->internalsock,backlog);
	}
	return listen(socket,backlog);
}

const struct in6_addr in6addr_any = {{ IN6ADDR_ANY_INIT }};

int __stdcall __declspec(dllexport) newbind(int sock,struct sockaddr *name,int namelen)
{
	struct socketstruct *target=findsocketstruct(sock);
	WriteToDebug("bind()\r\n");
	if(target)
	{
		if(namelen==sizeof(struct sockaddr_in))
		{
			struct sockaddr_in *info=name;
			short port=htons(info->sin_port);

			//if enabled (yes by default), also bind a socket to [::]
			if((port==113)&&(GetPrivateProfileInt(APPNAME,"ipv6ident",1,INIFILE)))
			{
				struct sockaddr_in6 info6;

				WriteToDebug("intercepted identd startup, using ipv6\r\n");

				identsocket=target->returnedsock;
				ipv6ident=makenewsocketstruct(socket(AF_INET6,SOCK_STREAM,0),AF_INET6);

				if(ipv6ident->internalsock==INVALID_SOCKET)
				{
					//return INVALID_SOCKET;
					//no,just continue to make sure the ipv4 identd works
					WriteToDebug("creating an ipv6 identd failed, INVALID_SOCKET\r\n");
					stopipv6identd();
				}
				else
				{
					memset((char *)&info6,0,sizeof(info6));
					info6.sin6_family=AF_INET6;
					info6.sin6_port=info->sin_port;
					info6.sin6_addr=in6addr_any;

					if(bind(ipv6ident->internalsock,(struct sockaddr*)&info6,sizeof(info6)))
					{
						WriteToDebug("creating an ipv6 identd failed during bind: error %d\r\n",WSAGetLastError());
						stopipv6identd();
					}
				}
			}
		}
		return bind(target->internalsock,name,namelen);
	}
	return bind(sock,name,namelen);
}

char* realinet_ntoa(struct in_addr in)
{
	WriteToDebug("newinet_ntoa()\r\n");
	if(ismagic(&in))
	{
		struct addrinfo *info=findmagicaddrinfo(&in);
		if(info)
			return tostring(&(((struct sockaddr_in6*)(info->ai_addr))->sin6_addr));
	}
	return inet_ntoa(in);
}
char* __cdecl __declspec(dllexport) newinet_ntoa(struct in_addr in)
{
	realinet_ntoa(in);
	_asm("pop %edi"); //grrrr, i just cant seem to get this right... HAXX
	_asm("pop %esi");
	_asm("leave");
	_asm("ret $0x4");
	return NULL;
}
HANDLE __stdcall __declspec(dllexport) newWSAAsyncGetHostByName(HWND hWnd,unsigned int wMsg,char *name, char *buf,int buflen)
{
	WriteToDebug("WSAAsyncGetHostByName(%s)\r\n",name);
	{	//we have an opinion of our own and we check for ipv6 dns
		struct addrinfo hints;
		struct addrinfo *result;
		struct hostent *retval=(struct hostent*)buf;

		ZeroMemory(&hints,sizeof(hints));
		hints.ai_family=AF_INET6;
		hints.ai_socktype=SOCK_STREAM;
		if(getaddrinfo(name,NULL,&hints,&result)==0)
		{
			WriteToDebug("IPv6 found\r\n");
			retval->h_name=name;
			retval->h_aliases=NULL;
			retval->h_length=4;
			retval->h_addr_list=(char **)(buf+sizeof(struct hostent));
			retval->h_addr_list[0]=(char*)(buf+sizeof(struct hostent)+2*sizeof(struct inaddr*));
			retval->h_addr_list[1]=NULL;

			addmagic(result,((unsigned char*)retval->h_addr_list[0]));

			//we lookup localhost to a dummy buffer and return a valid HANDLE and proper signaling is done. We win.
			//return WSAAsyncGetHostByName(hWnd,wMsg,"localhost",dummyhostent,MAXGETHOSTSTRUCT);
			PostMessage(hWnd,wMsg,7,0);
			return (HANDLE)7;
		}
	}
	return WSAAsyncGetHostByName(hWnd,wMsg,name,buf,buflen);
}
int __stdcall __declspec(dllexport) newWSAAsyncSelect(int socket,HWND hWnd,unsigned int wMsg,long lEvent)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("newWSAAsyncSelect()\r\n");
	if(target)
	{
		target->eventhandler=hWnd;
		target->eventmsg=wMsg;
		target->eventlevent=lEvent;

		if(socket==identsocket)
		{
			ipv6ident->eventhandler=hWnd;	//steal this info from the ipv4 socket
			ipv6ident->eventmsg=wMsg;		//we'll do some evil in accept aswell to make this work
			ipv6ident->eventlevent=lEvent;

			//request an async select on the ipv6 socket with the same signal info
			if(WSAAsyncSelect(ipv6ident->internalsock,eventhandler,EVENTMESSAGE,lEvent))
			{
				WriteToDebug("asyncselect on ipv6 ident failed: error %d\r\n",WSAGetLastError());
				stopipv6identd();
			}
		}

		return WSAAsyncSelect(target->internalsock,eventhandler,EVENTMESSAGE,lEvent);
	}
	return WSAAsyncSelect(socket,hWnd,wMsg,lEvent);
}
int __stdcall __declspec(dllexport) newsetsockopt(int socket,int level,int optname,char *optval,int optlen)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("setsockopt()\r\n");
	if(target)
		return setsockopt(target->internalsock,level,optname,optval,optlen);
	return setsockopt(socket,level,optname,optval,optlen);
}
int __stdcall __declspec(dllexport) newshutdown(int socket,int how)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("shutdown()\r\n");
	if(target)
		return shutdown(target->internalsock,how);
	return shutdown(socket,how);
}
int __stdcall __declspec(dllexport) newaccept(int socket,struct sockaddr *addr,int *addrlen)
{
	int acceptedsocket;
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("accept()\r\n");
	if(target)
	{
		//accepting on the ident socket && the ipv6 socket is flagged, then use the ipv6 socket
		if((socket==identsocket)&&(ipv6ident->eventpending))
		{
			int infolen=sizeof(struct sockaddr_in6);
			struct sockaddr_in6 info6;
			acceptedsocket=accept(ipv6ident->internalsock,(struct sockaddr*)&info6,&infolen);

			//lazy me...
			memset(addr,0,*addrlen);
			target=makenewsocketstruct(acceptedsocket,AF_INET6);
		}
		else
		{
			acceptedsocket=accept(target->internalsock,addr,addrlen);
			target=makenewsocketstruct(acceptedsocket,target->family);
		}

		if(acceptedsocket==INVALID_SOCKET)
		{
			closesocketstruct(target->returnedsock);
			return INVALID_SOCKET;
		}

		return target->returnedsock; //!!!88888!!888!!8888!!!!888
	}
	return accept(socket,addr,addrlen);
}
int __stdcall __declspec(dllexport) newgetsockname(int socket,struct sockaddr *name,int *namelen)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("getsockname()\r\n");
	if(target)
	{
		int retval=getsockname(target->internalsock,name,namelen);
		WriteToDebug("retval %d\r\n",retval);
		return retval;
	}
	return getsockname(socket,name,namelen);
}
int __stdcall __declspec(dllexport) newsendto(int socket,char *buf,int len,int flags,struct sockaddr *to,int tolen)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("sendto()\r\n");
	if(target)
		return sendto(target->internalsock,buf,len,flags,to,tolen);
	return sendto(socket,buf,len,flags,to,tolen);
}
int __stdcall __declspec(dllexport) newrecvfrom(int socket,char *buf,int len,int flags,struct sockaddr *from,int *fromlen)
{
	struct socketstruct *target=findsocketstruct(socket);
	WriteToDebug("recvfrom()\r\n");
	if(target)
		return recvfrom(target->internalsock,buf,len,flags,from,fromlen);
	return recvfrom(socket,buf,len,flags,from,fromlen);
}
int __stdcall __declspec(dllexport) newsend(int socket,char *buf,int len,int flags)
{
	struct socketstruct *target=findsocketstruct(socket);
	//WriteToDebug("send()\r\n");
	if(target)
			return send(target->internalsock,buf,len,flags);
	return send(socket,buf,len,flags);
}
int __stdcall __declspec(dllexport) newrecv(int socket,char *buf,int len,int flags)
{
	struct socketstruct *target=findsocketstruct(socket);
	//WriteToDebug("recv()\r\n");
	if(target)
		return recv(target->internalsock,buf,len,flags);
	return recv(socket,buf,len,flags);
}
int __stdcall __declspec(dllexport) newconnect(int sock,struct sockaddr_in *name,int namelen)
{
	struct socketstruct *target=findsocketstruct(sock);
	WriteToDebug("connect()\r\n");
	if(target)
	{
		if(ismagic(&(name->sin_addr)))
		{
			int m=findmagicindex(&(name->sin_addr));
			if(m>-1)
			{
				char buffer[1000];
				struct addrinfo *info;

				WriteToDebug("bingo! it's magic!\r\n");
				closesocket(target->internalsock);
				target->internalsock=socket(AF_INET6,SOCK_STREAM,0);
				target->family=AF_INET6;
				if(target->internalsock==INVALID_SOCKET)
					return INVALID_SOCKET;

				//try to bind to a specified IP (from inifile)
				if(GetPrivateProfileString(APPNAME,"bindhost","",buffer,150,INIFILE)>0)
				{
					struct addrinfo hints;
					struct addrinfo *result;

					ZeroMemory(&hints,sizeof(hints));
					hints.ai_family=AF_INET6;
					hints.ai_socktype=SOCK_STREAM;
					hints.ai_flags=AI_PASSIVE;	//we'll use it to bind()
					if(getaddrinfo(buffer,NULL,&hints,&result)==0)
					{
						if(bind(target->internalsock,result->ai_addr,result->ai_addrlen)==SOCKET_ERROR)
							WriteToDebug("bind failed: error %d in bind()\r\n",WSAGetLastError());
						freeaddrinfo(result);
					}
					else
					{
						WriteToDebug("bind failed: could not resolve '%s'\r\n",buffer);
					}
				}

				info=getmagicaddrinfo(m);

				WSAAsyncSelect(target->internalsock,eventhandler,EVENTMESSAGE,target->eventlevent);
				((struct sockaddr_in*)(info->ai_addr))->sin_port=name->sin_port;
				return connect(target->internalsock,(struct sockaddr*)info->ai_addr,info->ai_addrlen);
			}
		}
		return connect(target->internalsock,(struct sockaddr*)name,namelen);
	}
	return connect(sock,(struct sockaddr*)name,namelen);
}
int __stdcall __declspec(dllexport) newsocket(int af,int type, int protocol)
{
	struct socketstruct *newsocketstruct;
	WriteToDebug("newsocket()\r\n");
	type=socket(af,type,protocol);
	if(type==INVALID_SOCKET)
		return INVALID_SOCKET;
	newsocketstruct=makenewsocketstruct(type,af);	//type is a new socket
	if(!newsocketstruct)
		return INVALID_SOCKET;
	return newsocketstruct->returnedsock;	//8888!!8888!!!!88888888
}
int __stdcall __declspec(dllexport) newclosesocket(int sock)
{
	int retval;
	WriteToDebug("closesocket()\r\n");
	if(sock==identsocket)
	{
		WriteToDebug("also closing ipv6identsocket\r\n");
		stopipv6identd();
	}
	if((retval=closesocketstruct(sock))!=SOCKET_ERROR)
		return retval;
	//WriteToDebug("Internal error! Unmanaged socket!\r\n");
	return closesocket(sock);
}
char* __declspec(dllexport) fakedcversion(void)
{
	return "$Id: faked.c,v 1.6 2006/08/03 17:45:05 davy Exp $";
}

