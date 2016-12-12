
/* =====[ PlaySound.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the PlaySound.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/31/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include <process.h>

#include "PlaySound.h"
#include "support.h"
#include "mxpad.h"
#include "misc.h"
#include "dsp.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=_T(__FILE__);
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Play message handlers

//	typedef struct {
//		WORD      wMid;
//		WORD      wPid;
//		MMVERSION vDriverVersion;
//		TCHAR      szPname[MAXPNAMELEN];
//		DWORD     dwFormats;
//		WORD      wChannels;
//		WORD      wReserved1;
//		DWORD     dwSupport; }
//

//typedef struct {
//    LPSTR  lpData;
//    DWORD  dwBufferLength;
//    DWORD  dwBytesRecorded;
//    DWORD  dwUser;
//    DWORD  dwFlags;
//    DWORD  dwLoops;
//    struct wavehdr_tag * lpNext;
//    DWORD  reserved;
//} WAVEHDR;

//typedef struct {
//	WORD  wFormatTag;
//	WORD  nChannels;
//	DWORD nSamplesPerSec;
//	DWORD nAvgBytesPerSec;
//	WORD  nBlockAlign;
//	WORD  wBitsPerSample;
//	WORD  cbSize;
//	} WAVEFORMATEX


void	CPlaySound::_PlayThread(void *lpParam)

{
	CPlaySound *ps = (CPlaySound*)lpParam;

	while(true)
		{
		WaitForSingleObject(ps->event, INFINITE);

		if(ps->endthread)
			break;

		// Process queue
		while(true)
			{			
			if(ps->endthread)
				break;

			if(ps->m_hPlay == NULL)
				break;

			ps->lock.Lock();
			int sss = ps->queue.GetSize();
			ps->lock.Unlock();
			
			if(sss == 0)
				break;

			ps->lock.Lock();
			LPWAVEHDR pwhdr = (LPWAVEHDR)ps->queue.GetAt(0);
			ps->lock.Unlock();
		
			//AP2N(_T("PlayThread got %p %d queue=%d user=%d\r\n"), 
			//			pwhdr->lpData, pwhdr->dwBufferLength, ps->queue.GetSize(), pwhdr->dwUser);
			
			// Done with this buffer, give it back
			if(pwhdr->dwUser & PS_AUTOFREE)
				{
				//AP2N(_T("Freeing %p\r\n"), pwhdr->lpData);

				pwhdr->dwUser &=  ~PS_AUTOFREE;
				free(pwhdr->lpData);
				}

			// Windows bug: locked when zero length buffer passed
			if(pwhdr->dwBufferLength && ps->m_hPlay !=  NULL)
				::waveOutUnprepareHeader(ps->m_hPlay, pwhdr, sizeof(WAVEHDR));

			// Signal the class this is free			
			pwhdr->dwUser &=  ~PS_BUSY;

			ps->lock.Lock();
			ps->queue.RemoveAt(0);
			ps->lock.Unlock();		
			}
		}

	TRACE(_T("Ended sound playing Thread\r\n"));

	_endthread();
 
    return;
}


void CALLBACK CPlaySound::_waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,  
					 DWORD dwParam1,  DWORD dwParam2)

{
	CPlaySound *ps = (CPlaySound*)dwInstance;

	switch(uMsg)
		{
		case MM_WOM_DONE:
			{
			LPWAVEHDR	lpHdr = (LPWAVEHDR) dwParam1 ;
			
			//AP2N(_T("MM_WOM_DONE hwo=%d buff=%p header=%p flags=%d user=%d len=%d\r\n"),
			//				hwo, lpHdr->lpData, lpHdr, lpHdr->dwFlags, 
			//					lpHdr->dwUser, lpHdr->dwBufferLength);
			
			//AP2N(_T("MM_WIM_DATA %p len=%d\r\n"), lpHdr->lpData, lpHdr->dwBytesRecorded);

			ps->lock.Lock();
			ps->queue.Add(lpHdr);
			ps->lock.Unlock();

			SetEvent(ps->event);
			}

		break;

		case MM_WOM_OPEN:
			//AP2N(_T("MM_WOM_OPEN\r\n"));
		break;

		case MM_WOM_CLOSE:
			//AP2N(_T("MM_WOM_CLOSE\r\n"));
		break;

		}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlaySound::CPlaySound()

{
	magic = PS_MAGIC;

	opened = stop_play = currhead = lastpos = 0;

	memset(xwhead, 0, sizeof(xwhead));
	memset(&openwfx, 0, sizeof(openwfx));
	memset(&devicewfx, 0, sizeof(devicewfx));

	devicewfx.wFormatTag      	=	WAVE_FORMAT_PCM;
	devicewfx.nChannels			=	2;
	devicewfx.nSamplesPerSec	=	44100;
	devicewfx.nAvgBytesPerSec	=	4 * 44100;
	devicewfx.nBlockAlign		=	4;
	devicewfx.wBitsPerSample	=	16;	  

	m_hPlay = NULL;

	silent = endthread = false;

	event = CreateEvent(NULL, 0, 0, NULL);
	thhandle = _beginthread(_PlayThread, 0, this);
}

//////////////////////////////////////////////////////////////////////

CPlaySound::~CPlaySound()

{
	TRACE(_T("CPlaySound::~CPlaySound()\r\n"));

	//ASSERT(magic == PS_MAGIC);

	//WaveClose();
	//while(IsPlaying())
	//	support.YieldToWinEx();;

	endthread = true;
	SetEvent(event);
	Sleep(10);
}

//////////////////////////////////////////////////////////////////////
// Play from memory

int CPlaySound::xSubmitWavHead(WAVEHDR *whdr)

{
	MMRESULT mmReturn;

	mmReturn = ::waveOutPrepareHeader(m_hPlay, whdr, sizeof(WAVEHDR));
	xDisplayError(mmReturn, _T("Prepare"));

	mmReturn = ::waveOutWrite(m_hPlay, whdr, sizeof(WAVEHDR));
	xDisplayError(mmReturn, _T("Write"));

	return(TRUE);
}

//////////////////////////////////////////////////////////////////////////

int CPlaySound::PlayMonoWave(char *buff, int len, int autofree)

{
	short *optr = (short*)malloc(len * sizeof(short)); //ASSERT(optr);
	dsp.UnSplitWave((short*)buff, (short*)buff, len, optr);

	if(autofree)
		{
		free(buff);
		}

	//support.HeapCheck(_T("CPlaySound::PlayMonoWave"));

	//return PlayWave((char*)optr, len * sizeof(short), true);
	return PlayWave((char*)optr, len * sizeof(short));
}

//////////////////////////////////////////////////////////////////////////

int CPlaySound::PlayWave(char *buff, int len, int autofree)

{
	//ASSERT(magic == PS_MAGIC);

	if(!opened)
		return (0);

	//mmt.wType = TIME_BYTES;
	stop_play = FALSE;

	//P2N(_T("Play Wave Open res = %d name %s\r\n"),
	//			res, woc.szPname);

	int	loop, numheads = sizeof(xwhead) / sizeof(WAVEHDR);

	for( loop = 0; loop < numheads; loop++)
		{
		if((xwhead[loop].dwUser & PS_BUSY) == 0)
			{
			// Signal for busy .. (reserve it)
			xwhead[loop].dwUser = PS_BUSY;
		
			memset(&xwhead[loop], 0, sizeof(WAVEHDR));
			
			// Signal for busy 
			xwhead[loop].dwUser |= PS_BUSY;

			// Signal for  freeing ...
			//if(autofree)
				xwhead[loop].dwUser |= PS_AUTOFREE;
	
			xwhead[loop].lpData = buff;
			xwhead[loop].dwBufferLength = len;		
		
			//AP2N(_T("used header %p buffer %p\r\n"), &whead[loop], buff);
			break;
			}
		}

	// Check if we have succeeded on finding a buffer
	if(loop == numheads)
		{
		AP2N(_T("CPlaySound::PlayWave Overflow\r\n"));
		return 0;
		}

	//P2N(_T("Started play buff=%p header=%p autofree=%d\r\n"), buff, &whead[loop], autofree);
	
	currhead = loop;
	xSubmitWavHead(&xwhead[loop]);

	//AP2N(_T("Ended Play\r\n"));
	
	return(TRUE);
}

//////////////////////////////////////////////////////////////////////

HWAVEOUT CPlaySound::WaveOpen()

{
	//P2N(_T("CPlaySound::WaveOpen\r\n"));

	if(opened)
		return (HWAVEOUT)-1;

	WAVEOUTCAPS woc; memset(&woc, 0, sizeof(woc));

	//DECL_STRUCT(woc, WAVEOUTCAPS);

	//res = waveOutGetDevCaps(0,  &woc, sizeof(WAVEOUTCAPS));
	//P2N(_T("WAV cap ret = %d name %s\r\n"), res, woc.szPname);

	//WAVE_FORMAT_4S16


	//MMRESULT res = waveOutOpen( &m_hPlay, 0, &devicewfx, 
	//	(DWORD)AfxGetApp()->m_pMainWnd->m_hWnd, 0, CALLBACK_WINDOW);

	MMRESULT mmReturn = waveOutOpen( &m_hPlay, 0, &devicewfx, 
		(DWORD)_waveOutProc, (DWORD)this, CALLBACK_FUNCTION);

	xDisplayError(mmReturn, _T("Open"));

	if(mmReturn == MMSYSERR_NOERROR)
		opened = true;

	return(m_hPlay);
}

//////////////////////////////////////////////////////////////////////
// Load and convert

int CPlaySound::LoadWave(const TCHAR *fname, char **buff, int *len)

{
	int ret = true;

	MMRESULT res;
	HMMIO mmh = mmioOpen((TCHAR *)fname, NULL, MMIO_READ);

	if(!mmh)
		return(0);

	//ASSERT(magic == PS_MAGIC);

	// Interpret Wave information:
	MMCKINFO ck;
	res = mmioDescend( mmh, &ck, NULL, 0);
	//P2N(_T("mmdescend1 id=%.4s\r\n"), &ck.ckid);

	// printf
	MMCKINFO ck2;
	res = mmioDescend( mmh, &ck2, NULL, 0);
	//P2N(_T("mmdescend2 id=%.4s\r\n"), &ck2.ckid);

	char *buf2 = (char*)malloc(ck2.cksize);
	if(!buf2)
		{
		P2N(_T("CPlaySound::LoadWave no memory line %d\r\n"), __LINE__);
		return(0);
		}

	mmioRead( mmh, (char*)buf2, ck2.cksize);

	WAVEFORMATEX *wfx = (WAVEFORMATEX *)buf2;

	// Make a copy for query it
	memcpy(&openwfx, wfx, sizeof(WAVEFORMATEX));

	//P2N(_T("openwfx.wFormatTag         %d\r\n"), openwfx.wFormatTag);
	//P2N(_T("openwfx.nChannels          %d\r\n"), openwfx.nChannels);
	//P2N(_T("openwfx.nSamplesPerSec     %d\r\n"), openwfx.nSamplesPerSec);
	//P2N(_T("openwfx.nAvgBytesPerSec    %d\r\n"), openwfx.nAvgBytesPerSec);
	//P2N(_T("openwfx.nBlockAlign        %d\r\n"), openwfx.nBlockAlign);
	//P2N(_T("openwfx.wBitsPerSample     %d\r\n"), openwfx.wBitsPerSample);

	res = mmioAscend( mmh, &ck2, 0);
	
	MMCKINFO ck3;
	res = mmioDescend( mmh, &ck3, NULL, 0);
	//P2N(_T("mmdescend 3 id=%.4s size=%d\r\n"), &ck3.ckid, ck3.cksize);
	
	char *buf1 = (char*)malloc(ck3.cksize);
	if(!buf1)
		{
		//P2N(_T("CPlaySound::LoadWave no memory line %d\r\n"), __LINE__);
		free(buf2);
		return(0);
		}

	int rlen = mmioRead( mmh, buf1, ck3.cksize);
	res = mmioAscend( mmh, &ck3, 0);

	//////////////////////////////////////////////////////////////////////
	// Read in list chunk
	MMCKINFO ck4;
	res = mmioDescend( mmh, &ck4, NULL, 0);

	if(res == MMSYSERR_NOERROR)
		{
		char xstr[255];

		//P2N(_T("mmdescend 4 id=%.4s fcc=%.4s size=%d\r\n"), 
		//					&ck4.ckid, &ck4.fccType, ck4.cksize);

		unsigned int list = mmioFOURCC('L', 'I', 'S', 'T');
		unsigned int info = mmioFOURCC('I', 'N', 'F', 'O');

		if(ck4.ckid == list && ck4.fccType == info)
			{
			unsigned int prog = 0, cnt = 0;
			
			while(true)
				{
				MMCKINFO ck5;
				MMRESULT res2 = mmioDescend( mmh, &ck5, NULL, 0);

				if(res2 != MMSYSERR_NOERROR)
					break;

				//P2N(_T("mmdescend 5 id=%.4s size=%d\r\n"), &ck5.ckid, ck5.cksize);

				mmioRead(mmh, xstr, min(sizeof(xstr), ck5.cksize));

				//P2N(_T("%.4s -> %s\r\n"), &ck5.ckid, xstr);

				res = mmioAscend( mmh, &ck5, 0);
				prog += ck5.cksize;

				if(prog >= ck4.cksize)
					break;

				if(cnt > 200)   // safety break
					break;
				}
			}
		}

	//P2N(_T("Loaded %d bytes (%s)\r\n"), *len, fname);

	if(wfx->wBitsPerSample != 16)
		{
		//P2N(_T("Not 16 bit sample depth\r\n"));

		ret = false;
		}

	// Do some basic conversion, wave in buf1, len in wavesize
	DWORD wavesize = ck3.cksize;

	if(wfx->nChannels != 2)
		{
		//P2N(_T("Converting wave to stereo\r\n"));

		char *buf4 = (char*)malloc(2 * wavesize);
		if(!buf4)
			{
			free(buf2);	free(buf1);
			//P2N(_T("Cannot allocate buffer for stereo conversion.\r\n"));
			return(false);
			}
		for(DWORD loop = 0; loop < wavesize / 2; loop += 1)
			{
			((short*)buf4)[2*loop]   =   ((short*)buf1)[loop];
			((short*)buf4)[2*loop+1] =   ((short*)buf1)[loop];
			}
		free(buf1);
		buf1 = buf4;
		wavesize *= 2;
		}
	
	// Conversion needed
	if(wfx->nSamplesPerSec != 44100)
		{
		int rat = 44100 / wfx->nSamplesPerSec;

		//P2N(_T("Converting Wave from %d to 44100 %d\r\n"), wfx->nSamplesPerSec, rat);

		char *buf3 = (char*)malloc(rat * wavesize);
		if(!buf3)
			{
			free(buf2);	free(buf1);
			//P2N(_T("Cannot allocate buffer for sample rate conversion\r\n"));
			return(false);
			}

		for(DWORD loop = 0; loop < wavesize / 2; loop += 1)
			{
			for(int loop2 = 0; loop2 < rat; loop2 += 1)
				{
				((short*)buf3)[ rat * loop + loop2 ] =  ((short*)buf1)[loop];
				}
			}

		free(buf1);
		buf1 = buf3;
		wavesize *= rat;	
		}

	// Commit wave values
	*buff = buf1;
	*len  = wavesize;

	mmioClose(mmh, 0);
	free (buf2);

	return(ret);
}

//////////////////////////////////////////////////////////////////////

int CPlaySound::WaveClose()

{
	//P2N(_T("CPlaySound::WaveClose\r\n"));

	if(!opened)
		return 0;

	opened = false;

	StopPlay();
	
	MMRESULT mmReturn;
	
	mmReturn = waveOutClose(m_hPlay);	
	//xDisplayError(mmReturn, "Close");
	m_hPlay = NULL;

	return(mmReturn);
}

//////////////////////////////////////////////////////////////////////

void CPlaySound::StopPlay()

{
	//AP2N(_T("CPlaySound::StopPlay\r\n"));

	stop_play = TRUE;
	MMRESULT mmReturn = waveOutReset(m_hPlay);
	xDisplayError(mmReturn, _T("Reset"));

	WaitForEnd();
}

//////////////////////////////////////////////////////////////////////////

int CPlaySound::IsPlaying()
 
 {
	if(!opened)
 		return (0);
 
 	int ret = 0;
 	int	numhead = sizeof(xwhead) / sizeof(WAVEHDR);
 
 	for(int loop = 0; loop < numhead; loop++)
 		{
 		// Any of the heads in queue?
 		if(xwhead[loop].dwUser & PS_BUSY)
 			{
 			ret = true;
 			break;
 			}
 		}
 	return ret;
}

//////////////////////////////////////////////////////////////////////////

int CPlaySound::WaitForEnd()

{
	if(!opened)
		return (0);

	int	numhead = sizeof(xwhead) / sizeof(WAVEHDR);

	for(int loop = 0; loop < numhead; loop++)
		{
		if(xwhead[loop].dwUser & PS_BUSY)
			{
			while(TRUE)
				{
				//AP2N(_T("Waiting\r\n"));

				if(!(xwhead[loop].dwUser & PS_BUSY))
					break;
		
				// Ease up on the processor
				Sleep(10);
				support.YieldToWinEx();
				}
			}
		}

	// Bad Idea, duplicate free
	// Are there any to autofree?
	//for(loop = 0; loop < numhead; loop++)
	//	{
	//	if(whead[loop].dwUser & PS_AUTOFREE)
	//		{
	//		//AP2N(_T("Freeing %p\r\n"), lpHdr->lpData);
	//		free(whead[loop].lpData);
	//		}
	//	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////

int		CPlaySound::GetCurrPos()

{
	if(!opened)
		return (-1);

	MMRESULT mmReturn; 
	MMTIME	mmt; memset(&mmt, 0, sizeof(mmt)); 	mmt.wType = TIME_BYTES;

	mmReturn = ::waveOutGetPosition(m_hPlay, &mmt, sizeof(mmt));

	//xDisplayError(mmReturn, "CurrPos");

	if(mmReturn != MMSYSERR_NOERROR )
		{
		AP2N(_T("CPlaySound::GetCurrPos error %d\r\n"), mmReturn);
		return -2;
		}

	//ASSERT(mmt.wType == TIME_BYTES);

	return mmt.u.cb;
}

//////////////////////////////////////////////////////////////////////////

void CPlaySound::WaveReset()

{
	lastpos = 0;
	waveOutReset(m_hPlay);
}

//////////////////////////////////////////////////////////////////////////

void CPlaySound::Play(char *buff, int len, int autofree)

{
	AP2N(_T("CPlaySound::Play started Play\r\n"));
	
	if(!opened)
		return ;

	PlayWave(buff, len, autofree);

	WaitForEnd();

	//AP2N(_T("CPlaySound::Play Ended Play\r\n"));
}

//////////////////////////////////////////////////////////////////////
// Save and convert

int CPlaySound::SaveWave(const TCHAR *fname, void *buff, int len)

{
	HMMIO hFile; 
	
	hFile = mmioOpen((TCHAR *)fname, NULL, MMIO_CREATE | MMIO_READWRITE); 

	if (hFile == NULL) 
		{
		AP2N(_T("CPlaySound::SaveWave Could not create file\r\n"));
		return 0;
		}

	//ASSERT(magic == PS_MAGIC);

	WAVEFORMATEX wfx; 
		
	wfx.wFormatTag      	=	WAVE_FORMAT_PCM;
	wfx.nChannels			=	2;
	wfx.nSamplesPerSec		=	44100;
	wfx.nAvgBytesPerSec		=	4 * 44100;
	wfx.nBlockAlign			=	4;
	wfx.wBitsPerSample		=	16;	  
	wfx.cbSize				=	0;		// WMPlayer bailed out on this

	short *optr = (short*)malloc(len * wfx.nChannels); //ASSERT(optr);

	dsp.UnSplitWave((short*)buff, (short*)buff, len, optr);

	// File created successfully. 

	MMCKINFO  mi; memset(&mi, 0, sizeof(mi));
	mi.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	mmioCreateChunk(hFile, &mi, MMIO_CREATERIFF );
	mmioAscend(hFile, &mi, 0);

	MMCKINFO  mi2; memset(&mi2, 0, sizeof(mi2));
	mi2.ckid = mmioFOURCC('f', 'm', 't',' ');
	mi2.cksize =  sizeof(WAVEFORMATEX);

	mmioCreateChunk(hFile, &mi2, 0 );
	mmioWrite(hFile, (char *)&wfx, sizeof(WAVEFORMATEX));
	mmioAscend(hFile, &mi2, 0);

	MMCKINFO  mi3; memset(&mi3, 0, sizeof(mi3));
	mi3.ckid = mmioFOURCC('d', 'a', 't','a');
	mi3.cksize =  len * wfx.nChannels;

	mmioCreateChunk(hFile, &mi3, 0 );
	mmioWrite(hFile, (char *)optr, mi3.cksize);
	mmioAscend(hFile, &mi3, 0);

#if 1
	// Create 
	MMCKINFO  mi4; memset(&mi4, 0, sizeof(mi4));
	mi4.ckid = mmioFOURCC('L', 'I', 'S', 'T');
	mi4.fccType = mmioFOURCC('I', 'N', 'F', 'O');
	mmioCreateChunk(hFile, &mi4, MMIO_CREATELIST);
	
	CString str; char n = 0;

	xWriteField(hFile, "INUL", "DACO Version 1.0");
	xWriteField(hFile, "ISFT", "Peter Glen's Voice Recognition");
	xWriteField(hFile, "IART", "Dog Bark File");
	xWriteField(hFile, "ICOP", "Voice Recognition");

	// Stamp it
	CTime ct = CTime::GetCurrentTime();
	CString datestr = ct.Format(_T("%A, %B %d, %Y - %I:%M %p"));

	char *astr; misc.UnicodeToAnsi(datestr, &astr);
	xWriteField(hFile, "ICRD", astr);
	
	// Mark User/Machine it was created on
	TCHAR comp[255];	unsigned long lenc = sizeof(comp);
	GetComputerName(comp, &lenc); 
	str = "File Created on "; str += comp; str += " by ";
	lenc = sizeof(comp);
	GetUserName(comp, &lenc); 
	str += comp;
	misc.UnicodeToAnsi(str, &astr);
	xWriteField(hFile, "ICMT", astr);

	// Ascend out to the list chunk
	mmioAscend(hFile, &mi4, 0);

#endif

	// Ascend out to the main chunk
	mmioAscend(hFile, &mi, 0);

	mmioFlush(hFile, 0);
	mmioClose(hFile, 0);

	free(optr);

	return(1);
}

//////////////////////////////////////////////////////////////////////////
// Write info field

int CPlaySound::xWriteField(HMMIO hFile, const char *four, const char *str)

{
	TCHAR nn = 0;
	//////////////////////////////////////////////////////////////////////////
	MMCKINFO  mi5; memset(&mi5, 0, sizeof(mi5));

	mi5.ckid = mmioFOURCC(four[0], four[1], four[2], four[3]);
	mmioCreateChunk(hFile, &mi5, 0);

	mmioWrite(hFile, str, strlen(str) + 1);
	//mmioWrite(hFile, &nn,  sizeof(n));
	mmioAscend(hFile, &mi5, 0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////

void CPlaySound::xDisplayError(int mmReturn, TCHAR errmsg[])

{
	if(mmReturn == MMSYSERR_NOERROR)
		return;

	TCHAR errorbuffer[MAX_PATH];
	waveOutGetErrorText(mmReturn, errorbuffer, MAX_PATH);

	CString tmp;
	tmp.Format(_T("CPlaySound: %s : %x : %s"), errmsg, mmReturn, errorbuffer);
	
	AP2N(_T("%s\r\n"), tmp);

	if(!silent)
		::MessageBox(NULL, tmp,  _T("CPlaySound"), MB_OK);
}

//////////////////////////////////////////////////////////////////////////

void	CPlaySound::SetFormat(int channels, int sampersec, int bitssample)

{
	int bytessamp = bitssample / 8;

	devicewfx.nChannels			=	channels;
	devicewfx.nSamplesPerSec	=	sampersec;
	devicewfx.nAvgBytesPerSec	=	bytessamp * channels * sampersec;
	devicewfx.nBlockAlign		=	bytessamp * channels;
	devicewfx.wBitsPerSample	=	bitssample;	  
}