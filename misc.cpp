
/* =====[ misc.cpp ]========================================== 
                                                                             
   Description:     The xraynotes project, implementation of the misc.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  6/11/2007  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"

//#define COMPILE_MULTIMON_STUBS
//#include "multimon.h"

#include "mxpad.h"
#include "misc.h"

#pragma comment(lib, "rpcrt4.lib")

// Disable security warnings TODO for later
#pragma warning (disable : 4996)

CMisc misc;

#if defined _UNICODE
	#define RPC_STR RPC_WSTR
#elif defined _MBCS 
	#define RPC_STR RPC_CSTR
#else
	#define RPC_STR RPC_CSTR
#endif

long	CMisc::hash_string(const TCHAR  *name)

{
    unsigned long    ret_val = 0;

    while(*name != '\0')
        {
        ret_val ^= (long)*name;
        ret_val  = ROTATE_LONG_RIGHT(ret_val, 3);          /* rotate right */
        *name++;
        }
    return((long)ret_val);
}

//////////////////////////////////////////////////////////////////////////
// Return true if a key is down

bool	CMisc::IsCtrlDown()

{
	return ((GetKeyState(VK_CONTROL) & 0x80) != false);
}

bool	CMisc::IsShiftDown()

{
	return ((GetKeyState(VK_SHIFT) & 0x80) != false);
}

bool	CMisc::IsAltDown()

{
	return ((GetKeyState(VK_MENU) & 0x80) != false);
}


//////////////////////////////////////////////////////////////////////////
// Get monitor dimentions from the middle of the window

void CMisc::WinMonitorRect(CWnd *win, RECT *rc)

{
	CRect	rc2; win->GetWindowRect(&rc2);	
	CPoint	here(rc2.top + rc2.Height() / 2, rc2.left + rc2.Width() / 2);

	misc.CurrentMonRect(here, rc);

	//P2N(_T("void CMisc::WinMonitorRect RECT *rc left=%d top=%d right=%d bottom=%d\r\n"),
	//				rc->left, rc->top, rc->right, rc->bottom);
}

//////////////////////////////////////////////////////////////////////////
// Get monitor dimetions from point

void CMisc::CurrentMonRect(POINT pt, RECT *rc)

{
	HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi; memset(&mi, 0, sizeof(mi));	mi.cbSize = sizeof(mi);
	
	int ret = GetMonitorInfo(mon, &mi);

	//P2N(_T("GetMonitorInfo ret=%d left=%d right=%d top=%d bottom=%d\r\n"), ret, mi.rcMonitor.left, 
	//						mi.rcMonitor.right, mi.rcMonitor.top, mi.rcMonitor.bottom);
	
	// Output results
	rc->left = mi.rcMonitor.left;	
	rc->top = mi.rcMonitor.top;	
	rc->right = mi.rcMonitor.right;	
	rc->bottom = mi.rcMonitor.bottom;	
}

//////////////////////////////////////////////////////////////////////////

CString		CMisc::uuid2str(UUID *id)

{
	CString str;

	const TCHAR  *ustr;

	//RPC_STATUS ret =  UuidToString(id, (RPC_WSTR *)&ustr);

	RPC_STATUS ret =  UuidToString(id, (RPC_STR *)&ustr);

	if(ret != RPC_S_OK)
		str = _T("Insufficent Memory in uuid2str");
	else
		str = ustr;

	RpcStringFree( (RPC_STR *) &ustr);

	return str;
}

//////////////////////////////////////////////////////////////////////////
// Convert a string to UUID

UUID CMisc::str2uuid(CString &str)

{
	UUID	uid;
	const TCHAR *ptr = (const TCHAR*)str.GetBuffer(_MAX_PATH + 1);

	RPC_STATUS ret =  UuidFromString((RPC_STR)ptr, &uid);

	str.ReleaseBuffer();

	if(ret != RPC_S_OK)
		{
		P2N(_T("CMisc::str2uuid %s RPC_S_INVALID_STRING_UUID\r\n"), str);
		}

	return uid;
}

//////////////////////////////////////////////////////////////////////////
// Modify color by mult and divide. Will clip to correct RGB

COLORREF CMisc::modcolor(COLORREF bcol, int mm, int dd)

{
	COLORREF ret;

	int rr = (bcol) & 0xff;
	int gg = (bcol >> 8) & 0xff;
	int bb = (bcol >> 16) & 0xff;

	rr = (mm * rr) / dd; rr = CLIP(rr, 0, 255);
	gg = (mm * gg) / dd; gg = CLIP(gg, 0, 255);
	bb = (mm * bb) / dd; bb = CLIP(bb, 0, 255);

	ret = RGB(rr, gg, bb);

	return ret;
}


// --------------------------------------------------------------
//
/// \brief   UnicodeToAnsi converts the Unicode string pszW to 
/// an ANSI string. 
/// 
/// \details Returns the ANSI string through ppszA. Space for the
/// converted string is allocated by this function. (care: free) 
/// \n \n 
/// Return type:      DWORD  
/// \n \n 
/// \param pszW Arg Type: LPWSTR Pointer to the UNICODE str to convert from
/// \param ppszA Arg Type: LPSTR* ptr to a buffer toi receve results

DWORD   CMisc::UnicodeToAnsi(LPCWSTR pszW, LPSTR* ppszA)

{
    ULONG cbAnsi, cCharacters;
    DWORD dwError;

    //Free old instance:
    //if(*ppszA)
    //    free(*ppszA);

    // If input is null then just return the same.
    if (pszW == NULL)
        {
        *ppszA = NULL;
        return NOERROR;
        }
    cCharacters = wcslen(pszW)+1;

    // Determine number of bytes to be allocated for ANSI string. An
    // ANSI string can have at most 2 bytes per character (for Double
    // Byte Character Strings.)

    cbAnsi = cCharacters*2;

    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You
    // can use your own allocator.

    *ppszA = (LPSTR) malloc(cbAnsi);

    if (NULL == *ppszA)
        return TRUE;

    // Convert to ANSI.

    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,
    cbAnsi, NULL, NULL))
        {
        dwError = GetLastError();
        free(*ppszA);
        *ppszA = NULL;
        return (dwError);
        }
    return NOERROR;
}

// The NOOP if already ANSI

DWORD   CMisc::UnicodeToAnsi(LPCSTR pszW, LPSTR* ppszA)

{
    ULONG cbAnsi, cCharacters;
    //DWORD dwError;

    //Free old instance:
    //if(*ppszA)
    //    free(*ppszA);

    // If input is null then just return the same.
    if (pszW == NULL)
        {
        *ppszA = NULL;
        return NOERROR;
        }
    cCharacters = strlen(pszW)+1;

    // Determine number of bytes to be allocated for ANSI string. An
    
    cbAnsi = cCharacters;

    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You
    // can use your own allocator.

    *ppszA = (LPSTR) malloc(cbAnsi);

    if (NULL == *ppszA)
        return TRUE;

    // Convert to ANSI by simply copying
    strncpy(*ppszA,  pszW, cCharacters);

	return NOERROR;
}

//////////////////////////////////////////////////////////////////////////
// Return Unicode CString from TCHAR * 

#if 0
CString	CMisc::AnsiToUnicode(LPCSTR pszA)

{
	CString str; AnsiToUnicode(pszA, str);
	return str;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Convert from TCHAR * CString

DWORD   CMisc::AnsiToUnicode(LPCSTR pszA, CString &strr)

{
	DWORD ret = 0;
	TCHAR *out;	ret = AnsiToUnicode(pszA, &out);
		
	if(ret)
		{
		strr = _T("err");
		return ret;
		}

	// Compose a string output
	int tlen = _tcslen(out);
	TCHAR *strbuff = strr.GetBufferSetLength(tlen * sizeof(TCHAR)); 
	memcpy(strbuff, out, tlen  * sizeof(TCHAR));
	free(out);
	strr.ReleaseBuffer(tlen);
	
	return ret;
}

// --------------------------------------------------------------
//
/// \brief   AnsiToUnicode converts the ANSI string pszA to a 
/// Unicode string. If the output is non unicode, just copy.
/// 

DWORD   CMisc::AnsiToUnicode(LPCSTR pszA, char **strr)

{
	if(!pszA)
		{
		*strr = NULL; return 0;
		}
	DWORD len = strlen(pszA);
	*strr = (char*)malloc(len + 1);
	if(!*strr)
		return 0;
	// Put it out
	memcpy(strr, pszA, len);
	// Terminate
	strr[len] = '\0';

	return len;
}

// --------------------------------------------------------------
//
/// \brief   AnsiToUnicode converts the ANSI string pszA to a 
/// Unicode string. 
/// 
/// \details AnsiToUnicode returns the Unicode string through ppszW. 
/// Space for the  the converted string is allocated by this 
/// function. (care: free)
/// \n  
/// Return type:      DWORD  
/// \n  
/// \param pszA Arg Type: LPCSTR 
/// \param ppszW Arg Type: LPWSTR* 

DWORD   CMisc::AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW)

{
    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == pszA)
        {
        *ppszW = NULL; return NOERROR;
        }

    // Determine number of wide characters to be allocated for the
    // Unicode string.

    cCharacters =  strlen(pszA)+1;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.

    *ppszW = (LPWSTR) malloc(cCharacters*2);

    if (NULL == *ppszW)
        return (TRUE);

    // Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters, *ppszW, cCharacters))
        {
        dwError = GetLastError();
        free(*ppszW);
        *ppszW = NULL;
        return (dwError);
        }
    return NOERROR;
}
