
/* =====[ mxpad.c ]=========================================================

    MAIN description:
 
        Access library for debug pad. Converted to a class to allow access 
		from MFC library.

    Revisions:

      REV     DATE		 BY           DESCRIPTION
      ----  --------	----------    --------------------------------------
      0.00  03/09/99	Peter Glen    Initial version.
	  0.00  03/20/99	Peter Glen    MFC
	  0.00  08/24/05	Peter Glen    Class conversion
	  0.00  22/22/22	Peter Glen    Remote deployment
	  0.00  22/22/22	Peter Glen    Fixed overflow and defines
	  0.00  24/jun/2013 Peter Glen    Compiled with VS 2005
	  0.00  16/jul/2013 Peter Glen    Compiled with UNICODE
	  0.00  02/apr/2016 Peter Glen    Compiled with UNICODE under VC2005
	
   ======================================================================= */

/* -------- System includes:  -------------------------------------------- */

#include "stdafx.h"

#include <windows.h>
#include <process.h>
 
#include "support.h"
#include "mxpad.h"

// Disable security warnings TODO for later
#pragma warning (disable : 4996)

// Declare one instance, visible globally

#ifdef _DEBUG
class MxPad mxpad;
#endif

//#define _CRT_SECURE_NO_WARNINGS

//////////////////////////////////////////////////////////////////////////
// Declare static(s)

SOCKET		MxPad::glsock2 = NULL;

int	 MxPad::constructed = 0;
TCHAR  MxPad::msg_no_error[] =      TEXT("No error occurred.");
TCHAR  MxPad::msg_unknown_error[] = TEXT("Unknown error.");

MxPad::ERROR_STRUCT MxPad::errlist[] =

{
	{WSAEINTR,           _T("WSAEINTR - Interrupted")},
	{WSAEBADF, 			_T("WSAEBADF - Bad file number")},
	{WSAEFAULT,   		_T("WSAEFAULT - Bad address")},
	{WSAEINVAL,          _T("WSAEINVAL - Invalid argument")},
	{WSAEMFILE,          _T("WSAEMFILE - Too many open files")},
/*
 *    Windows Sockets definitions of regular Berkeley error constants
 */

	{WSAEWOULDBLOCK,   	_T("WSAEWOULDBLOCK - Socket marked as non-blocking")},
	{WSAEINPROGRESS,     _T("WSAEINPROGRESS - Blocking call in progress")},
	{WSAEALREADY,        _T("WSAEALREADY - Command already completed")},
	{WSAENOTSOCK,        _T("WSAENOTSOCK - Descriptor is not a socket")},
	{WSAEDESTADDRREQ,    _T("WSAEDESTADDRREQ - Destination address required")},
	{WSAEMSGSIZE,        _T("WSAEMSGSIZE - Data size too large")},
	{WSAEPROTOTYPE,      _T("WSAEPROTOTYPE - Protocol is of wrong type for this socket")},
	{WSAENOPROTOOPT,     _T("WSAENOPROTOOPT - Protocol option not supported for this socket type")},
	{WSAEPROTONOSUPPORT, _T("WSAEPROTONOSUPPORT - Protocol is not supported")},
	{WSAESOCKTNOSUPPORT, _T("WSAESOCKTNOSUPPORT - Socket type not supported by this address family")},
	{WSAEOPNOTSUPP,      _T("WSAEOPNOTSUPP - Option not supported")},
	{WSAEPFNOSUPPORT,    _T("WSAEPFNOSUPPORT - ")},
	{WSAEAFNOSUPPORT,    _T("WSAEAFNOSUPPORT - Address family not supported by this protocol")},
	{WSAEADDRINUSE,      _T("WSAEADDRINUSE - Address is in use")},
	{WSAEADDRNOTAVAIL,   _T("WSAEADDRNOTAVAIL - Address not available from local machine")},
	{WSAENETDOWN,        _T("WSAENETDOWN - Network subsystem is down")},
	{WSAENETUNREACH,     _T("WSAENETUNREACH - Network cannot be reached")},
	{WSAENETRESET,       _T("WSAENETRESET - Connection has been dropped")},
	{WSAECONNABORTED,    _T("WSAECONNABORTED - ")},
	{WSAECONNRESET,      _T("WSAECONNRESET - ")},
	{WSAENOBUFS,         _T("WSAENOBUFS - No buffer space available")},
	{WSAEISCONN,         _T("WSAEISCONN - Socket is already connected")},
	{WSAENOTCONN,        _T("WSAENOTCONN - Socket is not connected")},
	{WSAESHUTDOWN,       _T("WSAESHUTDOWN - Socket has been shut down")},
	{WSAETOOMANYREFS,    _T("WSAETOOMANYREFS - ")},
	{WSAETIMEDOUT,       _T("WSAETIMEDOUT - Command timed out")},
	{WSAECONNREFUSED,    _T("WSAECONNREFUSED - Connection refused")},
	{WSAELOOP,           _T("WSAELOOP - ")},
	{WSAENAMETOOLONG,    _T("WSAENAMETOOLONG - ")},
	{WSAEHOSTDOWN,       _T("WSAEHOSTDOWN - ")},
	{WSAEHOSTUNREACH,    _T("WSAEHOSTUNREACH - ")},
	{WSAENOTEMPTY,       _T("WSAENOTEMPTY - ")},
	{WSAEPROCLIM,        _T("WSAEPROCLIM - ")},
	{WSAEUSERS,          _T("WSAEUSERS - ")},
	{WSAEDQUOT,          _T("WSAEDQUOT - ")},
	{WSAESTALE,          _T("WSAESTALE - ")},
	{WSAEREMOTE,         _T("WSAEREMOTE - ")},
/*
 *    Extended Windows Sockets error constant definitions
 */
	{WSASYSNOTREADY,     _T("WSASYSNOTREADY - Network subsystem not ready")},
	{WSAVERNOTSUPPORTED, _T("WSAVERNOTSUPPORTED - Version not supported")},
	{WSANOTINITIALISED,  _T("WSANOTINITIALISED - WSAStartup() has not been successfully called")},
/*
 *    Other error constants.
 */
	{WSAHOST_NOT_FOUND,  _T("WSAHOST_NOT_FOUND - Host not found")},
	{WSATRY_AGAIN,       _T("WSATRY_AGAIN - Host not found or SERVERFAIL")},
	{WSANO_RECOVERY,     _T("WSANO_RECOVERY - Non-recoverable error")},
	{WSANO_DATA,         _T("WSANO_DATA - (or WSANO_ADDRESS) - No data record of requested type")},


{-1, NULL}

};

/////////////////////////////////////////////////////////////////////
//#ifdef AfxGetApp

void 	MxPad::_RunMxThread(void *lpParam)

{
	MxPad  *ptr = (MxPad *)lpParam;
	
	//TRACE(_T("Started _RunMxThread\r\n"));

	while(true)
		{
		//P2N(_T("Looping MxThread\r\n"));
		
		ptr->ascstr = _T("");

		while(true)
			{
			TCHAR  chh;

			if(!ptr->circ.Get(&chh))
				break;
				
			ptr->ascstr += chh;
	
			if(ptr->ascstr.GetLength() > MXOUTLEN / 10 &&
					chh == '\n')
				break;
			}

		if (ptr->ascstr != ")")
		{
			TCHAR *buff = ptr->ascstr.GetBuffer();
			ptr->xToNotepad(buff);
			ptr->ascstr.ReleaseBuffer();
		}
    //support.YieldToWinEx();

	if(ptr->endthread)
		break;

	// Ease up on the processor ...
    Sleep(20);

	if(ptr->endthread)
		break;
	}

	//P2N(_T("Ended circular buffer Thread\r\n"));

	 _endthreadex(0);
 
    return;
}

//#endif

HWND    MxPad::child;


BOOL    CALLBACK MxPad::_EnumChildProc(HWND  hwnd, LPARAM  lParam)

{
    child = hwnd;
    return(0);
}

HWND    MxPad::_GetChild(HWND  hwndParent)

{
	child = NULL;

    EnumChildWindows(
                hwndParent,         // handle of parent window
                _EnumChildProc,      // address of callback function
                0                   // application-defined value
               );

    return(child);
}

SOCKADDR_IN MxPad::rec_sin2;

//////////////////////////////////////////////////////////////////////////
// Worker thread for resolving hosts and sending data

DWORD	MxPad::_SendThreadProc(DWORD *ptp)

{
	int ret = 0;

	MxPad *me = (MxPad*)ptp;
	
	//P2N(_T("MxPad::_SendThreadProc\r\n"));

	//ASSERT(me->magic == MxPad_Magic);
	
	while(true)
		{
		ResetEvent(me->event);
		WaitForSingleObject(me->event, INFINITE);

		//TRACE(_T("MxPad::_SendThreadProc after wait\r\n"));

		if(me->endthread)
			break;

		if(!me->resolved)
			me->xResolve();

		if(!me->connected)
			me->xConnect();

		// Still not connected, break
		if(!me->connected)
			break;

		while(true)
			{
			char *str =  "";
			me->lock.Lock();
			int cnt = (int)me->ptrlist.GetCount();
			me->lock.Unlock();
			
			if(cnt <= 0)
				break;
			
			me->lock.Lock();
			str = (char *)me->ptrlist.RemoveHead();
			me->lock.Unlock();

			//TRACE(_T("MxPad::_SendThreadProc sending %d '%s' \r\n"), cnt, str);

			int len = strlen(str);
			
			//struct sockaddr sa;	int slen = sizeof(sa);
			//int ret = getpeername(glsock2, &sa, &slen); 
			//TRACE(_T("ret=%d sa='%s'\r\n"), ret, &sa.sa_data);

			if(glsock2)
				{
				if(send(glsock2, str, len, 0) < 0)
					{
					//MessageBox(NULL, "Cannot send", "", MB_OK);
					CString str2; 
					str2.Format(_T("MxPad::_SendThreadProc Cannot Send '%s'\r\n"), str);
					TRACE(str2);
					}
				}
			free(str);				
			Sleep(20);
			}
		}
	return ret;
}

/*
 * Only one of these will run, no access control needed.
 */

DWORD	MxPad::_AcceptThreadProc(DWORD *ptp)

{
    char	nd  = 1;
    char	*str;

    SOCKADDR_IN rec_sin;

    int     rec_sin_len = sizeof(rec_sin), ndlen  = sizeof(char);
    int     ret_val;

    SOCKET	sock2, sock3;

    //mxMBOX(_T("Hello thread %d"));

    // --------------------------------------------------------------
    // Listen for server answer back:

    sock2 = socket(AF_INET, SOCK_STREAM, 0);
    //TRACE(_T("After sock2.\n"));

    ret_val = setsockopt (sock2, SOL_SOCKET, SO_REUSEADDR, &nd, sizeof(int));
    //TRACE(_T("After ling sockoption ret_val=%d\r\n"), ret_val);
    ret_val = getsockopt (sock2, SOL_SOCKET, SO_REUSEADDR, &nd, &ndlen);
    
	//SO_REUSEADDR SO_DONTLINGER

	//TRACE(_T("After ling getsockoption ret_val=%d ---  nd=%d\r\n"), ret_val, nd);

	memset(&rec_sin, 0, sizeof(rec_sin));

    rec_sin.sin_addr.s_addr = INADDR_ANY;
    rec_sin.sin_port = htons(MX_CONN_PORT);
    rec_sin.sin_family = AF_INET;

    if (bind(sock2, (struct sockaddr FAR *) &rec_sin, sizeof(rec_sin))
        == SOCKET_ERROR)
        {
		CString tmp(_T("none")); int err = WSAGetLastError();
		//tmp.Format(_T("%d %s"), err, sockerr2str(err));

		TRACE(_T("MxPad::AcceptThreadProc -- Cannot Bind socket on port %d %d %s\r\n"), 
							rec_sin.sin_port, ntohs(rec_sin.sin_port), tmp);
		
		*ptp = -1;
		return(1);
        }

    if (listen(sock2, 1) < 0)
        {
        CString tmp(_T("none")); int err = WSAGetLastError();
		//tmp.Format(_T("%d is the error %s"), err, mxpad.sockerr2str(err));

		TRACE(_T("MxPad::AcceptThreadProc Cannot listen on socket2 %s\r\n"), tmp);

		*ptp = -1;
		return(2);
        }

    //printf(_T("After listen2.\n\r"));

    rec_sin_len = sizeof(rec_sin2);

    sock3 = accept(sock2, (struct sockaddr FAR *) &rec_sin2,
            (int FAR *) &rec_sin_len );

    str = inet_ntoa( *((struct in_addr*) &rec_sin2.sin_addr.s_addr));

    //mxMBOX(str);

    closesocket(sock3); closesocket(sock2);

    //(_T("End thread.\n\r"));
    //fflush(stdout);
    // Release crap

    *ptp = 1;

    return(11); 
}

//////////////////////////////////////////////////////////////////////////
/// Clear Debug Pad 

void     MxPad::ClearNotepad()

{
	HWND    chld;

	if(remote)
		{
		//xToNotepad(_T("Clear Requested\r\n"));
		SendStr(TEXT("##Clear##\r\n"));
		}
	
	//if(local)
		{
		 if((chld = _GetChild(FindWindow((LPCTSTR)appname, NULL)) ))
			{
			//Edit_SetSel(chld, 0, -1); 
			//Edit_ReplaceSel(chld, "");

			::SendMessage(chld, EM_SETSEL, 0, -1); 
			::SendMessage(chld, EM_REPLACESEL, 0, (DWORD)""); 
			}
		}	
}

//////////////////////////////////////////////////////////////////////////

MxPad::~MxPad()

{
	//TRACE(_T("MxPad::~MxPad()\r\n"));

	// Stop the async thread, let it breathe
	endthread = true;	
	constructed = false;

	SetEvent(event); Sleep(20);

	lock.Lock();
	while(!ptrlist.IsEmpty())
		free((char*)ptrlist.RemoveHead());
	lock.Unlock();

	//Wait till all is output .... and dump it out
	//if(ascstr != "")
	//	xToNotepad(ascstr);

	//if(dump_str) 
	//	free(dump_str), dump_str = NULL;

	//if(szOutString) 
	//	free(szOutString), szOutString = NULL ;

	//if(aszOutString) 
	//	free(aszOutString), aszOutString = NULL;

	//if(gl_th.str)
	//	free(gl_th.str);
}

//////////////////////////////////////////////////////////////////////////

MxPad::MxPad()

{
	//TRACE(_T("Constructing MxPad\r\n"));

	nd = 1;

	_tcscpy_s(appname,  sizeof(appname), APPNAME);

	//aszOutString = dump_str = szOutString = NULL;

	//dump_str = (TCHAR *) malloc(MXDUMPLEN);	//ASSERT(dump_str);
	//memset(dump_str, 'a', DUMPLEN);

	//szOutString = (TCHAR *) malloc(MXOUTLEN); //ASSERT(szOutString);
	//memset(szOutString, 'a', OUTLEN);

	//aszOutString = (TCHAR *) malloc(MXOUTLEN); //ASSERT(aszOutString);
	//memset(szOutString, 'a', OUTLEN);
	
	ndlen = sizeof(int);
	connected = resolved = false;

	local = true;
	remote = false;
	trace = false;

	handle = 0;
	endthread = false;
	//force = false;
	//force = true;

	constructed = true;
	magic = MxPad_Magic;

	event = CreateEvent(NULL, 0, 0, NULL);
	
	//DWORD res;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) _SendThreadProc,
	//												  (void*)this, 0, &res);

	//memset(&gl_th, 0, sizeof(gl_th));

	//TRACE(_T("Constructed MxPad\r\n"));
}

//////////////////////////////////////////////////////////////////////////
/// \brief Just like print, but prints to debug pad

int     MxPad::printf(const TCHAR  *Format, ...)

{
	#ifndef _DEBUG
		if(!force)
			return (0);
	#endif

	if(!constructed)
		{
		TRACE(_T("Warning: Called Mxpad before construction\r\n"));
		TRACE(_T("Format str was:'%s'\r\n"), Format);
		return 0;
		}

	if(!szOutString)
		{
		TRACE(_T("Warning: Called Mxpad after destruction\r\n"));
		TRACE(_T("Format str was:'%s'\r\n"), Format);
		return 0;
		}

	//TRACE(_T("MxPad::printf with '%s'\r\n"), Format);

//#ifdef		AfxGetApp

	// Start thread if none
	if(!handle)
		{
		handle = _beginthread(_RunMxThread, 0, this);

		//TRACE(_T("Thread handle %d MxPad\r\n"), handle);
		}

//#endif

	// Wait till all is output from the async thread
	while(true)
		{
		TCHAR  chh;

		if(!circ.Get(&chh))
			break;
			
		ascstr += chh;
		}

	TCHAR *buff = ascstr.GetBuffer();
	xToNotepad(buff);
	ascstr.ReleaseBuffer();
	ascstr = "";

	//while(true)
	//	{
	//	if(circ.IsEmpty())
	//		break;
//
//		Sleep(20);
//		}

	//memset(szOutString, 0, MXOUTLEN);

	int len;
	try
		{
		va_list ArgList; va_start(ArgList, Format);

		len = _vsntprintf_s(szOutString, BYTE2TXT(MXOUTLEN - sizeof(TCHAR)), _TRUNCATE, Format, ArgList);

		// Terminate if overflowed
		if(len < 0)	
			{
			szOutString[MXOUTLEN - 1] = '\0';
			xToNotepad(TEXT("\r\nMxpad encountered a buffer overflow.\r\n"));
			}

		xToNotepad(szOutString);
		}

	catch(...)
		{
		xToNotepad(_T("\r\n")
				  _T("----------------------------------------------------------------------------\r\n"));
		xToNotepad(_T("Exception caught on P2N. Possible mismatching of FORMAT str vs. FORMAT arg\r\n"));
		xToNotepad(_T("Here is what we got so far: (between [])\r\n["));
		
		xToNotepad(szOutString);
		
		xToNotepad(_T("]\r\nException End"));
		xToNotepad(_T("\r\n")
			  	  _T("----------------------------------------------------------------------------\r\n"));
		xToNotepad(_T("\r\n"));
		}

	//if(len < 0)	
	//	{
	//	xToNotepad(_T("\r\nWarning: Output string was cut to prevent buffer overflow.\r\n"));
	//	}

    return(0);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Just like hexdump, but prints to debug pad

int     MxPad::dump(const void *mem, int len)

{
    TCHAR     *dump_ptr = dump_str;
    TCHAR    *ptr = (TCHAR *)mem;
    int     slen = 0;
	int		chunk = CHUNK;

	#ifndef _DEBUG
		if(!force)
			return (0);
	#endif
	
	if(!dump_str)
		{
		TRACE(_T("Warning: Called Mxpad after destruction\r\n"));
		TRACE(_T("Memory dump of mem=%p len=%d\r\n"), mem, len);
		return 0;
		}

    if (!mem)
        {
        xToNotepad(_T("MxPad::dump NULL pointer\r\n")); 
		return(0);
        }

	if(!len)
		{
        xToNotepad(_T("MxPad::dump Buffer with zero length.\r\n")); 
		return(0);
        }

	if(!AfxIsValidAddress(mem, len))
		{
		xToNotepad(_T("MxPad::dump Invalid pointer passed.\r\n")); 
		return 0;
		}

    while(TRUE)
        {
        int aa;

        if(len <= 0)                                  /* end of buffer */
            break;

		 if(slen >=  MXDUMPLEN - 96)				/* end of local buffer */
			{
			slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("Warning: dump string was cut to prevent dump buffer overflow.\r\n"));
            break;
			}

        slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("  %08p  -  "), ptr);

        for(aa = 0; aa < chunk; aa++)
			{
			if(aa == 4)
				slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T(" "));

			if(aa == 8)
				slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("  "));

			if(aa == 12)
				slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T(" "));

            slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("%02x "), *(ptr+aa) & 0xff);

			}

		if(chunk < CHUNK)
			{
	        for(aa = 0; aa < CHUNK - chunk; aa++)
				slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("   "));
			}

        slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("  -  "));

        for(aa = 0; aa < chunk; aa++)
            {
            TCHAR chh = *(ptr+aa);

            if(!isprint((int)chh & 0xff))                 /* convert to ASC if not prt */
                chh = '.';

			if(chh == '\b')
				chh = '.';

            slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("%c"), chh);
            }

		// Do Second line
		//slen += sprintf(dump_str + slen, "\r\n            ");

		//for(aa = 0; aa < chunk/2; aa++)
          //  {
            //WORD ww = * (((WORD*)ptr) + aa);
//
//			slen += sprintf(dump_str + slen, "  %04x", ww & 0xffff);
//			}

		if(chunk < CHUNK)
			{
	        for(aa = 0; aa < CHUNK - chunk; aa++)
				slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T(" "));
			}

        slen += _stprintf_s(dump_str + slen, _TRUNCATE, _T("  - \r\n"));
        
        len -= chunk; ptr += chunk;

		if(len < chunk)
			chunk = len;
        }

    xToNotepad(dump_str);

    return(0);
}

/// Set local output flag

void	MxPad::SetLocal(int flag)

{
	local = flag;
}

/// Set remote output flag

void	MxPad::SetRemote(int flag)

{
	remote = flag;
}

// --------------------------------------------------------------
// Return true if local agent is found

/// \brief   Return true if local Mxpad agent is found.
/// 
/// \details This function will test if the local agent has started. 
/// That way the app can elect to test ONCE and never to try again.
/// \n  
/// Return type:      int  
/// \n  

int     MxPad::IsLocalAvail()

{
	int ret = 0;
	ret = (int)FindWindow((LPCTSTR)appname, NULL);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
// Return true if remote agent is found
	
int     MxPad::IsRemoteAvail()

{
	#ifndef _DEBUG
		return 0;
	#endif

	return xResolve();
}

//////////////////////////////////////////////////////////////////////////
/// Set Trace flag. Will output into C++ debug window instead. 
/// Returns old trace flag.

int		MxPad::SetTrace(int flag)

{
	int old = trace;
	trace = flag;
	return old;
}



//////////////////////////////////////////////////////////////////////////
// This is the main output routine

int     MxPad::xToNotepad(TCHAR *str)

{
	//ASSERT(magic == MxPad_Magic);

	#ifndef _DEBUG
		if(!force)
			return (0);
	#endif 

	if(local)
		{
		HWND    chld;

		if((chld = _GetChild(FindWindow((LPCTSTR)appname, NULL)) ))
			{
			//Edit_ReplaceSel(chld, str);

			int len = SendMessage(chld,  WM_GETTEXTLENGTH, 0, 0);
				
			// Seek to end
			SendMessage(chld, EM_SETSEL, len, len); 
			SendMessage(chld, EM_REPLACESEL, 0, (DWORD)str); 
			}
		}

	if(trace)
		{
#ifdef TRACE
		//TRACE(str);
#else
		// Error, no trace
#endif
		}

	if(remote)
		{	
		SendStr(str);
		}

    return(0);
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

DWORD   MxPad::UnicodeToAnsi(LPCWSTR pszW, LPSTR* ppszA)

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

DWORD   MxPad::AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW)

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
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
    *ppszW, cCharacters))
        {
        dwError = GetLastError();
        free(*ppszW);
        *ppszW = NULL;
        return (dwError);
        }
    return NOERROR;
}

// --------------------------------------------------------------
//
/// \brief   Convert numeric message number to text 
/// 
/// \details DescHere 
/// \n  
/// Return type:      TCHAR  
/// \n  
/// \param msg Arg Type: int 

TCHAR    *MxPad::num2msg(int msg)

{
	static	TCHAR  buff[32];

    TCHAR *str = _T("WM_UNKNOWN");

    switch(msg)
        {
        case WM_NULL              : str = _T("WM_NULL"); break;
        case WM_CREATE            : str = _T("WM_CREATE"); break;
        case WM_DESTROY           : str = _T("WM_DESTROY"); break;
        case WM_MOVE              : str = _T("WM_MOVE"); break;
        case WM_SIZE              : str = _T("WM_SIZE"); break;
        case WM_ACTIVATE          : str = _T("WM_ACTIVATE"); break;
        case WM_SETFOCUS          : str = _T("WM_SETFOCUS"); break;
        case WM_KILLFOCUS         : str = _T("WM_KILLFOCUS"); break;
        case WM_ENABLE            : str = _T("WM_ENABLE"); break;
        case WM_SETREDRAW         : str = _T("WM_SETREDRAW"); break;
        case WM_SETTEXT           : str = _T("WM_SETTEXT"); break;
        case WM_GETTEXT           : str = _T("WM_GETTEXT"); break;
        case WM_GETTEXTLENGTH     : str = _T("WM_GETTEXTLENGTH"); break;
        case WM_PAINT             : str = _T("WM_PAINT"); break;
        case WM_CLOSE             : str = _T("WM_CLOSE"); break;
        case WM_QUERYENDSESSION   : str = _T("WM_QUERYENDSESSION"); break;
        case WM_QUIT              : str = _T("WM_QUIT"); break;
        case WM_QUERYOPEN         : str = _T("WM_QUERYOPEN"); break;
        case WM_ERASEBKGND        : str = _T("WM_ERASEBKGND"); break;
        case WM_SYSCOLORCHANGE    : str = _T("WM_SYSCOLORCHANGE"); break;
        case WM_ENDSESSION        : str = _T("WM_ENDSESSION"); break;
        case WM_SHOWWINDOW        : str = _T("WM_SHOWWINDOW"); break;
        case WM_WININICHANGE      : str = _T("WM_WININICHANGE"); break;
        case WM_DEVMODECHANGE     : str = _T("WM_DEVMODECHANGE"); break;
        case WM_ACTIVATEAPP       : str = _T("WM_ACTIVATEAPP"); break;
        case WM_FONTCHANGE        : str = _T("WM_FONTCHANGE"); break;
        case WM_TIMECHANGE        : str = _T("WM_TIMECHANGE"); break;
        case WM_CANCELMODE        : str = _T("WM_CANCELMODE"); break;
        case WM_SETCURSOR         : str = _T("WM_SETCURSOR"); break;
        case WM_MOUSEACTIVATE     : str = _T("WM_MOUSEACTIVATE"); break;
        case WM_CHILDACTIVATE     : str = _T("WM_CHILDACTIVATE"); break;
        case WM_QUEUESYNC         : str = _T("WM_QUEUESYNC"); break;
        case WM_GETMINMAXINFO     : str = _T("WM_GETMINMAXINFO"); break;
        case WM_PAINTICON         : str = _T("WM_PAINTICON"); break;
        case WM_ICONERASEBKGND    : str = _T("WM_ICONERASEBKGND"); break;
        case WM_NEXTDLGCTL        : str = _T("WM_NEXTDLGCTL"); break;
        case WM_SPOOLERSTATUS     : str = _T("WM_SPOOLERSTATUS"); break;
        case WM_DRAWITEM          : str = _T("WM_DRAWITEM"); break;
        case WM_MEASUREITEM       : str = _T("WM_MEASUREITEM"); break;
        case WM_DELETEITEM        : str = _T("WM_DELETEITEM"); break;
        case WM_VKEYTOITEM        : str = _T("WM_VKEYTOITEM"); break;
        case WM_CHARTOITEM        : str = _T("WM_CHARTOITEM"); break;
        case WM_SETFONT           : str = _T("WM_SETFONT"); break;
        case WM_GETFONT           : str = _T("WM_GETFONT"); break;
        case WM_SETHOTKEY         : str = _T("WM_SETHOTKEY"); break;
        case WM_GETHOTKEY         : str = _T("WM_GETHOTKEY"); break;
        case WM_QUERYDRAGICON     : str = _T("WM_QUERYDRAGICON"); break;
        case WM_COMPAREITEM       : str = _T("WM_COMPAREITEM"); break;
        case WM_COMPACTING        : str = _T("WM_COMPACTING"); break;
        case WM_COMMNOTIFY        : str = _T("WM_COMMNOTIFY"); break;
        case WM_WINDOWPOSCHANGING : str = _T("WM_WINDOWPOSCHANGING"); break;
        case WM_WINDOWPOSCHANGED  : str = _T("WM_WINDOWPOSCHANGED"); break;
        case WM_POWER             : str = _T("WM_POWER"); break;
        case WM_COPYDATA          : str = _T("WM_COPYDATA"); break;
        case WM_CANCELJOURNAL     : str = _T("WM_CANCELJOURNAL"); break;
        case WM_NCCREATE          : str = _T("WM_NCCREATE"); break;
        case WM_NCDESTROY         : str = _T("WM_NCDESTROY"); break;
        case WM_NCCALCSIZE        : str = _T("WM_NCCALCSIZE"); break;
        case WM_NCHITTEST         : str = _T("WM_NCHITTEST"); break;
        case WM_NCPAINT           : str = _T("WM_NCPAINT"); break;
        case WM_NCACTIVATE        : str = _T("WM_NCACTIVATE"); break;
        case WM_GETDLGCODE        : str = _T("WM_GETDLGCODE"); break;
        case WM_NCMOUSEMOVE       : str = _T("WM_NCMOUSEMOVE"); break;
        case WM_NCLBUTTONDOWN     : str = _T("WM_NCLBUTTONDOWN"); break;
        case WM_NCLBUTTONUP       : str = _T("WM_NCLBUTTONUP"); break;
        case WM_NCLBUTTONDBLCLK   : str = _T("WM_NCLBUTTONDBLCLK"); break;
        case WM_NCRBUTTONDOWN     : str = _T("WM_NCRBUTTONDOWN"); break;
        case WM_NCRBUTTONUP       : str = _T("WM_NCRBUTTONUP"); break;
        case WM_NCRBUTTONDBLCLK   : str = _T("WM_NCRBUTTONDBLCLK"); break;
        case WM_NCMBUTTONDOWN     : str = _T("WM_NCMBUTTONDOWN"); break;
        case WM_NCMBUTTONUP       : str = _T("WM_NCMBUTTONUP"); break;
        case WM_NCMBUTTONDBLCLK   : str = _T("WM_NCMBUTTONDBLCLK"); break;
        //case WM_KEYFIRST          : str = _T("WM_KEYFIRST"); break;
        case WM_KEYDOWN           : str = _T("WM_KEYDOWN"); break;
        case WM_KEYUP             : str = _T("WM_KEYUP"); break;
        //case WM_TCHAR              : str = _T("WM_CHAR"); break;
        //case WM_DEADTCHAR          : str = _T("WM_DEADCHAR"); break;
        case WM_SYSKEYDOWN        : str = _T("WM_SYSKEYDOWN"); break;
        case WM_SYSKEYUP          : str = _T("WM_SYSKEYUP"); break;
        //case WM_SYSTCHAR           : str = _T("WM_SYSCHAR"); break;
        //case WM_SYSDEADTCHAR       : str = _T("WM_SYSDEADCHAR"); break;
        case WM_KEYLAST           : str = _T("WM_KEYLAST"); break;
        case WM_INITDIALOG        : str = _T("WM_INITDIALOG"); break;
        case WM_COMMAND           : str = _T("WM_COMMAND"); break;
        case WM_SYSCOMMAND        : str = _T("WM_SYSCOMMAND"); break;
        case WM_TIMER             : str = _T("WM_TIMER"); break;
        case WM_HSCROLL           : str = _T("WM_HSCROLL"); break;
        case WM_VSCROLL           : str = _T("WM_VSCROLL"); break;
        case WM_INITMENU          : str = _T("WM_INITMENU"); break;
        case WM_INITMENUPOPUP     : str = _T("WM_INITMENUPOPUP"); break;
        case WM_MENUSELECT        : str = _T("WM_MENUSELECT"); break;
        //case WM_MENUTCHAR          : str = _T("WM_MENUCHAR"); break;
        case WM_ENTERIDLE         : str = _T("WM_ENTERIDLE"); break;
        case WM_CTLCOLORMSGBOX    : str = _T("WM_CTLCOLORMSGBOX"); break;
        case WM_CTLCOLOREDIT      : str = _T("WM_CTLCOLOREDIT"); break;
        case WM_CTLCOLORLISTBOX   : str = _T("WM_CTLCOLORLISTBOX"); break;
        case WM_CTLCOLORBTN       : str = _T("WM_CTLCOLORBTN"); break;
        case WM_CTLCOLORDLG       : str = _T("WM_CTLCOLORDLG"); break;
        case WM_CTLCOLORSCROLLBAR : str = _T("WM_CTLCOLORSCROLLBAR"); break;
        case WM_CTLCOLORSTATIC    : str = _T("WM_CTLCOLORSTATIC"); break;
        //case WM_MOUSEFIRST        : str = _T("WM_MOUSEFIRST"); break;
        case WM_MOUSEMOVE         : str = _T("WM_MOUSEMOVE"); break;
        case WM_LBUTTONDOWN       : str = _T("WM_LBUTTONDOWN"); break;
        case WM_LBUTTONUP         : str = _T("WM_LBUTTONUP"); break;
        case WM_LBUTTONDBLCLK     : str = _T("WM_LBUTTONDBLCLK"); break;
        case WM_RBUTTONDOWN       : str = _T("WM_RBUTTONDOWN"); break;
        case WM_RBUTTONUP         : str = _T("WM_RBUTTONUP"); break;
        case WM_RBUTTONDBLCLK     : str = _T("WM_RBUTTONDBLCLK"); break;
        case WM_MBUTTONDOWN       : str = _T("WM_MBUTTONDOWN"); break;
        case WM_MBUTTONUP         : str = _T("WM_MBUTTONUP"); break;
        case WM_MBUTTONDBLCLK     : str = _T("WM_MBUTTONDBLCLK"); break;
        //case WM_MOUSELAST         : str = _T("WM_MOUSELAST"); break;
        case WM_PARENTNOTIFY      : str = _T("WM_PARENTNOTIFY"); break;
        case WM_ENTERMENULOOP     : str = _T("WM_ENTERMENULOOP"); break;
        case WM_EXITMENULOOP      : str = _T("WM_EXITMENULOOP"); break;
        case WM_MDICREATE         : str = _T("WM_MDICREATE"); break;
        case WM_MDIDESTROY        : str = _T("WM_MDIDESTROY"); break;
        case WM_MDIACTIVATE       : str = _T("WM_MDIACTIVATE"); break;
        case WM_MDIRESTORE        : str = _T("WM_MDIRESTORE"); break;
        case WM_MDINEXT           : str = _T("WM_MDINEXT"); break;
        case WM_MDIMAXIMIZE       : str = _T("WM_MDIMAXIMIZE"); break;
        case WM_MDITILE           : str = _T("WM_MDITILE"); break;
        case WM_MDICASCADE        : str = _T("WM_MDICASCADE"); break;
        case WM_MDIICONARRANGE    : str = _T("WM_MDIICONARRANGE"); break;
        case WM_MDIGETACTIVE      : str = _T("WM_MDIGETACTIVE"); break;
        case WM_MDISETMENU        : str = _T("WM_MDISETMENU"); break;
        case WM_DROPFILES         : str = _T("WM_DROPFILES"); break;
        case WM_MDIREFRESHMENU    : str = _T("WM_MDIREFRESHMENU"); break;
        case WM_CUT               : str = _T("WM_CUT"); break;
        case WM_COPY              : str = _T("WM_COPY"); break;
        case WM_PASTE             : str = _T("WM_PASTE"); break;
        case WM_CLEAR             : str = _T("WM_CLEAR"); break;
        case WM_UNDO              : str = _T("WM_UNDO"); break;
        case WM_RENDERFORMAT      : str = _T("WM_RENDERFORMAT"); break;
        case WM_RENDERALLFORMATS  : str = _T("WM_RENDERALLFORMATS"); break;
        case WM_DESTROYCLIPBOARD  : str = _T("WM_DESTROYCLIPBOARD"); break;
        case WM_DRAWCLIPBOARD     : str = _T("WM_DRAWCLIPBOARD"); break;
        case WM_PAINTCLIPBOARD    : str = _T("WM_PAINTCLIPBOARD"); break;
        case WM_VSCROLLCLIPBOARD  : str = _T("WM_VSCROLLCLIPBOARD"); break;
        case WM_SIZECLIPBOARD     : str = _T("WM_SIZECLIPBOARD"); break;
        case WM_ASKCBFORMATNAME   : str = _T("WM_ASKCBFORMATNAME"); break;
        case WM_CHANGECBCHAIN     : str = _T("WM_CHANGECBCHAIN"); break;
        case WM_HSCROLLCLIPBOARD  : str = _T("WM_HSCROLLCLIPBOARD"); break;
        case WM_QUERYNEWPALETTE   : str = _T("WM_QUERYNEWPALETTE"); break;
        case WM_PALETTEISCHANGING : str = _T("WM_PALETTEISCHANGING"); break;
        case WM_PALETTECHANGED    : str = _T("WM_PALETTECHANGED"); break;
        case WM_HOTKEY            : str = _T("WM_HOTKEY"); break;
        case WM_PENWINFIRST       : str = _T("WM_PENWINFIRST"); break;
        case WM_PENWINLAST        : str = _T("WM_PENWINLAST"); break;
        case WM_USER              : str = _T("WM_USER"); break;

		default:
			_stprintf_s(buff, sizeof(buff), _T("WM_UNKOWN %d"), msg);
			str = buff;
			break;
        }

    return(str);
}

//////////////////////////////////////////////////////////////////////////
// Non client messages are identified:
//

int     MxPad::is_nonclient(int msg)

{
    int     ret_val = FALSE;

    switch(msg)
        {
        case WM_NCCREATE:
        case WM_NCDESTROY:
        case WM_NCCALCSIZE:
        case WM_NCHITTEST:
        case WM_NCPAINT:
        case WM_NCACTIVATE:
        case WM_NCMOUSEMOVE:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
            ret_val = TRUE;
            break;
        }
    return(ret_val);
}

int     MxPad::is_junk(int msg)

{
    int     ret_val = FALSE;

    switch(msg)
        {

        case WM_MOUSEMOVE:
        case WM_SETCURSOR:
        case WM_TIMER:
        case WM_PAINT:
            ret_val = TRUE;
            break;
        }
    return(ret_val);
}

// --------------------------------------------------------------
//
/// \brief   Convert a numeric error to text 
/// 
/// \n  
/// Return type:      TCHAR  
/// \n  
/// \param msg Arg Type: int -- error number to convert from

TCHAR    *MxPad::num2err(int msg)

{
    TCHAR *str = _T("ERROR_UNKNOWN");

    switch(msg)
        {
        case ERROR_SUCCESS                      : str = _T("ERROR_SUCCESS");
        case ERROR_INVALID_FUNCTION             : str = _T("ERROR_INVALID_FUNCTION");
        case ERROR_FILE_NOT_FOUND               : str = _T("ERROR_FILE_NOT_FOUND");
        case ERROR_PATH_NOT_FOUND               : str = _T("ERROR_PATH_NOT_FOUND");
        case ERROR_TOO_MANY_OPEN_FILES          : str = _T("ERROR_TOO_MANY_OPEN_FILES");
        case ERROR_ACCESS_DENIED                : str = _T("ERROR_ACCESS_DENIED");
        case ERROR_INVALID_HANDLE               : str = _T("ERROR_INVALID_HANDLE");
        case ERROR_ARENA_TRASHED                : str = _T("ERROR_ARENA_TRASHED");
        case ERROR_NOT_ENOUGH_MEMORY            : str = _T("ERROR_NOT_ENOUGH_MEMORY");
        case ERROR_INVALID_BLOCK                : str = _T("ERROR_INVALID_BLOCK");
        case ERROR_BAD_ENVIRONMENT              : str = _T("ERROR_BAD_ENVIRONMENT");
        case ERROR_BAD_FORMAT                   : str = _T("ERROR_BAD_FORMAT");
        case ERROR_INVALID_ACCESS               : str = _T("ERROR_INVALID_ACCESS");
        case ERROR_INVALID_DATA                 : str = _T("ERROR_INVALID_DATA");
        case ERROR_OUTOFMEMORY                  : str = _T("ERROR_OUTOFMEMORY");
        case ERROR_INVALID_DRIVE                : str = _T("ERROR_INVALID_DRIVE");
        case ERROR_CURRENT_DIRECTORY            : str = _T("ERROR_CURRENT_DIRECTORY");
        case ERROR_NOT_SAME_DEVICE              : str = _T("ERROR_NOT_SAME_DEVICE");
        case ERROR_NO_MORE_FILES                : str = _T("ERROR_NO_MORE_FILES");
        case ERROR_WRITE_PROTECT                : str = _T("ERROR_WRITE_PROTECT");
        case ERROR_BAD_UNIT                     : str = _T("ERROR_BAD_UNIT");
        case ERROR_NOT_READY                    : str = _T("ERROR_NOT_READY");
        case ERROR_BAD_COMMAND                  : str = _T("ERROR_BAD_COMMAND");
        case ERROR_CRC                          : str = _T("ERROR_CRC");
        case ERROR_BAD_LENGTH                   : str = _T("ERROR_BAD_LENGTH");
        case ERROR_SEEK                         : str = _T("ERROR_SEEK");
        case ERROR_NOT_DOS_DISK                 : str = _T("ERROR_NOT_DOS_DISK");
        case ERROR_SECTOR_NOT_FOUND             : str = _T("ERROR_SECTOR_NOT_FOUND");
        case ERROR_OUT_OF_PAPER                 : str = _T("ERROR_OUT_OF_PAPER");
        case ERROR_WRITE_FAULT                  : str = _T("ERROR_WRITE_FAULT");
        case ERROR_READ_FAULT                   : str = _T("ERROR_READ_FAULT");
        case ERROR_GEN_FAILURE                  : str = _T("ERROR_GEN_FAILURE");
        case ERROR_SHARING_VIOLATION            : str = _T("ERROR_SHARING_VIOLATION");
        case ERROR_LOCK_VIOLATION               : str = _T("ERROR_LOCK_VIOLATION");
        case ERROR_WRONG_DISK                   : str = _T("ERROR_WRONG_DISK");
        case ERROR_SHARING_BUFFER_EXCEEDED      : str = _T("ERROR_SHARING_BUFFER_EXCEEDED");
        case ERROR_HANDLE_EOF                   : str = _T("ERROR_HANDLE_EOF");
        case ERROR_HANDLE_DISK_FULL             : str = _T("ERROR_HANDLE_DISK_FULL");
        case ERROR_NOT_SUPPORTED                : str = _T("ERROR_NOT_SUPPORTED");
        case ERROR_REM_NOT_LIST                 : str = _T("ERROR_REM_NOT_LIST");
        case ERROR_DUP_NAME                     : str = _T("ERROR_DUP_NAME");
        case ERROR_BAD_NETPATH                  : str = _T("ERROR_BAD_NETPATH");
        case ERROR_NETWORK_BUSY                 : str = _T("ERROR_NETWORK_BUSY");
        case ERROR_DEV_NOT_EXIST                : str = _T("ERROR_DEV_NOT_EXIST");
        case ERROR_TOO_MANY_CMDS                : str = _T("ERROR_TOO_MANY_CMDS");
        case ERROR_ADAP_HDW_ERR                 : str = _T("ERROR_ADAP_HDW_ERR");
        case ERROR_BAD_NET_RESP                 : str = _T("ERROR_BAD_NET_RESP");
        case ERROR_UNEXP_NET_ERR                : str = _T("ERROR_UNEXP_NET_ERR");
        case ERROR_BAD_REM_ADAP                 : str = _T("ERROR_BAD_REM_ADAP");
        case ERROR_PRINTQ_FULL                  : str = _T("ERROR_PRINTQ_FULL");
        case ERROR_NO_SPOOL_SPACE               : str = _T("ERROR_NO_SPOOL_SPACE");
        case ERROR_PRINT_CANCELLED              : str = _T("ERROR_PRINT_CANCELLED");
        case ERROR_NETNAME_DELETED              : str = _T("ERROR_NETNAME_DELETED");
        case ERROR_NETWORK_ACCESS_DENIED        : str = _T("ERROR_NETWORK_ACCESS_DENIED");
        case ERROR_BAD_DEV_TYPE                 : str = _T("ERROR_BAD_DEV_TYPE");
        case ERROR_BAD_NET_NAME                 : str = _T("ERROR_BAD_NET_NAME");
        case ERROR_TOO_MANY_NAMES               : str = _T("ERROR_TOO_MANY_NAMES");
        case ERROR_TOO_MANY_SESS                : str = _T("ERROR_TOO_MANY_SESS");
        case ERROR_SHARING_PAUSED               : str = _T("ERROR_SHARING_PAUSED");
        case ERROR_REQ_NOT_ACCEP                : str = _T("ERROR_REQ_NOT_ACCEP");
        case ERROR_REDIR_PAUSED                 : str = _T("ERROR_REDIR_PAUSED");
        case ERROR_FILE_EXISTS                  : str = _T("ERROR_FILE_EXISTS");
        case ERROR_CANNOT_MAKE                  : str = _T("ERROR_CANNOT_MAKE");
        case ERROR_FAIL_I24                     : str = _T("ERROR_FAIL_I24");
        case ERROR_OUT_OF_STRUCTURES            : str = _T("ERROR_OUT_OF_STRUCTURES");
        case ERROR_ALREADY_ASSIGNED             : str = _T("ERROR_ALREADY_ASSIGNED");
        case ERROR_INVALID_PASSWORD             : str = _T("ERROR_INVALID_PASSWORD");
        case ERROR_INVALID_PARAMETER            : str = _T("ERROR_INVALID_PARAMETER");
        case ERROR_NET_WRITE_FAULT              : str = _T("ERROR_NET_WRITE_FAULT");
        case ERROR_NO_PROC_SLOTS                : str = _T("ERROR_NO_PROC_SLOTS");
        case ERROR_TOO_MANY_SEMAPHORES          : str = _T("ERROR_TOO_MANY_SEMAPHORES");
        case ERROR_EXCL_SEM_ALREADY_OWNED       : str = _T("ERROR_EXCL_SEM_ALREADY_OWNED");
        case ERROR_SEM_IS_SET                   : str = _T("ERROR_SEM_IS_SET");
        case ERROR_TOO_MANY_SEM_REQUESTS        : str = _T("ERROR_TOO_MANY_SEM_REQUESTS");
        case ERROR_INVALID_AT_INTERRUPT_TIME    : str = _T("ERROR_INVALID_AT_INTERRUPT_TIME");
        case ERROR_SEM_OWNER_DIED               : str = _T("ERROR_SEM_OWNER_DIED");
        case ERROR_SEM_USER_LIMIT               : str = _T("ERROR_SEM_USER_LIMIT");
        case ERROR_DISK_CHANGE                  : str = _T("ERROR_DISK_CHANGE");
        case ERROR_DRIVE_LOCKED                 : str = _T("ERROR_DRIVE_LOCKED");
        case ERROR_BROKEN_PIPE                  : str = _T("ERROR_BROKEN_PIPE");
        case ERROR_OPEN_FAILED                  : str = _T("ERROR_OPEN_FAILED");
        case ERROR_BUFFER_OVERFLOW              : str = _T("ERROR_BUFFER_OVERFLOW");
        case ERROR_DISK_FULL                    : str = _T("ERROR_DISK_FULL");
        case ERROR_NO_MORE_SEARCH_HANDLES       : str = _T("ERROR_NO_MORE_SEARCH_HANDLES");
        case ERROR_INVALID_TARGET_HANDLE        : str = _T("ERROR_INVALID_TARGET_HANDLE");
        case ERROR_INVALID_CATEGORY             : str = _T("ERROR_INVALID_CATEGORY");
        case ERROR_INVALID_VERIFY_SWITCH        : str = _T("ERROR_INVALID_VERIFY_SWITCH");
        case ERROR_BAD_DRIVER_LEVEL             : str = _T("ERROR_BAD_DRIVER_LEVEL");
        case ERROR_CALL_NOT_IMPLEMENTED         : str = _T("ERROR_CALL_NOT_IMPLEMENTED");
        case ERROR_SEM_TIMEOUT                  : str = _T("ERROR_SEM_TIMEOUT");
        case ERROR_INSUFFICIENT_BUFFER          : str = _T("ERROR_INSUFFICIENT_BUFFER");
        case ERROR_INVALID_NAME                 : str = _T("ERROR_INVALID_NAME");
        case ERROR_INVALID_LEVEL                : str = _T("ERROR_INVALID_LEVEL");
        case ERROR_NO_VOLUME_LABEL              : str = _T("ERROR_NO_VOLUME_LABEL");
        case ERROR_MOD_NOT_FOUND                : str = _T("ERROR_MOD_NOT_FOUND");
        case ERROR_PROC_NOT_FOUND               : str = _T("ERROR_PROC_NOT_FOUND");
        case ERROR_WAIT_NO_CHILDREN             : str = _T("ERROR_WAIT_NO_CHILDREN");
        case ERROR_CHILD_NOT_COMPLETE           : str = _T("ERROR_CHILD_NOT_COMPLETE");
        case ERROR_DIRECT_ACCESS_HANDLE         : str = _T("ERROR_DIRECT_ACCESS_HANDLE");
        case ERROR_NEGATIVE_SEEK                : str = _T("ERROR_NEGATIVE_SEEK");
        case ERROR_SEEK_ON_DEVICE               : str = _T("ERROR_SEEK_ON_DEVICE");
        case ERROR_IS_JOIN_TARGET               : str = _T("ERROR_IS_JOIN_TARGET");
        case ERROR_IS_JOINED                    : str = _T("ERROR_IS_JOINED");
        case ERROR_IS_SUBSTED                   : str = _T("ERROR_IS_SUBSTED");
        case ERROR_NOT_JOINED                   : str = _T("ERROR_NOT_JOINED");
        case ERROR_NOT_SUBSTED                  : str = _T("ERROR_NOT_SUBSTED");
        case ERROR_JOIN_TO_JOIN                 : str = _T("ERROR_JOIN_TO_JOIN");
        case ERROR_SUBST_TO_SUBST               : str = _T("ERROR_SUBST_TO_SUBST");
        case ERROR_JOIN_TO_SUBST                : str = _T("ERROR_JOIN_TO_SUBST");
        case ERROR_SUBST_TO_JOIN                : str = _T("ERROR_SUBST_TO_JOIN");
        case ERROR_BUSY_DRIVE                   : str = _T("ERROR_BUSY_DRIVE");
        case ERROR_SAME_DRIVE                   : str = _T("ERROR_SAME_DRIVE");
        case ERROR_DIR_NOT_ROOT                 : str = _T("ERROR_DIR_NOT_ROOT");
        case ERROR_DIR_NOT_EMPTY                : str = _T("ERROR_DIR_NOT_EMPTY");
        case ERROR_IS_SUBST_PATH                : str = _T("ERROR_IS_SUBST_PATH");
        case ERROR_IS_JOIN_PATH                 : str = _T("ERROR_IS_JOIN_PATH");
        case ERROR_PATH_BUSY                    : str = _T("ERROR_PATH_BUSY");
        case ERROR_IS_SUBST_TARGET              : str = _T("ERROR_IS_SUBST_TARGET");
        case ERROR_SYSTEM_TRACE                 : str = _T("ERROR_SYSTEM_TRACE");
        case ERROR_INVALID_EVENT_COUNT          : str = _T("ERROR_INVALID_EVENT_COUNT");
        case ERROR_TOO_MANY_MUXWAITERS          : str = _T("ERROR_TOO_MANY_MUXWAITERS");
        case ERROR_INVALID_LIST_FORMAT          : str = _T("ERROR_INVALID_LIST_FORMAT");
        case ERROR_LABEL_TOO_LONG               : str = _T("ERROR_LABEL_TOO_LONG");
        case ERROR_TOO_MANY_TCBS                : str = _T("ERROR_TOO_MANY_TCBS");
        case ERROR_SIGNAL_REFUSED               : str = _T("ERROR_SIGNAL_REFUSED");
        case ERROR_DISCARDED                    : str = _T("ERROR_DISCARDED");
        case ERROR_NOT_LOCKED                   : str = _T("ERROR_NOT_LOCKED");
        case ERROR_BAD_THREADID_ADDR            : str = _T("ERROR_BAD_THREADID_ADDR");
        case ERROR_BAD_ARGUMENTS                : str = _T("ERROR_BAD_ARGUMENTS");
        case ERROR_BAD_PATHNAME                 : str = _T("ERROR_BAD_PATHNAME");
        case ERROR_SIGNAL_PENDING               : str = _T("ERROR_SIGNAL_PENDING");
        case ERROR_MAX_THRDS_REACHED            : str = _T("ERROR_MAX_THRDS_REACHED");
        case ERROR_LOCK_FAILED                  : str = _T("ERROR_LOCK_FAILED");
        case ERROR_BUSY                         : str = _T("ERROR_BUSY");
        case ERROR_CANCEL_VIOLATION             : str = _T("ERROR_CANCEL_VIOLATION");
        case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED   : str = _T("ERROR_ATOMIC_LOCKS_NOT_SUPPORTED");
        case ERROR_INVALID_SEGMENT_NUMBER       : str = _T("ERROR_INVALID_SEGMENT_NUMBER");
        case ERROR_INVALID_ORDINAL              : str = _T("ERROR_INVALID_ORDINAL");
        case ERROR_ALREADY_EXISTS               : str = _T("ERROR_ALREADY_EXISTS");
        case ERROR_INVALID_FLAG_NUMBER          : str = _T("ERROR_INVALID_FLAG_NUMBER");
        case ERROR_SEM_NOT_FOUND                : str = _T("ERROR_SEM_NOT_FOUND");
        case ERROR_INVALID_STARTING_CODESEG     : str = _T("ERROR_INVALID_STARTING_CODESEG");
        case ERROR_INVALID_STACKSEG             : str = _T("ERROR_INVALID_STACKSEG");
        case ERROR_INVALID_MODULETYPE           : str = _T("ERROR_INVALID_MODULETYPE");
        case ERROR_INVALID_EXE_SIGNATURE        : str = _T("ERROR_INVALID_EXE_SIGNATURE");
        case ERROR_EXE_MARKED_INVALID           : str = _T("ERROR_EXE_MARKED_INVALID");
        case ERROR_BAD_EXE_FORMAT               : str = _T("ERROR_BAD_EXE_FORMAT");
        case ERROR_ITERATED_DATA_EXCEEDS_64k    : str = _T("ERROR_ITERATED_DATA_EXCEEDS_64k");
        case ERROR_INVALID_MINALLOCSIZE         : str = _T("ERROR_INVALID_MINALLOCSIZE");
        case ERROR_DYNLINK_FROM_INVALID_RING    : str = _T("ERROR_DYNLINK_FROM_INVALID_RING");
        case ERROR_IOPL_NOT_ENABLED             : str = _T("ERROR_IOPL_NOT_ENABLED");
        case ERROR_INVALID_SEGDPL               : str = _T("ERROR_INVALID_SEGDPL");
        case ERROR_AUTODATASEG_EXCEEDS_64k      : str = _T("ERROR_AUTODATASEG_EXCEEDS_64k");
        case ERROR_RING2SEG_MUST_BE_MOVABLE     : str = _T("ERROR_RING2SEG_MUST_BE_MOVABLE");
        case ERROR_RELOC_CHAIN_XEEDS_SEGLIM     : str = _T("ERROR_RELOC_CHAIN_XEEDS_SEGLIM");
        case ERROR_INFLOOP_IN_RELOC_CHAIN       : str = _T("ERROR_INFLOOP_IN_RELOC_CHAIN");
        case ERROR_ENVVAR_NOT_FOUND             : str = _T("ERROR_ENVVAR_NOT_FOUND");
        case ERROR_NO_SIGNAL_SENT               : str = _T("ERROR_NO_SIGNAL_SENT");
        case ERROR_FILENAME_EXCED_RANGE         : str = _T("ERROR_FILENAME_EXCED_RANGE");
        case ERROR_RING2_STACK_IN_USE           : str = _T("ERROR_RING2_STACK_IN_USE");
        case ERROR_META_EXPANSION_TOO_LONG      : str = _T("ERROR_META_EXPANSION_TOO_LONG");
        case ERROR_INVALID_SIGNAL_NUMBER        : str = _T("ERROR_INVALID_SIGNAL_NUMBER");
        case ERROR_THREAD_1_INACTIVE            : str = _T("ERROR_THREAD_1_INACTIVE");
        case ERROR_LOCKED                       : str = _T("ERROR_LOCKED");
        case ERROR_TOO_MANY_MODULES             : str = _T("ERROR_TOO_MANY_MODULES");
        case ERROR_NESTING_NOT_ALLOWED          : str = _T("ERROR_NESTING_NOT_ALLOWED");
        case ERROR_BAD_PIPE                     : str = _T("ERROR_BAD_PIPE");
        case ERROR_PIPE_BUSY                    : str = _T("ERROR_PIPE_BUSY");
        case ERROR_NO_DATA                      : str = _T("ERROR_NO_DATA");
        case ERROR_PIPE_NOT_CONNECTED           : str = _T("ERROR_PIPE_NOT_CONNECTED");
        case ERROR_MORE_DATA                    : str = _T("ERROR_MORE_DATA");
        case ERROR_VC_DISCONNECTED              : str = _T("ERROR_VC_DISCONNECTED");
        case ERROR_INVALID_EA_NAME              : str = _T("ERROR_INVALID_EA_NAME");
        case ERROR_EA_LIST_INCONSISTENT         : str = _T("ERROR_EA_LIST_INCONSISTENT");
        case ERROR_NO_MORE_ITEMS                : str = _T("ERROR_NO_MORE_ITEMS");
        case ERROR_CANNOT_COPY                  : str = _T("ERROR_CANNOT_COPY");
        case ERROR_DIRECTORY                    : str = _T("ERROR_DIRECTORY");
        case ERROR_EAS_DIDNT_FIT                : str = _T("ERROR_EAS_DIDNT_FIT");
        case ERROR_EA_FILE_CORRUPT              : str = _T("ERROR_EA_FILE_CORRUPT");
        case ERROR_EA_TABLE_FULL                : str = _T("ERROR_EA_TABLE_FULL");
        case ERROR_INVALID_EA_HANDLE            : str = _T("ERROR_INVALID_EA_HANDLE");
        case ERROR_EAS_NOT_SUPPORTED            : str = _T("ERROR_EAS_NOT_SUPPORTED");
        case ERROR_NOT_OWNER                    : str = _T("ERROR_NOT_OWNER");
        case ERROR_TOO_MANY_POSTS               : str = _T("ERROR_TOO_MANY_POSTS");
        case ERROR_PARTIAL_COPY                 : str = _T("ERROR_PARTIAL_COPY");
        case ERROR_MR_MID_NOT_FOUND             : str = _T("ERROR_MR_MID_NOT_FOUND");
        case ERROR_INVALID_ADDRESS              : str = _T("ERROR_INVALID_ADDRESS");
        case ERROR_ARITHMETIC_OVERFLOW          : str = _T("ERROR_ARITHMETIC_OVERFLOW");
        case ERROR_PIPE_CONNECTED               : str = _T("ERROR_PIPE_CONNECTED");
        case ERROR_PIPE_LISTENING               : str = _T("ERROR_PIPE_LISTENING");
        case ERROR_EA_ACCESS_DENIED             : str = _T("ERROR_EA_ACCESS_DENIED");
        case ERROR_OPERATION_ABORTED            : str = _T("ERROR_OPERATION_ABORTED");
        case ERROR_IO_INCOMPLETE                : str = _T("ERROR_IO_INCOMPLETE");
        case ERROR_IO_PENDING                   : str = _T("ERROR_IO_PENDING");
        case ERROR_NOACCESS                     : str = _T("ERROR_NOACCESS");
        case ERROR_SWAPERROR                    : str = _T("ERROR_SWAPERROR");
        case ERROR_STACK_OVERFLOW               : str = _T("ERROR_STACK_OVERFLOW");
        case ERROR_INVALID_MESSAGE              : str = _T("ERROR_INVALID_MESSAGE");
        case ERROR_CAN_NOT_COMPLETE             : str = _T("ERROR_CAN_NOT_COMPLETE");
        case ERROR_INVALID_FLAGS                : str = _T("ERROR_INVALID_FLAGS");
        case ERROR_UNRECOGNIZED_VOLUME          : str = _T("ERROR_UNRECOGNIZED_VOLUME");
        case ERROR_FILE_INVALID                 : str = _T("ERROR_FILE_INVALID");
        case ERROR_FULLSCREEN_MODE              : str = _T("ERROR_FULLSCREEN_MODE");
        case ERROR_NO_TOKEN                     : str = _T("ERROR_NO_TOKEN");
        case ERROR_BADDB                        : str = _T("ERROR_BADDB");
        case ERROR_BADKEY                       : str = _T("ERROR_BADKEY");
        case ERROR_CANTOPEN                     : str = _T("ERROR_CANTOPEN");
        case ERROR_CANTREAD                     : str = _T("ERROR_CANTREAD");
        case ERROR_CANTWRITE                    : str = _T("ERROR_CANTWRITE");
        case ERROR_REGISTRY_RECOVERED           : str = _T("ERROR_REGISTRY_RECOVERED");
        case ERROR_REGISTRY_CORRUPT             : str = _T("ERROR_REGISTRY_CORRUPT");
        case ERROR_REGISTRY_IO_FAILED           : str = _T("ERROR_REGISTRY_IO_FAILED");
        case ERROR_NOT_REGISTRY_FILE            : str = _T("ERROR_NOT_REGISTRY_FILE");
        case ERROR_KEY_DELETED                  : str = _T("ERROR_KEY_DELETED");
        case ERROR_NO_LOG_SPACE                 : str = _T("ERROR_NO_LOG_SPACE");
        case ERROR_KEY_HAS_CHILDREN             : str = _T("ERROR_KEY_HAS_CHILDREN");
        case ERROR_CHILD_MUST_BE_VOLATILE       : str = _T("ERROR_CHILD_MUST_BE_VOLATILE");
        case ERROR_NOTIFY_ENUM_DIR              : str = _T("ERROR_NOTIFY_ENUM_DIR");
        case ERROR_DEPENDENT_SERVICES_RUNNING   : str = _T("ERROR_DEPENDENT_SERVICES_RUNNING");
        case ERROR_INVALID_SERVICE_CONTROL      : str = _T("ERROR_INVALID_SERVICE_CONTROL");
        case ERROR_SERVICE_REQUEST_TIMEOUT      : str = _T("ERROR_SERVICE_REQUEST_TIMEOUT");
        case ERROR_SERVICE_NO_THREAD            : str = _T("ERROR_SERVICE_NO_THREAD");
        case ERROR_SERVICE_DATABASE_LOCKED      : str = _T("ERROR_SERVICE_DATABASE_LOCKED");
        case ERROR_SERVICE_ALREADY_RUNNING      : str = _T("ERROR_SERVICE_ALREADY_RUNNING");
        case ERROR_INVALID_SERVICE_ACCOUNT      : str = _T("ERROR_INVALID_SERVICE_ACCOUNT");
        case ERROR_SERVICE_DISABLED             : str = _T("ERROR_SERVICE_DISABLED");
        case ERROR_CIRCULAR_DEPENDENCY          : str = _T("ERROR_CIRCULAR_DEPENDENCY");
        case ERROR_SERVICE_DOES_NOT_EXIST       : str = _T("ERROR_SERVICE_DOES_NOT_EXIST");
        case ERROR_SERVICE_CANNOT_ACCEPT_CTRL   : str = _T("ERROR_SERVICE_CANNOT_ACCEPT_CTRL");
        case ERROR_SERVICE_NOT_ACTIVE           : str = _T("ERROR_SERVICE_NOT_ACTIVE");
        case ERROR_EXCEPTION_IN_SERVICE         : str = _T("ERROR_EXCEPTION_IN_SERVICE");
        case ERROR_DATABASE_DOES_NOT_EXIST      : str = _T("ERROR_DATABASE_DOES_NOT_EXIST");
        case ERROR_SERVICE_SPECIFIC_ERROR       : str = _T("ERROR_SERVICE_SPECIFIC_ERROR");
        case ERROR_PROCESS_ABORTED              : str = _T("ERROR_PROCESS_ABORTED");
        case ERROR_SERVICE_DEPENDENCY_FAIL      : str = _T("ERROR_SERVICE_DEPENDENCY_FAIL");
        case ERROR_SERVICE_LOGON_FAILED         : str = _T("ERROR_SERVICE_LOGON_FAILED");
        case ERROR_SERVICE_START_HANG           : str = _T("ERROR_SERVICE_START_HANG");
        case ERROR_INVALID_SERVICE_LOCK         : str = _T("ERROR_INVALID_SERVICE_LOCK");
        case ERROR_SERVICE_MARKED_FOR_DELETE    : str = _T("ERROR_SERVICE_MARKED_FOR_DELETE");
        case ERROR_SERVICE_EXISTS               : str = _T("ERROR_SERVICE_EXISTS");
        case ERROR_ALREADY_RUNNING_LKG          : str = _T("ERROR_ALREADY_RUNNING_LKG");
        case ERROR_SERVICE_DEPENDENCY_DELETED   : str = _T("ERROR_SERVICE_DEPENDENCY_DELETED");
        case ERROR_BOOT_ALREADY_ACCEPTED        : str = _T("ERROR_BOOT_ALREADY_ACCEPTED");
        case ERROR_SERVICE_NEVER_STARTED        : str = _T("ERROR_SERVICE_NEVER_STARTED");
        case ERROR_DUPLICATE_SERVICE_NAME       : str = _T("ERROR_DUPLICATE_SERVICE_NAME");
        case ERROR_END_OF_MEDIA                 : str = _T("ERROR_END_OF_MEDIA");
        case ERROR_FILEMARK_DETECTED            : str = _T("ERROR_FILEMARK_DETECTED");
        case ERROR_BEGINNING_OF_MEDIA           : str = _T("ERROR_BEGINNING_OF_MEDIA");
        case ERROR_SETMARK_DETECTED             : str = _T("ERROR_SETMARK_DETECTED");
        case ERROR_NO_DATA_DETECTED             : str = _T("ERROR_NO_DATA_DETECTED");
        case ERROR_PARTITION_FAILURE            : str = _T("ERROR_PARTITION_FAILURE");
        case ERROR_INVALID_BLOCK_LENGTH         : str = _T("ERROR_INVALID_BLOCK_LENGTH");
        case ERROR_DEVICE_NOT_PARTITIONED       : str = _T("ERROR_DEVICE_NOT_PARTITIONED");
        case ERROR_UNABLE_TO_LOCK_MEDIA         : str = _T("ERROR_UNABLE_TO_LOCK_MEDIA");
        case ERROR_UNABLE_TO_UNLOAD_MEDIA       : str = _T("ERROR_UNABLE_TO_UNLOAD_MEDIA");
        case ERROR_MEDIA_CHANGED                : str = _T("ERROR_MEDIA_CHANGED");
        case ERROR_BUS_RESET                    : str = _T("ERROR_BUS_RESET");
        case ERROR_NO_MEDIA_IN_DRIVE            : str = _T("ERROR_NO_MEDIA_IN_DRIVE");
        case ERROR_NO_UNICODE_TRANSLATION       : str = _T("ERROR_NO_UNICODE_TRANSLATION");
        case ERROR_DLL_INIT_FAILED              : str = _T("ERROR_DLL_INIT_FAILED");
        case ERROR_SHUTDOWN_IN_PROGRESS         : str = _T("ERROR_SHUTDOWN_IN_PROGRESS");
        case ERROR_NO_SHUTDOWN_IN_PROGRESS      : str = _T("ERROR_NO_SHUTDOWN_IN_PROGRESS");
        case ERROR_IO_DEVICE                    : str = _T("ERROR_IO_DEVICE");
        case ERROR_SERIAL_NO_DEVICE             : str = _T("ERROR_SERIAL_NO_DEVICE");
        case ERROR_IRQ_BUSY                     : str = _T("ERROR_IRQ_BUSY");
        case ERROR_MORE_WRITES                  : str = _T("ERROR_MORE_WRITES");
        case ERROR_COUNTER_TIMEOUT              : str = _T("ERROR_COUNTER_TIMEOUT");
        case ERROR_FLOPPY_ID_MARK_NOT_FOUND     : str = _T("ERROR_FLOPPY_ID_MARK_NOT_FOUND");
        case ERROR_FLOPPY_WRONG_CYLINDER        : str = _T("ERROR_FLOPPY_WRONG_CYLINDER");
        case ERROR_FLOPPY_UNKNOWN_ERROR         : str = _T("ERROR_FLOPPY_UNKNOWN_ERROR");
        case ERROR_FLOPPY_BAD_REGISTERS         : str = _T("ERROR_FLOPPY_BAD_REGISTERS");
        case ERROR_DISK_RECALIBRATE_FAILED      : str = _T("ERROR_DISK_RECALIBRATE_FAILED");
        case ERROR_DISK_OPERATION_FAILED        : str = _T("ERROR_DISK_OPERATION_FAILED");
        case ERROR_DISK_RESET_FAILED            : str = _T("ERROR_DISK_RESET_FAILED");
        case ERROR_EOM_OVERFLOW                 : str = _T("ERROR_EOM_OVERFLOW");
        case ERROR_NOT_ENOUGH_SERVER_MEMORY     : str = _T("ERROR_NOT_ENOUGH_SERVER_MEMORY");
        case ERROR_POSSIBLE_DEADLOCK            : str = _T("ERROR_POSSIBLE_DEADLOCK");
        case ERROR_MAPPED_ALIGNMENT             : str = _T("ERROR_MAPPED_ALIGNMENT");
        case ERROR_BAD_USERNAME                 : str = _T("ERROR_BAD_USERNAME");
        case ERROR_NOT_CONNECTED                : str = _T("ERROR_NOT_CONNECTED");
        case ERROR_OPEN_FILES                   : str = _T("ERROR_OPEN_FILES");
        case ERROR_ACTIVE_CONNECTIONS           : str = _T("ERROR_ACTIVE_CONNECTIONS");
        case ERROR_DEVICE_IN_USE                : str = _T("ERROR_DEVICE_IN_USE");
        case ERROR_BAD_DEVICE                   : str = _T("ERROR_BAD_DEVICE");
        case ERROR_CONNECTION_UNAVAIL           : str = _T("ERROR_CONNECTION_UNAVAIL");
        case ERROR_DEVICE_ALREADY_REMEMBERED    : str = _T("ERROR_DEVICE_ALREADY_REMEMBERED");
        case ERROR_NO_NET_OR_BAD_PATH           : str = _T("ERROR_NO_NET_OR_BAD_PATH");
        case ERROR_BAD_PROVIDER                 : str = _T("ERROR_BAD_PROVIDER");
        case ERROR_CANNOT_OPEN_PROFILE          : str = _T("ERROR_CANNOT_OPEN_PROFILE");
        case ERROR_BAD_PROFILE                  : str = _T("ERROR_BAD_PROFILE");
        case ERROR_NOT_CONTAINER                : str = _T("ERROR_NOT_CONTAINER");
        case ERROR_EXTENDED_ERROR               : str = _T("ERROR_EXTENDED_ERROR");
        case ERROR_INVALID_GROUPNAME            : str = _T("ERROR_INVALID_GROUPNAME");
        case ERROR_INVALID_COMPUTERNAME         : str = _T("ERROR_INVALID_COMPUTERNAME");
        case ERROR_INVALID_EVENTNAME            : str = _T("ERROR_INVALID_EVENTNAME");
        case ERROR_INVALID_DOMAINNAME           : str = _T("ERROR_INVALID_DOMAINNAME");
        case ERROR_INVALID_SERVICENAME          : str = _T("ERROR_INVALID_SERVICENAME");
        case ERROR_INVALID_NETNAME              : str = _T("ERROR_INVALID_NETNAME");
        case ERROR_INVALID_SHARENAME            : str = _T("ERROR_INVALID_SHARENAME");
        case ERROR_INVALID_PASSWORDNAME         : str = _T("ERROR_INVALID_PASSWORDNAME");
        case ERROR_INVALID_MESSAGENAME          : str = _T("ERROR_INVALID_MESSAGENAME");
        case ERROR_INVALID_MESSAGEDEST          : str = _T("ERROR_INVALID_MESSAGEDEST");
        case ERROR_DUP_DOMAINNAME               : str = _T("ERROR_DUP_DOMAINNAME");
        case ERROR_NO_NETWORK                   : str = _T("ERROR_NO_NETWORK");
        case ERROR_CANCELLED                    : str = _T("ERROR_CANCELLED");
        case ERROR_USER_MAPPED_FILE             : str = _T("ERROR_USER_MAPPED_FILE");
        case ERROR_CONNECTION_REFUSED           : str = _T("ERROR_CONNECTION_REFUSED");
        case ERROR_GRACEFUL_DISCONNECT          : str = _T("ERROR_GRACEFUL_DISCONNECT");
        case ERROR_ADDRESS_ALREADY_ASSOCIATED   : str = _T("ERROR_ADDRESS_ALREADY_ASSOCIATED");
        case ERROR_ADDRESS_NOT_ASSOCIATED       : str = _T("ERROR_ADDRESS_NOT_ASSOCIATED");
        case ERROR_CONNECTION_INVALID           : str = _T("ERROR_CONNECTION_INVALID");
        case ERROR_CONNECTION_ACTIVE            : str = _T("ERROR_CONNECTION_ACTIVE");
        case ERROR_NETWORK_UNREACHABLE          : str = _T("ERROR_NETWORK_UNREACHABLE");
        case ERROR_HOST_UNREACHABLE             : str = _T("ERROR_HOST_UNREACHABLE");
        case ERROR_PROTOCOL_UNREACHABLE         : str = _T("ERROR_PROTOCOL_UNREACHABLE");
        case ERROR_PORT_UNREACHABLE             : str = _T("ERROR_PORT_UNREACHABLE");
        case ERROR_REQUEST_ABORTED              : str = _T("ERROR_REQUEST_ABORTED");
        case ERROR_CONNECTION_ABORTED           : str = _T("ERROR_CONNECTION_ABORTED");
        case ERROR_RETRY                        : str = _T("ERROR_RETRY");
        case ERROR_CONNECTION_COUNT_LIMIT       : str = _T("ERROR_CONNECTION_COUNT_LIMIT");
        case ERROR_LOGIN_TIME_RESTRICTION       : str = _T("ERROR_LOGIN_TIME_RESTRICTION");
        case ERROR_LOGIN_WKSTA_RESTRICTION      : str = _T("ERROR_LOGIN_WKSTA_RESTRICTION");
        case ERROR_INCORRECT_ADDRESS            : str = _T("ERROR_INCORRECT_ADDRESS");
        case ERROR_ALREADY_REGISTERED           : str = _T("ERROR_ALREADY_REGISTERED");
        case ERROR_SERVICE_NOT_FOUND            : str = _T("ERROR_SERVICE_NOT_FOUND");
        case ERROR_NOT_ALL_ASSIGNED             : str = _T("ERROR_NOT_ALL_ASSIGNED");
        case ERROR_SOME_NOT_MAPPED              : str = _T("ERROR_SOME_NOT_MAPPED");
        case ERROR_NO_QUOTAS_FOR_ACCOUNT        : str = _T("ERROR_NO_QUOTAS_FOR_ACCOUNT");
        case ERROR_LOCAL_USER_SESSION_KEY       : str = _T("ERROR_LOCAL_USER_SESSION_KEY");
        case ERROR_NULL_LM_PASSWORD             : str = _T("ERROR_NULL_LM_PASSWORD");
        case ERROR_UNKNOWN_REVISION             : str = _T("ERROR_UNKNOWN_REVISION");
        case ERROR_REVISION_MISMATCH            : str = _T("ERROR_REVISION_MISMATCH");
        case ERROR_INVALID_OWNER                : str = _T("ERROR_INVALID_OWNER");
        case ERROR_INVALID_PRIMARY_GROUP        : str = _T("ERROR_INVALID_PRIMARY_GROUP");
        case ERROR_NO_IMPERSONATION_TOKEN       : str = _T("ERROR_NO_IMPERSONATION_TOKEN");
        case ERROR_CANT_DISABLE_MANDATORY       : str = _T("ERROR_CANT_DISABLE_MANDATORY");
        case ERROR_NO_LOGON_SERVERS             : str = _T("ERROR_NO_LOGON_SERVERS");
        case ERROR_NO_SUCH_LOGON_SESSION        : str = _T("ERROR_NO_SUCH_LOGON_SESSION");
        case ERROR_NO_SUCH_PRIVILEGE            : str = _T("ERROR_NO_SUCH_PRIVILEGE");
        case ERROR_PRIVILEGE_NOT_HELD           : str = _T("ERROR_PRIVILEGE_NOT_HELD");
        case ERROR_INVALID_ACCOUNT_NAME         : str = _T("ERROR_INVALID_ACCOUNT_NAME");
        case ERROR_USER_EXISTS                  : str = _T("ERROR_USER_EXISTS");
        case ERROR_NO_SUCH_USER                 : str = _T("ERROR_NO_SUCH_USER");
        case ERROR_GROUP_EXISTS                 : str = _T("ERROR_GROUP_EXISTS");
        case ERROR_NO_SUCH_GROUP                : str = _T("ERROR_NO_SUCH_GROUP");
        case ERROR_MEMBER_IN_GROUP              : str = _T("ERROR_MEMBER_IN_GROUP");
        case ERROR_MEMBER_NOT_IN_GROUP          : str = _T("ERROR_MEMBER_NOT_IN_GROUP");
        case ERROR_LAST_ADMIN                   : str = _T("ERROR_LAST_ADMIN");
        case ERROR_WRONG_PASSWORD               : str = _T("ERROR_WRONG_PASSWORD");
        case ERROR_ILL_FORMED_PASSWORD          : str = _T("ERROR_ILL_FORMED_PASSWORD");
        case ERROR_PASSWORD_RESTRICTION         : str = _T("ERROR_PASSWORD_RESTRICTION");
        case ERROR_LOGON_FAILURE                : str = _T("ERROR_LOGON_FAILURE");
        case ERROR_ACCOUNT_RESTRICTION          : str = _T("ERROR_ACCOUNT_RESTRICTION");
        case ERROR_INVALID_LOGON_HOURS          : str = _T("ERROR_INVALID_LOGON_HOURS");
        case ERROR_INVALID_WORKSTATION          : str = _T("ERROR_INVALID_WORKSTATION");
        case ERROR_PASSWORD_EXPIRED             : str = _T("ERROR_PASSWORD_EXPIRED");
        case ERROR_ACCOUNT_DISABLED             : str = _T("ERROR_ACCOUNT_DISABLED");
        case ERROR_NONE_MAPPED                  : str = _T("ERROR_NONE_MAPPED");
        case ERROR_TOO_MANY_LUIDS_REQUESTED     : str = _T("ERROR_TOO_MANY_LUIDS_REQUESTED");
        case ERROR_LUIDS_EXHAUSTED              : str = _T("ERROR_LUIDS_EXHAUSTED");
        case ERROR_INVALID_SUB_AUTHORITY        : str = _T("ERROR_INVALID_SUB_AUTHORITY");
        case ERROR_INVALID_ACL                  : str = _T("ERROR_INVALID_ACL");
        case ERROR_INVALID_SID                  : str = _T("ERROR_INVALID_SID");
        case ERROR_INVALID_SECURITY_DESCR       : str = _T("ERROR_INVALID_SECURITY_DESCR");
        case ERROR_BAD_INHERITANCE_ACL          : str = _T("ERROR_BAD_INHERITANCE_ACL");
        case ERROR_SERVER_DISABLED              : str = _T("ERROR_SERVER_DISABLED");
        case ERROR_SERVER_NOT_DISABLED          : str = _T("ERROR_SERVER_NOT_DISABLED");
        case ERROR_INVALID_ID_AUTHORITY         : str = _T("ERROR_INVALID_ID_AUTHORITY");
        case ERROR_ALLOTTED_SPACE_EXCEEDED      : str = _T("ERROR_ALLOTTED_SPACE_EXCEEDED");
        case ERROR_INVALID_GROUP_ATTRIBUTES     : str = _T("ERROR_INVALID_GROUP_ATTRIBUTES");
        case ERROR_BAD_IMPERSONATION_LEVEL      : str = _T("ERROR_BAD_IMPERSONATION_LEVEL");
        case ERROR_CANT_OPEN_ANONYMOUS          : str = _T("ERROR_CANT_OPEN_ANONYMOUS");
        case ERROR_BAD_VALIDATION_CLASS         : str = _T("ERROR_BAD_VALIDATION_CLASS");
        case ERROR_BAD_TOKEN_TYPE               : str = _T("ERROR_BAD_TOKEN_TYPE");
        case ERROR_NO_SECURITY_ON_OBJECT        : str = _T("ERROR_NO_SECURITY_ON_OBJECT");
        case ERROR_CANT_ACCESS_DOMAIN_INFO      : str = _T("ERROR_CANT_ACCESS_DOMAIN_INFO");
        case ERROR_INVALID_SERVER_STATE         : str = _T("ERROR_INVALID_SERVER_STATE");
        case ERROR_INVALID_DOMAIN_STATE         : str = _T("ERROR_INVALID_DOMAIN_STATE");
        case ERROR_INVALID_DOMAIN_ROLE          : str = _T("ERROR_INVALID_DOMAIN_ROLE");
        case ERROR_NO_SUCH_DOMAIN               : str = _T("ERROR_NO_SUCH_DOMAIN");
        case ERROR_DOMAIN_EXISTS                : str = _T("ERROR_DOMAIN_EXISTS");
        case ERROR_DOMAIN_LIMIT_EXCEEDED        : str = _T("ERROR_DOMAIN_LIMIT_EXCEEDED");
        case ERROR_INTERNAL_DB_CORRUPTION       : str = _T("ERROR_INTERNAL_DB_CORRUPTION");
        case ERROR_INTERNAL_ERROR               : str = _T("ERROR_INTERNAL_ERROR");
        case ERROR_GENERIC_NOT_MAPPED           : str = _T("ERROR_GENERIC_NOT_MAPPED");
        case ERROR_BAD_DESCRIPTOR_FORMAT        : str = _T("ERROR_BAD_DESCRIPTOR_FORMAT");
        case ERROR_NOT_LOGON_PROCESS            : str = _T("ERROR_NOT_LOGON_PROCESS");
        case ERROR_LOGON_SESSION_EXISTS         : str = _T("ERROR_LOGON_SESSION_EXISTS");
        case ERROR_NO_SUCH_PACKAGE              : str = _T("ERROR_NO_SUCH_PACKAGE");
        case ERROR_BAD_LOGON_SESSION_STATE      : str = _T("ERROR_BAD_LOGON_SESSION_STATE");
        case ERROR_LOGON_SESSION_COLLISION      : str = _T("ERROR_LOGON_SESSION_COLLISION");
        case ERROR_INVALID_LOGON_TYPE           : str = _T("ERROR_INVALID_LOGON_TYPE");
        case ERROR_CANNOT_IMPERSONATE           : str = _T("ERROR_CANNOT_IMPERSONATE");
        case ERROR_RXACT_INVALID_STATE          : str = _T("ERROR_RXACT_INVALID_STATE");
        case ERROR_RXACT_COMMIT_FAILURE         : str = _T("ERROR_RXACT_COMMIT_FAILURE");
        case ERROR_SPECIAL_ACCOUNT              : str = _T("ERROR_SPECIAL_ACCOUNT");
        case ERROR_SPECIAL_GROUP                : str = _T("ERROR_SPECIAL_GROUP");
        case ERROR_SPECIAL_USER                 : str = _T("ERROR_SPECIAL_USER");
        case ERROR_MEMBERS_PRIMARY_GROUP        : str = _T("ERROR_MEMBERS_PRIMARY_GROUP");
        case ERROR_TOKEN_ALREADY_IN_USE         : str = _T("ERROR_TOKEN_ALREADY_IN_USE");
        case ERROR_NO_SUCH_ALIAS                : str = _T("ERROR_NO_SUCH_ALIAS");
        case ERROR_MEMBER_NOT_IN_ALIAS          : str = _T("ERROR_MEMBER_NOT_IN_ALIAS");
        case ERROR_MEMBER_IN_ALIAS              : str = _T("ERROR_MEMBER_IN_ALIAS");
        case ERROR_ALIAS_EXISTS                 : str = _T("ERROR_ALIAS_EXISTS");
        case ERROR_LOGON_NOT_GRANTED            : str = _T("ERROR_LOGON_NOT_GRANTED");
        case ERROR_TOO_MANY_SECRETS             : str = _T("ERROR_TOO_MANY_SECRETS");
        case ERROR_SECRET_TOO_LONG              : str = _T("ERROR_SECRET_TOO_LONG");
        case ERROR_INTERNAL_DB_ERROR            : str = _T("ERROR_INTERNAL_DB_ERROR");
        case ERROR_TOO_MANY_CONTEXT_IDS         : str = _T("ERROR_TOO_MANY_CONTEXT_IDS");
        case ERROR_LOGON_TYPE_NOT_GRANTED       : str = _T("ERROR_LOGON_TYPE_NOT_GRANTED");
        case ERROR_NO_SUCH_MEMBER               : str = _T("ERROR_NO_SUCH_MEMBER");
        case ERROR_INVALID_MEMBER               : str = _T("ERROR_INVALID_MEMBER");
        case ERROR_TOO_MANY_SIDS                : str = _T("ERROR_TOO_MANY_SIDS");
        case ERROR_NO_INHERITANCE               : str = _T("ERROR_NO_INHERITANCE");
        case ERROR_FILE_CORRUPT                 : str = _T("ERROR_FILE_CORRUPT");
        case ERROR_DISK_CORRUPT                 : str = _T("ERROR_DISK_CORRUPT");
        case ERROR_NO_USER_SESSION_KEY          : str = _T("ERROR_NO_USER_SESSION_KEY");
        case ERROR_INVALID_WINDOW_HANDLE        : str = _T("ERROR_INVALID_WINDOW_HANDLE");
        case ERROR_INVALID_MENU_HANDLE          : str = _T("ERROR_INVALID_MENU_HANDLE");
        case ERROR_INVALID_CURSOR_HANDLE        : str = _T("ERROR_INVALID_CURSOR_HANDLE");
        case ERROR_INVALID_ACCEL_HANDLE         : str = _T("ERROR_INVALID_ACCEL_HANDLE");
        case ERROR_INVALID_HOOK_HANDLE          : str = _T("ERROR_INVALID_HOOK_HANDLE");
        case ERROR_INVALID_DWP_HANDLE           : str = _T("ERROR_INVALID_DWP_HANDLE");
        case ERROR_TLW_WITH_WSCHILD             : str = _T("ERROR_TLW_WITH_WSCHILD");
        case ERROR_CANNOT_FIND_WND_CLASS        : str = _T("ERROR_CANNOT_FIND_WND_CLASS");
        case ERROR_WINDOW_OF_OTHER_THREAD       : str = _T("ERROR_WINDOW_OF_OTHER_THREAD");
        case ERROR_HOTKEY_ALREADY_REGISTERED    : str = _T("ERROR_HOTKEY_ALREADY_REGISTERED");
        case ERROR_CLASS_ALREADY_EXISTS         : str = _T("ERROR_CLASS_ALREADY_EXISTS");
        case ERROR_CLASS_DOES_NOT_EXIST         : str = _T("ERROR_CLASS_DOES_NOT_EXIST");
        case ERROR_CLASS_HAS_WINDOWS            : str = _T("ERROR_CLASS_HAS_WINDOWS");
        case ERROR_INVALID_INDEX                : str = _T("ERROR_INVALID_INDEX");
        case ERROR_INVALID_ICON_HANDLE          : str = _T("ERROR_INVALID_ICON_HANDLE");
        case ERROR_PRIVATE_DIALOG_INDEX         : str = _T("ERROR_PRIVATE_DIALOG_INDEX");
        case ERROR_LISTBOX_ID_NOT_FOUND         : str = _T("ERROR_LISTBOX_ID_NOT_FOUND");
        case ERROR_NO_WILDCARD_CHARACTERS       : str = _T("ERROR_NO_WILDCARD_CHARACTERS");
        case ERROR_CLIPBOARD_NOT_OPEN           : str = _T("ERROR_CLIPBOARD_NOT_OPEN");
        case ERROR_HOTKEY_NOT_REGISTERED        : str = _T("ERROR_HOTKEY_NOT_REGISTERED");
        case ERROR_WINDOW_NOT_DIALOG            : str = _T("ERROR_WINDOW_NOT_DIALOG");
        case ERROR_CONTROL_ID_NOT_FOUND         : str = _T("ERROR_CONTROL_ID_NOT_FOUND");
        case ERROR_INVALID_COMBOBOX_MESSAGE     : str = _T("ERROR_INVALID_COMBOBOX_MESSAGE");
        case ERROR_WINDOW_NOT_COMBOBOX          : str = _T("ERROR_WINDOW_NOT_COMBOBOX");
        case ERROR_INVALID_EDIT_HEIGHT          : str = _T("ERROR_INVALID_EDIT_HEIGHT");
        case ERROR_DC_NOT_FOUND                 : str = _T("ERROR_DC_NOT_FOUND");
        case ERROR_INVALID_HOOK_FILTER          : str = _T("ERROR_INVALID_HOOK_FILTER");
        case ERROR_INVALID_FILTER_PROC          : str = _T("ERROR_INVALID_FILTER_PROC");
        case ERROR_HOOK_NEEDS_HMOD              : str = _T("ERROR_HOOK_NEEDS_HMOD");
        case ERROR_GLOBAL_ONLY_HOOK             : str = _T("ERROR_GLOBAL_ONLY_HOOK");
        case ERROR_JOURNAL_HOOK_SET             : str = _T("ERROR_JOURNAL_HOOK_SET");
        case ERROR_HOOK_NOT_INSTALLED           : str = _T("ERROR_HOOK_NOT_INSTALLED");
        case ERROR_INVALID_LB_MESSAGE           : str = _T("ERROR_INVALID_LB_MESSAGE");
        case ERROR_SETCOUNT_ON_BAD_LB           : str = _T("ERROR_SETCOUNT_ON_BAD_LB");
        case ERROR_LB_WITHOUT_TABSTOPS          : str = _T("ERROR_LB_WITHOUT_TABSTOPS");
        case ERROR_CHILD_WINDOW_MENU            : str = _T("ERROR_CHILD_WINDOW_MENU");
        case ERROR_NO_SYSTEM_MENU               : str = _T("ERROR_NO_SYSTEM_MENU");
        case ERROR_INVALID_MSGBOX_STYLE         : str = _T("ERROR_INVALID_MSGBOX_STYLE");
        case ERROR_INVALID_SPI_VALUE            : str = _T("ERROR_INVALID_SPI_VALUE");
        case ERROR_SCREEN_ALREADY_LOCKED        : str = _T("ERROR_SCREEN_ALREADY_LOCKED");
        case ERROR_HWNDS_HAVE_DIFF_PARENT       : str = _T("ERROR_HWNDS_HAVE_DIFF_PARENT");
        case ERROR_NOT_CHILD_WINDOW             : str = _T("ERROR_NOT_CHILD_WINDOW");
        case ERROR_INVALID_GW_COMMAND           : str = _T("ERROR_INVALID_GW_COMMAND");
        case ERROR_INVALID_THREAD_ID            : str = _T("ERROR_INVALID_THREAD_ID");
        case ERROR_NON_MDICHILD_WINDOW          : str = _T("ERROR_NON_MDICHILD_WINDOW");
        case ERROR_POPUP_ALREADY_ACTIVE         : str = _T("ERROR_POPUP_ALREADY_ACTIVE");
        case ERROR_NO_SCROLLBARS                : str = _T("ERROR_NO_SCROLLBARS");
        case ERROR_INVALID_SCROLLBAR_RANGE      : str = _T("ERROR_INVALID_SCROLLBAR_RANGE");
        case ERROR_INVALID_SHOWWIN_COMMAND      : str = _T("ERROR_INVALID_SHOWWIN_COMMAND");
        case ERROR_EVENTLOG_FILE_CORRUPT        : str = _T("ERROR_EVENTLOG_FILE_CORRUPT");
        case ERROR_EVENTLOG_CANT_START          : str = _T("ERROR_EVENTLOG_CANT_START");
        case ERROR_LOG_FILE_FULL                : str = _T("ERROR_LOG_FILE_FULL");
        case ERROR_EVENTLOG_FILE_CHANGED        : str = _T("ERROR_EVENTLOG_FILE_CHANGED");
        case ERROR_INVALID_USER_BUFFER          : str = _T("ERROR_INVALID_USER_BUFFER");
        case ERROR_UNRECOGNIZED_MEDIA           : str = _T("ERROR_UNRECOGNIZED_MEDIA");
        case ERROR_NO_TRUST_LSA_SECRET          : str = _T("ERROR_NO_TRUST_LSA_SECRET");
        case ERROR_NO_TRUST_SAM_ACCOUNT         : str = _T("ERROR_NO_TRUST_SAM_ACCOUNT");
        case ERROR_TRUSTED_DOMAIN_FAILURE       : str = _T("ERROR_TRUSTED_DOMAIN_FAILURE");
        case ERROR_NETLOGON_NOT_STARTED         : str = _T("ERROR_NETLOGON_NOT_STARTED");
        case ERROR_ACCOUNT_EXPIRED              : str = _T("ERROR_ACCOUNT_EXPIRED");
        case ERROR_UNKNOWN_PORT                 : str = _T("ERROR_UNKNOWN_PORT");
        case ERROR_UNKNOWN_PRINTER_DRIVER       : str = _T("ERROR_UNKNOWN_PRINTER_DRIVER");
        case ERROR_UNKNOWN_PRINTPROCESSOR       : str = _T("ERROR_UNKNOWN_PRINTPROCESSOR");
        case ERROR_INVALID_SEPARATOR_FILE       : str = _T("ERROR_INVALID_SEPARATOR_FILE");
        case ERROR_INVALID_PRIORITY             : str = _T("ERROR_INVALID_PRIORITY");
        case ERROR_INVALID_PRINTER_NAME         : str = _T("ERROR_INVALID_PRINTER_NAME");
        case ERROR_PRINTER_ALREADY_EXISTS       : str = _T("ERROR_PRINTER_ALREADY_EXISTS");
        case ERROR_INVALID_PRINTER_COMMAND      : str = _T("ERROR_INVALID_PRINTER_COMMAND");
        case ERROR_INVALID_DATATYPE             : str = _T("ERROR_INVALID_DATATYPE");
        case ERROR_INVALID_ENVIRONMENT          : str = _T("ERROR_INVALID_ENVIRONMENT");
        case ERROR_DOMAIN_TRUST_INCONSISTENT    : str = _T("ERROR_DOMAIN_TRUST_INCONSISTENT");
        case ERROR_SERVER_HAS_OPEN_HANDLES      : str = _T("ERROR_SERVER_HAS_OPEN_HANDLES");
        case ERROR_RESOURCE_DATA_NOT_FOUND      : str = _T("ERROR_RESOURCE_DATA_NOT_FOUND");
        case ERROR_RESOURCE_TYPE_NOT_FOUND      : str = _T("ERROR_RESOURCE_TYPE_NOT_FOUND");
        case ERROR_RESOURCE_NAME_NOT_FOUND      : str = _T("ERROR_RESOURCE_NAME_NOT_FOUND");
        case ERROR_RESOURCE_LANG_NOT_FOUND      : str = _T("ERROR_RESOURCE_LANG_NOT_FOUND");
        case ERROR_NOT_ENOUGH_QUOTA             : str = _T("ERROR_NOT_ENOUGH_QUOTA");
        case ERROR_INVALID_TIME                 : str = _T("ERROR_INVALID_TIME");
        case ERROR_INVALID_FORM_NAME            : str = _T("ERROR_INVALID_FORM_NAME");
        case ERROR_INVALID_FORM_SIZE            : str = _T("ERROR_INVALID_FORM_SIZE");
        case ERROR_ALREADY_WAITING              : str = _T("ERROR_ALREADY_WAITING");
        case ERROR_PRINTER_DELETED              : str = _T("ERROR_PRINTER_DELETED");
        case ERROR_INVALID_PRINTER_STATE        : str = _T("ERROR_INVALID_PRINTER_STATE");
        case ERROR_PASSWORD_MUST_CHANGE         : str = _T("ERROR_PASSWORD_MUST_CHANGE");
        case ERROR_ACCOUNT_LOCKED_OUT           : str = _T("ERROR_ACCOUNT_LOCKED_OUT");
        case ERROR_NO_BROWSER_SERVERS_FOUND     : str = _T("ERROR_NO_BROWSER_SERVERS_FOUND");
        case ERROR_INVALID_PIXEL_FORMAT         : str = _T("ERROR_INVALID_PIXEL_FORMAT");
        case ERROR_BAD_DRIVER                   : str = _T("ERROR_BAD_DRIVER");
        case ERROR_INVALID_WINDOW_STYLE         : str = _T("ERROR_INVALID_WINDOW_STYLE");
        case ERROR_METAFILE_NOT_SUPPORTED       : str = _T("ERROR_METAFILE_NOT_SUPPORTED");
        case ERROR_TRANSFORM_NOT_SUPPORTED      : str = _T("ERROR_TRANSFORM_NOT_SUPPORTED");
        case ERROR_CLIPPING_NOT_SUPPORTED       : str = _T("ERROR_CLIPPING_NOT_SUPPORTED");
        case ERROR_UNKNOWN_PRINT_MONITOR        : str = _T("ERROR_UNKNOWN_PRINT_MONITOR");
        case ERROR_PRINTER_DRIVER_IN_USE        : str = _T("ERROR_PRINTER_DRIVER_IN_USE");
        case ERROR_SPOOL_FILE_NOT_FOUND         : str = _T("ERROR_SPOOL_FILE_NOT_FOUND");
        case ERROR_SPL_NO_STARTDOC              : str = _T("ERROR_SPL_NO_STARTDOC");
        case ERROR_SPL_NO_ADDJOB                : str = _T("ERROR_SPL_NO_ADDJOB");
        case ERROR_WINS_INTERNAL                : str = _T("ERROR_WINS_INTERNAL");
        case ERROR_CAN_NOT_DEL_LOCAL_WINS       : str = _T("ERROR_CAN_NOT_DEL_LOCAL_WINS");
        case ERROR_STATIC_INIT                  : str = _T("ERROR_STATIC_INIT");
        case ERROR_INC_BACKUP                   : str = _T("ERROR_INC_BACKUP");
        case ERROR_FULL_BACKUP                  : str = _T("ERROR_FULL_BACKUP");
        case ERROR_REC_NON_EXISTENT             : str = _T("ERROR_REC_NON_EXISTENT");
        case ERROR_RPL_NOT_ALLOWED              : str = _T("ERROR_RPL_NOT_ALLOWED");
        }
    return(str);
}

//////////////////////////////////////////////////////////////////////////

int MxPad::xResolve()

{
	char	bc  = 1;
	int		bclen  = sizeof(char);
	int     res2 = 0;
	unsigned long     res;
	unsigned int     tme;
	
	#ifndef _DEBUG
		return 0;
	#endif

	SOCKET sock = socket( AF_INET, SOCK_DGRAM, 0);
	//printf(_T("After sock %d\n"),sock);

	int ret_val = setsockopt (sock, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(int));

	//TRACE(_T("After sockoption %d\n"), ret_val);

	// Verify if it took
	//ret_val = getsockopt (sock, SOL_SOCKET, SO_BROADCAST, &bc, &bclen);
	//printf(_T("After getsockoption %d ---  %d\n"), ret_val, bc);

	// Start listening before sending
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) _AcceptThreadProc,
														   &res2, 0, &res);
	Sleep(20);

	local_sin.sin_addr.s_addr = INADDR_BROADCAST;
	local_sin.sin_port = htons(MX_RES_PORT);
	local_sin.sin_family = AF_INET;
	acc_sin_len = sizeof(local_sin);

	ret_val = sendto(sock, "mxpad", 6, 0, (struct sockaddr *)&local_sin, acc_sin_len);

	//TRACE(_T("After sendto %d \n"), ret_val);
	//TRACE(_T("Created: %x\n"), res);

	tme = GetTickCount() + 2000;

	while(1)
		{
		MSG  msg;

		if(tme < GetTickCount())		// Timeout
			break;

		if(res2 != 0)                    // Done
			break;

		// Let windows do its thing ...
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
			TranslateMessage(&msg), DispatchMessage(&msg);
			}

		Sleep(20);			// Make the machine happy
		}

	if(res2 > 0)
		{
		// Address in rec_sin2
		resolved = true;
		}
	else
		{
		TRACE(_T("MxPad::Resolve failed\r\n"));
		}

	TRACE(_T("MxPad::xResolve\r\n"));

	return resolved;
}

//////////////////////////////////////////////////////////////////////////

int		MxPad::xConnect()

{
	SOCKADDR_IN     dest_sin;                 /* Local socket - internet style */

	#ifndef _DEBUG
		return 0;
	#endif

	int nd = 1, ndlen = sizeof(int);

	glsock2 = socket(AF_INET, SOCK_STREAM, 0);

	if(glsock2 == INVALID_SOCKET)
		{
		//MessageBox(NULL, "no socket", "", MB_OK);
		TRACE(_T("MxPad::xConnect No Socket"));
		}

	dest_sin.sin_family = AF_INET;
	dest_sin.sin_port = htons(MX_DATA_PORT);

	//dest_sin.sin_addr.s_addr = inet_addr(_T("127.0.0.1"));
	dest_sin.sin_addr.s_addr = rec_sin2.sin_addr.s_addr;
	
	if (connect(glsock2, (PSOCKADDR) &dest_sin, sizeof(dest_sin)) < 0)
		{
		#ifdef PG_DEBUG
		//PrintxToNotepad(_T("SER32: Cannot connect back.\r\n"));
		//mxMBOX(_T("cannot connect"), MB_OK);
		TRACE(_T("MxPad::Connect: cannot connect\r\n"));
		#endif

		return 0;
		}

	connected = true;

	//TRACE(_T("MxPad::xConnect\r\n"));

	return connected;
}

//////////////////////////////////////////////////////////////////////////
/// \brief The Async version of printf. 
/// Will output to temporary buffer so it is really fast. 
/// Can be used to debug real time code, as it
/// writes to memory, and has no IO.\n
/// It is also useful to debug constructors of global variables, as the 
/// CRT is not operational on global construct time. (go figure)

int     MxPad::aprintf(const TCHAR  *Format, ...)

{
	#ifndef _DEBUG
		if(!force)
			return (0);
	#endif

	//MessageBox(NULL, Format, "", MB_OK);

	if(!constructed)
		{
		TRACE(_T("Warning: Called Mxpad before construction\r\n"));
		TRACE(_T("Format str was:'%s'\r\n"), Format);
		return 0;
		}

	if(!aszOutString)
		{
		TRACE(_T("Warning: Called Mxpad after destruction\r\n"));
		TRACE(_T("Format str was:'%s'\r\n"), Format);
		return 0;
		}

	// Start thread if none
	if(!handle)
		{
		handle = _beginthread(_RunMxThread, 0, this);
		}

	int len = 0;
	try
		{
		va_list ArgList; va_start(ArgList, Format);
		len = _vsntprintf_s(aszOutString,BYTE2TXT(MXOUTLEN - sizeof(TCHAR)), _TRUNCATE, Format, ArgList);
		}

	catch(...)
		{
		xToNotepad(_T("\r\n")
				  _T("----------------------------------------------------------------------------\r\n"));
		xToNotepad(_T("Exception caught on P2N. Possible mismatching of FORMAT str vs. FORMAT arg\r\n"));
		xToNotepad(_T("Here is what we got so far: (between [])\r\n["));
		
		xToNotepad(aszOutString);
		
		xToNotepad(_T("]\r\nException End"));
		xToNotepad(_T("\r\n")
			  	  _T("----------------------------------------------------------------------------\r\n"));
		xToNotepad(_T("\r\n"));
		}

	// Terminate if overflowed
	if(len < 0)	
		{
		aszOutString[MXOUTLEN - 1] = '\0';
		}

	for(int loop = 0; loop < len; loop++)
		{
		if(!circ.Add(aszOutString[loop]))
			{
			break;
			}
		}
    
	if(len < 0)	
		{
		xToNotepad(_T("\r\nWarning: Output string was cut to prevent buffer overflow.\r\n"));
		}

    return(0);
}

//////////////////////////////////////////////////////////////////////////

MxPad::CxCircular::CxCircular()

{
	size = CIRC_DEF_SIZE;
	InitVars();
}

MxPad::CxCircular::CxCircular(int bufsize)

{
	size = bufsize;
	InitVars();
}

void MxPad::CxCircular::InitVars()

{
	head = tail = 0;

	buff = (TCHAR  *)malloc(size);
	////ASSERT(buff);

	//return 0;
}

MxPad::CxCircular::~CxCircular()

{
	if(buff) 
		free (buff);
}

//////////////////////////////////////////////////////////////////////////
//
//  ---------------------------------
//	    ^tail        ^head
//
//  ---------------------------------
//	           ^head       ^tail 
//

/// Internal: Add to circular buffer

int MxPad::CxCircular::Add(TCHAR  chh)

{
	// Full?
	if (head == tail - 1)
		return false;
	
	// Full? - special case
	if((tail == 0) && (head == size - 1))
		return false;

	lock.Lock();

	buff[head] = chh;

	head++;

	if(head >= size)
		head = 0;

	lock.Unlock();

	return true;
}

//////////////////////////////////////////////////////////////////////////
/// Internal: read TCHAR from circular buffer

int MxPad::CxCircular::Get(TCHAR  *chh)

{
	// Empty?
	if (head == tail)
		return false;

	lock.Lock();

	*chh = 	buff[tail];

	tail++;

	if(tail >= size)
		tail = 0;

	lock.Unlock();

	return true;
}

//////////////////////////////////////////////////////////////////////////
/// Return TRUE if circular buffer is empty

int MxPad::CxCircular::IsEmpty()

{
	return (head == tail);
}

#ifdef CWnd

/// Helper: dump window styles as textual information

void MxPad::DumpStyles(CWnd *ptr)

{
	P2N(_T("Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(ptr->m_hWnd, GWL_STYLE);

	if(style & WS_BORDER   )
		P2N(_T("WS_BORDER   "));
	if(style & WS_CAPTION   )
		P2N(_T("WS_CAPTION   "));
	if(style & WS_CHILD   )
		P2N(_T("WS_CHILD   "));
	if(style & WS_CLIPCHILDREN   )
		P2N(_T("WS_CLIPCHILDREN   "));
	if(style & WS_CLIPSIBLINGS   )
		P2N(_T("WS_CLIPSIBLINGS   "));
	if(style & WS_DISABLED   )
		P2N(_T("WS_DISABLED   "));
	if(style & WS_DLGFRAME   )
		P2N(_T("WS_DLGFRAME   "));
	if(style & WS_GROUP   )
		P2N(_T("WS_GROUP   "));
	if(style & WS_HSCROLL   )
		P2N(_T("WS_HSCROLL   "));
	if(style & WS_MAXIMIZE   )
		P2N(_T("WS_MAXIMIZE   "));
	if(style & WS_MAXIMIZEBOX   )
		P2N(_T("WS_MAXIMIZEBOX   "));
	if(style & WS_MINIMIZE   )
		P2N(_T("WS_MINIMIZE   "));
	if(style & WS_MINIMIZEBOX   )
		P2N(_T("WS_MINIMIZEBOX   "));
	if(style & WS_OVERLAPPED   )
		P2N(_T("WS_OVERLAPPED   "));
	if(style & WS_OVERLAPPEDWINDOW   )
		P2N(_T("WS_OVERLAPPEDWINDOW   "));
	if(style & WS_POPUP   )
		P2N(_T("WS_POPUP   "));
	if(style & WS_POPUPWINDOW   )
		P2N(_T("WS_POPUPWINDOW   "));
	if(style & WS_SYSMENU   )
		P2N(_T("WS_SYSMENU   "));
	if(style & WS_TABSTOP   )
		P2N(_T("WS_TABSTOP   "));
	if(style & WS_THICKFRAME   )
		P2N(_T("WS_THICKFRAME   "));
	if(style & WS_VISIBLE   )
		P2N(_T("WS_VISIBLE   "));
	if(style & WS_VSCROLL   )
		P2N(_T("WS_VSCROLL   "));

	P2N(_T("\r\n"));
}

/// Helper: Dump extended styles as textual information

void MxPad::DumpEdS(CWnd *ptr)

{
	// CEdit

	P2N(_T("Edit Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(ptr->m_hWnd, GWL_STYLE);

	if(style & ES_AUTOHSCROLL   )
		P2N(_T("  ES_AUTOHSCROLL  "));
	if(style & ES_AUTOVSCROLL   )
		P2N(_T("  ES_AUTOVSCROLL  "));
	if(style & ES_CENTER   )
		P2N(_T("  ES_CENTER  "));
	if(style & ES_LEFT   )
		P2N(_T("  ES_LEFT  "));
	if(style & ES_LOWERCASE   )
		P2N(_T("  ES_LOWERCASE  "));
	if(style & ES_MULTILINE   )
		P2N(_T("  ES_MULTILINE  "));
	if(style & ES_NOHIDESEL   )
		P2N(_T("  ES_NOHIDESEL  "));
	if(style & ES_OEMCONVERT   )
		P2N(_T("  ES_OEMCONVERT  "));
	if(style & ES_PASSWORD   )
		P2N(_T("  ES_PASSWORD  "));
	if(style & ES_RIGHT   )
		P2N(_T("  ES_RIGHT  "));
	if(style & ES_UPPERCASE   )
		P2N(_T("  ES_UPPERCASE  "));
	if(style & ES_READONLY   )
		P2N(_T("  ES_READONLY  "));
	if(style & ES_WANTRETURN   )
		P2N(_T("  ES_WANTRETURN  "));

	P2N(_T("\r\n"));

}

/// Helper: Dump static styles as textual information

void MxPad::DumpSS(CWnd *ptr)

{
	// CStatic 

	P2N(_T("Static Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(ptr->m_hWnd, GWL_STYLE);

	if(style & SS_BLACKFRAME   )
		P2N(_T("  SS_BLACKFRAME   "));
	if(style & SS_BLACKRECT   )
		P2N(_T("  SS_BLACKRECT   "));
	if(style & SS_CENTER   )
		P2N(_T("  SS_CENTER   "));
	if(style & SS_GRAYFRAME   )
		P2N(_T("  SS_GRAYFRAME   "));
	if(style & SS_GRAYRECT   )
		P2N(_T("  SS_GRAYRECT   "));
	if(style & SS_ICON   )
		P2N(_T("  SS_ICON   "));
	if(style & SS_LEFT   )
		P2N(_T("  SS_LEFT   "));
	if(style & SS_LEFTNOWORDWRAP   )
		P2N(_T("  SS_LEFTNOWORDWRAP   "));
	if(style & SS_NOPREFIX   )
		P2N(_T("  SS_NOPREFIX   "));
	if(style & SS_RIGHT   )
		P2N(_T("  SS_RIGHT   "));
	if(style & SS_SIMPLE   )
		P2N(_T("  SS_SIMPLE   "));
	if(style & SS_USERITEM   )
		P2N(_T("  SS_USERITEM   "));
	if(style & SS_WHITEFRAME   )
		P2N(_T("  SS_WHITEFRAME   "));
	if(style & SS_WHITERECT   )
		P2N(_T("  SS_WHITERECT   "));

	
	P2N(_T("\r\n"));
}

/// Helper: Dump extended styles as textual information

void MxPad::DumpExStyles(CWnd *ptr)

{
	P2N(_T("Extended Window Styles:\r\n"));
	DWORD style = ::GetWindowLong(ptr->m_hWnd, GWL_EXSTYLE);

	if(style & WS_EX_ACCEPTFILES   )
		P2N(_T("  WS_EX_ACCEPTFILES  "));
	if(style & WS_EX_CLIENTEDGE   )
		P2N(_T("WS_EX_CLIENTEDGE  "));
	if(style & WS_EX_CONTEXTHELP   )
		P2N(_T("WS_EX_CONTEXTHELP  "));
	if(style & WS_EX_CONTROLPARENT   )
		P2N(_T("WS_EX_CONTROLPARENT  "));
	if(style & WS_EX_DLGMODALFRAME   )
		P2N(_T("WS_EX_DLGMODALFRAME  "));
	if(style & WS_EX_LEFT   )
		P2N(_T("WS_EX_LEFT  "));
	if(style & WS_EX_LEFTSCROLLBAR   )
		P2N(_T("WS_EX_LEFTSCROLLBAR  "));
	if(style & WS_EX_LTRREADING   )
		P2N(_T("WS_EX_LTRREADING  "));
	if(style & WS_EX_MDICHILD   )
		P2N(_T("WS_EX_MDICHILD  "));
	if(style & WS_EX_NOPARENTNOTIFY   )
		P2N(_T("WS_EX_NOPARENTNOTIFY  "));
	if(style & WS_EX_OVERLAPPEDWINDOW   )
		P2N(_T("WS_EX_OVERLAPPEDWINDOW  "));
	if(style & WS_EX_PALETTEWINDOW   )
		P2N(_T("WS_EX_PALETTEWINDOW  "));
	if(style & WS_EX_RIGHT   )
		P2N(_T("WS_EX_RIGHT  "));
	if(style & WS_EX_RIGHTSCROLLBAR   )
		P2N(_T("WS_EX_RIGHTSCROLLBAR  "));
	if(style & WS_EX_RTLREADING   )
		P2N(_T("WS_EX_RTLREADING  "));
	if(style & WS_EX_STATICEDGE   )
		P2N(_T("WS_EX_STATICEDGE  "));
	if(style & WS_EX_TOOLWINDOW   )
		P2N(_T("WS_EX_TOOLWINDOW  "));
	if(style & WS_EX_TOPMOST   )
		P2N(_T("WS_EX_TOPMOST  "));
	if(style & WS_EX_TRANSPARENT   )
		P2N(_T("WS_EX_TRANSPARENT  "));
	if(style & WS_EX_WINDOWEDGE   )
		P2N(_T("WS_EX_WINDOWEDGE  "));

	P2N(_T("\r\n"));
}

#endif

/// Helper: convert keystroke integer to textual 

TCHAR	*MxPad::vkey2keystr(int code)

{
	static 	TCHAR xx[5];
	TCHAR *str = _T("none");
	
	//P2N(_T("MxPad::vkey2keystr code = %d\r\n"), code);

	if(isupper(code))
		{	
		xx[0]  = code;	xx[1]  = 0;	
		str = xx;
		}
	else switch(code)
		{
		case VK_LBUTTON: str= _T("VK_LBUTTON"); break;
		case VK_RBUTTON: str= _T("VK_RBUTTON"); break;
		case VK_CANCEL: str= _T("VK_CANCEL"); break;
		case VK_MBUTTON: str= _T("VK_MBUTTON"); break;
		case VK_BACK: str= _T("VK_BACK"); break;
		case VK_TAB: str= _T("VK_TAB"); break;
		case VK_CLEAR: str= _T("VK_CLEAR"); break;
		case VK_RETURN: str= _T("VK_RETURN"); break;
		case VK_SHIFT: str= _T("VK_SHIFT"); break;
		case VK_CONTROL: str= _T("VK_CONTROL"); break;
		case VK_MENU: str= _T("VK_MENU"); break;
		case VK_PAUSE: str= _T("VK_PAUSE"); break;
		case VK_CAPITAL: str= _T("VK_CAPITAL"); break;
		case VK_KANA: str= _T("VK_KANA"); break;
		case VK_JUNJA: str= _T("VK_JUNJA"); break;
		case VK_FINAL: str= _T("VK_FINAL"); break;
		case VK_HANJA: str= _T("VK_HANJA"); break;
		case VK_ESCAPE: str= _T("VK_ESCAPE"); break;
		case VK_CONVERT: str= _T("VK_CONVERT"); break;
		case VK_NONCONVERT: str= _T("VK_NONCONVERT"); break;
		case VK_ACCEPT: str= _T("VK_ACCEPT"); break;
		case VK_MODECHANGE: str= _T("VK_MODECHANGE"); break;
		case VK_SPACE: str= _T("VK_SPACE"); break;
		case VK_PRIOR: str= _T("VK_PRIOR"); break;
		case VK_NEXT: str= _T("VK_NEXT"); break;
		case VK_END: str= _T("VK_END"); break;
		case VK_HOME: str= _T("VK_HOME"); break;
		case VK_LEFT: str= _T("VK_LEFT"); break;
		case VK_UP: str= _T("VK_UP"); break;
		case VK_RIGHT: str= _T("VK_RIGHT"); break;
		case VK_DOWN: str= _T("VK_DOWN"); break;
		case VK_SELECT: str= _T("VK_SELECT"); break;
		case VK_PRINT: str= _T("VK_PRINT"); break;
		case VK_EXECUTE: str= _T("VK_EXECUTE"); break;
		case VK_SNAPSHOT: str= _T("VK_SNAPSHOT"); break;
		case VK_INSERT: str= _T("VK_INSERT"); break;
		case VK_DELETE: str= _T("VK_DELETE"); break;
		case VK_HELP: str= _T("VK_HELP"); break;
		case VK_LWIN: str= _T("VK_LWIN"); break;
		case VK_RWIN: str= _T("VK_RWIN"); break;
		case VK_APPS: str= _T("VK_APPS"); break;
		case VK_NUMPAD0: str= _T("VK_NUMPAD0"); break;
		case VK_NUMPAD1: str= _T("VK_NUMPAD1"); break;
		case VK_NUMPAD2: str= _T("VK_NUMPAD2"); break;
		case VK_NUMPAD3: str= _T("VK_NUMPAD3"); break;
		case VK_NUMPAD4: str= _T("VK_NUMPAD4"); break;
		case VK_NUMPAD5: str= _T("VK_NUMPAD5"); break;
		case VK_NUMPAD6: str= _T("VK_NUMPAD6"); break;
		case VK_NUMPAD7: str= _T("VK_NUMPAD7"); break;
		case VK_NUMPAD8: str= _T("VK_NUMPAD8"); break;
		case VK_NUMPAD9: str= _T("VK_NUMPAD9"); break;
		case VK_MULTIPLY: str= _T("VK_MULTIPLY"); break;
		case VK_ADD: str= _T("VK_ADD"); break;
		case VK_SEPARATOR: str= _T("VK_SEPARATOR"); break;
		case VK_SUBTRACT: str= _T("VK_SUBTRACT"); break;
		case VK_DECIMAL: str= _T("VK_DECIMAL"); break;
		case VK_DIVIDE: str= _T("VK_DIVIDE"); break;
		case VK_F1:	str=_T("VK_F1"); break; 
		case VK_F2:	str=_T("VK_F2"); break; 
		case VK_F3:	str=_T("VK_F3"); break; 
		case VK_F4:	str=_T("VK_F4"); break; 
		case VK_F5:	str=_T("VK_F5"); break; 
		case VK_F6:	str=_T("VK_F6"); break; 
		case VK_F7:	str=_T("VK_F7"); break; 
		case VK_F8:	str=_T("VK_F8"); break; 
		case VK_F9:	str=_T("VK_F9"); break; 
		case VK_F10:	str=_T("VK_F10"); break; 
		case VK_F11:	str=_T("VK_F11"); break; 
		case VK_F12:	str=_T("VK_F12"); break; 
		case VK_F13:	str=_T("VK_F13"); break; 
		case VK_F14:	str=_T("VK_F14"); break; 
		case VK_F15:	str=_T("VK_F15"); break; 
		case VK_F16:	str=_T("VK_F16"); break; 
		case VK_F17:	str=_T("VK_F17"); break; 
		case VK_F18:	str=_T("VK_F18"); break; 
		case VK_F19:	str=_T("VK_F19"); break; 
		case VK_F20:	str=_T("VK_F20"); break; 
		case VK_F21:	str=_T("VK_F21"); break; 
		case VK_F22:	str=_T("VK_F22"); break; 
		case VK_F23:	str=_T("VK_F23"); break; 
		case VK_F24:	str=_T("VK_F24"); break; 
		case VK_NUMLOCK:	str=_T("VK_NUMLOCK"); break; 
		case VK_SCROLL:	str=_T("VK_SCROLL"); break; 
		case VK_LSHIFT:	str=_T("VK_LSHIFT"); break; 
		case VK_RSHIFT:	str=_T("VK_RSHIFT"); break; 
		case VK_LCONTROL:	str=_T("VK_LCONTROL"); break; 
		case VK_RCONTROL:	str=_T("VK_RCONTROL"); break; 
		case VK_LMENU:	str=_T("VK_LMENU"); break; 
		case VK_RMENU:	str=_T("VK_RMENU"); break; 
		case VK_PROCESSKEY:	str=_T("VK_PROCESSKEY"); break; 
		case VK_ATTN:	str=_T("VK_ATTN"); break; 
		case VK_CRSEL:	str=_T("VK_CRSEL"); break; 
		case VK_EXSEL:	str=_T("VK_EXSEL"); break; 
		case VK_EREOF:	str=_T("VK_EREOF"); break; 
		case VK_PLAY:	str=_T("VK_PLAY"); break; 
		case VK_ZOOM:	str=_T("VK_ZOOM"); break; 
		case VK_NONAME:	str=_T("VK_NONAME"); break; 
		case VK_PA1:	str=_T("VK_PA1"); break; 
		case VK_OEM_CLEAR:	str=_T("VK_OEM_CLEAR"); break; 

		default: str = _T("no key");
		}
	return str;
}

/// Sets the output application name as returned by FindWindow().
/// Can be used to direct the debug output to any application that had the Edit
/// control at its core. (NotePad for example)
/// Make sure you know what you are doing, as it makes for a confused user
/// seeing programming data in notepad, (seemingly) coming from nowhere.
/// \n\n Must call this before any other function as the found window is cached.

void	MxPad::SetAppname(const TCHAR  *str)

{
	_tcscpy_s(appname, sizeof(appname), str);
}

///	Set up mxpad to autodetect conditions present

void	MxPad::Detect()

{
	#ifndef _DEBUG
		if(!force)
			return;
	#endif

	// Set up mxpad to autodetect
	int got = false;
	if(IsLocalAvail())
		{
		got++;
		//CString str; str.Format(_T("Local Agent is running.\r\n"));
		//TRACE(str);
		//mxpad.SetLocal(true);
		SetLocal(true);

		}
	if(IsRemoteAvail())
		{
		got++;
		//CString str; str.Format(_T("Remote Agent is running.\r\n"));
		//TRACE(str);
		//mxpad.SetRemote(true);
		SetRemote(true);
		}
	if(!got)
		{
#ifdef CString
		CString str; str.Format(
			"No debug agent is running, routing to TRACE instead.\r\n");
		TRACE(str);
	
#endif

		//mxpad.SetTrace(true);
		SetTrace(true);
		}
	else
		{

#ifdef CString
		CString str; str.Format(
			_T("Started to log to MxPad debug agent.\r\n\r\n"));
		TRACE(str);
#endif
		}
}


TCHAR *MxPad::werr2str(int err)

{
	TCHAR *str = _T("unknown error");

	switch (err)
		{
		case ERROR_SUCCESS : str = 	_T("The operation completed successfully.  "); break;
		case ERROR_INVALID_FUNCTION : str = 	_T("Incorrect function.  "); break;
		case ERROR_FILE_NOT_FOUND : str = 	_T("The system cannot find the file specified.  "); break;
		case ERROR_PATH_NOT_FOUND : str = 	_T("The system cannot find the path specified.  "); break;
		case ERROR_TOO_MANY_OPEN_FILES : str = 	_T("The system cannot open the file.  "); break;
		case ERROR_ACCESS_DENIED : str = 	_T("Access is denied.  "); break;
		case ERROR_INVALID_HANDLE : str = 	_T("The handle is invalid.  "); break;
		case ERROR_ARENA_TRASHED : str = 	_T("The storage control blocks were destroyed.  "); break;
		case ERROR_NOT_ENOUGH_MEMORY : str = 	_T("Not enough storage is available to process this command.  "); break;
		case ERROR_INVALID_BLOCK : str = 	_T("The storage control block address is invalid.  "); break;
		case ERROR_BAD_ENVIRONMENT : str = 	_T("The environment is incorrect.  "); break;
		case ERROR_BAD_FORMAT : str = 	_T("An attempt was made to load a program with an incorrect format.  "); break;
		case ERROR_INVALID_ACCESS : str = 	_T("The access code is invalid.  "); break;
		case ERROR_INVALID_DATA : str = 	_T("The data is invalid.  "); break;
		case ERROR_OUTOFMEMORY : str = 	_T("Not enough storage is available to complete this operation.  "); break;
		case ERROR_INVALID_DRIVE : str = 	_T("The system cannot find the drive specified.  "); break;
		case ERROR_CURRENT_DIRECTORY : str = 	_T("The directory cannot be removed.  "); break;
		case ERROR_NOT_SAME_DEVICE : str = 	_T("The system cannot move the file to a different disk drive.  "); break;
		case ERROR_NO_MORE_FILES : str = 	_T("There are no more files.  "); break;
		case ERROR_WRITE_PROTECT : str = 	_T("The media is write protected.  "); break;
		case ERROR_BAD_UNIT : str = 	_T("The system cannot find the device specified.  "); break;
		case ERROR_NOT_READY : str = 	_T("The device is not ready.  "); break;
		case ERROR_BAD_COMMAND : str = 	_T("The device does not recognize the command.  "); break;
		case ERROR_CRC : str = 	_T("Data error (cyclic redundancy check).  "); break;
		case ERROR_BAD_LENGTH : str = 	_T("The program issued a command but the command length is incorrect.  "); break;
		case ERROR_SEEK : str = 	_T("The drive cannot locate a specific area or track on the disk.  "); break;
		case ERROR_NOT_DOS_DISK : str = 	_T("The specified disk or diskette cannot be accessed.  "); break;
		case ERROR_SECTOR_NOT_FOUND : str = 	_T("The drive cannot find the sector requested.  "); break;
		case ERROR_OUT_OF_PAPER : str = 	_T("The printer is out of paper.  "); break;
		case ERROR_WRITE_FAULT : str = 	_T("The system cannot write to the specified device.  "); break;
		case ERROR_READ_FAULT : str = 	_T("The system cannot read from the specified device.  "); break;
		case ERROR_GEN_FAILURE : str = 	_T("A device attached to the system is not functioning.  "); break;
		case ERROR_SHARING_VIOLATION : str = 	_T("The process cannot access the file because it is being used by another process.  "); break;
		case ERROR_LOCK_VIOLATION : str = 	_T("The process cannot access the file because another process has locked a portion of the file.  "); break;
		case ERROR_WRONG_DISK : str = 	_T("The wrong diskette is in the drive. Insert %2 (Volume Serial Number: str =  %3) into drive %1.  "); break;
		case ERROR_SHARING_BUFFER_EXCEEDED : str = 	_T("Too many files opened for sharing.  "); break;
		case ERROR_HANDLE_EOF : str = 	_T("Reached the end of the file.  "); break;
		case ERROR_HANDLE_DISK_FULL : str = 	_T("The disk is full.  "); break;
		case ERROR_NOT_SUPPORTED : str = 	_T("The network request is not supported.  "); break;
		case ERROR_REM_NOT_LIST : str = 	_T("The remote computer is not available.  "); break;
		case ERROR_DUP_NAME : str = 	_T("A duplicate name exists on the network.  "); break;
		case ERROR_BAD_NETPATH : str = 	_T("The network path was not found.  "); break;
		case ERROR_NETWORK_BUSY : str = 	_T("The network is busy.  "); break;
		case ERROR_DEV_NOT_EXIST : str = 	_T("The specified network resource or device is no longer available.  "); break;
		case ERROR_TOO_MANY_CMDS : str = 	_T("The network BIOS command limit has been reached.  "); break;
		case ERROR_ADAP_HDW_ERR : str = 	_T("A network adapter hardware error occurred.  "); break;
		case ERROR_BAD_NET_RESP : str = 	_T("The specified server cannot perform the requested operation.  "); break;
		case ERROR_UNEXP_NET_ERR : str = 	_T("An unexpected network error occurred.  "); break;
		case ERROR_BAD_REM_ADAP : str = 	_T("The remote adapter is not compatible.  "); break;
		case ERROR_PRINTQ_FULL : str = 	_T("The printer queue is full.  "); break;
		case ERROR_NO_SPOOL_SPACE : str = 	_T("Space to store the file waiting to be printed is not available on the server.  "); break;
		case ERROR_PRINT_CANCELLED : str = 	_T("Your file waiting to be printed was deleted.  "); break;
		case ERROR_NETNAME_DELETED : str = 	_T("The specified network name is no longer available.  "); break;
		case ERROR_NETWORK_ACCESS_DENIED : str = 	_T("Network access is denied.  "); break;
		case ERROR_BAD_DEV_TYPE : str = 	_T("The network resource type is not correct.  "); break;
		case ERROR_BAD_NET_NAME : str = 	_T("The network name cannot be found.  "); break;
		case ERROR_TOO_MANY_NAMES : str = 	_T("The name limit for the local computer network adapter card was exceeded.  "); break;
		case ERROR_TOO_MANY_SESS : str = 	_T("The network BIOS session limit was exceeded.  "); break;
		case ERROR_SHARING_PAUSED : str = 	_T("The remote server has been paused or is in the process of being started.  "); break;
		case ERROR_REQ_NOT_ACCEP : str = 	_T("No more connections can be made to this remote computer at this time because there are already as many connections as the computer can accept.  "); break;
		case ERROR_REDIR_PAUSED : str = 	_T("The specified printer or disk device has been paused.  "); break;
		case ERROR_FILE_EXISTS : str = 	_T("The file exists.  "); break;
		case ERROR_CANNOT_MAKE : str = 	_T("The directory or file cannot be created.  "); break;
		case ERROR_FAIL_I24 : str = 	_T("Fail on INT 24.  "); break;
		case ERROR_OUT_OF_STRUCTURES : str = 	_T("Storage to process this request is not available.  "); break;
		case ERROR_ALREADY_ASSIGNED : str = 	_T("The local device name is already in use.  "); break;
		case ERROR_INVALID_PASSWORD : str = 	_T("The specified network password is not correct.  "); break;
		case ERROR_INVALID_PARAMETER : str = 	_T("The parameter is incorrect.  "); break;
		case ERROR_NET_WRITE_FAULT : str = 	_T("A write fault occurred on the network.  "); break;
		case ERROR_NO_PROC_SLOTS : str = 	_T("The system cannot start another process at this time.  "); break;
		case ERROR_TOO_MANY_SEMAPHORES : str = 	_T("Cannot create another system semaphore.  "); break;
		case ERROR_EXCL_SEM_ALREADY_OWNED : str = 	_T("The exclusive semaphore is owned by another process.  "); break;
		case ERROR_SEM_IS_SET : str = 	_T("The semaphore is set and cannot be closed.  "); break;
		case ERROR_TOO_MANY_SEM_REQUESTS : str = 	_T("The semaphore cannot be set again.  "); break;
		case ERROR_INVALID_AT_INTERRUPT_TIME : str = 	_T("Cannot request exclusive semaphores at interrupt time.  "); break;
		case ERROR_SEM_OWNER_DIED : str = 	_T("The previous ownership of this semaphore has ended.  "); break;
		case ERROR_SEM_USER_LIMIT : str = 	_T("Insert the diskette for drive %1.  "); break;
		case ERROR_DISK_CHANGE : str = 	_T("The program stopped because an alternate diskette was not inserted.  "); break;
		case ERROR_DRIVE_LOCKED : str = 	_T("The disk is in use or locked by another process.  "); break;
		case ERROR_BROKEN_PIPE : str = 	_T("The pipe has been ended.  "); break;
		case ERROR_OPEN_FAILED : str = 	_T("The system cannot open the device or file specified.  "); break;
		case ERROR_BUFFER_OVERFLOW : str = 	_T("The file name is too long.  "); break;
		case ERROR_DISK_FULL : str = 	_T("There is not enough space on the disk.  "); break;
		case ERROR_NO_MORE_SEARCH_HANDLES : str = 	_T("No more internal file identifiers available.  "); break;
		case ERROR_INVALID_TARGET_HANDLE : str = 	_T("The target internal file identifier is incorrect.  "); break;
		case ERROR_INVALID_CATEGORY : str = 	_T("The IOCTL call made by the application program is not correct.  "); break;
		case ERROR_INVALID_VERIFY_SWITCH : str = 	_T("The verify-on-write switch parameter value is not correct.  "); break;
		case ERROR_BAD_DRIVER_LEVEL : str = 	_T("The system does not support the command requested.  "); break;
		case ERROR_CALL_NOT_IMPLEMENTED : str = 	_T("This function is not supported on this system.  "); break;
		case ERROR_SEM_TIMEOUT : str = 	_T("The semaphore timeout period has expired.  "); break;
		case ERROR_INSUFFICIENT_BUFFER : str = 	_T("The data area passed to a system call is too small.  "); break;
		case ERROR_INVALID_NAME : str = 	_T("The filename, directory name, or volume label syntax is incorrect.  "); break;
		case ERROR_INVALID_LEVEL : str = 	_T("The system call level is not correct.  "); break;
		case ERROR_NO_VOLUME_LABEL : str = 	_T("The disk has no volume label.  "); break;
		case ERROR_MOD_NOT_FOUND : str = 	_T("The specified module could not be found.  "); break;
		case ERROR_PROC_NOT_FOUND : str = 	_T("The specified procedure could not be found.  "); break;
		case ERROR_WAIT_NO_CHILDREN : str = 	_T("There are no child processes to wait for.  "); break;
		case ERROR_CHILD_NOT_COMPLETE : str = 	_T("The %1 application cannot be run in Win32 mode.  "); break;
		case ERROR_DIRECT_ACCESS_HANDLE : str = 	_T("Attempt to use a file handle to an open disk partition for an operation other than raw disk I/O.  "); break;
		case ERROR_NEGATIVE_SEEK : str = 	_T("An attempt was made to move the file pointer before the beginning of the file.  "); break;
		case ERROR_SEEK_ON_DEVICE : str = 	_T("The file pointer cannot be set on the specified device or file.  "); break;
		case ERROR_IS_JOIN_TARGET : str = 	_T("A JOIN or SUBST command cannot be used for a drive that contains previously joined drives.  "); break;
		case ERROR_IS_JOINED : str = 	_T("An attempt was made to use a JOIN or SUBST command on a drive that has already been joined.  "); break;
		case ERROR_IS_SUBSTED : str = 	_T("An attempt was made to use a JOIN or SUBST command on a drive that has already been substituted.  "); break;
		case ERROR_NOT_JOINED : str = 	_T("The system tried to delete the JOIN of a drive that is not joined.  "); break;
		case ERROR_NOT_SUBSTED : str = 	_T("The system tried to delete the substitution of a drive that is not substituted.  "); break;
		case ERROR_JOIN_TO_JOIN : str = 	_T("The system tried to join a drive to a directory on a joined drive.  "); break;
		case ERROR_SUBST_TO_SUBST : str = 	_T("The system tried to substitute a drive to a directory on a substituted drive.  "); break;
		case ERROR_JOIN_TO_SUBST : str = 	_T("The system tried to join a drive to a directory on a substituted drive.  "); break;
		case ERROR_SUBST_TO_JOIN : str = 	_T("The system tried to SUBST a drive to a directory on a joined drive.  "); break;
		case ERROR_BUSY_DRIVE : str = 	_T("The system cannot perform a JOIN or SUBST at this time.  "); break;
		case ERROR_SAME_DRIVE : str = 	_T("The system cannot join or substitute a drive to or for a directory on the same drive.  "); break;
		case ERROR_DIR_NOT_ROOT : str = 	_T("The directory is not a subdirectory of the root directory.  "); break;
		case ERROR_DIR_NOT_EMPTY : str = 	_T("The directory is not empty.  "); break;
		case ERROR_IS_SUBST_PATH : str = 	_T("The path specified is being used in a substitute.  "); break;
		case ERROR_IS_JOIN_PATH : str = 	_T("Not enough resources are available to process this command.  "); break;
		case ERROR_PATH_BUSY : str = 	_T("The path specified cannot be used at this time.  "); break;
		case ERROR_IS_SUBST_TARGET : str = 	_T("An attempt was made to join or substitute a drive for which a directory on the drive is the target of a previous substitute.  "); break;
		case ERROR_SYSTEM_TRACE : str = 	_T("System trace information was not specified in your CONFIG.SYS file, or tracing is disallowed.  "); break;
		case ERROR_INVALID_EVENT_COUNT : str = 	_T("The number of specified semaphore events for DosMuxSemWait is not correct.  "); break;
		case ERROR_TOO_MANY_MUXWAITERS : str = 	_T("DosMuxSemWait did not execute; too many semaphores are already set.  "); break;
		case ERROR_INVALID_LIST_FORMAT : str = 	_T("The DosMuxSemWait list is not correct.  "); break;
		case ERROR_LABEL_TOO_LONG : str = 	_T("The volume label you entered exceeds the label character limit of the target file system.  "); break;
		case ERROR_TOO_MANY_TCBS : str = 	_T("Cannot create another thread.  "); break;
		case ERROR_SIGNAL_REFUSED : str = 	_T("The recipient process has refused the signal.  "); break;
		case ERROR_DISCARDED : str = 	_T("The segment is already discarded and cannot be locked.  "); break;
		case ERROR_NOT_LOCKED : str = 	_T("The segment is already unlocked.  "); break;
		case ERROR_BAD_THREADID_ADDR : str = 	_T("The address for the thread ID is not correct.  "); break;
		case ERROR_BAD_ARGUMENTS : str = 	_T("The argument string passed to DosExecPgm is not correct.  "); break;
		case ERROR_BAD_PATHNAME : str = 	_T("The specified path is invalid.  "); break;
		case ERROR_SIGNAL_PENDING : str = 	_T("A signal is already pending.  "); break;
		case ERROR_MAX_THRDS_REACHED : str = 	_T("No more threads can be created in the system.  "); break;
		case ERROR_LOCK_FAILED : str = 	_T("Unable to lock a region of a file.  "); break;
		case ERROR_BUSY : str = 	_T("The requested resource is in use.  "); break;
		case ERROR_CANCEL_VIOLATION : str = 	_T("A lock request was not outstanding for the supplied cancel region.  "); break;
		case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED : str = 	_T("The file system does not support atomic changes to the lock type.  "); break;
		case ERROR_INVALID_SEGMENT_NUMBER : str = 	_T("The system detected a segment number that was not correct.  "); break;
		case ERROR_INVALID_ORDINAL : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_ALREADY_EXISTS : str = 	_T("Cannot create a file when that file already exists.  "); break;
		case ERROR_INVALID_FLAG_NUMBER : str = 	_T("The flag passed is not correct.  "); break;
		case ERROR_SEM_NOT_FOUND : str = 	_T("The specified system semaphore name was not found.  "); break;
		case ERROR_INVALID_STARTING_CODESEG : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_INVALID_STACKSEG : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_INVALID_MODULETYPE : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_INVALID_EXE_SIGNATURE : str = 	_T("Cannot run %1 in Win32 mode.  "); break;
		case ERROR_EXE_MARKED_INVALID : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_BAD_EXE_FORMAT : str = 	_T("is not a valid Win32 application.  "); break;
		case ERROR_ITERATED_DATA_EXCEEDS_64k : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_INVALID_MINALLOCSIZE : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_DYNLINK_FROM_INVALID_RING : str = 	_T("The operating system cannot run this application program.  "); break;
		case ERROR_IOPL_NOT_ENABLED : str = 	_T("The operating system is not presently configured to run this application.  "); break;
		case ERROR_INVALID_SEGDPL : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_AUTODATASEG_EXCEEDS_64k : str = 	_T("The operating system cannot run this application program.  "); break;
		case ERROR_RING2SEG_MUST_BE_MOVABLE : str = 	_T("The code segment cannot be greater than or equal to 64K.  "); break;
		case ERROR_RELOC_CHAIN_XEEDS_SEGLIM : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_INFLOOP_IN_RELOC_CHAIN : str = 	_T("The operating system cannot run %1.  "); break;
		case ERROR_ENVVAR_NOT_FOUND : str = 	_T("The system could not find the environment option that was entered.  "); break;
		case ERROR_NO_SIGNAL_SENT : str = 	_T("No process in the command subtree has a signal handler.  "); break;
		case ERROR_FILENAME_EXCED_RANGE : str = 	_T("The filename or extension is too long.  "); break;
		case ERROR_RING2_STACK_IN_USE : str = 	_T("The ring 2 stack is in use.  "); break;
		case ERROR_META_EXPANSION_TOO_LONG : str = 	_T("The global filename characters, * or ?, are entered incorrectly or too many global filename characters are specified.  "); break;
		case ERROR_INVALID_SIGNAL_NUMBER : str = 	_T("The signal being posted is not correct.  "); break;
		case ERROR_THREAD_1_INACTIVE : str = 	_T("The signal handler cannot be set.  "); break;
		case ERROR_LOCKED : str = 	_T("The segment is locked and cannot be reallocated.  "); break;
		case ERROR_TOO_MANY_MODULES : str = 	_T("Too many dynamic-link modules are attached to this program or dynamic-link module.  "); break;
		case ERROR_NESTING_NOT_ALLOWED : str = 	_T("Can't nest calls to LoadModule.  "); break;
		case ERROR_EXE_MACHINE_TYPE_MISMATCH : str = 	_T("The image file %1 is valid, but is for a machine type other than the current machine.  "); break;
		case ERROR_BAD_PIPE : str = 	_T("The pipe state is invalid.  "); break;
		case ERROR_PIPE_BUSY : str = 	_T("All pipe instances are busy.  "); break;
		case ERROR_NO_DATA : str = 	_T("The pipe is being closed.  "); break;
		case ERROR_PIPE_NOT_CONNECTED : str = 	_T("No process is on the other end of the pipe.  "); break;
		case ERROR_MORE_DATA : str = 	_T("More data is available.  "); break;
		case ERROR_VC_DISCONNECTED : str = 	_T("The session was canceled.  "); break;
		case ERROR_INVALID_EA_NAME : str = 	_T("The specified extended attribute name was invalid.  "); break;
		case ERROR_EA_LIST_INCONSISTENT : str = 	_T("The extended attributes are inconsistent.  "); break;
		case ERROR_NO_MORE_ITEMS : str = 	_T("No more data is available.  "); break;
		case ERROR_CANNOT_COPY : str = 	_T("The copy functions cannot be used.  "); break;
		case ERROR_DIRECTORY : str = 	_T("The directory name is invalid.  "); break;
		case ERROR_EAS_DIDNT_FIT : str = 	_T("The extended attributes did not fit in the buffer.  "); break;
		case ERROR_EA_FILE_CORRUPT : str = 	_T("The extended attribute file on the mounted file system is corrupt.  "); break;
		case ERROR_EA_TABLE_FULL : str = 	_T("The extended attribute table file is full.  "); break;
		case ERROR_INVALID_EA_HANDLE : str = 	_T("The specified extended attribute handle is invalid.  "); break;
		case ERROR_EAS_NOT_SUPPORTED : str = 	_T("The mounted file system does not support extended attributes.  "); break;
		case ERROR_NOT_OWNER : str = 	_T("Attempt to release mutex not owned by caller.  "); break;
		case ERROR_TOO_MANY_POSTS : str = 	_T("Too many posts were made to a semaphore.  "); break;
		case ERROR_PARTIAL_COPY : str = 	_T("Only part of a ReadProcessMemoty or WriteProcessMemory request was completed.  "); break;
		case ERROR_OPLOCK_NOT_GRANTED : str = 	_T("The oplock request is denied.  "); break;
		case ERROR_INVALID_OPLOCK_PROTOCOL : str = 	_T("An invalid oplock acknowledgment was received by the system.  "); break;
		case ERROR_MR_MID_NOT_FOUND : str = 	_T("The system cannot find message text for message number 0x%1 in the message file for %2.  "); break;
		case ERROR_INVALID_ADDRESS : str = 	_T("Attempt to access invalid address.  "); break;
		case ERROR_ARITHMETIC_OVERFLOW : str = 	_T("Arithmetic result exceeded 32 bits.  "); break;
		case ERROR_PIPE_CONNECTED : str = 	_T("There is a process on other end of the pipe.  "); break;
		case ERROR_PIPE_LISTENING : str = 	_T("Waiting for a process to open the other end of the pipe.  "); break;
		case ERROR_EA_ACCESS_DENIED : str = 	_T("Access to the extended attribute was denied.  "); break;
		case ERROR_OPERATION_ABORTED : str = 	_T("The I/O operation has been aborted because of either a thread exit or an application request.  "); break;
		case ERROR_IO_INCOMPLETE : str = 	_T("Overlapped I/O event is not in a signaled state.  "); break;
		case ERROR_IO_PENDING : str = 	_T("Overlapped I/O operation is in progress.  "); break;
		case ERROR_NOACCESS : str = 	_T("Invalid access to memory location.  "); break;
		case ERROR_SWAPERROR : str = 	_T("Error performing inpage operation.  "); break;
		case ERROR_STACK_OVERFLOW : str = 	_T("Recursion too deep; the stack overflowed.  "); break;
		case ERROR_INVALID_MESSAGE : str = 	_T("The window cannot act on the sent message.  "); break;
		case ERROR_CAN_NOT_COMPLETE : str = 	_T("Cannot complete this function.  "); break;
		case ERROR_INVALID_FLAGS : str = 	_T("Invalid flags.  "); break;
		case ERROR_UNRECOGNIZED_VOLUME : str = 	_T("The volume does not contain a recognized file system. Please make sure that all required file system drivers are loaded and that the volume is not corrupted.  "); break;
		case ERROR_FILE_INVALID : str = 	_T("The volume for a file has been externally altered so that the opened file is no longer valid.  "); break;
		case ERROR_FULLSCREEN_MODE : str = 	_T("The requested operation cannot be performed in full-screen mode.  "); break;
		case ERROR_NO_TOKEN : str = 	_T("An attempt was made to reference a token that does not exist.  "); break;
		case ERROR_BADDB : str = 	_T("The configuration registry database is corrupt.  "); break;
		case ERROR_BADKEY : str = 	_T("The configuration registry key is invalid.  "); break;
		case ERROR_CANTOPEN : str = 	_T("The configuration registry key could not be opened.  "); break;
		case ERROR_CANTREAD : str = 	_T("The configuration registry key could not be read.  "); break;
		case ERROR_CANTWRITE : str = 	_T("The configuration registry key could not be written.  "); break;
		case ERROR_REGISTRY_RECOVERED : str = 	_T("One of the files in the registry database had to be recovered by use of a log or alternate copy. The recovery was successful.  "); break;
		case ERROR_REGISTRY_CORRUPT : str = 	_T("The registry is corrupted. The structure of one of the files that contains registry data is corrupted, or the system's image of the file in memory is corrupted, or the file could not be recovered because the alternate copy or log was absent or corrupted.  "); break;
		case ERROR_REGISTRY_IO_FAILED : str = 	_T("An I/O operation initiated by the registry failed unrecoverably. The registry could not read in, or write out, or flush, one of the files that contain the system's image of the registry.  "); break;
		case ERROR_NOT_REGISTRY_FILE : str = 	_T("The system has attempted to load or restore a file into the registry, but the specified file is not in a registry file format.  "); break;
		case ERROR_KEY_DELETED : str = 	_T("Illegal operation attempted on a registry key that has been marked for deletion.  "); break;
		case ERROR_NO_LOG_SPACE : str = 	_T("System could not allocate the required space in a registry log.  "); break;
		case ERROR_KEY_HAS_CHILDREN : str = 	_T("Cannot create a symbolic link in a registry key that already has subkeys or values.  "); break;
		case ERROR_CHILD_MUST_BE_VOLATILE : str = 	_T("Cannot create a stable subkey under a volatile parent key.  "); break;
		case ERROR_NOTIFY_ENUM_DIR : str = 	_T("A notify change request is being completed and the information is not being returned in the caller's buffer. The caller now needs to enumerate the files to find the changes.  "); break;
		case ERROR_DEPENDENT_SERVICES_RUNNING : str = 	_T("A stop control has been sent to a service that other running services are dependent on.  "); break;
		case ERROR_INVALID_SERVICE_CONTROL : str = 	_T("The requested control is not valid for this service.  "); break;
		case ERROR_SERVICE_REQUEST_TIMEOUT : str = 	_T("The service did not respond to the start or control request in a timely fashion.  "); break;
		case ERROR_SERVICE_NO_THREAD : str = 	_T("A thread could not be created for the service.  "); break;
		case ERROR_SERVICE_DATABASE_LOCKED : str = 	_T("The service database is locked.  "); break;
		case ERROR_SERVICE_ALREADY_RUNNING : str = 	_T("An instance of the service is already running.  "); break;
		case ERROR_INVALID_SERVICE_ACCOUNT : str = 	_T("The account name is invalid or does not exist.  "); break;
		case ERROR_SERVICE_DISABLED : str = 	_T("The service cannot be started, either because it is disabled or because it has no enabled devices associated with it.  "); break;
		case ERROR_CIRCULAR_DEPENDENCY : str = 	_T("Circular service dependency was specified.  "); break;
		case ERROR_SERVICE_DOES_NOT_EXIST : str = 	_T("The specified service does not exist as an installed service.  "); break;
		case ERROR_SERVICE_CANNOT_ACCEPT_CTRL : str = 	_T("The service cannot accept control messages at this time.  "); break;
		case ERROR_SERVICE_NOT_ACTIVE : str = 	_T("The service has not been started.  "); break;
		case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT : str = 	_T("The service process could not connect to the service controller.  "); break;
		case ERROR_EXCEPTION_IN_SERVICE : str = 	_T("An exception occurred in the service when handling the control request.  "); break;
		case ERROR_DATABASE_DOES_NOT_EXIST : str = 	_T("The database specified does not exist.  "); break;
		case ERROR_SERVICE_SPECIFIC_ERROR : str = 	_T("The service has returned a service-specific error code.  "); break;
		case ERROR_PROCESS_ABORTED : str = 	_T("The process terminated unexpectedly.  "); break;
		case ERROR_SERVICE_DEPENDENCY_FAIL : str = 	_T("The dependency service or group failed to start.  "); break;
		case ERROR_SERVICE_LOGON_FAILED : str = 	_T("The service did not start due to a logon failure.  "); break;
		case ERROR_SERVICE_START_HANG : str = 	_T("After starting, the service hung in a start-pending state.  "); break;
		case ERROR_INVALID_SERVICE_LOCK : str = 	_T("The specified service database lock is invalid.  "); break;
		case ERROR_SERVICE_MARKED_FOR_DELETE : str = 	_T("The specified service has been marked for deletion.  "); break;
		case ERROR_SERVICE_EXISTS : str = 	_T("The specified service already exists.  "); break;
		case ERROR_ALREADY_RUNNING_LKG : str = 	_T("The system is currently running with the last-known-good configuration.  "); break;
		case ERROR_SERVICE_DEPENDENCY_DELETED : str = 	_T("The dependency service does not exist or has been marked for deletion.  "); break;
		case ERROR_BOOT_ALREADY_ACCEPTED : str = 	_T("The current boot has already been accepted for use as the last-known-good control set.  "); break;
		case ERROR_SERVICE_NEVER_STARTED : str = 	_T("No attempts to start the service have been made since the last boot.  "); break;
		case ERROR_DUPLICATE_SERVICE_NAME : str = 	_T("The name is already in use as either a service name or a service display name.  "); break;
		case ERROR_DIFFERENT_SERVICE_ACCOUNT : str = 	_T("The account specified for this service is different from the account specified for other services running in the same process.  "); break;
		case ERROR_CANNOT_DETECT_DRIVER_FAILURE : str = 	_T("Failure actions can only be set for Win32 services, not for drivers.  "); break;
		case ERROR_CANNOT_DETECT_PROCESS_ABORT : str = 	_T("This service runs in the same process as the service control manager. Therefore, the service control manager cannot take action if this service's process terminates unexpectedly.  "); break;
		case ERROR_NO_RECOVERY_PROGRAM : str = 	_T("No recovery program has been configured for this service.  "); break;
		case ERROR_END_OF_MEDIA : str = 	_T("The physical end of the tape has been reached.  "); break;
		case ERROR_FILEMARK_DETECTED : str = 	_T("A tape access reached a filemark.  "); break;
		case ERROR_BEGINNING_OF_MEDIA : str = 	_T("The beginning of the tape or a partition was encountered.  "); break;
		case ERROR_SETMARK_DETECTED : str = 	_T("A tape access reached the end of a set of files.  "); break;
		case ERROR_NO_DATA_DETECTED : str = 	_T("No more data is on the tape.  "); break;
		case ERROR_PARTITION_FAILURE : str = 	_T("Tape could not be partitioned.  "); break;
		case ERROR_INVALID_BLOCK_LENGTH : str = 	_T("When accessing a new tape of a multivolume partition, the current blocksize is incorrect.  "); break;
		case ERROR_DEVICE_NOT_PARTITIONED : str = 	_T("Tape partition information could not be found when loading a tape.  "); break;
		case ERROR_UNABLE_TO_LOCK_MEDIA : str = 	_T("Unable to lock the media eject mechanism.  "); break;
		case ERROR_UNABLE_TO_UNLOAD_MEDIA : str = 	_T("Unable to unload the media.  "); break;
		case ERROR_MEDIA_CHANGED : str = 	_T("The media in the drive may have changed.  "); break;
		case ERROR_BUS_RESET : str = 	_T("The I/O bus was reset.  "); break;
		case ERROR_NO_MEDIA_IN_DRIVE : str = 	_T("No media in drive.  "); break;
		case ERROR_NO_UNICODE_TRANSLATION : str = 	_T("No mapping for the Unicode character exists in the target multi-byte code page.  "); break;
		case ERROR_DLL_INIT_FAILED : str = 	_T("A dynamic link library (DLL) initialization routine failed.  "); break;
		case ERROR_SHUTDOWN_IN_PROGRESS : str = 	_T("A system shutdown is in progress.  "); break;
		case ERROR_NO_SHUTDOWN_IN_PROGRESS : str = 	_T("Unable to abort the system shutdown because no shutdown was in progress.  "); break;
		case ERROR_IO_DEVICE : str = 	_T("The request could not be performed because of an I/O device error.  "); break;
		case ERROR_SERIAL_NO_DEVICE : str = 	_T("No serial device was successfully initialized. The serial driver will unload.  "); break;
		case ERROR_IRQ_BUSY : str = 	_T("Unable to open a device that was sharing an interrupt request (IRQ) with other devices. At least one other device that uses that IRQ was already opened.  "); break;
		case ERROR_MORE_WRITES : str = 	_T("A serial I/O operation was completed by another write to the serial port. The IOCTL_SERIAL_XOFF_COUNTER reached zero.)  "); break;
		case ERROR_COUNTER_TIMEOUT : str = 	_T("A serial I/O operation completed because the timeout period expired. The IOCTL_SERIAL_XOFF_COUNTER did not reach zero.)  "); break;
		case ERROR_FLOPPY_ID_MARK_NOT_FOUND : str = 	_T("No ID address mark was found on the floppy disk.  "); break;
		case ERROR_FLOPPY_WRONG_CYLINDER : str = 	_T("Mismatch between the floppy disk sector ID field and the floppy disk controller track address.  "); break;
		case ERROR_FLOPPY_UNKNOWN_ERROR : str = 	_T("The floppy disk controller reported an error that is not recognized by the floppy disk driver.  "); break;
		case ERROR_FLOPPY_BAD_REGISTERS : str = 	_T("The floppy disk controller returned inconsistent results in its registers.  "); break;
		case ERROR_DISK_RECALIBRATE_FAILED : str = 	_T("While accessing the hard disk, a recalibrate operation failed, even after retries.  "); break;
		case ERROR_DISK_OPERATION_FAILED : str = 	_T("While accessing the hard disk, a disk operation failed even after retries.  "); break;
		case ERROR_DISK_RESET_FAILED : str = 	_T("While accessing the hard disk, a disk controller reset was needed, but even that failed.  "); break;
		case ERROR_EOM_OVERFLOW : str = 	_T("Physical end of tape encountered.  "); break;
		case ERROR_NOT_ENOUGH_SERVER_MEMORY : str = 	_T("Not enough server storage is available to process this command.  "); break;
		case ERROR_POSSIBLE_DEADLOCK : str = 	_T("A potential deadlock condition has been detected.  "); break;
		case ERROR_MAPPED_ALIGNMENT : str = 	_T("The base address or the file offset specified does not have the proper alignment.  "); break;
		case ERROR_SET_POWER_STATE_VETOED : str = 	_T("An attempt to change the system power state was vetoed by another application or driver.  "); break;
		case ERROR_SET_POWER_STATE_FAILED : str = 	_T("The system BIOS failed an attempt to change the system power state.  "); break;
		case ERROR_TOO_MANY_LINKS : str = 	_T("An attempt was made to create more links on a file than the file system supports.  "); break;
		case ERROR_OLD_WIN_VERSION : str = 	_T("The specified program requires a newer version of Windows.  "); break;
		case ERROR_APP_WRONG_OS : str = 	_T("The specified program is not a Windows or MS-DOS program.  "); break;
		case ERROR_SINGLE_INSTANCE_APP : str = 	_T("Cannot start more than one instance of the specified program.  "); break;
		case ERROR_RMODE_APP : str = 	_T("The specified program was written for an earlier version of Windows.  "); break;
		case ERROR_INVALID_DLL : str = 	_T("One of the library files needed to run this application is damaged.  "); break;
		case ERROR_NO_ASSOCIATION : str = 	_T("No application is associated with the specified file for this operation.  "); break;
		case ERROR_DDE_FAIL : str = 	_T("An error occurred in sending the command to the application.  "); break;
		case ERROR_DLL_NOT_FOUND : str = 	_T("One of the library files needed to run this application cannot be found.  "); break;
		case ERROR_NO_MORE_USER_HANDLES : str = 	_T("The current process has used all of its system allowance of handles for Window Manager objects.  "); break;
		case ERROR_MESSAGE_SYNC_ONLY : str = 	_T("The message can be used only with synchronous operations.  "); break;
		case ERROR_SOURCE_ELEMENT_EMPTY : str = 	_T("The indicated source element has no media.  "); break;
		case ERROR_DESTINATION_ELEMENT_FULL : str = 	_T("The indicated destination element already contains media.  "); break;
		case ERROR_ILLEGAL_ELEMENT_ADDRESS : str = 	_T("The indicated element does not exist.  "); break;
		case ERROR_MAGAZINE_NOT_PRESENT : str = 	_T("The indicated element is part of a magazine that is not present.  "); break;
		case ERROR_DEVICE_REINITIALIZATION_NEEDED : str = 	_T("The indicated device requires reinitialization due to hardware errors.  "); break;
		case ERROR_DEVICE_REQUIRES_CLEANING : str = 	_T("The device has indicated that cleaning is required before further operations are attempted.  "); break;
		case ERROR_DEVICE_DOOR_OPEN : str = 	_T("The device has indicated that its door is open.  "); break;
		case ERROR_DEVICE_NOT_CONNECTED : str = 	_T("The device is not connected.  "); break;
		case ERROR_NOT_FOUND : str = 	_T("Element not found.  "); break;
		case ERROR_NO_MATCH : str = 	_T("There was no match for the specified key in the index.  "); break;
		case ERROR_SET_NOT_FOUND : str = 	_T("The property set specified does not exist on the object.  "); break;
		case ERROR_POINT_NOT_FOUND : str = 	_T("The point passed to GetMouseMovePoints is not in the buffer.  "); break;
		case ERROR_NO_TRACKING_SERVICE : str = 	_T("The tracking (workstation) service is not running.  "); break;
		case ERROR_NO_VOLUME_ID : str = 	_T("The Volume ID could not be found.  "); break;
		case ERROR_BAD_DEVICE : str = 	_T("The specified device name is invalid.  "); break;
		case ERROR_CONNECTION_UNAVAIL : str = 	_T("The device is not currently connected but it is a remembered connection.  "); break;
		case ERROR_DEVICE_ALREADY_REMEMBERED : str = 	_T("An attempt was made to remember a device that had previously been remembered.  "); break;
		case ERROR_NO_NET_OR_BAD_PATH : str = 	_T("No network provider accepted the given network path.  "); break;
		case ERROR_BAD_PROVIDER : str = 	_T("The specified network provider name is invalid.  "); break;
		case ERROR_CANNOT_OPEN_PROFILE : str = 	_T("Unable to open the network connection profile.  "); break;
		case ERROR_BAD_PROFILE : str = 	_T("The network connection profile is corrupted.  "); break;
		case ERROR_NOT_CONTAINER : str = 	_T("Cannot enumerate a noncontainer.  "); break;
		case ERROR_EXTENDED_ERROR : str = 	_T("An extended error has occurred.  "); break;
		case ERROR_INVALID_GROUPNAME : str = 	_T("The format of the specified group name is invalid.  "); break;
		case ERROR_INVALID_COMPUTERNAME : str = 	_T("The format of the specified computer name is invalid.  "); break;
		case ERROR_INVALID_EVENTNAME : str = 	_T("The format of the specified event name is invalid.  "); break;
		case ERROR_INVALID_DOMAINNAME : str = 	_T("The format of the specified domain name is invalid.  "); break;
		case ERROR_INVALID_SERVICENAME : str = 	_T("The format of the specified service name is invalid.  "); break;
		case ERROR_INVALID_NETNAME : str = 	_T("The format of the specified network name is invalid.  "); break;
		case ERROR_INVALID_SHARENAME : str = 	_T("The format of the specified share name is invalid.  "); break;
		case ERROR_INVALID_PASSWORDNAME : str = 	_T("The format of the specified password is invalid.  "); break;
		case ERROR_INVALID_MESSAGENAME : str = 	_T("The format of the specified message name is invalid.  "); break;
		case ERROR_INVALID_MESSAGEDEST : str = 	_T("The format of the specified message destination is invalid.  "); break;
		case ERROR_SESSION_CREDENTIAL_CONFLICT : str = 	_T("The credentials supplied conflict with an existing set of credentials.  "); break;
		case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED : str = 	_T("An attempt was made to establish a session to a network server, but there are already too many sessions established to that server.  "); break;
		case ERROR_DUP_DOMAINNAME : str = 	_T("The workgroup or domain name is already in use by another computer on the network.  "); break;
		case ERROR_NO_NETWORK : str = 	_T("The network is not present or not started.  "); break;
		case ERROR_CANCELLED : str = 	_T("The operation was canceled by the user.  "); break;
		case ERROR_USER_MAPPED_FILE : str = 	_T("The requested operation cannot be performed on a file with a user-mapped section open.  "); break;
		case ERROR_CONNECTION_REFUSED : str = 	_T("The remote system refused the network connection.  "); break;
		case ERROR_GRACEFUL_DISCONNECT : str = 	_T("The network connection was gracefully closed.  "); break;
		case ERROR_ADDRESS_ALREADY_ASSOCIATED : str = 	_T("The network transport endpoint already has an address associated with it.  "); break;
		case ERROR_ADDRESS_NOT_ASSOCIATED : str = 	_T("An address has not yet been associated with the network endpoint.  "); break;
		case ERROR_CONNECTION_INVALID : str = 	_T("An operation was attempted on a nonexistent network connection.  "); break;
		case ERROR_CONNECTION_ACTIVE : str = 	_T("An invalid operation was attempted on an active network connection.  "); break;
		case ERROR_NETWORK_UNREACHABLE : str = 	_T("The remote network is not reachable by the transport.  "); break;
		case ERROR_HOST_UNREACHABLE : str = 	_T("The remote system is not reachable by the transport.  "); break;
		case ERROR_PROTOCOL_UNREACHABLE : str = 	_T("The remote system does not support the transport protocol.  "); break;
		case ERROR_PORT_UNREACHABLE : str = 	_T("No service is operating at the destination network endpoint on the remote system.  "); break;
		case ERROR_REQUEST_ABORTED : str = 	_T("The request was aborted.  "); break;
		case ERROR_CONNECTION_ABORTED : str = 	_T("The network connection was aborted by the local system.  "); break;
		case ERROR_RETRY : str = 	_T("The operation could not be completed. A retry should be performed.  "); break;
		case ERROR_CONNECTION_COUNT_LIMIT : str = 	_T("A connection to the server could not be made because the limit on the number of concurrent connections for this account has been reached.  "); break;
		case ERROR_LOGIN_TIME_RESTRICTION : str = 	_T("Attempting to log in during an unauthorized time of day for this account.  "); break;
		case ERROR_LOGIN_WKSTA_RESTRICTION : str = 	_T("The account is not authorized to log in from this station.  "); break;
		case ERROR_INCORRECT_ADDRESS : str = 	_T("The network address could not be used for the operation requested.  "); break;
		case ERROR_ALREADY_REGISTERED : str = 	_T("The service is already registered.  "); break;
		case ERROR_SERVICE_NOT_FOUND : str = 	_T("The specified service does not exist.  "); break;
		case ERROR_NOT_AUTHENTICATED : str = 	_T("The operation being requested was not performed because the user has not been authenticated.  "); break;
		case ERROR_NOT_LOGGED_ON : str = 	_T("The operation being requested was not performed because the user has not logged on to the network. The specified service does not exist.  "); break;
		case ERROR_CONTINUE : str = 	_T("Continue with work in progress.  "); break;
		case ERROR_ALREADY_INITIALIZED : str = 	_T("An attempt was made to perform an initialization operation when initialization has already been completed.  "); break;
		case ERROR_NO_MORE_DEVICES : str = 	_T("No more local devices.  "); break;
		case ERROR_NO_SUCH_SITE : str = 	_T("The specified site does not exist.  "); break;
		case ERROR_DOMAIN_CONTROLLER_EXISTS : str = 	_T("A domain controller with the specified name already exists.  "); break;
		case ERROR_DS_NOT_INSTALLED : str = 	_T("An error occurred while installing the Windows NT directory service. Please view the event log for more information.  "); break;
		case ERROR_NOT_ALL_ASSIGNED : str = 	_T("Not all privileges referenced are assigned to the caller.  "); break;
		case ERROR_SOME_NOT_MAPPED : str = 	_T("Some mapping between account names and security IDs was not done.  "); break;
		case ERROR_NO_QUOTAS_FOR_ACCOUNT : str = 	_T("No system quota limits are specifically set for this account.  "); break;
		case ERROR_LOCAL_USER_SESSION_KEY : str = 	_T("No encryption key is available. A well-known encryption key was returned.  "); break;
		case ERROR_NULL_LM_PASSWORD : str = 	_T("The Windows NT password is too complex to be converted to a LAN Manager password. The LAN Manager password returned is a NULL string.  "); break;
		case ERROR_UNKNOWN_REVISION : str = 	_T("The revision level is unknown.  "); break;
		case ERROR_REVISION_MISMATCH : str = 	_T("Indicates two revision levels are incompatible.  "); break;
		case ERROR_INVALID_OWNER : str = 	_T("This security ID may not be assigned as the owner of this object.  "); break;
		case ERROR_INVALID_PRIMARY_GROUP : str = 	_T("This security ID may not be assigned as the primary group of an object.  "); break;
		case ERROR_NO_IMPERSONATION_TOKEN : str = 	_T("An attempt has been made to operate on an impersonation token by a thread that is not currently impersonating a client.  "); break;
		case ERROR_CANT_DISABLE_MANDATORY : str = 	_T("The group may not be disabled.  "); break;
		case ERROR_NO_LOGON_SERVERS : str = 	_T("There are currently no logon servers available to service the logon request.  "); break;
		case ERROR_NO_SUCH_LOGON_SESSION : str = 	_T("A specified logon session does not exist. It may already have been terminated.  "); break;
		case ERROR_NO_SUCH_PRIVILEGE : str = 	_T("A specified privilege does not exist.  "); break;
		case ERROR_PRIVILEGE_NOT_HELD : str = 	_T("A required privilege is not held by the client.  "); break;
		case ERROR_INVALID_ACCOUNT_NAME : str = 	_T("The name provided is not a properly formed account name.  "); break;
		case ERROR_USER_EXISTS : str = 	_T("The specified user already exists.  "); break;
		case ERROR_NO_SUCH_USER : str = 	_T("The specified user does not exist.  "); break;
		case ERROR_GROUP_EXISTS : str = 	_T("The specified group already exists.  "); break;
		case ERROR_NO_SUCH_GROUP : str = 	_T("The specified group does not exist.  "); break;
		case ERROR_MEMBER_IN_GROUP : str = 	_T("Either the specified user account is already a member of the specified group, or the specified group cannot be deleted because it contains a member.  "); break;
		case ERROR_MEMBER_NOT_IN_GROUP : str = 	_T("The specified user account is not a member of the specified group account.  "); break;
		case ERROR_LAST_ADMIN : str = 	_T("The last remaining administration account cannot be disabled or deleted.  "); break;
		case ERROR_WRONG_PASSWORD : str = 	_T("Unable to update the password. The value provided as the current password is incorrect.  "); break;
		case ERROR_ILL_FORMED_PASSWORD : str = 	_T("Unable to update the password. The value provided for the new password contains values that are not allowed in passwords.  "); break;
		case ERROR_PASSWORD_RESTRICTION : str = 	_T("Unable to update the password because a password update rule has been violated.  "); break;
		case ERROR_LOGON_FAILURE : str = 	_T("Logon failure: str =  unknown user name or bad password.  "); break;
		case ERROR_ACCOUNT_RESTRICTION : str = 	_T("Logon failure: str =  user account restriction.  "); break;
		case ERROR_INVALID_LOGON_HOURS : str = 	_T("Logon failure: str =  account logon time restriction violation.  "); break;
		case ERROR_INVALID_WORKSTATION : str = 	_T("Logon failure: str =  user not allowed to log on to this computer.  "); break;
		case ERROR_PASSWORD_EXPIRED : str = 	_T("Logon failure: str =  the specified account password has expired.  "); break;
		case ERROR_ACCOUNT_DISABLED : str = 	_T("Logon failure: str =  account currently disabled.  "); break;
		case ERROR_NONE_MAPPED : str = 	_T("No mapping between account names and security IDs was done.  "); break;
		case ERROR_TOO_MANY_LUIDS_REQUESTED : str = 	_T("Too many local user identifiers (LUIDs) were requested at one time.  "); break;
		case ERROR_LUIDS_EXHAUSTED : str = 	_T("No more local user identifiers (LUIDs) are available.  "); break;
		case ERROR_INVALID_SUB_AUTHORITY : str = 	_T("The subauthority part of a security ID is invalid for this particular use.  "); break;
		case ERROR_INVALID_ACL : str = 	_T("The access control list (ACL) structure is invalid.  "); break;
		case ERROR_INVALID_SID : str = 	_T("The security ID structure is invalid.  "); break;
		case ERROR_INVALID_SECURITY_DESCR : str = 	_T("The security descriptor structure is invalid.  "); break;
		case ERROR_BAD_INHERITANCE_ACL : str = 	_T("The inherited access control list (ACL) or access control entry (ACE) could not be built.  "); break;
		case ERROR_SERVER_DISABLED : str = 	_T("The server is currently disabled.  "); break;
		case ERROR_SERVER_NOT_DISABLED : str = 	_T("The server is currently enabled.  "); break;
		case ERROR_INVALID_ID_AUTHORITY : str = 	_T("The value provided was an invalid value for an identifier authority.  "); break;
		case ERROR_ALLOTTED_SPACE_EXCEEDED : str = 	_T("No more memory is available for security information updates.  "); break;
		case ERROR_INVALID_GROUP_ATTRIBUTES : str = 	_T("The specified attributes are invalid, or incompatible with the attributes for the group as a whole.  "); break;
		case ERROR_BAD_IMPERSONATION_LEVEL : str = 	_T("Either a required impersonation level was not provided, or the provided impersonation level is invalid.  "); break;
		case ERROR_CANT_OPEN_ANONYMOUS : str = 	_T("Cannot open an anonymous level security token.  "); break;
		case ERROR_BAD_VALIDATION_CLASS : str = 	_T("The validation information class requested was invalid.  "); break;
		case ERROR_BAD_TOKEN_TYPE : str = 	_T("The type of the token is inappropriate for its attempted use.  "); break;
		case ERROR_NO_SECURITY_ON_OBJECT : str = 	_T("Unable to perform a security operation on an object that has no associated security.  "); break;
		case ERROR_CANT_ACCESS_DOMAIN_INFO : str = 	_T("Indicates a Windows NT Server could not be contacted or that objects within the domain are protected such that necessary information could not be retrieved.  "); break;
		case ERROR_INVALID_SERVER_STATE : str = 	_T("The security account manager (SAM) or local security authority (LSA) server was in the wrong state to perform the security operation.  "); break;
		case ERROR_INVALID_DOMAIN_STATE : str = 	_T("The domain was in the wrong state to perform the security operation.  "); break;
		case ERROR_INVALID_DOMAIN_ROLE : str = 	_T("This operation is only allowed for the Primary Domain Controller of the domain.  "); break;
		case ERROR_NO_SUCH_DOMAIN : str = 	_T("The specified domain did not exist.  "); break;
		case ERROR_DOMAIN_EXISTS : str = 	_T("The specified domain already exists.  "); break;
		case ERROR_DOMAIN_LIMIT_EXCEEDED : str = 	_T("An attempt was made to exceed the limit on the number of domains per server.  "); break;
		case ERROR_INTERNAL_DB_CORRUPTION : str = 	_T("Unable to complete the requested operation because of either a catastrophic media failure or a data structure corruption on the disk.  "); break;
		case ERROR_INTERNAL_ERROR : str = 	_T("The security account database contains an internal inconsistency.  "); break;
		case ERROR_GENERIC_NOT_MAPPED : str = 	_T("Generic access types were contained in an access mask which should already be mapped to nongeneric types.  "); break;
		case ERROR_BAD_DESCRIPTOR_FORMAT : str = 	_T("A security descriptor is not in the right format (absolute or self-relative).  "); break;
		case ERROR_NOT_LOGON_PROCESS : str = 	_T("The requested action is restricted for use by logon processes only. The calling process has not registered as a logon process.  "); break;
		case ERROR_LOGON_SESSION_EXISTS : str = 	_T("Cannot start a new logon session with an ID that is already in use.  "); break;
		case ERROR_NO_SUCH_PACKAGE : str = 	_T("A specified authentication package is unknown.  "); break;
		case ERROR_BAD_LOGON_SESSION_STATE : str = 	_T("The logon session is not in a state that is consistent with the requested operation.  "); break;
		case ERROR_LOGON_SESSION_COLLISION : str = 	_T("The logon session ID is already in use.  "); break;
		case ERROR_INVALID_LOGON_TYPE : str = 	_T("A logon request contained an invalid logon type value.  "); break;
		case ERROR_CANNOT_IMPERSONATE : str = 	_T("Unable to impersonate using a named pipe until data has been read from that pipe.  "); break;
		case ERROR_RXACT_INVALID_STATE : str = 	_T("The transaction state of a registry subtree is incompatible with the requested operation.  "); break;
		case ERROR_RXACT_COMMIT_FAILURE : str = 	_T("An internal security database corruption has been encountered.  "); break;
		case ERROR_SPECIAL_ACCOUNT : str = 	_T("Cannot perform this operation on built-in accounts.  "); break;
		case ERROR_SPECIAL_GROUP : str = 	_T("Cannot perform this operation on this built-in special group.  "); break;
		case ERROR_SPECIAL_USER : str = 	_T("Cannot perform this operation on this built-in special user.  "); break;
		case ERROR_MEMBERS_PRIMARY_GROUP : str = 	_T("The user cannot be removed from a group because the group is currently the user's primary group.  "); break;
		case ERROR_TOKEN_ALREADY_IN_USE : str = 	_T("The token is already in use as a primary token.  "); break;
		case ERROR_NO_SUCH_ALIAS : str = 	_T("The specified local group does not exist.  "); break;
		case ERROR_MEMBER_NOT_IN_ALIAS : str = 	_T("The specified account name is not a member of the local group.  "); break;
		case ERROR_MEMBER_IN_ALIAS : str = 	_T("The specified account name is already a member of the local group.  "); break;
		case ERROR_ALIAS_EXISTS : str = 	_T("The specified local group already exists.  "); break;
		case ERROR_LOGON_NOT_GRANTED : str = 	_T("Logon failure: str =  the user has not been granted the requested logon type at this computer.  "); break;
		case ERROR_TOO_MANY_SECRETS : str = 	_T("The maximum number of secrets that may be stored in a single system has been exceeded.  "); break;
		case ERROR_SECRET_TOO_LONG : str = 	_T("The length of a secret exceeds the maximum length allowed.  "); break;
		case ERROR_INTERNAL_DB_ERROR : str = 	_T("The local security authority database contains an internal inconsistency.  "); break;
		case ERROR_TOO_MANY_CONTEXT_IDS : str = 	_T("During a logon attempt, the user's security context accumulated too many security IDs.  "); break;
		case ERROR_LOGON_TYPE_NOT_GRANTED : str = 	_T("Logon failure: str =  the user has not been granted the requested logon type at this computer.  "); break;
		case ERROR_NT_CROSS_ENCRYPTION_REQUIRED : str = 	_T("A cross-encrypted password is necessary to change a user password.  "); break;
		case ERROR_NO_SUCH_MEMBER : str = 	_T("A new member could not be added to a local group because the member does not exist.  "); break;
		case ERROR_INVALID_MEMBER : str = 	_T("A new member could not be added to a local group because the member has the wrong account type.  "); break;
		case ERROR_TOO_MANY_SIDS : str = 	_T("Too many security IDs have been specified.  "); break;
		case ERROR_LM_CROSS_ENCRYPTION_REQUIRED : str = 	_T("A cross-encrypted password is necessary to change this user password.  "); break;
		case ERROR_NO_INHERITANCE : str = 	_T("Indicates an ACL contains no inheritable components.  "); break;
		case ERROR_FILE_CORRUPT : str = 	_T("The file or directory is corrupted and unreadable.  "); break;
		case ERROR_DISK_CORRUPT : str = 	_T("The disk structure is corrupted and unreadable.  "); break;
		case ERROR_NO_USER_SESSION_KEY : str = 	_T("There is no user session key for the specified logon session.  "); break;
		case ERROR_LICENSE_QUOTA_EXCEEDED : str = 	_T("The service being accessed is licensed for a particular number of connections. No more connections can be made to the service at this time because there are already as many connections as the service can accept.  "); break;
		case ERROR_INVALID_WINDOW_HANDLE : str = 	_T("Invalid window handle.  "); break;
		case ERROR_INVALID_MENU_HANDLE : str = 	_T("Invalid menu handle.  "); break;
		case ERROR_INVALID_CURSOR_HANDLE : str = 	_T("Invalid cursor handle.  "); break;
		case ERROR_INVALID_ACCEL_HANDLE : str = 	_T("Invalid accelerator table handle.  "); break;
		case ERROR_INVALID_HOOK_HANDLE : str = 	_T("Invalid hook handle.  "); break;
		case ERROR_INVALID_DWP_HANDLE : str = 	_T("Invalid handle to a multiple-window position structure.  "); break;
		case ERROR_TLW_WITH_WSCHILD : str = 	_T("Cannot create a top-level child window.  "); break;
		case ERROR_CANNOT_FIND_WND_CLASS : str = 	_T("Cannot find window class.  "); break;
		case ERROR_WINDOW_OF_OTHER_THREAD : str = 	_T("Invalid window; it belongs to other thread.  "); break;
		case ERROR_HOTKEY_ALREADY_REGISTERED : str = 	_T("Hot key is already registered.  "); break;
		case ERROR_CLASS_ALREADY_EXISTS : str = 	_T("Class already exists.  "); break;
		case ERROR_CLASS_DOES_NOT_EXIST : str = 	_T("Class does not exist.  "); break;
		case ERROR_CLASS_HAS_WINDOWS : str = 	_T("Class still has open windows.  "); break;
		case ERROR_INVALID_INDEX : str = 	_T("Invalid index.  "); break;
		case ERROR_INVALID_ICON_HANDLE : str = 	_T("Invalid icon handle.  "); break;
		case ERROR_PRIVATE_DIALOG_INDEX : str = 	_T("Using private DIALOG window words.  "); break;
		case ERROR_LISTBOX_ID_NOT_FOUND : str = 	_T("The list box identifier was not found.  "); break;
		case ERROR_NO_WILDCARD_CHARACTERS : str = 	_T("No wildcards were found.  "); break;
		case ERROR_CLIPBOARD_NOT_OPEN : str = 	_T("Thread does not have a clipboard open.  "); break;
		case ERROR_HOTKEY_NOT_REGISTERED : str = 	_T("Hot key is not registered.  "); break;
		case ERROR_WINDOW_NOT_DIALOG : str = 	_T("The window is not a valid dialog window.  "); break;
		case ERROR_CONTROL_ID_NOT_FOUND : str = 	_T("Control ID not found.  "); break;
		case ERROR_INVALID_COMBOBOX_MESSAGE : str = 	_T("Invalid message for a combo box because it does not have an edit control.  "); break;
		case ERROR_WINDOW_NOT_COMBOBOX : str = 	_T("The window is not a combo box.  "); break;
		case ERROR_INVALID_EDIT_HEIGHT : str = 	_T("Height must be less than 256.  "); break;
		case ERROR_DC_NOT_FOUND : str = 	_T("Invalid device context (DC) handle.  "); break;
		case ERROR_INVALID_HOOK_FILTER : str = 	_T("Invalid hook procedure type.  "); break;
		case ERROR_INVALID_FILTER_PROC : str = 	_T("Invalid hook procedure.  "); break;
		case ERROR_HOOK_NEEDS_HMOD : str = 	_T("Cannot set nonlocal hook without a module handle.  "); break;
		case ERROR_GLOBAL_ONLY_HOOK : str = 	_T("This hook procedure can only be set globally.  "); break;
		case ERROR_JOURNAL_HOOK_SET : str = 	_T("The journal hook procedure is already installed.  "); break;
		case ERROR_HOOK_NOT_INSTALLED : str = 	_T("The hook procedure is not installed.  "); break;
		case ERROR_INVALID_LB_MESSAGE : str = 	_T("Invalid message for single-selection list box.  "); break;
		case ERROR_SETCOUNT_ON_BAD_LB : str = 	_T("LB_SETCOUNT sent to non-lazy list box.  "); break;
		case ERROR_LB_WITHOUT_TABSTOPS : str = 	_T("This list box does not support tab stops.  "); break;
		case ERROR_DESTROY_OBJECT_OF_OTHER_THREAD : str = 	_T("Cannot destroy object created by another thread.  "); break;
		case ERROR_CHILD_WINDOW_MENU : str = 	_T("Child windows cannot have menus.  "); break;
		case ERROR_NO_SYSTEM_MENU : str = 	_T("The window does not have a system menu.  "); break;
		case ERROR_INVALID_MSGBOX_STYLE : str = 	_T("Invalid message box style.  "); break;
		case ERROR_INVALID_SPI_VALUE : str = 	_T("Invalid system-wide (SPI_*) parameter.  "); break;
		case ERROR_SCREEN_ALREADY_LOCKED : str = 	_T("Screen already locked.  "); break;
		case ERROR_HWNDS_HAVE_DIFF_PARENT : str = 	_T("All handles to windows in a multiple-window position structure must have the same parent.  "); break;
		case ERROR_NOT_CHILD_WINDOW : str = 	_T("The window is not a child window.  "); break;
		case ERROR_INVALID_GW_COMMAND : str = 	_T("Invalid GW_* command.  "); break;
		case ERROR_INVALID_THREAD_ID : str = 	_T("Invalid thread identifier.  "); break;
		case ERROR_NON_MDICHILD_WINDOW : str = 	_T("Cannot process a message from a window that is not a multiple document interface (MDI) window.  "); break;
		case ERROR_POPUP_ALREADY_ACTIVE : str = 	_T("Popup menu already active.  "); break;
		case ERROR_NO_SCROLLBARS : str = 	_T("The window does not have scroll bars.  "); break;
		case ERROR_INVALID_SCROLLBAR_RANGE : str = 	_T("Scroll bar range cannot be greater than 0x7FFF.  "); break;
		case ERROR_INVALID_SHOWWIN_COMMAND : str = 	_T("Cannot show or remove the window in the way specified.  "); break;
		case ERROR_NO_SYSTEM_RESOURCES : str = 	_T("Insufficient system resources exist to complete the requested service.  "); break;
		case ERROR_NONPAGED_SYSTEM_RESOURCES : str = 	_T("Insufficient system resources exist to complete the requested service.  "); break;
		case ERROR_PAGED_SYSTEM_RESOURCES : str = 	_T("Insufficient system resources exist to complete the requested service.  "); break;
		case ERROR_WORKING_SET_QUOTA : str = 	_T("Insufficient quota to complete the requested service.  "); break;
		case ERROR_PAGEFILE_QUOTA : str = 	_T("Insufficient quota to complete the requested service.  "); break;
		case ERROR_COMMITMENT_LIMIT : str = 	_T("The paging file is too small for this operation to complete.  "); break;
		case ERROR_MENU_ITEM_NOT_FOUND : str = 	_T("A menu item was not found.  "); break;
		case ERROR_INVALID_KEYBOARD_HANDLE : str = 	_T("Invalid keyboard layout handle.  "); break;
		case ERROR_HOOK_TYPE_NOT_ALLOWED : str = 	_T("Hook type not allowed.  "); break;
		case ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION : str = 	_T("This operation requires an interactive window station.  "); break;
		case ERROR_TIMEOUT : str = 	_T("This operation returned because the timeout period expired.  "); break;
		case ERROR_INVALID_MONITOR_HANDLE : str = 	_T("Invalid monitor handle.  "); break;
		case ERROR_EVENTLOG_FILE_CORRUPT : str = 	_T("The event log file is corrupted.  "); break;
		case ERROR_EVENTLOG_CANT_START : str = 	_T("No event log file could be opened, so the event logging service did not start.  "); break;
		case ERROR_LOG_FILE_FULL : str = 	_T("The event log file is full.  "); break;
		case ERROR_EVENTLOG_FILE_CHANGED : str = 	_T("The event log file has changed between read operations.  "); break;
		//case ERROR_INSTALL_SERVICE : str = 	_T("Failure accessing install service.  "); break;
		case ERROR_INSTALL_USEREXIT : str = 	_T("The user canceled the installation.  "); break;
		case ERROR_INSTALL_FAILURE : str = 	_T("Fatal error during installation.  "); break;
		case ERROR_INSTALL_SUSPEND : str = 	_T("Installation suspended, incomplete.  "); break;
		case ERROR_UNKNOWN_PRODUCT : str = 	_T("Product code not registered.  "); break;
		case ERROR_UNKNOWN_FEATURE : str = 	_T("Feature ID not registered.  "); break;
		case ERROR_UNKNOWN_COMPONENT : str = 	_T("Component ID not registered.  "); break;
		case ERROR_UNKNOWN_PROPERTY : str = 	_T("Unknown property.  "); break;
		case ERROR_INVALID_HANDLE_STATE : str = 	_T("Handle is in an invalid state.  "); break;
		case ERROR_BAD_CONFIGURATION : str = 	_T("Configuration data corrupt.  "); break;
		case ERROR_INDEX_ABSENT : str = 	_T("Language not available.  "); break;
		case ERROR_INSTALL_SOURCE_ABSENT : str = 	_T("Install source unavailable.  "); break;
		//case ERROR_BAD_DATABASE_VERSION : str = 	_T("Database version unsupported.  "); break;
		case ERROR_PRODUCT_UNINSTALLED : str = 	_T("Product is uninstalled.  "); break;
		case ERROR_BAD_QUERY_SYNTAX : str = 	_T("SQL query syntax invalid or unsupported.  "); break;
		case ERROR_INVALID_FIELD : str = 	_T("Record field does not exist.  "); break;
		case RPC_S_INVALID_STRING_BINDING : str = 	_T("The string binding is invalid.  "); break;
		case RPC_S_WRONG_KIND_OF_BINDING : str = 	_T("The binding handle is not the correct type.  "); break;
		case RPC_S_INVALID_BINDING : str = 	_T("The binding handle is invalid.  "); break;
		case RPC_S_PROTSEQ_NOT_SUPPORTED : str = 	_T("The RPC protocol sequence is not supported.  "); break;
		case RPC_S_INVALID_RPC_PROTSEQ : str = 	_T("The RPC protocol sequence is invalid.  "); break;
		case RPC_S_INVALID_STRING_UUID : str = 	_T("The string universal unique identifier (UUID) is invalid.  "); break;
		case RPC_S_INVALID_ENDPOINT_FORMAT : str = 	_T("The endpoint format is invalid.  "); break;
		case RPC_S_INVALID_NET_ADDR : str = 	_T("The network address is invalid.  "); break;
		case RPC_S_NO_ENDPOINT_FOUND : str = 	_T("No endpoint was found.  "); break;
		case RPC_S_INVALID_TIMEOUT : str = 	_T("The timeout value is invalid.  "); break;
		case RPC_S_OBJECT_NOT_FOUND : str = 	_T("The object universal unique identifier (UUID) was not found.  "); break;
		case RPC_S_ALREADY_REGISTERED : str = 	_T("The object universal unique identifier (UUID) has already been registered.  "); break;
		case RPC_S_TYPE_ALREADY_REGISTERED : str = 	_T("The type universal unique identifier (UUID) has already been registered.  "); break;
		case RPC_S_ALREADY_LISTENING : str = 	_T("The RPC server is already listening.  "); break;
		case RPC_S_NO_PROTSEQS_REGISTERED : str = 	_T("No protocol sequences have been registered.  "); break;
		case RPC_S_NOT_LISTENING : str = 	_T("The RPC server is not listening.  "); break;
		case RPC_S_UNKNOWN_MGR_TYPE : str = 	_T("The manager type is unknown.  "); break;
		case RPC_S_UNKNOWN_IF : str = 	_T("The interface is unknown.  "); break;
		case RPC_S_NO_BINDINGS : str = 	_T("There are no bindings.  "); break;
		case RPC_S_NO_PROTSEQS : str = 	_T("There are no protocol sequences.  "); break;
		case RPC_S_CANT_CREATE_ENDPOINT : str = 	_T("The endpoint cannot be created.  "); break;
		case RPC_S_OUT_OF_RESOURCES : str = 	_T("Not enough resources are available to complete this operation.  "); break;
		case RPC_S_SERVER_UNAVAILABLE : str = 	_T("The RPC server is unavailable.  "); break;
		case RPC_S_SERVER_TOO_BUSY : str = 	_T("The RPC server is too busy to complete this operation.  "); break;
		case RPC_S_INVALID_NETWORK_OPTIONS : str = 	_T("The network options are invalid.  "); break;
		case RPC_S_NO_CALL_ACTIVE : str = 	_T("There are no remote procedure calls active on this thread.  "); break;
		case RPC_S_CALL_FAILED : str = 	_T("The remote procedure call failed.  "); break;
		case RPC_S_CALL_FAILED_DNE : str = 	_T("The remote procedure call failed and did not execute.  "); break;
		case RPC_S_PROTOCOL_ERROR : str = 	_T("A remote procedure call (RPC) protocol error occurred.  "); break;
		case RPC_S_UNSUPPORTED_TRANS_SYN : str = 	_T("The transfer syntax is not supported by the RPC server.  "); break;
		case RPC_S_UNSUPPORTED_TYPE : str = 	_T("The universal unique identifier (UUID) type is not supported.  "); break;
		case RPC_S_INVALID_TAG : str = 	_T("The tag is invalid.  "); break;
		case RPC_S_INVALID_BOUND : str = 	_T("The array bounds are invalid.  "); break;
		case RPC_S_NO_ENTRY_NAME : str = 	_T("The binding does not contain an entry name.  "); break;
		case RPC_S_INVALID_NAME_SYNTAX : str = 	_T("The name syntax is invalid.  "); break;
		case RPC_S_UNSUPPORTED_NAME_SYNTAX : str = 	_T("The name syntax is not supported.  "); break;
		case RPC_S_UUID_NO_ADDRESS : str = 	_T("No network address is available to use to construct a universal unique identifier (UUID).  "); break;
		case RPC_S_DUPLICATE_ENDPOINT : str = 	_T("The endpoint is a duplicate.  "); break;
		case RPC_S_UNKNOWN_AUTHN_TYPE : str = 	_T("The authentication type is unknown.  "); break;
		case RPC_S_MAX_CALLS_TOO_SMALL : str = 	_T("The maximum number of calls is too small.  "); break;
		case RPC_S_STRING_TOO_LONG : str = 	_T("The string is too long.  "); break;
		case RPC_S_PROTSEQ_NOT_FOUND : str = 	_T("The RPC protocol sequence was not found.  "); break;
		case RPC_S_PROCNUM_OUT_OF_RANGE : str = 	_T("The procedure number is out of range.  "); break;
		case RPC_S_BINDING_HAS_NO_AUTH : str = 	_T("The binding does not contain any authentication information.  "); break;
		case RPC_S_UNKNOWN_AUTHN_SERVICE : str = 	_T("The authentication service is unknown.  "); break;
		case RPC_S_UNKNOWN_AUTHN_LEVEL : str = 	_T("The authentication level is unknown.  "); break;
		case RPC_S_INVALID_AUTH_IDENTITY : str = 	_T("The security context is invalid.  "); break;
		case RPC_S_UNKNOWN_AUTHZ_SERVICE : str = 	_T("The authorization service is unknown.  "); break;
		case EPT_S_INVALID_ENTRY : str = 	_T("The entry is invalid.  "); break;
		case EPT_S_CANT_PERFORM_OP : str = 	_T("The server endpoint cannot perform the operation.  "); break;
		case EPT_S_NOT_REGISTERED : str = 	_T("There are no more endpoints available from the endpoint mapper.  "); break;
		case RPC_S_NOTHING_TO_EXPORT : str = 	_T("No interfaces have been exported.  "); break;
		case RPC_S_INCOMPLETE_NAME : str = 	_T("The entry name is incomplete.  "); break;
		case RPC_S_INVALID_VERS_OPTION : str = 	_T("The version option is invalid.  "); break;
		case RPC_S_NO_MORE_MEMBERS : str = 	_T("There are no more members.  "); break;
		case RPC_S_NOT_ALL_OBJS_UNEXPORTED : str = 	_T("There is nothing to unexport.  "); break;
		case RPC_S_INTERFACE_NOT_FOUND : str = 	_T("The interface was not found.  "); break;
		case RPC_S_ENTRY_ALREADY_EXISTS : str = 	_T("The entry already exists.  "); break;
		case RPC_S_ENTRY_NOT_FOUND : str = 	_T("The entry is not found.  "); break;
		case RPC_S_NAME_SERVICE_UNAVAILABLE : str = 	_T("The name service is unavailable.  "); break;
		case RPC_S_INVALID_NAF_ID : str = 	_T("The network address family is invalid.  "); break;
		case RPC_S_CANNOT_SUPPORT : str = 	_T("The requested operation is not supported.  "); break;
		case RPC_S_NO_CONTEXT_AVAILABLE : str = 	_T("No security context is available to allow impersonation.  "); break;
		case RPC_S_INTERNAL_ERROR : str = 	_T("An internal error occurred in a remote procedure call (RPC).  "); break;
		case RPC_S_ZERO_DIVIDE : str = 	_T("The RPC server attempted an integer division by zero.  "); break;
		case RPC_S_ADDRESS_ERROR : str = 	_T("An addressing error occurred in the RPC server.  "); break;
		case RPC_S_FP_DIV_ZERO : str = 	_T("A floating-point operation at the RPC server caused a division by zero.  "); break;
		case RPC_S_FP_UNDERFLOW : str = 	_T("A floating-point underflow occurred at the RPC server.  "); break;
		case RPC_S_FP_OVERFLOW : str = 	_T("A floating-point overflow occurred at the RPC server.  "); break;
		case RPC_X_NO_MORE_ENTRIES : str = 	_T("The list of RPC servers available for the binding of auto handles has been exhausted.  "); break;
		case RPC_X_SS_CHAR_TRANS_OPEN_FAIL : str = 	_T("Unable to open the character translation table file.  "); break;
		case RPC_X_SS_CHAR_TRANS_SHORT_FILE : str = 	_T("The file containing the character translation table has fewer than bytes.  "); break;
		case RPC_X_SS_IN_NULL_CONTEXT : str = 	_T("A null context handle was passed from the client to the host during a remote procedure call.  "); break;
		case RPC_X_SS_CONTEXT_DAMAGED : str = 	_T("The context handle changed during a remote procedure call.  "); break;
		case RPC_X_SS_HANDLES_MISMATCH : str = 	_T("The binding handles passed to a remote procedure call do not match.  "); break;
		case RPC_X_SS_CANNOT_GET_CALL_HANDLE : str = 	_T("The stub is unable to get the remote procedure call handle.  "); break;
		case RPC_X_NULL_REF_POINTER : str = 	_T("A null reference pointer was passed to the stub.  "); break;
		case RPC_X_ENUM_VALUE_OUT_OF_RANGE : str = 	_T("The enumeration value is out of range.  "); break;
		case RPC_X_BYTE_COUNT_TOO_SMALL : str = 	_T("The byte count is too small.  "); break;
		case RPC_X_BAD_STUB_DATA : str = 	_T("The stub received bad data.  "); break;
		case ERROR_INVALID_USER_BUFFER : str = 	_T("The supplied user buffer is not valid for the requested operation.  "); break;
		case ERROR_UNRECOGNIZED_MEDIA : str = 	_T("The disk media is not recognized. It may not be formatted.  "); break;
		case ERROR_NO_TRUST_LSA_SECRET : str = 	_T("The workstation does not have a trust secret.  "); break;
		case ERROR_NO_TRUST_SAM_ACCOUNT : str = 	_T("The SAM database on the Windows NT Server does not have a computer account for this workstation trust relationship.  "); break;
		case ERROR_TRUSTED_DOMAIN_FAILURE : str = 	_T("The trust relationship between the primary domain and the trusted domain failed.  "); break;
		case ERROR_TRUSTED_RELATIONSHIP_FAILURE : str = 	_T("The trust relationship between this workstation and the primary domain failed.  "); break;
		case ERROR_TRUST_FAILURE : str = 	_T("The network logon failed.  "); break;
		case RPC_S_CALL_IN_PROGRESS : str = 	_T("A remote procedure call is already in progress for this thread.  "); break;
		case ERROR_NETLOGON_NOT_STARTED : str = 	_T("An attempt was made to logon, but the network logon service was not started.  "); break;
		case ERROR_ACCOUNT_EXPIRED : str = 	_T("The user's account has expired.  "); break;
		case ERROR_REDIRECTOR_HAS_OPEN_HANDLES : str = 	_T("The redirector is in use and cannot be unloaded.  "); break;
		case ERROR_PRINTER_DRIVER_ALREADY_INSTALLED : str = 	_T("The specified printer driver is already installed.  "); break;
		case ERROR_UNKNOWN_PORT : str = 	_T("The specified port is unknown.  "); break;
		case ERROR_UNKNOWN_PRINTER_DRIVER : str = 	_T("The printer driver is unknown.  "); break;
		case ERROR_UNKNOWN_PRINTPROCESSOR : str = 	_T("The print processor is unknown.  "); break;
		case ERROR_INVALID_SEPARATOR_FILE : str = 	_T("The specified separator file is invalid.  "); break;
		case ERROR_INVALID_PRIORITY : str = 	_T("The specified priority is invalid.  "); break;
		case ERROR_INVALID_PRINTER_NAME : str = 	_T("The printer name is invalid.  "); break;
		case ERROR_PRINTER_ALREADY_EXISTS : str = 	_T("The printer already exists.  "); break;
		case ERROR_INVALID_PRINTER_COMMAND : str = 	_T("The printer command is invalid.  "); break;
		case ERROR_INVALID_DATATYPE : str = 	_T("The specified datatype is invalid.  "); break;
		case ERROR_INVALID_ENVIRONMENT : str = 	_T("The environment specified is invalid.  "); break;
		case RPC_S_NO_MORE_BINDINGS : str = 	_T("There are no more bindings.  "); break;
		case ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT : str = 	_T("The account used is an interdomain trust account. Use your global user account or local user account to access this server.  "); break;
		case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT : str = 	_T("The account used is a computer account. Use your global user account or local user account to access this server.  "); break;
		case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT : str = 	_T("The account used is a server trust account. Use your global user account or local user account to access this server.  "); break;
		case ERROR_DOMAIN_TRUST_INCONSISTENT : str = 	_T("The name or security ID (SID) of the domain specified is inconsistent with the trust information for that domain.  "); break;
		case ERROR_SERVER_HAS_OPEN_HANDLES : str = 	_T("The server is in use and cannot be unloaded.  "); break;
		case ERROR_RESOURCE_DATA_NOT_FOUND : str = 	_T("The specified image file did not contain a resource section.  "); break;
		case ERROR_RESOURCE_TYPE_NOT_FOUND : str = 	_T("The specified resource type cannot be found in the image file.  "); break;
		case ERROR_RESOURCE_NAME_NOT_FOUND : str = 	_T("The specified resource name cannot be found in the image file.  "); break;
		case ERROR_RESOURCE_LANG_NOT_FOUND : str = 	_T("The specified resource language ID cannot be found in the image file.  "); break;
		case ERROR_NOT_ENOUGH_QUOTA : str = 	_T("Not enough quota is available to process this command.  "); break;
		case RPC_S_NO_INTERFACES : str = 	_T("No interfaces have been registered.  "); break;
		case RPC_S_CALL_CANCELLED : str = 	_T("The remote procedure call was cancelled.  "); break;
		case RPC_S_BINDING_INCOMPLETE : str = 	_T("The binding handle does not contain all required information.  "); break;
		case RPC_S_COMM_FAILURE : str = 	_T("A communications failure occurred during a remote procedure call.  "); break;
		case RPC_S_UNSUPPORTED_AUTHN_LEVEL : str = 	_T("The requested authentication level is not supported.  "); break;
		case RPC_S_NO_PRINC_NAME : str = 	_T("No principal name registered.  "); break;
		case RPC_S_NOT_RPC_ERROR : str = 	_T("The error specified is not a valid Windows RPC error code.  "); break;
		case RPC_S_UUID_LOCAL_ONLY : str = 	_T("A UUID that is valid only on this computer has been allocated.  "); break;
		case RPC_S_SEC_PKG_ERROR : str = 	_T("A security package specific error occurred.  "); break;
		case RPC_S_NOT_CANCELLED : str = 	_T("Thread is not canceled.  "); break;
		case RPC_X_INVALID_ES_ACTION : str = 	_T("Invalid operation on the encoding/decoding handle.  "); break;
		case RPC_X_WRONG_ES_VERSION : str = 	_T("Incompatible version of the serializing package.  "); break;
		case RPC_X_WRONG_STUB_VERSION : str = 	_T("Incompatible version of the RPC stub.  "); break;
		case RPC_X_INVALID_PIPE_OBJECT : str = 	_T("The RPC pipe object is invalid or corrupted.  "); break;
		case RPC_X_WRONG_PIPE_ORDER : str = 	_T("An invalid operation was attempted on an RPC pipe object.  "); break;
		case RPC_X_WRONG_PIPE_VERSION : str = 	_T("Unsupported RPC pipe version.  "); break;
		case RPC_S_GROUP_MEMBER_NOT_FOUND : str = 	_T("The group member was not found.  "); break;
		case EPT_S_CANT_CREATE : str = 	_T("The endpoint mapper database entry could not be created.  "); break;
		case RPC_S_INVALID_OBJECT : str = 	_T("The object universal unique identifier (UUID) is the nil UUID.  "); break;
		case ERROR_INVALID_TIME : str = 	_T("The specified time is invalid.  "); break;
		case ERROR_INVALID_FORM_NAME : str = 	_T("The specified form name is invalid.  "); break;
		case ERROR_INVALID_FORM_SIZE : str = 	_T("The specified form size is invalid.  "); break;
		case ERROR_ALREADY_WAITING : str = 	_T("The specified printer handle is already being waited on  "); break;
		case ERROR_PRINTER_DELETED : str = 	_T("The specified printer has been deleted.  "); break;
		case ERROR_INVALID_PRINTER_STATE : str = 	_T("The state of the printer is invalid.  "); break;
		case ERROR_PASSWORD_MUST_CHANGE : str = 	_T("The user must change his password before he logs on the first time.  "); break;
		case ERROR_DOMAIN_CONTROLLER_NOT_FOUND : str = 	_T("Could not find the domain controller for this domain.  "); break;
		case ERROR_ACCOUNT_LOCKED_OUT : str = 	_T("The referenced account is currently locked out and may not be logged on to.  "); break;
		case OR_INVALID_OXID : str = 	_T("The object exporter specified was not found.  "); break;
		case OR_INVALID_OID : str = 	_T("The object specified was not found.  "); break;
		case OR_INVALID_SET : str = 	_T("The object resolver set specified was not found.  "); break;
		case RPC_S_SEND_INCOMPLETE : str = 	_T("Some data remains to be sent in the request buffer.  "); break;
		case RPC_S_INVALID_ASYNC_HANDLE : str = 	_T("Invalid asynchronous remote procedure call handle.  "); break;
		case RPC_S_INVALID_ASYNC_CALL : str = 	_T("Invalid asynchronous RPC call handle for this operation.  "); break;
		case RPC_X_PIPE_CLOSED : str = 	_T("The RPC pipe object has already been closed.  "); break;
		case RPC_X_PIPE_DISCIPLINE_ERROR : str = 	_T("The RPC call completed before all pipes were processed.  "); break;
		case RPC_X_PIPE_EMPTY : str = 	_T("No more data is available from the RPC pipe.  "); break;
		case ERROR_NO_SITENAME : str = 	_T("No site name is available for this machine.  "); break;
		case ERROR_CANT_ACCESS_FILE : str = 	_T("The file can not be accessed by the system.  "); break;
		case ERROR_CANT_RESOLVE_FILENAME : str = 	_T("The name of the file cannot be resolved by the system.  "); break;
		case ERROR_DS_MEMBERSHIP_EVALUATED_LOCALLY : str = 	_T("The directory service evaluated group memberships locally.  "); break;
		case ERROR_DS_NO_ATTRIBUTE_OR_VALUE : str = 	_T("The specified directory service attribute or value does not exist.  "); break;
		case ERROR_DS_INVALID_ATTRIBUTE_SYNTAX : str = 	_T("The attribute syntax specified to the directory service is invalid.  "); break;
		case ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED : str = 	_T("The attribute type specified to the directory service is not defined.  "); break;
		case ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS : str = 	_T("The specified directory service attribute or value already exists.  "); break;
		case ERROR_DS_BUSY : str = 	_T("The directory service is busy.  "); break;
		case ERROR_DS_UNAVAILABLE : str = 	_T("The directory service is unavailable.  "); break;
		case ERROR_DS_NO_RIDS_ALLOCATED : str = 	_T("The directory service was unable to allocate a relative identifier.  "); break;
		case ERROR_DS_NO_MORE_RIDS : str = 	_T("The directory service has exhausted the pool of relative identifiers.  "); break;
		case ERROR_DS_INCORRECT_ROLE_OWNER : str = 	_T("The requested operation could not be performed because the directory service is not the master for that type of operation.  "); break;
		case ERROR_DS_RIDMGR_INIT_ERROR : str = 	_T("The directory service was unable to initialize the subsystem that allocates relative identifiers.  "); break;
		case ERROR_DS_OBJ_CLASS_VIOLATION : str = 	_T("The requested operation did not satisfy one or more constraints associated with the class of the object.  "); break;
		case ERROR_DS_CANT_ON_NON_LEAF : str = 	_T("The directory service can perform the requested operation only on a leaf object.  "); break;
		case ERROR_DS_CANT_ON_RDN : str = 	_T("The directory service cannot perform the requested operation on the RDN attribute of an object.  "); break;
		case ERROR_DS_CANT_MOD_OBJ_CLASS : str = 	_T("The directory service detected an attempt to modify the object class of an object.  "); break;
		case ERROR_DS_CROSS_DOM_MOVE_ERROR : str = 	_T("The requested cross domain move operation could not be performed.  "); break;
		case ERROR_DS_GC_NOT_AVAILABLE : str = 	_T("Unable to contact the global catalog server.  "); break;
		case ERROR_INVALID_PIXEL_FORMAT : str = 	_T("The pixel format is invalid.  "); break;
		case ERROR_BAD_DRIVER : str = 	_T("The specified driver is invalid.  "); break;
		case ERROR_INVALID_WINDOW_STYLE : str = 	_T("The window style or class attribute is invalid for this operation.  "); break;
		case ERROR_METAFILE_NOT_SUPPORTED : str = 	_T("The requested metafile operation is not supported.  "); break;
		case ERROR_TRANSFORM_NOT_SUPPORTED : str = 	_T("The requested transformation operation is not supported.  "); break;
		case ERROR_CLIPPING_NOT_SUPPORTED : str = 	_T("The requested clipping operation is not supported.  "); break;
		case ERROR_CONNECTED_OTHER_PASSWORD : str = 	_T("The network connection was made successfully, but the user had to be prompted for a password other than the one originally specified.  "); break;
		case ERROR_BAD_USERNAME : str = 	_T("The specified username is invalid.  "); break;
		case ERROR_NOT_CONNECTED : str = 	_T("This network connection does not exist.  "); break;
		case ERROR_INVALID_CMM : str = 	_T("The specified color management module is invalid.  "); break;
		case ERROR_INVALID_PROFILE : str = 	_T("The specified color profile is invalid.  "); break;
		case ERROR_TAG_NOT_FOUND : str = 	_T("The specified tag was not found.  "); break;
		case ERROR_TAG_NOT_PRESENT : str = 	_T("A required tag is not present.  "); break;
		case ERROR_DUPLICATE_TAG : str = 	_T("The specified tag is already present.  "); break;
		case ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE : str = 	_T("The specified color profile is not associated with any device.  "); break;
		case ERROR_PROFILE_NOT_FOUND : str = 	_T("The specified color profile was not found.  "); break;
		case ERROR_INVALID_COLORSPACE : str = 	_T("The specified color space is invalid.  "); break;
		case ERROR_ICM_NOT_ENABLED : str = 	_T("Image Color Management is not enabled.  "); break;
		case ERROR_DELETING_ICM_XFORM : str = 	_T("There was an error while deleting the color transform.  "); break;
		case ERROR_INVALID_TRANSFORM : str = 	_T("The specified color transform is invalid.  "); break;
		case ERROR_OPEN_FILES : str = 	_T("This network connection has files open or requests pending.  "); break;
		case ERROR_ACTIVE_CONNECTIONS : str = 	_T("Active connections still exist.  "); break;
		case ERROR_DEVICE_IN_USE : str = 	_T("The device is in use by an active process and cannot be disconnected.  "); break;
		case ERROR_UNKNOWN_PRINT_MONITOR : str = 	_T("The specified print monitor is unknown.  "); break;
		case ERROR_PRINTER_DRIVER_IN_USE : str = 	_T("The specified printer driver is currently in use.  "); break;
		case ERROR_SPOOL_FILE_NOT_FOUND : str = 	_T("The spool file was not found.  "); break;
		case ERROR_SPL_NO_STARTDOC : str = 	_T("A StartDocPrinter call was not issued.  "); break;
		case ERROR_SPL_NO_ADDJOB : str = 	_T("An AddJob call was not issued.  "); break;
		case ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED : str = 	_T("The specified print processor has already been installed.  "); break;
		case ERROR_PRINT_MONITOR_ALREADY_INSTALLED : str = 	_T("The specified print monitor has already been installed.  "); break;
		case ERROR_INVALID_PRINT_MONITOR : str = 	_T("The specified print monitor does not have the required functions.  "); break;
		case ERROR_PRINT_MONITOR_IN_USE : str = 	_T("The specified print monitor is currently in use.  "); break;
		case ERROR_PRINTER_HAS_JOBS_QUEUED : str = 	_T("The requested operation is not allowed when there are jobs queued to the printer.  "); break;
		case ERROR_SUCCESS_REBOOT_REQUIRED : str = 	_T("The requested operation is successful. Changes will not be effective until the system is rebooted.  "); break;
		case ERROR_SUCCESS_RESTART_REQUIRED : str = 	_T("The requested operation is successful. Changes will not be effective until the service is restarted.  "); break;
		case ERROR_WINS_INTERNAL : str = 	_T("WINS encountered an error while processing the command.  "); break;
		case ERROR_CAN_NOT_DEL_LOCAL_WINS : str = 	_T("The local WINS can not be deleted.  "); break;
		case ERROR_STATIC_INIT : str = 	_T("The importation from the file failed.  "); break;
		case ERROR_INC_BACKUP : str = 	_T("The backup failed. Was a full backup done before?  "); break;
		case ERROR_FULL_BACKUP : str = 	_T("The backup failed. Check the directory to which you are backing the database.  "); break;
		case ERROR_REC_NON_EXISTENT : str = 	_T("The name does not exist in the WINS database.  "); break;
		case ERROR_RPL_NOT_ALLOWED : str = 	_T("Replication with a nonconfigured partner is not allowed.  "); break;
		case ERROR_DHCP_ADDRESS_CONFLICT : str = 	_T("The DHCP client has obtained an IP address that is already in use on the network. The local interface will be disabled until the DHCP client can obtain a new address.  "); break;
		case ERROR_WMI_GUID_NOT_FOUND : str = 	_T("The GUID passed was not recognized as valid by a WMI data provider.  "); break;
		case ERROR_WMI_INSTANCE_NOT_FOUND : str = 	_T("The instance name passed was not recognized as valid by a WMI data provider.  "); break;
		case ERROR_WMI_ITEMID_NOT_FOUND : str = 	_T("The data item ID passed was not recognized as valid by a WMI data provider.  "); break;
		case ERROR_WMI_TRY_AGAIN : str = 	_T("The WMI request could not be completed and should be retried.  "); break;
		case ERROR_WMI_DP_NOT_FOUND : str = 	_T("The WMI data provider could not be located.  "); break;
		case ERROR_WMI_UNRESOLVED_INSTANCE_REF : str = 	_T("The WMI data provider references an instance set that has not been registered.  "); break;
		case ERROR_WMI_ALREADY_ENABLED : str = 	_T("The WMI data block or event notification has already been enabled.  "); break;
		case ERROR_WMI_GUID_DISCONNECTED : str = 	_T("The WMI data block is no longer available.  "); break;
		case ERROR_WMI_SERVER_UNAVAILABLE : str = 	_T("The WMI data service is not available.  "); break;
		case ERROR_WMI_DP_FAILED : str = 	_T("The WMI data provider failed to carry out the request.  "); break;
		case ERROR_WMI_INVALID_MOF : str = 	_T("The WMI MOF information is not valid.  "); break;
		case ERROR_WMI_INVALID_REGINFO : str = 	_T("The WMI registration information is not valid.  "); break;
		case ERROR_INVALID_MEDIA : str = 	_T("The media identifier does not represent a valid medium.  "); break;
		case ERROR_INVALID_LIBRARY : str = 	_T("The library identifier does not represent a valid library.  "); break;
		case ERROR_INVALID_MEDIA_POOL : str = 	_T("The media pool identifier does not represent a valid media pool.  "); break;
		case ERROR_DRIVE_MEDIA_MISMATCH : str = 	_T("The drive and medium are not compatible or exist in different libraries.  "); break;
		case ERROR_MEDIA_OFFLINE : str = 	_T("The medium currently exists in an offline library and must be online to perform this operation.  "); break;
		case ERROR_LIBRARY_OFFLINE : str = 	_T("The operation cannot be performed on an offline library.  "); break;
		case ERROR_EMPTY : str = 	_T("The library, drive, or media pool is empty.  "); break;
		case ERROR_NOT_EMPTY : str = 	_T("The library, drive, or media pool must be empty to perform this operation.  "); break;
		case ERROR_MEDIA_UNAVAILABLE : str = 	_T("No media is currently available in this media pool or library.  "); break;
		case ERROR_RESOURCE_DISABLED : str = 	_T("A resource required for this operation is disabled.  "); break;
		case ERROR_INVALID_CLEANER : str = 	_T("The media identifier does not represent a valid cleaner.  "); break;
		case ERROR_UNABLE_TO_CLEAN : str = 	_T("The drive cannot be cleaned or does not support cleaning.  "); break;
		case ERROR_OBJECT_NOT_FOUND : str = 	_T("The object identifier does not represent a valid object.  "); break;
		case ERROR_DATABASE_FAILURE : str = 	_T("Unable to read from or write to the database.  "); break;
		case ERROR_DATABASE_FULL : str = 	_T("The database is full.  "); break;
		case ERROR_MEDIA_INCOMPATIBLE : str = 	_T("The medium is not compatible with the device or media pool.  "); break;
		case ERROR_RESOURCE_NOT_PRESENT : str = 	_T("The resource required for this operation does not exist.  "); break;
		case ERROR_INVALID_OPERATION : str = 	_T("The operation identifier is not valid.  "); break;
		case ERROR_MEDIA_NOT_AVAILABLE : str = 	_T("The media is not mounted or ready for use.  "); break;
		case ERROR_DEVICE_NOT_AVAILABLE : str = 	_T("The device is not ready for use.  "); break;
		case ERROR_REQUEST_REFUSED : str = 	_T("The operator or administrator has refused the request.  "); break;
		case ERROR_FILE_OFFLINE : str = 	_T("The remote storage service was not able to recall the file.  "); break;
		case ERROR_REMOTE_STORAGE_NOT_ACTIVE : str = 	_T("The remote storage service is not operational at this time.  "); break;
		case ERROR_REMOTE_STORAGE_MEDIA_ERROR : str = 	_T("The remote storage service encountered a media error.  "); break;
		case ERROR_NOT_A_REPARSE_POINT : str = 	_T("The file or directory is not a reparse point.  "); break;
		case ERROR_REPARSE_ATTRIBUTE_CONFLICT : str = 	_T("The reparse point attribute cannot be set because it conflicts with an existing attribute.  "); break;
		case ERROR_DEPENDENT_RESOURCE_EXISTS : str = 	_T("The cluster resource cannot be moved to another group because other resources are dependent on it.  "); break;
		case ERROR_DEPENDENCY_NOT_FOUND : str = 	_T("The cluster resource dependency cannot be found.  "); break;
		case ERROR_DEPENDENCY_ALREADY_EXISTS : str = 	_T("The cluster resource cannot be made dependent on the specified resource because it is already dependent.  "); break;
		case ERROR_RESOURCE_NOT_ONLINE : str = 	_T("The cluster resource is not online.  "); break;
		case ERROR_HOST_NODE_NOT_AVAILABLE : str = 	_T("A cluster node is not available for this operation.  "); break;
		case ERROR_RESOURCE_NOT_AVAILABLE : str = 	_T("The cluster resource is not available.  "); break;
		case ERROR_RESOURCE_NOT_FOUND : str = 	_T("The cluster resource could not be found.  "); break;
		case ERROR_SHUTDOWN_CLUSTER : str = 	_T("The cluster is being shut down.  "); break;
		case ERROR_CANT_EVICT_ACTIVE_NODE : str = 	_T("A cluster node cannot be evicted from the cluster while it is online.  "); break;
		case ERROR_OBJECT_ALREADY_EXISTS : str = 	_T("The object already exists.  "); break;
		case ERROR_OBJECT_IN_LIST : str = 	_T("The object is already in the list.  "); break;
		case ERROR_GROUP_NOT_AVAILABLE : str = 	_T("The cluster group is not available for any new requests.  "); break;
		case ERROR_GROUP_NOT_FOUND : str = 	_T("The cluster group could not be found.  "); break;
		case ERROR_GROUP_NOT_ONLINE : str = 	_T("The operation could not be completed because the cluster group is not online.  "); break;
		case ERROR_HOST_NODE_NOT_RESOURCE_OWNER : str = 	_T("The cluster node is not the owner of the resource.  "); break;
		case ERROR_HOST_NODE_NOT_GROUP_OWNER : str = 	_T("The cluster node is not the owner of the group.  "); break;
		case ERROR_RESMON_CREATE_FAILED : str = 	_T("The cluster resource could not be created in the specified resource monitor.  "); break;
		case ERROR_RESMON_ONLINE_FAILED : str = 	_T("The cluster resource could not be brought online by the resource monitor.  "); break;
		case ERROR_RESOURCE_ONLINE : str = 	_T("The operation could not be completed because the cluster resource is online.  "); break;
		case ERROR_QUORUM_RESOURCE : str = 	_T("The cluster resource could not be deleted or brought offline because it is the quorum resource.  "); break;
		case ERROR_NOT_QUORUM_CAPABLE : str = 	_T("The cluster could not make the specified resource a quorum resource because it is not capable of being a quorum resource.  "); break;
		case ERROR_CLUSTER_SHUTTING_DOWN : str = 	_T("The cluster software is shutting down.  "); break;
		case ERROR_INVALID_STATE : str = 	_T("The group or resource is not in the correct state to perform the requested operation.  "); break;
		case ERROR_RESOURCE_PROPERTIES_STORED : str = 	_T("The properties were stored but not all changes will take effect until the next time the resource is brought online.  "); break;
		case ERROR_NOT_QUORUM_CLASS : str = 	_T("The cluster could not make the specified resource a quorum resource because it does not belong to a shared storage class.  "); break;
		case ERROR_CORE_RESOURCE : str = 	_T("The cluster resource could not be deleted since it is a core resource.  "); break;
		case ERROR_QUORUM_RESOURCE_ONLINE_FAILED : str = 	_T("The quorum resource failed to come online.  "); break;
		case ERROR_QUORUMLOG_OPEN_FAILED : str = 	_T("The quorum log could not be created or mounted successfully.  "); break;
		case ERROR_CLUSTERLOG_CORRUPT : str = 	_T("The cluster log is corrupt.  "); break;
		case ERROR_CLUSTERLOG_RECORD_EXCEEDS_MAXSIZE : str = 	_T("The record could not be written to the cluster log since it exceeds the maximum size.  "); break;
		case ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE : str = 	_T("The cluster log exceeds its maximum size.  "); break;
		case ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND : str = 	_T("No checkpoint record was found in the cluster log.  "); break;
		case ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE : str = 	_T("The minimum required disk space needed for logging is not available.  "); break;
		case ERROR_ENCRYPTION_FAILED : str = 	_T("The specified file could not be encrypted.  "); break;
		case ERROR_DECRYPTION_FAILED : str = 	_T("The specified file could not be decrypted.  "); break;
		case ERROR_FILE_ENCRYPTED : str = 	_T("The specified file is encrypted and the user does not have the ability to decrypt it.  "); break;
		case ERROR_NO_RECOVERY_POLICY : str = 	_T("There is no encryption recovery policy configured for this system.  "); break;
		case ERROR_NO_EFS : str = 	_T("The required encryption driver is not loaded for this system.  "); break;
		case ERROR_WRONG_EFS : str = 	_T("The file was encrypted with a different encryption driver than is currently loaded.  "); break;
		case ERROR_NO_USER_KEYS : str = 	_T("There are no EFS keys defined for the user.  "); break;
		case ERROR_FILE_NOT_ENCRYPTED : str = 	_T("The specified file is not encrypted.  "); break;
		case ERROR_NOT_EXPORT_FORMAT : str = 	_T("The specified file is not in the defined EFS export format.  "); break;
		case ERROR_NO_BROWSER_SERVERS_FOUND : str = 	_T("The list of servers for this workgroup is not currently available  "); break;
	}
	return str;
}

//////////////////////////////////////////////////////////////////////////

int MxPad::Version()

{
	return(1);
}

//////////////////////////////////////////////////////////////////////////

TCHAR  *MxPad::sockerr2str(int err)

{
	// No error
	if(err == 0)
		return(_T("no error"));

	int idx = 0;
	
	// search for matching error code
	while(true)
		{
		// Terminal condition?
		if(errlist[idx].err == -1)
			return(_T("unknown error"));

		if(errlist[idx].err == err)
			return(errlist[idx].str);

		idx++;
		}

	// Not reached
	return _T("unkown error");
}


//////////////////////////////////////////////////////////////////////////

int MxPad::SendStr(LPCTSTR str)

{
	//ULONG	res;

	// Duplicate string here
	int lenx = strlen((char*)str);
	char *str2 = (char*)malloc(lenx + 1); //ASSERT(str2);
	strcpy_s(str2, lenx, (const char*)str);
	
	// Add it to the send list
	lock.Lock();
	ptrlist.AddTail(str2);
	lock.Unlock();
	
	SetEvent(event);

	return 0;
}




