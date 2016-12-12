
/* =====[ misc.h ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the misc.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  10/20/2008  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#if !defined(AFX_SHADES_H__10CE72D1_1678_4810_8D52_1EA02C2176E1__INCLUDED_)
#define AFX_SHADES_H__10CE72D1_1678_4810_8D52_1EA02C2176E1__INCLUDED_

#ifndef ROTATE_LONG_RIGHT
#define ROTATE_LONG_RIGHT(xx, rot) ( (xx << rot) | (xx >> (32 - rot)) )
#endif

#ifndef CLIP
#define CLIP(vv, mm, xx) vv = min(vv, xx); vv = max(vv, mm)
#endif

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"

class CMisc

{

public:

	UUID	str2uuid(CString &str);
	CString	uuid2str(UUID *id);

	void	WinMonitorRect(CWnd *win, RECT *rc);
	void	CurrentMonRect(POINT pt, RECT *rc);

	long	hash_string(const TCHAR  *name);
	bool	IsCtrlDown();
	bool	IsShiftDown();
	bool	IsAltDown();

	COLORREF modcolor(COLORREF bcol, int mm, int dd);

	//CString	AnsiToUnicode(LPCSTR pszA);
	DWORD   AnsiToUnicode(LPCSTR pszA, CString &strr);
	DWORD   AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW);
	DWORD   AnsiToUnicode(LPCSTR pszA, char **strr);

	DWORD   UnicodeToAnsi(LPCSTR pszW, LPSTR* ppszA);
	DWORD   UnicodeToAnsi(LPCWSTR pszW, LPSTR* ppszA);

};

extern CMisc misc;

#endif
