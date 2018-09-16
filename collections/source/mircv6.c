/* $Id: mircv6.c,v 1.8 2006/08/03 18:57:55 davy Exp $ */
//mircv6.c by davy hollevoet
#include "mircv6.h"
#include <stdarg.h>

HMODULE funcdll=NULL;

void (*opendebug)();
int (*initfaked)()=NULL;
char* (*fakedcversion)();
char* (*socketscversion)();
char* (*funcutilscversion)();
void (*WriteToDebug)(const char *fmt,...)=NULL;

int __stdcall DllMain(HINSTANCE hinstDLL,unsigned long fdwReason,void *lpvReserved)
{
	return 1;
}

///////////////////////////EXPORTED STUFF////////////////////////////

void __stdcall  __declspec(dllexport) LoadDll(struct LOADINFO *loadinfo)
{
	unsigned long datalen=0;		//length of datasection
	unsigned long datasection=0;	//this will be set from binarypatcher

	//load the dll with the faked functions
	//this way the functions are still loaded even when this dll gets unloaded by mIRC
	funcdll=LoadLibrary(FUNCTIONDLL);
	if(!funcdll) //could not load FUNCTIONDLL
	{
		char bufferke[1000];
		//WriteToDebug(bufferke,wsprintf(bufferke,"Unable to load "FUNCTIONDLL": error %ld\r\nPatching cancelled\r\n",GetLastError()));
		wsprintf(bufferke,"Unable to load "FUNCTIONDLL": error %ld\r\nPatching cancelled\r\n",GetLastError());
		//the line above is silly ofcourse
		MessageBox(loadinfo->handle,bufferke,MSGBOXTITLE,0x10);
		return;
	}
	else
	{
		opendebug=(void(*)())GetProcAddress(funcdll,"opendebug");
		WriteToDebug=(void(*)(char*,int))GetProcAddress(funcdll,"WriteToDebug");
		if(!(opendebug&&WriteToDebug))
		{
			char bufferke[1000];
			wsprintf(bufferke,"Could import functions (opendebug,WriteToDebug) from "FUNCTIONDLL": error %ld\r\nPatching cancelled\r\n%x %x\r\n",GetLastError(),opendebug,WriteToDebug);
			MessageBox(loadinfo->handle,bufferke,MSGBOXTITLE,0x10);
			return;
		}
		opendebug();
		WriteToDebug(MIRCV6HVERSION	"\r\n%s\r\n%s\r\n%s\r\n",
			mircv6cversion(),binarypatchercversion(),utilscversion());

		initfaked=(int(*)())GetProcAddress(funcdll,"initfaked");
		fakedcversion=(char*(*)())GetProcAddress(funcdll,"fakedcversion");
		funcutilscversion=(char*(*)())GetProcAddress(funcdll,"funcutilscversion");
		socketscversion=(char*(*)())GetProcAddress(funcdll,"socketscversion");
		if(!(initfaked&&fakedcversion&&funcutilscversion&&socketscversion)) //one or more failed
		{
			char bufferke[1000];
			wsprintf(bufferke,"Unable to find symbols in "FUNCTIONDLL": error %ld\r\nPatching cancelled\r\n%x %x %x %x\r\n",GetLastError(),initfaked,fakedcversion,socketscversion,funcutilscversion);
			WriteToDebug(bufferke);
			MessageBox(loadinfo->handle,bufferke,MSGBOXTITLE,0x10);
			return;
		}
		else
		{
			WriteToDebug("%s\r\n%s\r\n%s\r\n",fakedcversion(),socketscversion(),funcutilscversion());
		}
	}


	if(patchimage("mirc.exe",&datasection,&datalen)>0)
	//if(patchimage("mirc.exe",&datasection,&datalen)==0&&0)
	{
		MessageBox(loadinfo->handle,"Patching of mirc.exe FAILED\nThis instance is unusable\nQuiting...",MSGBOXTITLE,0x10);
		ExitProcess(-1);
	}
	patchimage("libeay32.dll",NULL,NULL); //might fail, dont bother checking

	//patch the version (yes by default)
	if(GetPrivateProfileInt(APPNAME,"patchversion",1,INIFILE))
	{
		unsigned long oldperm, bla;
		char *found=findstring((char*)datasection,datalen,"NOTICE %s :\x01VERSION");
		if(found)
		{
			WriteToDebug("found version at 0x%x\r\n",found);
			found+=20;
			if(VirtualProtect(found,NEWVERSIONLEN,PAGE_READWRITE,&oldperm)!=0)
			{
				CopyMemory(found,NEWVERSION,NEWVERSIONLEN);
				VirtualProtect(found,NEWVERSIONLEN,oldperm,&bla);
			}
		}
		else
			WriteToDebug("version not found\r\n");
	}
	else
		WriteToDebug("version not patched, as requested\r\n");
}
int __stdcall  __declspec(dllexport) UnloadDll(int timeout)
{
	if(WriteToDebug) //no logging available when funcdll is not loaded
		WriteToDebug("unload called!\r\n");
	return 1; //this is permitted now
}
int __stdcall  __declspec(dllexport) mircv6(HWND mWnd, HWND aWnd, char *data, char *parms, BOOL show, BOOL nopause)
{
	if(initfaked&&initfaked())
		strcpy(data,"/echo mircv6 by davy hollevoet");
	else
		strcpy(data,"/echo mircv6: init failed");
	return 2;
}
char *mircv6cversion(void)
{
	return "$Id: mircv6.c,v 1.8 2006/08/03 18:57:55 davy Exp $";
}
int __stdcall   __declspec(dllexport)  getversion(HWND mWnd, HWND aWnd, char *data, char *parms, BOOL show, BOOL nopause)
{
	wsprintf(data,"//echo Version "VERSION" | /echo %s | /echo "MIRCV6HVERSION" | /echo %s | /echo %s | /echo %s | /echo %s | /echo %s",
				mircv6cversion(),binarypatchercversion(),fakedcversion(),socketscversion(),utilscversion(),funcutilscversion());
	return 2;
}
