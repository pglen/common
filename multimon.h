
/* =====[ multimon.h ]========================================================== 
                                                                             
   Description:     Stub module that fakes multiple monitor apis on Win32 OSes
					without them.                                                         
                       
	By using this header your code will get back default values from
	GetSystemMetrics() for new metrics, and the new multimonitor APIs
	will act like only one display is present on a Win32 OS without
	multimonitor APIs.

	Compiled:        MS-VC 6.00                                               
                     				   
	Notes:           <Empty Notes>                                            
                                                                             
	Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  2/18/2011  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

//#define COMPILE_MULTIMON_STUBS

#ifdef __cplusplus
extern "C" {            // Assume C declarations for C++
#endif // __cplusplus

#define PRIMARY_MONITOR				(HMONITOR)0x12340042

#if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)

//
// If we are building with Win95/NT4 headers, we need to declare
// the multimonitor-related metrics and APIs ourselves.
//
#define DM_ORIENTATION          0x00000001L
#define DM_PAPERSIZE            0x00000002L
#define DM_PAPERLENGTH          0x00000004L
#define DM_PAPERWIDTH           0x00000008L
#define DM_SCALE                0x00000010L
#define DM_POSITION             0x00000020L
#define DM_NUP                  0x00000040L

#ifndef SM_CMONITORS

#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#define SM_SAMEDISPLAYFORMAT    81

#define DM_ORIENTATION          0x00000001L
#define DM_PAPERSIZE            0x00000002L
#define DM_PAPERLENGTH          0x00000004L

#define DM_PAPERWIDTH           0x00000008L
#define DM_SCALE                0x00000010L
#define DM_POSITION             0x00000020L
#define DM_NUP                  0x00000040L
 
#endif

#ifndef MONITORINFOF_PRIMARY

	// HMONITOR is already declared if _WIN32_WINNT >= 0x0500 in windef.h
// This is for components built with an older version number.
//


#if !defined(HMONITOR_DECLARED) && (_WIN32_WINNT < 0x0500)

#ifndef HMONITOR
DECLARE_HANDLE(HMONITOR);
#endif

#define HMONITOR_DECLARED
#endif

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002

#define MONITORINFOF_PRIMARY        0x00000001

#ifndef MONITORINFO
typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;
#endif

#ifndef CCHDEVICENAME
#define CCHDEVICENAME 32
#endif

#ifdef __cplusplus

#ifndef tagMONITORINFOEX
typedef struct tagMONITORINFOEX : public tagMONITORINFO
{
    TCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEX, *LPMONITORINFOEX;

#endif

#else

typedef struct
{
    MONITORINFO;
    TCHAR       szDevice[CCHDEVICENAME];
} MONITORINFOEX, *LPMONITORINFOEX;
#endif

typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

#endif  // SM_CMONITORS

#undef GetMonitorInfo
#undef GetSystemMetrics
#undef MonitorFromWindow
#undef MonitorFromRect
#undef MonitorFromPoint
#undef EnumDisplayMonitors

#endif

//
// Define COMPILE_MULTIMON_STUBS to compile the stubs;
// otherwise, you get the declarations.
//
#ifdef COMPILE_MULTIMON_STUBS

//-----------------------------------------------------------------------------
//
// Declare / Implement the API stubs.
//
//-----------------------------------------------------------------------------

int WINAPI xGetSystemMetrics(int nIndex);
HMONITOR WINAPI xMonitorFromPoint(POINT ptScreenCoords, DWORD dwFlags);
HMONITOR WINAPI xMonitorFromRect(LPCRECT lprcScreenCoords, DWORD dwFlags);
HMONITOR WINAPI xMonitorFromWindow(HWND hWnd, DWORD dwFlags);
BOOL WINAPI xGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo);
BOOL WINAPI xEnumDisplayMonitors(
        HDC             hdcOptionalForPainting,
        LPCRECT         lprcEnumMonitorsThatIntersect,
        MONITORENUMPROC lpfnEnumProc,
        LPARAM          dwData);


//#undef xPRIMARY_MONITOR
//#undef COMPILE_MULTIMON_STUBS

#else   // COMPILE_MULTIMON_STUBS

extern int  WINAPI xGetSystemMetrics(int);
extern HMONITOR WINAPI xMonitorFromWindow(HWND, DWORD);
extern HMONITOR WINAPI xMonitorFromRect(LPCRECT, DWORD);
extern HMONITOR WINAPI xMonitorFromPoint(POINT, DWORD);
extern BOOL WINAPI xGetMonitorInfo(HMONITOR, LPMONITORINFO);
extern BOOL WINAPI xEnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);

#endif  // COMPILE_MULTIMON_STUBS

//
// Build defines that replace the regular APIs with our versions
//

#undef GetMonitorInfo
#define GetSystemMetrics    xGetSystemMetrics
#define MonitorFromWindow   xMonitorFromWindow
#define MonitorFromRect     xMonitorFromRect
#define MonitorFromPoint    xMonitorFromPoint
#define GetMonitorInfo      xGetMonitorInfo
#define EnumDisplayMonitors xEnumDisplayMonitors

#ifdef __cplusplus
}
#endif  // __cplusplus
