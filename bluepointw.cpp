
/* =====[ bluepoint.cpp ]========================================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the bluepoint.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE          BY            DESCRIPTION                       
      ----  --------    -----------  ----------------------------   
      0.00  6/17/2010    Peter Glen   Initial version.                         
      0.00  jul/16/2013  Peter Glen   Unicode version
                                                                             
   ======================================================================= */

// -------------------------------------------------------------------------
// Bluepoint encryption routines.
//
//   How it works:
//
//     Strings are walked TCHAR by TCHAR with the loop:
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
//         substr($_[0], $loop, 1) = pack(_T("c"), $aa);
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
// How to use:
//
//  bluepoint_encrypt($orig, $pass);                -- encrypted in place
//  bluepoint_decrypt($cypher, $pass);              -- decrypted in place
//  $hash       = bluepoint_hash($orig, $pass);
//  $crypthash  = bluepoint_crypthash($orig, $pass);
//
// The reference implementation for version 1.0 contains a (default) sample
// clear text and a sample cypher text.
// Porting is correct if the new cypher text is a duplicate of the following:
//
// orignal='abcdefghijklmnopqrstuvwxyz' pass='1234'
// ENCRYPTED:
// -2b-e4-5c-46-75-9e-05-c3-74-d4-35-76-5b-84-10-f8-b7-7e-f4-07-0a-37-50-07-69-3d
// END ENCRYPTED
// decrypted='abcdefghijklmnopqrstuvwxyz'
// HASH:
// -754656719 0xd304da31
// CRYPTHASH:
// -1382909316 0xad927a7c
//
///////////////////////////////////////////////////////////////////////////
// At this point PERL and C implementations exist, here is a session dump:
//
// ant:/srv/www/archive/bluepoint/bluepoint3 # make; ./test_blue
// make: `test_blue' is up to date.
// orignal='abcdefghijklmnopqrstuvwxyz' pass='1234'
// ENCRYPTED:
// -2b-e4-5c-46-75-9e-05-c3-74-d4-35-76-5b-84-10-f8-b7-7e-f4-07-0a-37-50-07-69-3d
// END ENCRYPTED
// decrypted='abcdefghijklmnopqrstuvwxyz'
// HASH:
// -754656719 0xd304da31
// CRYPTHASH:
// -1382909316 0xad927a7c
//
// ant:/srv/www/archive/bluepoint/bluepoint3 # perl test_blue.pl
// original='abcdefghijklmnopqrstuvwxyz'  pass='1234'
// ENCRYPTED:
// -2b-e4-5c-46-75-9e-05-c3-74-d4-35-76-5b-84-10-f8-b7-7e-f4-07-0a-37-50-07-69-3d
// END ENCRYPTED
// decrypted='abcdefghijklmnopqrstuvwxyz'
// HASH:
// -754656719  0xd304da31
// CryptHASH:
// -1382909316  0xad927a7c
//
///////////////////////////////////////////////////////////////////////////
// History:

// 08-Apr-2008 Adapted to visual C++
// 10-Apr-2008 Defence for longer passwords
// 19-Jul-2013 Unicode works
//
///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bluepointw.h"
#include "mxpad.h"

// Define the desired debug output

#define	 PRINTF P2N

//////////////////////////////////////////////////////////////////////////

// Declare an instance
CBluePointW bluepointw;

//////////////////////////////////////////////////////////////////////////

CBluePointW::CBluePointW()

{
	vector		= _T("crypt");		//# influence encryption algorythm

	forward		= 0x55;				//# Constant propagated on forward pass
	backward	= 0x5a;				//# Constant propagated on backward pass
	addend		= 17;				//# Constant used adding to encrypted values

	//# -------------------------------------------------------------------------
	//# These vars can be set show op details

	verbose    = 0;                    //# Specify this to show working details
	functrace  = 0;                    //# Specify this to show function args
}

//# -------------------------------------------------------------------------
//# Use: encrypt($str, $password);

void    CBluePointW::encrypt(TCHAR *buff, int blen, const TCHAR *pass, int plen)

{
    TCHAR newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

   if(functrace)
       {
       PRINTF(_T("CBluePointW::encrypt\nbuff=%s\n"), bluepoint_dumphex(buff, blen));
       PRINTF(_T("pass=%s\n"), bluepoint_dumphex(pass, plen) );
       }

    xprep_pass(pass, plen, newpass);

    xdo_encrypt(buff, blen, newpass, PASSLIM);

	// Destroy pass 
	memset(newpass, 0, PASSLIM * sizeof(TCHAR));
}

//# -------------------------------------------------------------------------
//# Use: bluepoint_decrypt($str, $password);

void    CBluePointW::decrypt(TCHAR *buff, int blen, const TCHAR *pass, int plen)

{
    TCHAR newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

    if(functrace)
        {
        PRINTF(_T("CBluePointW::decrypt()\nbuff=%s\n"), bluepoint_dumphex(buff, blen));
        PRINTF(_T("pass=%s\n"), bluepoint_dumphex(pass, plen) );
        }

    xprep_pass(pass, plen, newpass);

    xdo_decrypt(buff, blen, newpass, PASSLIM);

	// Destroy pass 
	memset(newpass, 0, PASSLIM * sizeof(TCHAR));
}

///////////////////////////////////////////////////////////////////////////
// Prepare pass

void    CBluePointW::xprep_pass(const TCHAR *pass, int plen, TCHAR *newpass)

{
    int loop;
    TCHAR vec2[PASSLIM + 1];

	memset(vec2, 0, sizeof(vec2));

    // Duplicate vector
    int vlen = _tcslen(vector);
    _tcscpy_s(vec2, TSIZEOF(vec2), vector);

	vec2[PASSLIM] = 0;

    newpass[0] = 0;

	// Expand/truncate pass to fit:
	if(plen >= PASSLIM)
		{
		_tcsncpy_s(newpass, PASSLIM, pass, PASSLIM);
		}
	else
		{
		for(loop = 0; loop < PASSLIM / (plen); loop++)
			{
			_tcsncat_s(newpass, PASSLIM, pass, _TRUNCATE);
			_tcsncat_s(newpass, PASSLIM, _T("_"), _TRUNCATE);
			}
		}

	// Terminate:
    newpass[PASSLIM] = 0;

	//PRINTF(_T("CBluePointW::xprep_pass newpass '%s'\r\n"), newpass);

    if(verbose)
        PRINTF(_T("prep_pass() newpass: %s\n"), newpass);

#ifndef NOPASSCRYPT
    CBluePointW::xdo_encrypt(vec2, vlen, vector, vlen);
#endif

    if(verbose)
        {
        PRINTF(_T("prep_pass() eVEC: "));
        bluepoint_dumphex(vec2, vlen);
        PRINTF(_T("\n"));
        }

#ifndef NOPASSCRYPT
    CBluePointW::xdo_encrypt(newpass, PASSLIM, vec2, vlen);
#endif

	//TCHAR  out[2 * PASSLIM + 1];
	//int loop2;
	//for(loop2 = 0; loop2 < 2 * PASSLIM; loop2 += 2)
	//	{
	//	unsigned TCHAR hi = ((unsigned char)(newpass[loop2 / 2] )) >> 4;
	//	unsigned TCHAR lo = (unsigned char) ((newpass[loop2 / 2] & 0xf));
	//
	//	out[loop2] = hi <= 9 ? hi + '0' : hi + ('a' - 10);
	//	out[loop2 + 1] = lo <= 9 ? lo + '0' : lo + ('a' - 10);
	//	}
	//out[2 * PASSLIM] = 0;
	//TRACE(_T("newpass: '%s'"), out);
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

ulong   CBluePointW::hash(const TCHAR *buff, int blen)

{
    unsigned long    sum = 0;
    int     loop;
  
    for (loop = 0; loop < blen; loop++)
        {
        sum ^= (unsigned long)buff[loop];
        sum = ROTATE_LONG_RIGHT(sum, 11);          /* rotate right */
        }

    return sum;
}

//# -------------------------------------------------------------------------
//# Crypt and hash:
//# use: crypthash = bluepoint_crypthash($str, "pass")

ulong   CBluePointW::crypthash(const TCHAR *buff, int blen, TCHAR *pass, int plen)

{
    unsigned long    sum = 0;

    // Duplicate buffer
    TCHAR *duplicate = (TCHAR *)malloc(blen + 4);
    if(!duplicate)
        {
        return(0L);
        }
    memcpy(duplicate, buff, blen * sizeof(TCHAR));

    encrypt(duplicate, blen, pass, plen);
    sum = hash(duplicate, blen);

	// Destroy pass 
	memset(duplicate, 0, blen * sizeof(TCHAR));

    free(duplicate);
    return(sum);
}

//# -------------------------------------------------------------------------
//# The following routines are internal to this module:

void    CBluePointW::xdo_encrypt(TCHAR *str2, int slen2, 
								const TCHAR *pass2, int plen2)

{
    int		loop, loop2 = 0;
    unsigned char	aa, bb;;

    if(verbose)
        {
        PRINTF(_T("CBluePointW::xdo_encrypt len=%d str=%s \n"),
										slen2, bluepoint_dumphex(str2, slen2));
		PRINTF(_T("CBluePointW::xdo_encrypt pass len=%d '%s'\r\n"), 
										plen2, bluepoint_dumphex(pass2, plen2));
		}

	// Convert into byte op
	TCHAR *str = (TCHAR *)str2; int slen = slen2 * sizeof(TCHAR);
	TCHAR *pass = (TCHAR *)pass2; int plen = plen2 * sizeof(TCHAR);

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

	if(verbose)
		{
		PRINTF(_T("CBluePointW::xdo_crypt got: len=%d str=%s \n"),
									slen2, bluepoint_dumphex(str2, slen2));
		}
}

//# -------------------------------------------------------------------------
//# Internal to this module:

void    CBluePointW::xdo_decrypt(TCHAR *str2, int slen2, const TCHAR *pass2, int plen2)

{
    int		loop, loop2 = 0;
    unsigned char	aa, bb, cc;

    if(verbose)
        {
        PRINTF(_T("CBluePointW::xdo_decrypt len=%d str=%s \n"),
										slen2, bluepoint_dumphex(str2, slen2));
		PRINTF(_T("CBluePointW::xdo_decrypt pass len=%d '%s'\r\n"), 
										plen2, bluepoint_dumphex(pass2, plen2));
        }

	// Convert into byte op
	TCHAR *str = (TCHAR *)str2; int slen = slen2 * sizeof(TCHAR);
	TCHAR *pass = (TCHAR *)pass2; int plen = plen2 * sizeof(TCHAR);


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
	if(verbose)
		{
		PRINTF(_T("CBluePointW::xdo_decrypt got: len=%d str=%s \n"),
									slen2, bluepoint_dumphex(str2, slen2));
		}
}

//# -------------------------------------------------------------------------
// use it for testing only as it has a 256 byte buffer limit
//# Use: mystr = bluepoint_dumphex($str)

#ifdef DEF_DUMPHEX

static TCHAR buff[256];

TCHAR    *CBluePointW::bluepoint_dumphex(const TCHAR *str, int len)

{
    buff[0] = 0;

    int loop = 0, pos = 0;
    for (loop = 0; loop < len; loop++)
        {
        pos += _stprintf_s(buff + pos, 6, _T("-%04x"), str[loop]);

        if(pos >= sizeof(buff) - 8)
            break;
        }
    return(buff);
}

#endif

//# -------------------------------------------------------------------------
// convert binary str to hex string
//# TCHAR    *bluepoint_tohex(TCHAR *str, int len, TCHAR *out, int *len)

TCHAR  *CBluePointW::tohex(const TCHAR *str, int len, TCHAR *out, int *olen, int llen)

{
    int loop = 0, wlen = 0, pos = 0, rlen = 8; 
	//rand() % 5 + 4;

    for (loop = 0; loop < len; loop++)
        {
        int ppp = _stprintf_s(out + pos, 5, _T("%04x"), str[loop]);
		//PRINTF(_T("ppp %d chh'%c' -- "), ppp, str[loop]);
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

//# -------------------------------------------------------------------------
// convert hex string to binary str
//# TCHAR    *bluepoint_fromhex(TCHAR *str, int len, TCHAR *out, int *len)
// isalnum

TCHAR   *CBluePointW::fromhex(const TCHAR *str, int len, TCHAR *out, int *olen)

{
    //TCHAR *str2 = (TCHAR *)str;

    TCHAR chhs[5]; chhs[4] = 0;

    int loop, pos = 0;

    for (loop = 0; loop < len; )
        {
        long vv;

		// Immunity from non hex digits
		chhs[0] = xImmune(str, &loop, len); loop++; 
		chhs[1] = xImmune(str, &loop, len); loop++;
		chhs[2] = xImmune(str, &loop, len); loop++;
		chhs[3] = xImmune(str, &loop, len); loop++;

		// Last TCHAR was noo good ... 
		if(loop > len)
			break;
			
#ifdef _UNICODE
		vv = wcstol(chhs, NULL, 16);
#else
		vv = strtol(chhs, NULL, 16);
#endif
		//PRINTF(_T("Converting '%s' %x\r\n"), chhs, vv);

		// Overflow at the end
		if(pos > *olen - 5)
			{
			PRINTF(_T("CBluePointW::fromhex premture end\r\n"));
			break;
			}

		out[pos++] =(TCHAR)vv;
		}

    // It aborted for just enough space to zero terminate
    out[pos] = 0;

    *olen = pos;
    return(out);
}


//////////////////////////////////////////////////////////////////////////
// Immunity from non hex digits

TCHAR CBluePointW::xImmune(const TCHAR *str, int *ploop, int len)

{
	TCHAR chh = 0; 
	
	while(true)
		{
		if(*ploop > len)
			break;

		chh = *(str + *ploop);

		if(isxdigit(chh))
			break;

		(*ploop)++;
		}

	return chh;
}