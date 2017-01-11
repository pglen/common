
/* =====[ bluepoint.cpp ]========================================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the bluepoint.cpp                
                                                                                                                                                          
   Compiled:        MS-VC 6.00, MSVS 2015                                            
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------     -----------  ----------------------------   
	  0.00  6/17/2010	 Peter Glen   Initial version.
	  0.00  dec/27/2016  Peter Glen   Reworked for UNICODE MBCS W10
	  
   ======================================================================= */

// Changes:
// 08-Apr-2008 Adapted to visual C++
// 10-Apr-2008 Defence for longer passwords

// -------------------------------------------------------------------------
// Bluepoint encryption routines.
//
//   How it works:
//
//     Strings are walked char by char with the loop:
//
//    for (loop = 0; loop < slen; loop++)
//        {
//        aa = str[loop];
//        }
//
//    In other languages:
//         {
//         $aa = ord(substr($_[0], $loop, 1));
//         do something with $aa
//         substr($_[0], $loop, 1) = pack("c", $aa);
//         }
//
//   Flow:
//         generate vector
//         generate pass
//         walk forward with password cycling loop
//         walk backwards with feedback encryption
//         walk forward with feedback encryption
//
//  The process guarantees that a single bit change in the original text
//  will change every byte in the resulting block.
//
//  The bit propagation is such a high quality, that it beats current
//  industrial strength encryptions.
//
//  Please see bit distribution study.
//
// -------------------------------------------------------------------------
//
// How to use in C++:
//	CString str("original"), pass("pass");
//  bluepoint.encrypt(str, pass);
//  bluepoint.decrypt(str, pass);
//
// OR:
//  bluepoint_encrypt($orig, $pass);                -- encrypted in place
//  bluepoint_decrypt($cypher, $pass);              -- decrypted in place
//  $hash       = bluepoint_hash($orig, $pass);
//  $crypthash  = bluepoint_crypthash($orig, $pass);
//
// Porting is correct if the new cypher text is a duplicate of the following:
//
//#CRYPT#
//a265 e400 6e4d 5f5b 9c47 8b37 d744 29db ef45 bd5a 26e8 4300 a7b1 cdbf 8e7a 09a6
//5606 3116 5e70 b9c2 e21e 976b 3753 294f ddc8 1281  8145938e
//#XCRYPT#
//
// orignal='abcdefghijklmnopqrstuvwxyz' pass='1234'
// The CString class stays 8 bit clean (nul characters) if you release buffers 
// with explicit length arguments. [ Like: str.ReleaseBuffer(length)]
//

///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bluepoint.h"
#include "mxpad.h"

//////////////////////////////////////////////////////////////////////////

// The crypt delimeters have hex chars in them 'C'

CString CBluePoint::pre = _T("#CRYPT#\r\n");
CString CBluePoint::post = _T("\r\n#XCRYPT#");

CBluePoint bluepoint;

//////////////////////////////////////////////////////////////////////////

xCBluePoint::xCBluePoint()

{
	vector		= "crypt";			//# influence encryption algorythm

	forward		= 0x55;				//# Constant propagated on forward pass
	backward	= 0x5a;				//# Constant propagated on backward pass
	addend		= 17;				//# Constant used adding to encrypted values

	//# -------------------------------------------------------------------------
	//# These vars can be set show op details

	verbose		= 0;				//# Specify this to show working details
	functrace	= 0;				//# Specify this to show function args
	debug		= 0;				//# Specify this to show P2Ns for test
}

void    CBluePoint::encrypt(CString &str, const CString &pass)
{
	TCHAR *buff = str.GetBuffer(); int len = str.GetLength(); 
	const TCHAR *pbuff = pass; int plen = pass.GetLength();
	int mult = sizeof(TCHAR);
	xCBluePoint::encrypt((char*)buff, len * mult, (const char*)pbuff, plen * mult);
	//TCHAR *buff3 = (TCHAR*)malloc(len * mult);
	//memcpy(buff3, buff, len * mult);
	str.ReleaseBuffer(len);

	//TCHAR *buff2 = str.GetBuffer();
	//int ret = memcmp(buff2, buff3, len * mult);
	//P2N(_T("memcmp %d.\r\n"), ret);
}

//////////////////////////////////////////////////////////////////////////
// Encrypt text, hash it , package it between delimiters.

void    CBluePoint::encrypthex(CString &str, const CString &pass)

{
	if (debug)
	{
		P2N(_T("org:\r\n"));
		const TCHAR *buff = str;
		bluepoint_dumphex((const char*)buff, str.GetLength() * sizeof(TCHAR));
	}

	CString fff, hex;
	ulong hhh = crypthash(str, pass); //CString("a"));
	fff.Format(_T(" %08x "), hhh);	
	//encrypt(str, pass);
	if (debug)
	{
		P2N(_T("enc:\r\n"));
		const TCHAR *buff = str;
		bluepoint_dumphex((const char*)buff, str.GetLength() * sizeof(TCHAR));
	}
	tohexW(str, hex, 16);
	str = CBluePoint::pre + hex + fff + CBluePoint::post;
	killpass(hex);
}

//# -------------------------------------------------------------------------
//# Use: encrypt($str, $password);

void    xCBluePoint::encrypt(char *buff, int blen, const char *pass, int plen)

{
    char newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

   if(functrace)
       {
       //printf("bluepoint_encrypt\nbuff=%s\n", bluepoint_dumphex(buff, blen));
       //printf("pass=%s\n", bluepoint_dumphex(pass, plen) );
       }

    prep_pass(pass, plen, newpass);
    do_encrypt(buff, blen, newpass, PASSLIM);

	for(int loop = 0; loop < PASSLIM; loop++)
		newpass[loop] = char(rand() % 255);
}

void    CBluePoint::decrypt(CString &str, const CString &pass)
{
	int mult = sizeof(TCHAR);
	TCHAR *buff = str.GetBuffer(); int len = str.GetLength();
	const TCHAR *pbuff = pass; int plen = pass.GetLength();
	xCBluePoint::decrypt((char*)buff, len * mult, (const char*)pbuff, plen * mult);
	str.ReleaseBuffer(len);
}

// xxxxxxxxxxxx#CRYPT#_________ hhhhhhhh #XCRYPT#xxxxxxxxxxx
//                    idx       idx4     idx3     idx2

int    CBluePoint::decrypthex(CString &str, const CString &pass)
{
	int idx = str.Find(CBluePoint::pre);
	if (idx < 0)
	{
		P2N(_T("No header found.\r\n"));
		return CRYPT_NOHEAD;
	}
	idx += CBluePoint::pre.GetLength();
	int idx2 = str.Find(CBluePoint::post);
	if (idx2 < 0)
		{
		P2N(_T("No trailer found.\r\n"));
		return CRYPT_NOTAIL;
		}
	// Both begin and end found, walk to digit
	int idx4 = idx2, idx3 = idx2;
	while (true) 
		{
		if (idx3 <= idx) break;
		TCHAR chh = str.GetAt(idx3);
		if (isxdigit(chh)) { idx4 = idx3 + 1; break; }
		idx3--;
		}
	// Walk back to non digit
	while (true)
		{
		if (idx3 <= idx) break;
		TCHAR chh = str.GetAt(idx3);
		if (!isxdigit(chh)) { idx3++; break; } // Point to first
		idx3--;
		}
	//P2N(_T("idx4=%d idx3=%d\r\n"), idx4, idx3);
	
	int len3 = idx3 - idx;
	if (len3 <= 0)
		{
		P2N(_T("Nothing to decrypt.\r\n"));
		return CRYPT_EMPTY;
		}
	int hlen = idx4 - idx3;
	CString hstr = str.Mid(idx3, hlen);
	ulong hhh =  _tcstoul(hstr, NULL, 16);
	if (verbose)
	{
		P2N(_T("Hash '%s' %x\r\n"), hstr, hhh);
	}
	CString uhex, hex2 = str.Mid(idx, len3);
	fromhexW(hex2, uhex);
	if (debug)
	{
		//P2N(_T("Hash '%s' %x\r\n"), hstr, hhh);
		//P2N(_T("Decr len=%d ulen=%d\r\n"), len3, hex2.GetLength());
		const TCHAR *buff = uhex;
		bluepoint_dumphex((const char*)buff, uhex.GetLength() * sizeof(TCHAR));
	}
	//decrypt(uhex, pass);
	ulong ccc = crypthash(uhex, pass); //CString("a"));
	if (verbose)
	{
		P2N(_T("Check org=%x new=%x\r\n"), hhh, ccc);
	}
	if (ccc != hhh)
		{
		P2N(_T("Bad pass.\r\n"));
		killpass(uhex);		
		return CRYPT_BADPASS;
		}
	// Got it, commit
	str = uhex;		
	killpass(uhex);
	
	return CRYPT_OK;
}

//# -------------------------------------------------------------------------
//# Use: bluepoint_decrypt($str, $password);

void    xCBluePoint::decrypt(char *buff, int blen, const char *pass, int plen)

{
    char newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

    if(functrace)
        {
        //printf("bluepoint_decrypt()\nbuff=%s\n", bluepoint_dumphex(buff, blen));
        //printf("pass=%s\n", bluepoint_dumphex(pass, plen) );
        }

    prep_pass(pass, plen, newpass);

    do_decrypt(buff, blen, newpass, PASSLIM);

	for (int loop = 0; loop < PASSLIM; loop++)
		newpass[loop] = char(rand() % 255);
}

//////////////////////////////////////////////////////////////////////////
// Destroy pass

void    CBluePoint::killpass(CString &str)
{
	CString strp; int len = str.GetLength();
	
	for (int loop = 0; loop < len; loop++)
		{
		//strp += TCHAR(rand() % ('z' - 'a') + 'a');
		// Make it look like more of a random string
		strp += TCHAR(rand() % 255);
	}
	str = strp;
}

///////////////////////////////////////////////////////////////////////////
// Prepare pass

void    xCBluePoint::prep_pass(const char *pass, int plen, char *newpass)

{
    int loop;
    char vec2[PASSLIM + 1];

	memset(vec2, 0, sizeof(vec2));

    // Duplicate vector
    int vlen = strlen(vector);
    strcpy(vec2, vector);
	vec2[PASSLIM] = 0;

    newpass[0] = 0;

	// Expand/truncate pass to fit:
	if(plen >= PASSLIM)
		{
		strncpy(newpass, pass, PASSLIM);
		}
	else
		{
		for(loop = 0; loop < PASSLIM / (plen); loop++)
			{
			strncat(newpass, pass, plen);
			strcat(newpass, "_");
			}
		}

	// Terminate:
    newpass[PASSLIM] = 0;

	//TRACE("newpass '%s'\r\n", newpass);

	if (verbose)
	{
		//P2N(_T("prep_pass() newpass: %s\n"), newpass);
	}
        

#ifndef NOPASSCRYPT
    xCBluePoint::do_encrypt(vec2, vlen, vector, vlen);
#endif

    if(verbose)
        {
		//P2N(_T("prep_pass() eVEC: "));
        //bluepoint_dumphex(vec2, vlen);
		//P2N(_T("\r\n"));
        }

#ifndef NOPASSCRYPT
    xCBluePoint::do_encrypt(newpass, PASSLIM, vec2, vlen);
#endif

	//char  out[2 * PASSLIM + 1];
	//int loop2;
	//for(loop2 = 0; loop2 < 2 * PASSLIM; loop2 += 2)
	//	{
	//	unsigned char hi = ((unsigned char)(newpass[loop2 / 2] )) >> 4;
	//	unsigned char lo = (unsigned char) ((newpass[loop2 / 2] & 0xf));
	//
	//	out[loop2] = hi <= 9 ? hi + '0' : hi + ('a' - 10);
	//	out[loop2 + 1] = lo <= 9 ? lo + '0' : lo + ('a' - 10);
	//	}
	//out[2 * PASSLIM] = 0;
	//TRACE("newpass: '%s'", out);
}

ulong   CBluePoint::hash(const CString &str)

{
	const TCHAR *buff = str; int len = str.GetLength();
	ulong ret = xCBluePoint::hash((const char *)buff, len * sizeof(TCHAR));
	return ret;
}

//# -------------------------------------------------------------------------
//# Hash:
//# use: hashvalue = hash($str)
//#
//# Implementing the following 'C' code
//#
//#   ret_val ^= (unsigned long)*name;
//#   ret_val  = ROTATE_LONG_RIGHT(ret_val, 10);          /* rotate right */sub hash
//#

ulong   xCBluePoint::hash(const char *buff, int blen)

{
    unsigned long    sum = 0xabcdef;
    int     loop;
    //char    aa, aa2, aa3;

    for (loop = 0; loop < blen; loop++)
        {
        sum ^= (unsigned char)buff[loop];
        sum = ROTATE_LONG_RIGHT(sum, 10);          /* rotate right */
		sum ^= (unsigned char)buff[loop];
	}

    return sum;
}

ulong   CBluePoint::crypthash(const CString &str, const CString &pass)

{
	int mult = sizeof(TCHAR);
	const TCHAR *buff = str; int len = str.GetLength();
	const TCHAR *pbuff = pass; int plen = pass.GetLength();
	ulong sum = xCBluePoint::crypthash(
			(const char*)buff, len * mult, (const char*)pbuff, plen * mult);
	return sum;
}


//# -------------------------------------------------------------------------
//# Crypt and hash:
//# use: crypthash = bluepoint_crypthash($str, "pass")

ulong   xCBluePoint::crypthash(const char *buff, int blen, const char *pass, int plen)

{
    unsigned long    sum = 0;

    // Duplicate buffer
    char *duplicate = (char *)malloc(blen + 4);
    if(!duplicate)
        {
        return(0L);
        }
    memcpy(duplicate, buff, blen);
    encrypt(duplicate, blen, pass, plen);
    sum = hash(duplicate, blen);
	// Kill duplicate, so no piring eyes
	for (int loop = 0; loop < blen; loop++)
		{
		duplicate[loop] = char(rand() % 255);
		}
    free(duplicate);
    return(sum);
}

//# -------------------------------------------------------------------------
//# The following routines are internal to this module:

void    xCBluePoint::do_encrypt(char *str, int slen, char *pass, int plen)

{
    int loop, loop2 = 0;
    unsigned char   aa, bb;;

    if(verbose)
        {
		//P2N(_T( "encrypt str='%s' len=%d pass='%s' plen=%d\n"),
        //         str, slen, pass, plen);
        }

    //# Pass loop  (encrypt)
    for (loop = 0; loop < slen; loop++)
        {
        aa = str[loop];

        aa = aa ^ pass[loop2];

        loop2++;
        if(loop2 >= plen) {loop2 = 0;}     //#wrap over

        str[loop] = aa;
        }

    //# Backward loop (encrypt)
    bb = 0;
    for (loop = slen-1; loop >= 0; loop--)
        {
        aa = str[loop];

        aa ^= backward;
        aa += addend;
        aa += bb;

        bb = aa;

        str[loop] = aa;
        }

    //# Forward loop  (encrypt)
    bb = 0;
    for (loop = 0; loop < slen; loop++)
        {
        aa = str[loop];

        aa ^= forward;
        aa  += addend;
        aa  += bb;

        #ifndef NOROTATE
        aa = ROTATE_CHAR_RIGHT(aa, 3);
        #endif

        bb = aa;

        str[loop] = aa;
        }
}

//# -------------------------------------------------------------------------
//# Internal to this module:

void    xCBluePoint::do_decrypt(char *str, int slen, char *pass, int plen)

{
    int loop, loop2 = 0;
    unsigned char  aa, bb, cc;

    if(verbose)
        {
		//P2N(_T( "decrypt(inp) str=%s len=%d pass=%s plen=%d\n"),
        //          str, slen, pass, plen);
        }

    //# Forward loop (decrypt)
    cc = 0;
    for (loop = 0; loop < slen; loop++)
        {
        bb = cc;

        cc = aa = str[loop];

        #ifndef NOROTATE
        aa = ROTATE_CHAR_LEFT(aa, 3);
        #endif

        aa -=  bb;
        aa -= addend;

        aa ^= forward;

        str[loop] = aa;
        }

    //# Backward loop  (decrypt)
    cc = 0;
    for (loop = slen-1; loop >= 0; loop--)
        {
        bb = cc;
        aa = cc = str[loop];

        aa -= bb;
        aa -= addend;
        aa ^= backward;

        str[loop] = aa;
        }

    //# Pass loop   (decrypt)
    for (loop = 0; loop < slen; loop++)
        {
        aa = str[loop];

        aa = aa ^ pass[loop2];

        loop2++; if(loop2 >= plen) {loop2 = 0;}     //#wrap over

        str[loop] = aa;
        }
}

//# -------------------------------------------------------------------------
// use it for testing only as it has a 256 byte buffer limit
//# Use: mystr = bluepoint_dumphex($str)

#ifdef DEF_DUMPHEX

static char buff[256];

char    *xCBluePoint::bluepoint_dumphex(const char *str, int len)

{
    buff[0] = 0;

    int loop = 0, pos = 0;
    for (loop = 0; loop < len; loop += 2)
        {
		//pos += sprintf(buff + pos, "-%02x", (unsigned char)str[loop]);
		P2N(_T(" %02x-%02x"), (unsigned char)str[loop+1], (unsigned char)str[loop]);

        if(pos >= sizeof(buff) - 8)
            break;
        }

	P2N(_T("\r\n"));
    return(buff);
}

#endif

//# -------------------------------------------------------------------------
// convert binary str to hex string
//# char    *bluepoint_tohex(char *str, int len, char *out, int *len)

char    *xCBluePoint::tohex(const char *str, int len, char *out, int *olen, int llen)

{
//#ifdef UNICODE
//#error "Call tohexW from unicode"
//#endif

    int loop = 0, wlen = 0, pos = 0, rlen = 8; 
	//rand() % 5 + 4;

    for (loop = 0; loop < len; loop++)
        {
        int ppp = sprintf(out + pos, "%02x", ( unsigned char) str[loop]);
		pos += ppp; wlen += ppp;
		
        if(pos >= *olen - 4)
            break;

		if(llen && pos)
			{
			// Output new lines
			if(pos % llen == 0)
				{
				*(out + pos) = '\r'; pos++; 
				*(out + pos) = '\n'; pos++;				
				wlen = 0;
				}			
			// Output spaces
			if(wlen % rlen == 0)
				{
				wlen = 0;
				*(out + pos) = ' '; pos++; 	
				}
			}		
        }
    *olen = pos;
    return(out);
}

// Convert to hex string
//

void	CBluePoint::tohexW(const CString &str, CString &out, int linelen)
{
	
	const TCHAR *buff = str; int len = str.GetLength();
	int olen = len * 6;
	TCHAR *hbuff = out.GetBufferSetLength(olen);
	xCBluePoint::tohexW(buff, len, hbuff, &olen, linelen);
	out.ReleaseBuffer(olen);	
}

TCHAR    *xCBluePoint::tohexW(const TCHAR *str, int len, TCHAR *out, int *olen, int llen)

{
	int loop = 0, wlen = 0, pos = 0, rlen = 8;
	
	for (loop = 0; loop < len; loop++)
	{
		int ppp = _stprintf(out + pos, _T("%04x "), (unsigned short)str[loop]);
		pos += ppp; wlen += ppp;
		//P2N(_T("%c ppp %d\r\n"), str[loop], ppp);

		if (pos >= *olen - 5)
			break;

		if (llen && loop)
		{
			// Output new lines
			if ((loop) % llen == llen - 1)
			{
				int ppp2 = _stprintf(out + pos, _T("\r\n"));
				//P2N(_T("ppp2 %d\r\n"), ppp2);
				pos += ppp2;
				wlen = 0;
			}
			// Output spaces
			//if (wlen % rlen == 0 && wlen)
			//{
			//	pos += _stprintf(out + pos, _T(" "));
			//	wlen = 0;				
			//}
		}
	}

	out[pos] = 0; 	// It aborted for just enough space to zero terminate
	*olen = pos;

	return(out);
}

//# -------------------------------------------------------------------------
// Convert hex string to binary str
//# char    *bluepoint_fromhex(char *str, int len, char *out, int *len)
// isalnum

char    *xCBluePoint::fromhex(const char *str, int len, char *out, int *olen)

{
    unsigned char *str2 = (unsigned char *)str;

    char chh[3]; chh[2] = 0;

    int loop = 0, pos = 0;
    for (loop = 0; loop < len; loop += 2)
        {
        ulong vv;
		
		// Immunity from non hex digits
		while(true)
			{
			chh[0] = *(str + loop);
			if(isxdigit(chh[0]))
				break;

			if(++loop >= len)
				break;
			}

		while(true)
			{
			chh[1] = *(str + loop + 1);
			if(isxdigit(chh[1]))
				break;

			if(++loop >= len)
				break;
			}

		//P2N("Expanding %s\r\n", chh);

		// Last char was noo good ... 
		if(loop >= len)
			break;
			
		vv = strtol(chh, NULL, 16);
		
		// Overflow at the end
		if(pos > *olen - 3)
			break;

		out[pos++] =(char)vv;
		}

    // It aborted for just enough space to zero terminate
    out[pos] = 0;

    *olen = pos;
    return(out);
}

void	CBluePoint::fromhexW(const CString &hex, CString &uhex)

{
	//CString uhex;
	int len = hex.GetLength();	int olen2 = len + 20;
	const TCHAR *hbuff = hex;
	TCHAR *uhbuff = uhex.GetBufferSetLength(olen2);

	xCBluePoint::fromhexW(hbuff, len, uhbuff, &olen2);
	uhex.ReleaseBuffer(olen2);
}

// Immunity from non hex digits 

TCHAR    *xCBluePoint::fromhexW(const TCHAR *str, int len, TCHAR *out, int *olen)

{
	int loop = 0, pos = 0;
	
	while(true)
	{
		TCHAR chh[5]; chh[4] = 0;
		if (loop > len)				// EOS
			break;

		// Find four consecutive digits
		int loop2 = 0, loop3 = 0;
		while (true)				// Level 1
		{
			if (loop2 + loop > len)  // EOS
				break;
			// Got four digits, make a character
			if (loop3 >= 4)
			{
				//P2N(_T("Expanding %s\r\n"), chh);
				ulong vv = _tcstoul(chh, NULL, 16);
				out[pos++] = (TCHAR)vv;				
				loop += (loop2 - 1);
				break;
			}
			TCHAR ccc = *(str + loop + loop2);
			//P2N(_T("Digit '%c' %d %d \r\n"), ccc, loop, loop2);
			// Got a hex digit? Yep, add; else: ignore
			if (_istxdigit(ccc))
				{
				chh[loop3] = ccc;
				loop3++;
				}
			
			loop2++;
		}
	loop++;

	// Overflow at the end
	if (pos > *olen - 3)
		break;
	}
	out[pos] = 0;		// It aborted for just enough space to zero terminate
	*olen = pos;		// Tell the caller how big we are
	return(out);
}