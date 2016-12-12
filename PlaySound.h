//////////////////////////////////////////////////////////////////////
// PlaySound.h: interface for the PlaySound class.
//

#if !defined(AFX_PLAYSOUND_H__263DF620_0B5C_11D6_90D8_00B04C390A3E__INCLUDED_)
#define AFX_PLAYSOUND_H__263DF620_0B5C_11D6_90D8_00B04C390A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

#include "mmsystem.h"
#include "soundpar.h"

#define PS_MAGIC 0x34242

// Bit defines for user buffer

#define PS_AUTOADD	0x40
#define PS_AUTOFREE	0x10
#define	PS_BUSY		0x20

//////////////////////////////////////////////////////////////////////////

class CPlaySound

{

private:

	static void CALLBACK CPlaySound::_waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,  
					 DWORD dwParam1,  DWORD dwParam2);

	static	void	CPlaySound::_PlayThread(void *lpParam);

public:		

	HWAVEOUT WaveOpen();
	void	WaveReset();
	int		WaveClose();
	int		IsPlaying();

	int		WaitForEnd();
	
	void	Play(char *buff, int len, int autofree = false);
	void	StopPlay();
	
	int		SaveWave(const TCHAR *fname, void *buff, int len);
	int		LoadWave(const TCHAR *fname, char **buff, int *len);

	int		PlayMonoWave(char *buff, int len, int autofree = false);
	int		PlayWave(char *buff, int len, int autofree = false);
	
	void	SetFormat(int channels, int sampersec, int bitssample);

	int		GetCurrPos();
	
	int		IsOpened() { return opened; };

	// Public Data

	// So it can be seen what happened
	WAVEFORMATEX devicewfx; 
	WAVEFORMATEX openwfx; 

	int		magic, silent;

	HWAVEOUT	m_hPlay;

	CPlaySound();
	virtual ~CPlaySound();

private:

	WAVEHDR	xwhead[10];

	void	xDisplayError(int mmReturn, TCHAR errmsg[]);
	int		xSubmitWavHead(WAVEHDR *whdr);
	int		xWriteField(HMMIO hFile, const char *four, const char *str);

	int		opened, lastpos, currhead;

	// Thread related
	int		thhandle, endthread;
	
	HANDLE  event;
	CPtrArray	queue;
	CCriticalSection lock;

	volatile int stop_play;
};

#endif // !defined(AFX_PLAYSOUND_H__263DF620_0B5C_11D6_90D8_00B04C390A3E__INCLUDED_)
