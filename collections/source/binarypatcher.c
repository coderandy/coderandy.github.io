/* $Id: binarypatcher.c,v 1.5 2006/08/03 18:57:55 davy Exp $ */
#include "mircv6.h"

//functions that should be patched in wsock32.dll
const unsigned int topatch=17;

const unsigned char *newfunctions[]={"newrecvfrom","newsendto","newgetsockname","newbind","newsocket","newlisten","newinet_ntoa","newWSAAsyncGetHostByName","newrecv","newsend","newconnect","newaccept","newWSAAsyncSelect","newshutdown","newclosesocket","newsetsockopt","newinet_addr"};
const unsigned short ordinals[]={	/*newrecvfrom*/0x11,
									/*newsendto*/0x14,
									/*newgetsockname*/0x6,
									/*newbind*/0x2,
									/*newsocket*/0x17,
									/*newlisten*/0xD,
									/*newinet_ntoa*/0xB,
									/*newWSAAsyncGetHostByName*/0x67,
									/*newrecv*/0x10,
									/*newsend*/0x13,
									/*newconnect*/0x4,
									/*newaccept*/0x1,
									/*newWSAAsyncSelect*/0x65,
									/*newshutdown*/0x16,
									/*newclosesocket*/0x3,
									/*newsetsockopt*/0x15,
									/*newinet_addr*/0xA};

int patchaddress(unsigned long *dst,unsigned long newval)
{
	unsigned long oldperm;
	unsigned long dummy;

	if(VirtualProtect(dst,4,PAGE_READWRITE,&oldperm)!=0)
	{
		*dst=newval;
		VirtualProtect(dst,4,oldperm,&dummy);
		return 0;
	}
	WriteToDebug("VirtualProject failed for address 0x%x\r\n",dst);
	return 1;
}

int patchtrampoline(unsigned long *dst,const char *functionname)
{
	unsigned long addr=(unsigned long)GetProcAddress(funcdll,functionname);

	return patchaddress(dst,addr);
}

struct importbyname
{
	unsigned short ordinalhint;
	char name;
};

int patchimage(char *modulename,unsigned long *datasection,unsigned long *datalen)
{
	unsigned int patched=0;
	unsigned int p;
	unsigned int m;
	unsigned long nsections;
	unsigned long data;
	unsigned long addr;
	unsigned long size;
	unsigned long *wsimporttable;			//pointer to import table
	unsigned long *wsordinaltable;			//pointer to import table with ordinals

	unsigned long imagebase=(unsigned long)GetModuleHandle(modulename);
	if(imagebase==0)
	{
		WriteToDebug("could not find %s\r\n",modulename);
		return 1;
	}
	WriteToDebug("%s at 0x%x\r\n",modulename,imagebase);

	addr=imagebase+getlong(imagebase+0x3C);	//offset to PEheader
	if(getlong(addr)!=0x4550)				//PE signature
	{
		WriteToDebug("Could not find PE header\r\n");
		return 2;
	}

	nsections=getlong(addr+0x6)&0xFFFF;
	WriteToDebug("%d sections\r\n",nsections);

	size=getlong(addr+0x14)&0xFFFF;
	WriteToDebug("optional header size: %d\r\n",size);

	if(datasection)
	{
		//search first data section
		for(m=0;m<nsections;m++)
		{
			data=getlong(addr+size+0x3C+(m*0x28));	//get the section flags
			if((data&0x40000040)==0x40000040)		//Readable and init'ed data
			{
				*datasection=getlong(addr+size+0x18+0xC+(m*0x28));	//get the rva of the section
				*datalen=getlong(addr+size+0x18+0x8+(m*0x28));		//length of the section
				*datasection+=imagebase;
				WriteToDebug("found datasection on %p (len %x)\r\n",*datasection,*datalen);
				break;
			}
		}
	}

	//investigate import table
	addr=getlong(addr+0x80);
	WriteToDebug("import table rva: 0x%x\r\n",addr);
	addr+=imagebase;

	for(m=0;(data=getlong(addr+0xC+(m*0x14)));m++)	//list them until we meet TARGETDLL
	{
		WriteToDebug("name rva: 0x%x\r\n",data);
		WriteToDebug("dll name: %8s\r\n",(char*)(data+imagebase));
		if(strcmpi(TARGETDLL,(char*)(data+imagebase))==0)
			break;
	}
	if(!data)					//no luck, we didnt find TARGETDLL
	{
		WriteToDebug("Could not find "TARGETDLL" import");
		return 3;
	}

	addr+=(m*0x14);
	wsordinaltable=(unsigned long*)(getlong(addr)+imagebase);		//points to first table (static ordinals)
	wsimporttable=(unsigned long*)(getlong(addr+0x10)+imagebase);	//points to second table (variable addresses)
	WriteToDebug("two arrays are at 0x%x and 0x%x\r\n",wsordinaltable,wsimporttable);
	//storetables(wsimporttable,wsordinaltable);

	for(m=0;(data=wsordinaltable[m]);m++)			//patch addresses
	{
		if(data&(1<<31))
		{
			unsigned short ordinal=data&0xFFFF;

			WriteToDebug("Image is looking for ordinal 0x%x\r\n",ordinal);

			for(p=0;p<topatch;p++)
			{
				if(ordinals[p]==ordinal)		//found address to patch
				{
					if(!patchtrampoline(&wsimporttable[m],newfunctions[p]))
					{
						patched++;
						WriteToDebug("set ordinal 0x%x to 0x%x\r\n",ordinal,addr);
					}
					break;
				}
			}
		}
		else
		{
			struct importbyname *ibn=(struct importbyname*)(data+imagebase);
			WriteToDebug("Image is looking for name '%s'\r\n",&ibn->name);

			for(p=0;p<topatch;p++)
			{
				if(!strcmp(newfunctions[p]+3,&ibn->name))
				{
					if(!patchtrampoline(&wsimporttable[m],newfunctions[p]))
					{
						patched++;
						WriteToDebug("set name '%s' to 0x%x\r\n",&ibn->name,addr);
					}

					break;
				}
			}
		}
	}
	WriteToDebug("patched %d/%d functions\r\n",patched,topatch);
	return topatch-patched;
}
char *binarypatchercversion(void)
{
	return "$Id: binarypatcher.c,v 1.5 2006/08/03 18:57:55 davy Exp $";
}
