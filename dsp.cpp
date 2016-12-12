
/* =====[ dsp.cpp ]========================================== 
                                                                             
   Description:     The daco project, implementation of the dsp.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  8/24/2009  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#include "stdafx.h"
#include "math.h"

#include "dsp.h"
#include "mxpad.h"
#include "misc.h"

// The one and only instance

CDsp dsp;

CDsp::CDsp()

{
	magic = CDsp_Magic;
}

//////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------
//
/// \brief   Split Stereo Wave to two mono waves
/// 
/// \details DescHere 
/// \n  
/// Return type:      void 
/// \n  
/// \param *dorg Arg Type: short 
/// \param len Arg Type: int 
/// \param *lptr Arg Type: short 
/// \param *rptr Arg Type: short 
/// \param lim Arg Type: int 

void	CDsp::SplitWave(short *dorg, int len, short *lptr, short *rptr, int lim)

{
	int len2 = len / sizeof(short);

	for(int loop = 0; loop < len2; loop += 2)
		{
		*rptr++ = *dorg++, *lptr++ = *dorg++;
		}
}

//////////////////////////////////////////////////////////////////////////
// Get the left channel

void	CDsp::SplitWaveLeft(short *dorg, int len, short *lptr, int lim)

{
	int len2 = len / sizeof(short);

	for(int loop = 0; loop < len2; loop += 2)
		{
		*lptr++ = *dorg++;
		
		dorg++;				// Skip channel
		}
}


//////////////////////////////////////////////////////////////////////////
// Unite the two channels

void	CDsp::UnSplitWave(short *lptr, short *rptr, int len, short *dorg)

{
	int len2 = len / sizeof(short);

	for(int loop = 0; loop < len2; loop ++)
		{
		*dorg++ = *lptr++;	*dorg++ = *rptr++;
		}
}

/////////////////////////////////////////////////////////////////////////
// Load a standard wave file 

int CDsp::LoadWave(const TCHAR *fname, char **buff, int *len, WAVEFORMATEX *pwfx)

{
	int ret = true;

	MMRESULT res;
	HMMIO mmh = mmioOpen((LPTSTR)fname, NULL, MMIO_READ);

	if(!mmh)
		return(0);

	//ASSERT(magic == CDsp_Magic);

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
		//P2N(_T("CPlaySound::LoadWave no memory line %d\r\n"), __LINE__);
		return(0);
		}

	mmioRead( mmh, (char*)buf2, ck2.cksize);

	WAVEFORMATEX *wfx = (WAVEFORMATEX *)buf2;

	// Make a copy for query it
	memcpy(pwfx, wfx, sizeof(WAVEFORMATEX));

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
		P2N(_T("Not 16 bit sample depth\r\n"));

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
// Save and convert

int CDsp::SaveWave(const TCHAR *fname, void *buff, int len, WAVEFORMATEX *pwfx)

{
	int ret = 0;
	HMMIO hFile; 
	
	hFile = mmioOpen((TCHAR *)fname, NULL, MMIO_CREATE | MMIO_READWRITE); 

	if (hFile == NULL) 
		{
		//P2N(_T("CPlaySound::SaveWave Could not create file\r\n"));
		return ret;
		}

	// File created successfully. 
	ret = true;

	//ASSERT(magic == CDsp_Magic);

	WAVEFORMATEX wfx; 

	if(pwfx == NULL)
		pwfx = &wfx;
		
	pwfx->wFormatTag      		=	WAVE_FORMAT_PCM;
	pwfx->nChannels				=	2;
	pwfx->nSamplesPerSec		=	44100;
	pwfx->nAvgBytesPerSec		=	4 * 44100;
	pwfx->nBlockAlign			=	4;
	pwfx->wBitsPerSample		=	16;	  
	pwfx->cbSize				=	0;		// WMPlayer bailed out on this

	short *optr = (short*)malloc(len * pwfx->nChannels); //ASSERT(optr);

	dsp.UnSplitWave((short*)buff, (short*)buff, len, optr);

	MMCKINFO  mi; memset(&mi, 0, sizeof(mi));
	mi.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	mmioCreateChunk(hFile, &mi, MMIO_CREATERIFF );
	mmioAscend(hFile, &mi, 0);

	MMCKINFO  mi2; memset(&mi2, 0, sizeof(mi2));
	mi2.ckid = mmioFOURCC('f', 'm', 't',' ');
	mi2.cksize =  sizeof(WAVEFORMATEX);

	mmioCreateChunk(hFile, &mi2, 0 );
	mmioWrite(hFile, (char *)pwfx, sizeof(WAVEFORMATEX));
	mmioAscend(hFile, &mi2, 0);

	MMCKINFO  mi3; memset(&mi3, 0, sizeof(mi3));
	mi3.ckid = mmioFOURCC('d', 'a', 't','a');
	mi3.cksize =  len * pwfx->nChannels;

	mmioCreateChunk(hFile, &mi3, 0 );
	mmioWrite(hFile, (char *)optr, mi3.cksize);
	mmioAscend(hFile, &mi3, 0);

#if 1
	// Create info chunk
	MMCKINFO  mi4; memset(&mi4, 0, sizeof(mi4));
	mi4.ckid = mmioFOURCC('L', 'I', 'S', 'T');
	mi4.fccType = mmioFOURCC('I', 'N', 'F', 'O');
	mmioCreateChunk(hFile, &mi4, MMIO_CREATELIST);
	
	TCHAR n = 0;

	xWriteField(hFile, "INUT", "Version 1.0");
	xWriteField(hFile, "ISFT", "Voice Recognition Internal Data");
	xWriteField(hFile, "IART", "Voice Recognition Wave File");
	xWriteField(hFile, "ICOP", "Voice Recognition");

	// Stamp it
	CTime ct = CTime::GetCurrentTime();
	CString datestr = ct.Format(_T("%A, %B %d, %Y - %I:%M %p"));

	char *astr; misc.UnicodeToAnsi(datestr, &astr);
	xWriteField(hFile, "ICRD", astr);

	// Mark User/Machine it was created on
	TCHAR comp[255]; unsigned long lenc = sizeof(comp);
	GetComputerName(comp, &lenc); 

	CString ustr = "File Created on "; ustr += comp; ustr += " by ";
	lenc = sizeof(comp);
	GetUserName(comp, &lenc); 
	ustr += comp;

	misc.UnicodeToAnsi(ustr, &astr);
	xWriteField(hFile, "ICMT", astr);

	// Ascend out to the list chunk
	mmioAscend(hFile, &mi4, 0);

#endif

	// Ascend out to the main chunk
	mmioAscend(hFile, &mi, 0);

	mmioFlush(hFile, 0);
	mmioClose(hFile, 0);

	free(optr);

	return(ret);
}

//////////////////////////////////////////////////////////////////////////
// Write info field

int CDsp::xWriteField(HMMIO hFile, const char *four, const char *str)

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
// Configure DSP normalization 

#define	NORM_SOUND	1
#define COMPRESS	1

//#define PROCPOW 0.5
#define PROCPOW 0.85		// process power 
//#define PROCPOW 1

#define PROCOP pow			// process operator
//#define PROCOP sqrt
//#define PROCOP log

//////////////////////////////////////////////////////////////////////////

void CDsp::NormalizeWav(char *buff, int len, char *out)

{
	int len2 = len / sizeof(short);	
	
	//AP2N(_T("void CxRecog::Normalize(TCHAR *buff, int len=%d, TCHAR *out)\r\n"), len);
			
#ifdef NORM_SOUND

	short *pptr = (short*)buff, *outptr = (short*)out;
	short *pptr3 = (short*)buff, *outptr3 = (short*)out;

	double factor;

	//////////////////////////////////////////////////////////////////////
	// Compress to square root

#ifdef COMPRESS

	short *ptmp = (short*)malloc(len); //ASSERT(ptmp);
	short *ptrsqr = (short*)buff, *outsqr = ptmp;

	double rate = (double)SHRT_MAX / PROCOP((double)SHRT_MAX, PROCPOW);

	for(int loopx = 0; loopx < len2; loopx++)
		{
		double dval = (double) *ptrsqr++;

		// SQRT pos and neg
		if(dval > 0 )
			dval = rate * PROCOP(dval, PROCPOW);
		else
			dval = - (rate * PROCOP(-dval, PROCPOW));

		*outsqr++ = short(dval);
		}

	// Assign temp as the new imput
	pptr = ptmp; pptr3 = ptmp;
#endif

	// Calc maximum value
	// Calc avarage value (2)
	int loop, maxx = 0, avg = 0; double sum = 0;
	for(loop = 0; loop < len2; loop++)
		{
		short val = abs(*pptr++);

		if(maxx < val)
			 maxx = val;

		sum += val;
		}
	avg = int(sum / len2);

	//AP2N(_T("avg = %d\r\n"), avg);
	
	// Calc scaling factor, use max
	factor = (double)SHRT_MAX / maxx;
	
	// Calc scaling factor, use avarage
	//factor = (double)SHRT_MAX / maxx;
	//factor /= 10;	// Assume some dynamics
	//factor /= 6;	// Assume some dynamics
	
	//AP2N(_T("CxRecog::Normalize factor=%.f\r\n"), factor);
	
	// Limit scaling factor
	if(factor < 100)
		{
		// Multiply
		for(loop = 0; loop < len2; loop++)
			{
			short val = *pptr3++; 
			double dval = factor * val;		

			// Clip it, so no artifacts
			if(dval > SHRT_MAX) dval  = SHRT_MAX;
			if(dval < -SHRT_MAX) dval = -SHRT_MAX;

			*outptr3++ = short(dval);
			}
		}
	else
		{
		// Too big scale factor, copy straight
		memcpy(out, buff, len);
		}

#ifdef COMPRESS
	free(ptmp);
#endif

#else
	memcpy(out, buff, len);
#endif
	
}

