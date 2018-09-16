/* Weditres generated include file. Do NOT edit */
#include <windows.h>
#include <lfc.h>
#define	DLG_0100	100
/*@ Prototypes @*/
#ifndef WEDIT_PROTOTYPES
#define WEDIT_PROTOTYPES
/*
 * Structure for dialog Dlg100
 */
struct _Dlg100 {
	HWND hwnd;
	WPARAM wParam;
	LPARAM lParam;
};


#endif
void SetDlgBkColor(HWND,COLORREF);
BOOL APIENTRY HandleCtlColor(UINT,DWORD);
/*
 * Callbacks for dialog Dlg100
 */
HWND StartDlg100(HWND parent);
int RunDlg100(HWND parent);
void AddGdiObject(HWND,HANDLE);
BOOL WINAPI HandleDefaultMessages(HWND hwnd,UINT msg,WPARAM wParam,DWORD lParam);
long Dlg100Init(ST_DIALOGBOX *,struct _Dlg100 *);
BOOL APIENTRY Dlg100(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
extern void *GetDialogArguments(HWND);
extern char *GetDico(int,long);
/*@@ End Prototypes @@*/
