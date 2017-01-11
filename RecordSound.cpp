
/* =====[ CRecordSound.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the CRecordSound.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/27/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include <process.h>

#include "RecordSound.h"
#include "dsp.h"

//#include "daco.h"
//#include "play.h"
//#include "recog.h"

#include "mxpad.h"
#include "support.h"

#include <mmsystem.h>
#include <mmreg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
//static TCHAR THIS_FILE[] = _T(__FILE__);
static const char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// Called from the sound driver to do our bidding. must free buffer
// when done.

void	CRecordSound::_ProcSound(void *lpParam, void *buff, int len)

{
	CRecordSound *rec = (CRecordSound*)lpParam;

	if(!len)
		{
		return;
		}

	// This is a demo of immidiate feedback loop - stereo
	//((CPlay*)AfxGetApp()->m_pMainWnd)->playsound.PlayWave((char*)buff, len, true);
	//return;
	
	int len2 = len / sizeof(short);

	//short *lptr2 = (short*)malloc(len2); //ASSERT(lptr2);
	//short *rptr2 = (short*)malloc(len2); //ASSERT(rptr2);
	//SplitWave((short*)buff, len, lptr2, rptr2, len2);
	//free(lptr2);

	short *lptr2 = (short*)malloc(len2); //ASSERT(lptr2);
	dsp.SplitWaveLeft((short*)buff, len, lptr2, len2);

	// We created a duplicate, free this one
	free(buff);

	//CPlay *pwnd = (CPlay*)AfxGetApp()->m_pMainWnd;

#if 0
	// Grab the first items for speed
	if(len > 4)
		{
		short *buffs = (short*)lptr2;
		int peak = abs(buffs[0]) + abs(buffs[1])
				+ abs(buffs[2]) + abs(buffs[3]);
		peak /= 4;

		if(rec->pprog)
			rec->pprog->SetPos(100 * peak / SHRT_MAX);
		}
#endif

	// This is a demo of immidiate feedback loop
	//pwnd->playsound.PlayMonoWave((char*)lptr2, len2, false);
	
	//P2N(_T("CRecordSound::ProcSound %p %d\r\n"), buff, len);

	// Call on recording pointers
	if(rec->callthis)
		rec->callthis((char*)lptr2, len2);
	else if(rec->callthis2)
		rec->callthis2(rec->orgclass, (char*)lptr2, len2);
	else
		free(lptr2);
	
	// This is what it did ...
	//recog.Recognize((char*)lptr2, len2);

	// Just consume some time for testing
	//Sleep(20);
}

//////////////////////////////////////////////////////////////////////////

void	CRecordSound::_RecThread(void *lpParam)

{
	CRecordSound *rec = (CRecordSound*)lpParam;

	while(true)
		{
		WaitForSingleObject(rec->event, INFINITE);

		if(rec->endthread)
			break;

		// Process queue
		while(true)
			{		
			// Check for running conditions

			if(rec->endthread)
				break;

			rec->lock.Lock();
			int cnt = rec->rec_queue.GetSize();
			rec->lock.Unlock();
		
			if(cnt == 0)
				break;

			if(rec->m_hRecord == NULL)
				break;

			rec->lock.Lock();
			WAVEHDR *pwhdr = (WAVEHDR*)rec->rec_queue.GetAt(0);
			rec->lock.Unlock();
		
			if(!pwhdr)
				break;
		
			if(!AfxIsValidAddress(pwhdr, sizeof(WAVEHDR)))
				{
				AP2N(_T("CRecordSound::_RecThread AfxIsValidAddress\r\n"));
				break;
				}

			//AP2N(_T("RecThread got %p %d queue=%d\r\n"), 
			//			pwhdr->lpData, pwhdr->dwBytesRecorded, rec->queue.GetSize() );
			
			::waveInUnprepareHeader(rec->m_hRecord, pwhdr, sizeof(WAVEHDR));

			//ProcSound(pwhdr->lpData, pwhdr->dwBytesRecorded);	

			// Here we duplicate the sound buffer, we may need to store it in recog
			int len = pwhdr->dwBytesRecorded;
			void *mem = malloc(len); //ASSERT(mem);
			memcpy(mem, pwhdr->lpData, len);
			
			// Pass it onto the recog engine
			_ProcSound(rec, mem, len);
		
			// Done with this buffer, give it back
			rec->lock.Lock();
			rec->rec_queue.RemoveAt(0);
			rec->lock.Unlock();
			
			// Done with it, send it back to the pool 
			::waveInPrepareHeader(rec->m_hRecord, pwhdr, sizeof(WAVEHDR));
			::waveInAddBuffer(rec->m_hRecord, pwhdr, sizeof(WAVEHDR));						
			}		
		}

	// Cannot use P2N as it is destructor time
	//TRACE(_T("Ended sound recording Thread\r\n"));
	_endthread();
 
    return;
}

//////////////////////////////////////////////////////////////////////////

void CALLBACK CRecordSound::_WaveInProc( HWAVEIN hwi,  UINT uMsg, DWORD dwInstance,  
								DWORD dwParam1,    DWORD dwParam2)

{
	CRecordSound *rec = (CRecordSound*)dwInstance;

	switch(uMsg)
		{
		case MM_WIM_DATA:
			{
			LPWAVEHDR lpHdr = (LPWAVEHDR)dwParam1 ;
		
			//AP2N(_T("MM_WIM_DATA %p len=%d bufflen=%d flags=%d user=%d\r\n"), lpHdr->lpData, lpHdr->dwBytesRecorded,
			//	lpHdr->dwBufferLength, lpHdr->dwFlags, lpHdr->dwUser);

			// Do not submit after reset is issued (zero length buffers)
			if(lpHdr->dwBytesRecorded != 0)
				{
				rec->lock.Lock();
				rec->rec_queue.Add(lpHdr);
				rec->lock.Unlock();

				SetEvent(rec->event);
				}
			break;
			}

		case WIM_OPEN :
			//AP2N(_T("WIM_OPEN\r\n"));
			break;

		case WIM_CLOSE :
			//AP2N(_T("WIM_CLOSE\r\n"));
			break;
		}
}


//////////////////////////////////////////////////////////////////////////

CRecordSound::CRecordSound()

{
	recording = FALSE;
	xolddev = 0;
	m_hRecord = NULL;

	xisopened = xisallocated = 0;      // memory is not allocated to wavebuffer

	// Create Headers for buffering
	xPreCreateHeader();

	// Setting WAVEFORMATEX  structure for the audio input

	memset(&devicewfx, 0x00, sizeof(devicewfx));

	devicewfx.wFormatTag		= WAVE_FORMAT_PCM;
	devicewfx.nChannels		= NUMCHANNELS;
	devicewfx.wBitsPerSample	= SAMPLEBITS;
	devicewfx.cbSize			= 0;
	devicewfx.nSamplesPerSec	= SAMPLERSEC; 
	devicewfx.nBlockAlign		= 4; 

	devicewfx.nAvgBytesPerSec = SAMPLERSEC * SAMPLEBITS/8;  

	magic = RECMAGIC;

	//AP2N(_T("RECBUFFER=%d\r\n"), RECBUFFER);

	callthis = NULL; callthis2 = NULL; orgclass = NULL;

	//pprog = NULL;

	endthread = false;
	event = CreateEvent(NULL, 0, 0, NULL);
	thhandle = _beginthread(_RecThread, 0, this);
}

//////////////////////////////////////////////////////////////////////////

CRecordSound::~CRecordSound()

{
	//TRACE(_T("RecordSound::~CRecordSound\r\n"));

	//ASSERT(magic == RECMAGIC);

	// Just in case
	StopRecord();
	WaveInClose();

	endthread = true;
	SetEvent(event);
	Sleep(10);

	// Clean
	if(xisallocated)
		{
		for(int ii = 0; ii < MAXRECBUFFER; ii++)
			free(xrechead[ii]->lpData);

		for(int iii = 0; iii < MAXRECBUFFER; iii++)
			free(xrechead[iii]);
		}
}

//////////////////////////////////////////////////////////////////////////

void CRecordSound::xPreCreateHeader()

{
	for(int i = 0; i < MAXRECBUFFER; i++)
		xrechead[i] = xCreateWaveHeader();

	xisallocated = true;
}

//////////////////////////////////////////////////////////////////////////

void CRecordSound::xDisplayError(int mmReturn, TCHAR errmsg[])

{
	if(mmReturn == MMSYSERR_NOERROR)
		return;

	TCHAR errorbuffer[MAX_PATH];
	waveInGetErrorText(mmReturn, errorbuffer, MAX_PATH);

	CString tmp;
	tmp.Format(_T("CRecordSound: %s : %x : %s"), 
							errmsg, mmReturn, errorbuffer);
	
	//AP2N(_T("%s\r\n"), tmp);

	if(!silent)
		::MessageBox(NULL, tmp,  _T("CRecordSound"), MB_OK);
}

//////////////////////////////////////////////////////////////////////////

LPWAVEHDR  CRecordSound::xCreateWaveHeader()

{
	LPWAVEHDR lpHdr = new WAVEHDR; //ASSERT(lpHdr);

	if(lpHdr == NULL)
		{
		return NULL;
		}

	ZeroMemory(lpHdr, sizeof(WAVEHDR));

	char* lpByte = new char[RECBUFFER]; //ASSERT(lpByte);

	if(lpByte == NULL)
		{
		return NULL;
		}
	lpHdr->lpData = lpByte;
	lpHdr->dwBufferLength = RECBUFFER;
	return lpHdr;
}

//////////////////////////////////////////////////////////////////////////

void CRecordSound::StartRecord()

{
	if(!xisopened)
		{
		//AP2N(_T("CRecordSound::StartRecord must open first\r\n"));
		return ;
		}
	if(recording)
		{
		//AP2N(_T("CRecordSound::StartRecord reentry\r\n"));
		return ;
		}

	MMRESULT mmReturn;

	for(int i = 0; i < MAXRECBUFFER; i++)
		{
		mmReturn = ::waveInUnprepareHeader(m_hRecord, xrechead[i], sizeof(WAVEHDR));
		mmReturn = ::waveInPrepareHeader(m_hRecord, xrechead[i], sizeof(WAVEHDR));
		mmReturn = ::waveInAddBuffer(m_hRecord, xrechead[i], sizeof(WAVEHDR));
		}

	//AP2N(_T("CRecordSound::StartRecord reclen=%d\r\n"), RECBUFFER);
	
	mmReturn = ::waveInStart(m_hRecord);

	if(mmReturn != MMSYSERR_NOERROR )
		xDisplayError(mmReturn, _T("Start"));
	else
		recording = TRUE;

	//AP2N(_T("CRecordSound::waveInStart ret=%d\r\n"), mmReturn);

	return ;
}

//////////////////////////////////////////////////////////////////////////

void CRecordSound::StopRecord()

{
	//AP2N(_T("CRecordSound::StopRecord\r\n"));

	if(!recording)
		{
		//AP2N(_T("CRecordSound::StopRecord reentry\r\n"));
		return ;
		}
	recording = FALSE;
	
	MMRESULT mmReturn;

	mmReturn = ::waveInStop(m_hRecord);
	xDisplayError(mmReturn,  _T("Stop"));
	
	// To get the remaining sound data from buffer, reset sound 
	mmReturn = ::waveInReset(m_hRecord);
	xDisplayError(mmReturn, _T("Reset"));
	
	Sleep(10);

	//lock.Unlock();				// just to have it ready 
	//endthread = true;
	//SetEvent(event);

}

//////////////////////////////////////////////////////////////////////////

HWAVEIN CRecordSound::WaveInOpen(int dev)

{
	if(xisopened)
		{
		// No reopening the same device
		if(dev == xolddev)
			return 0;

		// Different device, force clean up
		WaveInClose();
		}

	//AP2N(_T("CRecordSound::WaveInOpen\r\n"));

	// Assume it is opened
	xisopened = true;

	//AP2N(_T("CRecordSound::WaveInOpen dev=%d reclen=%d\r\n"), dev, RECBUFFER); 

	MMRESULT mmReturn = ::waveInOpen(&m_hRecord, dev,
			&devicewfx, (DWORD)_WaveInProc, (DWORD)this, CALLBACK_FUNCTION);
	
	// Error has occured while opening device
	if(mmReturn != MMSYSERR_NOERROR )
		{
		xisopened = false;

		xDisplayError(mmReturn, _T("Open"));
		
		//AP2N(_T("Error on opening MM sound Device\r\n"));
		return (HWAVEIN)-1;
		}

	xolddev = dev;
	return m_hRecord;
}

//////////////////////////////////////////////////////////////////////////

void	CRecordSound::WaveInClose()

{
	if(!xisopened)
		return;

	//AP2N(_T("CRecordSound::WaveInClose dev=%d\r\n"), olddev);

	xisopened = false;

	StopRecord();
	
	MMRESULT mmReturn;

	mmReturn = ::waveInReset(m_hRecord);
	xDisplayError(mmReturn, _T("Reset"));

	mmReturn = ::waveInClose(m_hRecord);
	xDisplayError(mmReturn, _T("Close"));	

	m_hRecord = NULL;
}

//////////////////////////////////////////////////////////////////////////

void	CRecordSound::SetFormat(int channels, int sampersec, int bitssample)

{
	int bytessamp = bitssample / 8;

	devicewfx.nChannels			=	channels;
	devicewfx.nSamplesPerSec	=	sampersec;
	devicewfx.nAvgBytesPerSec	=	bytessamp * channels * sampersec;
	devicewfx.nBlockAlign		=	bytessamp * channels;
	devicewfx.wBitsPerSample	=	bitssample;	  
}