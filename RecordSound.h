
/* =====[ CRecordSound.h ]========================================== 
                                                                             
   Description:     The daco project, implementation of the CRecordSound.h                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/27/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRecordSound_H__EA26EF41_4169_11D6_8886_F00753C10001__INCLUDED_)
#define AFX_CRecordSound_H__EA26EF41_4169_11D6_8886_F00753C10001__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

#include "mmsystem.h"
#include "soundpar.h"

#define RECMAGIC	0x123123


//////////////////////////////////////////////////////////////////////////

class CRecordSound 

{
public:
	
	typedef void (*PFUNCSI)(char *str, int len);
	typedef void (*PFUNPVCSI)(void *ptr, char *str, int len);

private:

	static void CALLBACK CRecordSound::_WaveInProc( HWAVEIN hwi,  
							UINT uMsg, DWORD dwInstance,  
								DWORD dwParam1,    DWORD dwParam2);

	static	void	CRecordSound::_RecThread(void *lpParam);
	static	void	CRecordSound::_ProcSound(void *lpParam, void *buff, int len);

public:

	//CProgressCtrl	*pprog;
	
	void		*orgclass;
	PFUNCSI		callthis;
	PFUNPVCSI	callthis2;

	CRecordSound();
	virtual ~CRecordSound();

	HWAVEIN	WaveInOpen(int dev = 0);
	void	WaveInClose();

	void	StartRecord();
	void	StopRecord();

	void	SetFormat(int channels, int sampersec, int bitssample);

	int		recording, silent, magic;

	WAVEFORMATEX	devicewfx;

private:


	// Thread related
	HANDLE  event;
	int		thhandle, endthread;

	HWAVEIN		m_hRecord;

	CPtrArray	rec_queue;
	CCriticalSection lock;
		
	LPWAVEHDR xrechead[MAXRECBUFFER];

	int		xisallocated, xisopened;
	int		xolddev;

	//WAVEFORMATEX	m_WaveFormatEx; 
	
	void	xPreCreateHeader();
	void	xDisplayError(int mmReturn, TCHAR errmsg[]);

	LPWAVEHDR	xCreateWaveHeader();

};

#endif // !defined(AFX_CRecordSound_H__EA26EF41_4169_11D6_8886_F00753C10001__INCLUDED_)
