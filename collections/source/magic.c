/* $Id: magic.c,v 1.2 2006/08/03 17:45:05 davy Exp $ */
#include "mv6functions.h"

int addrlistindex=0;					//next slot to used
struct addrinfo *addrlist[ADDRLISTLEN];	//temporary storage for resolved ipv6 addresses

unsigned char magic1=7;	//magic agents
unsigned char magic2=8;
unsigned long addrhash[ADDRLISTLEN];	//hash of the string repres of ipv6 address (for inet_addr to return our magic)
unsigned char magiclist[ADDRLISTLEN][2];	//connect(255.[m][0].[m][1].sum) ->connect(addrlist[m])

void initmagic(void)
{
	ZeroMemory(addrlist,sizeof(struct addrinfo*)*ADDRLISTLEN);
}
unsigned long getmagiclong(unsigned long hash)
{
	int p;
	for(p=0;p<ADDRLISTLEN;p++)
	{
		if(addrlist[p]&&(hash==addrhash[p]))
		{
			WriteToDebug("Identified ipv6\r\n");
			return (0xFF+(magiclist[p][0]<<8)+(magiclist[p][1]<<16)+((magiclist[p][0]+magiclist[p][1])<<24));
		}
	}
	return 0;
}
int ismagic(struct in_addr *in)
{
	return (in->S_un.S_un_b.s_b1==0xFF)&&(in->S_un.S_un_b.s_b2+in->S_un.S_un_b.s_b3==in->S_un.S_un_b.s_b4);
}
int findmagicindex(struct in_addr *in)
{
	int m;
	for(m=0;m<ADDRLISTLEN;m++)
		if((addrlist[m])&&(in->S_un.S_un_b.s_b2==magiclist[m][0])&&(in->S_un.S_un_b.s_b3==magiclist[m][1]))
			return m;
	return -1;
}
struct addrinfo *findmagicaddrinfo(struct in_addr *in)
{
	int m=findmagicindex(in);
	return m>-1?addrlist[m]:NULL;
}
struct addrinfo *getmagicaddrinfo(int index)
{
	return addrlist[index];
}
void addmagic(struct addrinfo *info,unsigned char *retval)
{
	if(addrlist[addrlistindex])
		freeaddrinfo(addrlist[addrlistindex]);
	addrlist[addrlistindex]=info;

	//when inet_addr asks for confirmation, we must be ready (used in scripting)
	addrhash[addrlistindex]=hashfunction(tostring(&(((struct sockaddr_in6*)(addrlist[addrlistindex]->ai_addr))->sin6_addr)));

	if(retval)
	{
		retval[0]=0xFF;
		retval[1]=magic1;
		retval[2]=magic2;
		retval[3]=magic1+magic2;
	}
	magiclist[addrlistindex][0]=magic1;
	magiclist[addrlistindex][1]=magic2;
	magic1+=3;
	magic2+=1;

	addrlistindex=(++addrlistindex)%ADDRLISTLEN;
}
