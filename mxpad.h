
/* =====[ MxPad.h ]=========================================================

   Description:

            Interface for MX debug pad.

      REV     DATE     BY           DESCRIPTION
      ----  --------  ----------    --------------------------------------
      0.00  03/09/99  Peter Glen    Initial version.
	  0.00  11/08/05  Peter Glen    Network Version
	  0.00  16/jul/2013 Peter Glen  Compiled with UNICODE

   ======================================================================= */

#ifndef mxpad_defined
#define mxpad_defined

#ifndef SO_DONTLINGER
#include "winsock2.h"
#endif

#include <afxmt.h>

#pragma comment(lib, "ws2_32.lib")

#ifndef TSIZEOF
// Sizeof variable in TCHAR
#define TSIZEOF(xx)      (sizeof (xx) / sizeof (TCHAR))
// Convert to / from textsize
#define TXT2BYTE(xx)      ((xx) * sizeof (TCHAR))
#define BYTE2TXT(xx)      ((xx) / sizeof (TCHAR))
#endif

//////////////////////////////////////////////////////////////////////////

#define CIRC_DEF_SIZE	40960

#define APPNAME _T("MxPad")
#define mxMBOX(aa)  MessageBox(NULL, aa, "", MB_OK)

#define	MXDUMPLEN	32000
#define	MXOUTLEN	32000

//define	OUTLEN	32		// for testing

#define CHUNK 16

//////////////////////////////////////////////////////////////////////////
// Defines for simplification:

#ifdef _DEBUG

#define P2N		mxpad.printf
#define D2N		mxpad.dump
#define C2N		mxpad.ClearNotepad()

#define AP2N	mxpad.aprintf

//////////////////////////////////////////////////////////////////////////
// Shorthands for misc variables:

#define P2N_RECT(var)											\
	P2N(_T("%s  left=%d top=%d right=%d bottom=%d\r\n"), #var,		\
			var.left, var.top, var.right, var.bottom);			\

#define P2N_INT(var)											\
	P2N(_T("%s%d\r\n"), #var, var);

//////////////////////////////////////////////////////////////////////////
// Flag macros:

#define P2NLOCAL(flag)	mxpad.local=flag;
#define P2NREMOTE(flag)	mxpad.remote=flag;

#else

#define P2N		
#define D2N	
#define C2N	
#define AP2N
#define P2N_RECT
#define P2N_INT
#define P2NLOCAL
#define P2NREMOTE
#endif

/* Messages we return if there was no error or we didn't have it in our list. */


/// \brief Prints like printf to the debug pad application (%MxPad)
///
/// This little class acts just like TRACE, except it prints to 
/// the Mxpad application.
///
/// Use: mxpad.printf(_T("printf compatible string\r\n"), var1, var2 ... ); \n
/// \n
/// Example: mxpad.printf(_T("Hello int var=%d\r\n"), var); \n
/// \n
/// The header will automatically define an instance of 'mxpad'\n
/// \n
/// <b>The following defines are available to lessen typing: </b>\n
/// \n
///	P2N -- PrintF to MXPAD ... example P2N(_T("Hello int var=%d\r\n"), var); \n 
///	D2N -- Dump (hexdump) to MXPAD ... example C2N(ptr, len); \n 
///	C2N	-- Clear MXPAD ... example C2N(); \n
/// \n
///	AP2N -- Asyncronous output to MXPAD ... example AP2N(_T("Hello int var=%d\r\n"), var); \n
// \n
/// A2PN print(f) to memory buffer, and output from a separate thread\n
/// Fast!

#define  MX_RES_PORT	2000
#define  MX_CONN_PORT	2001
#define  MX_DATA_PORT	2002


#define  MxPad_Magic	0x2e3e3e

class MxPad

{

//////////////////////////////////////////////////////////////////////////
/// This class implements a circular buffer for the %MxPad class

	class CxCircular  

	{
	public:

		int IsEmpty();
		int Add(TCHAR  chh);
		int Get(TCHAR  *chh);

		CxCircular();
		CxCircular(int bufsize);

		virtual ~CxCircular();

	protected:

		CCriticalSection lock;	  
		

		int		head, tail, size;
		TCHAR	*buff;
				
		void InitVars();
	};

//////////////////////////////////////////////////////////////////////////

	typedef struct _ERROR_STRUCT 
		{
		int		err;
		TCHAR	*str;
		}	ERROR_STRUCT ;

	typedef struct _TH_STRUCT 
		{
		MxPad	*me;
		char	*str;
		int		len;
		}	TH_STRUCT ;

	static	ERROR_STRUCT MxPad::errlist[];
	static	TCHAR  msg_no_error[];
	static	TCHAR  msg_unknown_error[];


	static  HWND    child;
	static	SOCKADDR_IN rec_sin2;
	
	static  HWND    _GetChild(HWND  hwndParent);
	static	BOOL	CALLBACK _EnumChildProc(HWND  hwnd, LPARAM  lParam);

	static	DWORD	_SendThreadProc(DWORD *ptp);
	static	DWORD	_AcceptThreadProc(DWORD *ptp);
	static	void	_RunMxThread(void *lpParam);

	static int	constructed;

	static	SOCKET	glsock2;	
	
public:
	
	// Set this to true if you want release mode debugging
	int force, magic;
	
protected:

	int		remote, local, trace, handle;

	//TCHAR    *szOutString;
	//char	*aszOutString;
	//char	*dump_str;
	
	TCHAR    szOutString[MXOUTLEN];
	TCHAR	aszOutString[MXOUTLEN];
	TCHAR	dump_str[MXDUMPLEN];
	
public:

	int Version();

	MxPad();
	~MxPad();

	HANDLE		event;
	CPtrList	ptrlist;	
	CCriticalSection lock;

	void	SetLocal(int flag);
	void	SetRemote(int flag);
	void	ClearNotepad();

	///	Will output to TRACE instead of %Mxpad
	int		SetTrace(int flag);
	
	TCHAR	*vkey2keystr(int code);

	void	Detect();

	// See if any agent is running (local, remote)	
	int IsLocalAvail();
	int IsRemoteAvail();

#ifdef CWnd

	// Dump WINDOW Styles
	void DumpExStyles(CWnd *ptr);
	void DumpStyles(CWnd *ptr);	
	void DumpSS(CWnd *ptr);
	void DumpEdS(CWnd *ptr); 

#endif

	// Translate WINDOWs errors to strings
	TCHAR *werr2str(int err);

	/// Handle/Convert UNICODE strings
	DWORD   UnicodeToAnsi(LPCWSTR pszW, LPSTR* ppszA);
	DWORD   AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW);

	/// Just like hexdump, but prints to debug pad
	int     dump(const void *mem, int len);

	/// Just like print, but prints to debug pad
	int     printf(const TCHAR  *Format, ...);

	/// Async version
	int     aprintf(const TCHAR  *Format, ...);

	/// Convert numeric message number to readable message
	TCHAR    *num2msg(int msg);

	/// Convert numeric error number to readable message
	TCHAR    *num2err(int msg);

	// Socket error to string
	TCHAR *sockerr2str(int err);

	/// Return TRUE if message is a non-client message
	int     is_nonclient(int msg);

	/// Return TRUE if message is async junc (mouse move etc ...)
	int     is_junk(int msg);

	/// Set Output Application Window name
	void	SetAppname(const TCHAR  *str);

	// Send a string to remote
	int		SendStr(LPCTSTR str);
	
protected:

	CxCircular circ;
	//CStringT<TCHAR, StrTraitMFC<TCHAR, ChTraitsCRT<TCHAR >>>  ascstr;
	CString ascstr;

	//SOCKET	glsock;

	/// Actual output done here
	int     xToNotepad(TCHAR *  str);
	//int     xToNotepad(const TCHAR  *str);

	// Connectivity related 
	SOCKADDR_IN     dest_sin, local_sin;	

	// Socket decriptors	
	int				acc_sin_len;    /* Accept socket address length */
	SOCKADDR		acc_sin;		/* Accept socket address */
	
	int nd, ndlen, connected, resolved; 

	int			xResolve();
	int			xConnect();

	int			endthread;

	//CString		appname;

	TCHAR	appname[_MAX_PATH];

};

#ifdef _DEBUG
extern MxPad	mxpad;
#endif


#endif // mxpad_defined

// EOF


