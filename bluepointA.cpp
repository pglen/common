
/* =====[ bluepoint.cpp ]========================================================== 
                                                                             
   Description:     The CryptoSticky project, implementation of the bluepoint.cpp                
                                                                             
                    Defines the behavior for the application.          
                                                                             
   Compiled:        MS-VC 6.00                                               
                                                                             
   Notes:           <Empty Notes>                                            
                                                                             
   Revisions:                                                                
                                                                             
      REV     DATE        BY            DESCRIPTION                       
      ----  --------  -----------  ----------------------------   
      0.00  6/17/2010  Peter Glen   Initial version.                         
                                                                             
   ======================================================================= */

#pragma warning(disable: 4996)

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
//
///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bluepointA.h"
#include "mxpad.h"

// Truied all these ...
//#define _AFX_SECURE_NO_DEPRECATE
//#define _DEPRECATION_DISABLE
//#ifdef _MSC_VER
//#define _CRT_SECURE_NO_WARNINGS
//#endif
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1


// Disable unsecure warnings
#pragma warning(disable: 4996)


//#pragma warning(push)
//#pragma warning(disable: warning-code) 
//4996 for _CRT_SECURE_NO_WARNINGS equivalent
// deprecated code here
//#pragma warning(pop)

//////////////////////////////////////////////////////////////////////////

CBluePointA bluepointa;

//////////////////////////////////////////////////////////////////////////

CBluePointA::CBluePointA()

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

void    CBluePointA::encrypt(char *buff, int blen, const TCHAR *pass, int plen)

{
    TCHAR newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

   if(functrace)
       {
       _tprintf(_T("bluepoint_encrypt\nbuff=%s\n"), bluepoint_dumphex(buff, blen));
       _tprintf(_T("pass=%s\n"), bluepoint_dumphex((char*)pass, plen) );
       }

    prep_pass(pass, plen, newpass);

    do_encrypt(buff, blen, newpass, PASSLIM);
}

//# -------------------------------------------------------------------------
//# Use: bluepoint_decrypt($str, $password);

void    CBluePointA::decrypt(char *buff, int blen, const TCHAR *pass, int plen)

{
	TCHAR newpass[4 * PASSLIM];

    if(plen == 0 || blen == 0)
        {
        return;
        }

    if(functrace)
        {
        _tprintf(_T("bluepoint_decrypt()\nbuff=%s\n"), bluepoint_dumphex(buff, blen));
        _tprintf(_T("pass=%s\n"), bluepoint_dumphex((char*)pass, plen) );
        }

    prep_pass(pass, plen, newpass);

    do_decrypt(buff, blen, newpass, PASSLIM);
}

///////////////////////////////////////////////////////////////////////////
// Prepare pass

void    CBluePointA::prep_pass(const TCHAR *pass, int plen, TCHAR *newpass)

{
    int loop;
	TCHAR vec2[PASSLIM + 1];

	memset(vec2, 0, sizeof(vec2));

    // Duplicate vector
    int vlen = _tcslen(vector);
	_tccpy(vec2, vector);
	vec2[PASSLIM] = 0;

    newpass[0] = 0;

	// Expand/truncate pass to fit:
	if(plen >= PASSLIM)
		{
		_tcsncpy(newpass, pass, PASSLIM);
		}
	else
		{
		for(loop = 0; loop < PASSLIM / (plen); loop++)
			{
			_tcsncat(newpass, pass, plen);
			_tcscat(newpass, _T("_"));
			}
		}

	// Terminate:
    newpass[PASSLIM] = 0;

	//TRACE(_T("newpass '%s'\r\n"), newpass);

    if(verbose)
        _tprintf(_T("prep_pass() newpass: %s\n"), newpass);

#ifndef NOPASSCRYPT
    CBluePointA::do_encrypt((char*)vec2, vlen / sizeof(TCHAR), vector, vlen);
#endif

    if(verbose)
        {
        _tprintf(_T("prep_pass() eVEC: "));
        bluepoint_dumphex((char*)vec2, vlen);
        _tprintf(_T("\n"));
        }

#ifndef NOPASSCRYPT
    CBluePointA::do_encrypt((char*)newpass, PASSLIM / sizeof(TCHAR), vec2, vlen);
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

ulong   CBluePointA::hash(const char *buff, int blen)

{
    unsigned long    sum = 0;
    int     loop;
    //char    aa, aa2, aa3;

    for (loop = 0; loop < blen; loop++)
        {
        sum ^= (unsigned char)buff[loop];
        sum = ROTATE_LONG_RIGHT(sum, 10);          /* rotate right */
        }

    return sum;
}

//# -------------------------------------------------------------------------
//# Crypt and hash:
//# use: crypthash = bluepoint_crypthash($str, "pass")

ulong   CBluePointA::crypthash(const char *buff, int blen, TCHAR *pass, int plen)

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

    free(duplicate);
    return(sum);
}

//# -------------------------------------------------------------------------
//# The following routines are internal to this module:

void    CBluePointA::do_encrypt(char *str, int slen, TCHAR *pass, int plen)

{
    int loop, loop2 = 0;
    unsigned char  aa, bb;;

    if(verbose)
        {
        _tprintf( _T("encrypt str='%s' len=%d pass='%Ts' plen=%d\n"),
                 str, slen, pass, plen);
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

void    CBluePointA::do_decrypt(char *str, int slen, TCHAR *pass, int plen)

{
    int loop, loop2 = 0;
    //unsigned char  aa, bb, cc;
    char  aa, bb, cc;

    if(verbose)
        {
        printf( "decrypt(inp) str=%s len=%d pass=%s plen=%d\n",
                  str, slen, pass, plen);
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

TCHAR buff[256];

TCHAR    *CBluePointA::bluepoint_dumphex(const char *str, int len)

{
    buff[0] = 0;

    int loop = 0, pos = 0;
    for (loop = 0; loop < len; loop++)
        {
        pos += _stprintf(buff + pos, _T("-%02x"), ( unsigned char)str[loop]);

        if(pos >= sizeof(buff) - 8)
            break;
        }

    return(buff);
}

#endif

//# -------------------------------------------------------------------------
// convert binary str to hex string
//# char    *bluepoint_tohex(char *str, int len, char *out, int *len)

TCHAR    *CBluePointA::tohex(const char *str, int len, TCHAR *out, int *olen, int llen)

{
    int loop = 0, pos = 0;

    for (loop = 0; loop < len; loop++)
        {
        pos += _stprintf(out + pos, _T("%02x"), ( unsigned char) str[loop]);

        if(pos >= *olen - 4)
            break;

		if(llen && pos)
			{
			if(pos % llen == 0)
				{
				*(out + pos) = '\r'; pos++; 
				*(out + pos) = '\n'; pos++;				
				}
			}		
        }
    *olen = pos;

    return(out);
}

//# -------------------------------------------------------------------------
// convert hex string to binary str
//# char    *bluepoint_fromhex(char *str, int len, char *out, int *len)
// isalnum

char    *CBluePointA::fromhex(const TCHAR *str, int len, char *out, int *olen)

{
    unsigned char *str2 = (unsigned char *)str;

    char chh[3]; chh[2] = 0;

    int loop = 0, pos = 0;
    for (loop = 0; loop < len; loop += 2)
        {
        long vv;
		
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

		//P2N(_T("Expanding %s\r\n"), chh);

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
