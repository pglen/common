
/* =====[ multimon.cpp ]========================================================== 
                                                                             
   Description:     The monsw project, implementation of the multimon.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  2/17/2011  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"

#define COMPILE_MULTIMON_STUBS
#include "multimon.h"

//////////////////////////////////////////////////////////////////////////
#ifndef MULTIMON_FNS_DEFINED

int      (WINAPI* g_pfnGetSystemMetrics)(int) = NULL;
HMONITOR (WINAPI* g_pfnMonitorFromWindow)(HWND, DWORD) = NULL;
HMONITOR (WINAPI* g_pfnMonitorFromRect)(LPCRECT, DWORD) = NULL;
HMONITOR (WINAPI* g_pfnMonitorFromPoint)(POINT, DWORD) = NULL;
BOOL     (WINAPI* g_pfnGetMonitorInfo)(HMONITOR, LPMONITORINFO) = NULL;
BOOL     (WINAPI* g_pfnEnumDisplayMonitors)(HDC, LPCRECT, MONITORENUMPROC, LPARAM) = NULL;
BOOL     g_fMultiMonInitDone = FALSE;

#endif

BOOL InitMultipleMonitorStubs(void)

{
    HMODULE hUser32;

    if (g_fMultiMonInitDone)
		{
        return g_pfnGetMonitorInfo != NULL;
		}

	hUser32 = GetModuleHandle(_T("USER32"));
	if (!hUser32)
		{
		return g_pfnGetMonitorInfo != NULL;
		}

    if ( (*(FARPROC*)&g_pfnGetSystemMetrics    = GetProcAddress(hUser32,"GetSystemMetrics")) &&
         (*(FARPROC*)&g_pfnMonitorFromWindow   = GetProcAddress(hUser32,"MonitorFromWindow")) &&
         (*(FARPROC*)&g_pfnMonitorFromRect     = GetProcAddress(hUser32,"MonitorFromRect")) &&
         (*(FARPROC*)&g_pfnMonitorFromPoint    = GetProcAddress(hUser32,"MonitorFromPoint")) &&
         (*(FARPROC*)&g_pfnEnumDisplayMonitors = GetProcAddress(hUser32,"EnumDisplayMonitors")) &&
#ifdef UNICODE
        (*(FARPROC*)&g_pfnGetMonitorInfo      = GetProcAddress(hUser32, "GetMonitorInfoW")) 
#else
        (*(FARPROC*)&g_pfnGetMonitorInfo      = GetProcAddress(hUser32, "GetMonitorInfoA")) 
#endif
		)
		{
        g_fMultiMonInitDone = TRUE;
        return TRUE;
		}
    else
		{
        g_pfnGetSystemMetrics    = NULL;
        g_pfnMonitorFromWindow   = NULL;
        g_pfnMonitorFromRect     = NULL;
        g_pfnMonitorFromPoint    = NULL;
        g_pfnGetMonitorInfo      = NULL;
        g_pfnEnumDisplayMonitors = NULL;

        g_fMultiMonInitDone = TRUE;
        return FALSE;
		}
}

//-----------------------------------------------------------------------------
//
// fake implementations of Monitor APIs that work with the primary display
// no special parameter validation is made since these run in client code
//
//-----------------------------------------------------------------------------

int WINAPI xGetSystemMetrics(int nIndex)
{
    if (InitMultipleMonitorStubs())
        return g_pfnGetSystemMetrics(nIndex);

    switch (nIndex)
		{
		case SM_CMONITORS:
		case SM_SAMEDISPLAYFORMAT:
			return 1;

		case SM_XVIRTUALSCREEN:
		case SM_YVIRTUALSCREEN:
			return 0;

		case SM_CXVIRTUALSCREEN:
			nIndex = SM_CXSCREEN;
			break;

		case SM_CYVIRTUALSCREEN:
			nIndex = SM_CYSCREEN;
			break;
		}

    return GetSystemMetrics(nIndex);
}

//////////////////////////////////////////////////////////////////////////

HMONITOR WINAPI xMonitorFromPoint(POINT ptScreenCoords, DWORD dwFlags)
{
    if (InitMultipleMonitorStubs())
        return g_pfnMonitorFromPoint(ptScreenCoords, dwFlags);

    if ((dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST)) ||
        ((ptScreenCoords.x >= 0) &&
        (ptScreenCoords.x < GetSystemMetrics(SM_CXSCREEN)) &&
        (ptScreenCoords.y >= 0) &&
        (ptScreenCoords.y < GetSystemMetrics(SM_CYSCREEN))))
    {
        return PRIMARY_MONITOR;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

HMONITOR WINAPI xMonitorFromRect(LPCRECT lprcScreenCoords, DWORD dwFlags)

{
    if (InitMultipleMonitorStubs())
        return g_pfnMonitorFromRect(lprcScreenCoords, dwFlags);

    if ((dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST)) ||
        ((lprcScreenCoords->right > 0) &&
        (lprcScreenCoords->bottom > 0) &&
        (lprcScreenCoords->left < GetSystemMetrics(SM_CXSCREEN)) &&
        (lprcScreenCoords->top < GetSystemMetrics(SM_CYSCREEN))))
		{
        return PRIMARY_MONITOR;
		}

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

HMONITOR WINAPI xMonitorFromWindow(HWND hWnd, DWORD dwFlags)

{
    WINDOWPLACEMENT wp;

    if (InitMultipleMonitorStubs())
        return g_pfnMonitorFromWindow(hWnd, dwFlags);

    if (dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST))
        return PRIMARY_MONITOR;

    if (IsIconic(hWnd) ? 
            GetWindowPlacement(hWnd, &wp) : 
            GetWindowRect(hWnd, &wp.rcNormalPosition)) {

        return xMonitorFromRect(&wp.rcNormalPosition, dwFlags);
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////////

BOOL WINAPI xGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)

{
    RECT rcWork;

    if (InitMultipleMonitorStubs())
        return g_pfnGetMonitorInfo(hMonitor, lpMonitorInfo);

    if ((hMonitor == PRIMARY_MONITOR) &&
        lpMonitorInfo &&
        (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) &&
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        lpMonitorInfo->rcMonitor.left = 0;
        lpMonitorInfo->rcMonitor.top  = 0;
        lpMonitorInfo->rcMonitor.right  = GetSystemMetrics(SM_CXSCREEN);
        lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;

        if (lpMonitorInfo->cbSize >= sizeof(MONITORINFOEX))
            lstrcpy(((MONITORINFOEX*)lpMonitorInfo)->szDevice, _T("DISPLAY"));

        return TRUE;
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////

BOOL WINAPI xEnumDisplayMonitors(
        HDC             hdcOptionalForPainting,
        LPCRECT         lprcEnumMonitorsThatIntersect,
        MONITORENUMPROC lpfnEnumProc,
        LPARAM          dwData)

{
    RECT rcLimit;

    if (InitMultipleMonitorStubs()) {
        return g_pfnEnumDisplayMonitors(
                hdcOptionalForPainting,
                lprcEnumMonitorsThatIntersect,
                lpfnEnumProc,
                dwData);
    }

    if (!lpfnEnumProc)
        return FALSE;

    rcLimit.left   = 0;
    rcLimit.top    = 0;
    rcLimit.right  = GetSystemMetrics(SM_CXSCREEN);
    rcLimit.bottom = GetSystemMetrics(SM_CYSCREEN);

    if (hdcOptionalForPainting)
    {
        RECT    rcClip;
        POINT   ptOrg;

        switch (GetClipBox(hdcOptionalForPainting, &rcClip))
        {
        default:
            if (!GetDCOrgEx(hdcOptionalForPainting, &ptOrg))
                return FALSE;

            OffsetRect(&rcLimit, -ptOrg.x, -ptOrg.y);
            if (IntersectRect(&rcLimit, &rcLimit, &rcClip) &&
                (!lprcEnumMonitorsThatIntersect ||
                     IntersectRect(&rcLimit, &rcLimit, lprcEnumMonitorsThatIntersect))) {

                break;
            }
            //fall thru
        case NULLREGION:
             return TRUE;
        case ERROR:
             return FALSE;
        }
    } else {
        if (    lprcEnumMonitorsThatIntersect &&
                !IntersectRect(&rcLimit, &rcLimit, lprcEnumMonitorsThatIntersect)) {

            return TRUE;
        }
    }

    return lpfnEnumProc(
            PRIMARY_MONITOR,
            hdcOptionalForPainting,
            &rcLimit,
            dwData);
}

