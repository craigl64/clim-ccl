/*
 */
/** 		Copyright OMRON Corporation. 1989, 1990, 1991, 1992, 1992
 *
 * Permission to use, copy, modify, distribute, and sell this software and its 
 * documentation for any purpose is hereby granted without fee, provided 
 * that all of the following conditions are satisfied:
 *
 * <1>	The above copyright notices appear in all copies
 * <2>	Both the copyright notices and this permission notice appear in 
 *	supporting documentation
 * <3>	The name of "cWnn" isn't changed unless substantial modifications 
 * 	are made
 * <4>	Following words followed by the above copyright notices appear
 *    	in all supporting documentation of software based on "cWnn":
 *
 * 	This software is based on the original version of cWnn developed 
 *	by OMRON Corporation and Wnn developed by Kyoto University Research 
 *	Institute for Mathematical Sciences (KURIMS), OMRON Corporation and 
 * 	ASTEC Inc."
 *
 * <5>	The names of OMRON may not be used in advertising or publicity 
 *	pertaining to distribution of the software without specific, written 
 *	prior permission
 *
 *  OMRON Corporation makes no representations about the suitability of this 
 *  software for any purpose.  It is provided "as is" without express or 
 *  implied warranty.
 *
 *  Wnn consortium is one of distributors of the official Wnn source code
 *  release.  Wnn consortium also makes no representations about the
 *  suitability of this software for any purpose.  It is provided "as is"
 *  without express or implied warranty.
 *
 *  OMRON AND WNN CONSORTIUM DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 *  SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 *  IN NO EVENT SHALL OMRON OR WNN CONSORTIUM BE LIABLE FOR ANY SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 *  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 *  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *  PERFORMANCE OF THIS SOFTWARE.
 *
 *  This sofeware is developed by OMRON Corporation, based on the original 
 *  version of Wnn which was developed by Kyoto University Research Institute 
 *  for Mathematical Sciences (KURIMS), OMRON Corporation and ASTEC Inc. 
 *
 *  	Copyright Kyoto University Research Institute for Mathematical 
 *		Sciences 1987,1988,1989,1990,1991, 1992
 * 	Copyright OMRON Corporation 1987,1988,1989,1990,1991, 1992
 *	Copyright ASTEC Inc. 1987, 1988,1989,1990,1991, 1992
 *
 *  Contact:	Tel:   	+81-75-951-5111
 *		Fax:   	+81-75-955-2442
 *		E-mail: zhong@ari.ncl.omron.co.jp
**/
/**  cWnn  Version 1.1	 **/

#include  <ctype.h>
#include "commonhd.h"
#ifdef CHINESE
#include "cplib.h"

/* pyshengmu_tbl[]: ShengMu table of Chinese PinYin */
char 	*py_shengmu_tbl[PY_NUM_SHENGMU] = {
	
	"",  "B", "C",  "Ch", "D",  
	"F", "G", "H",  "J",  "K", 
	"L", "M", "N",  "P",  "Q", 
	"R", "S", "Sh", "T",  "W",
	"X", "Y", "Z",  "Zh" 
	};
	
/* py_yunmu_tbl[]: YunMu table of Chinese ZhuYin */
char 	*py_yunmu_tbl[PY_NUM_YUNMU*5] = {

	"��",    "��",    "��",    "��",    "��",
	"a��",    "����",    "����",    "����",    "����",
	"ai��",   "��i��",   "��i��",   "��i��",   "��i��",
	"an��",   "��n��",   "��n��",   "��n��",   "��n��",
	"ang��",  "��ng��",  "��ng��",  "��ng��",  "��ng��",
	"ao��",   "��o��",   "��o��",   "��o��",   "��o��",
	"e��",    "����",    "����",    "����",    "����",
	"ei��",   "��i��",   "��i��",   "��i��",   "��i��",
	"en��",   "��n��",   "��n��",   "��n��",   "��n��",
	"eng��",  "��ng��",  "��ng��",  "��ng��",  "��ng��",
	"er��",   "��r��",   "��r��",   "��r��",   "��r��",
	"i��",    "����",    "����",    "����",    "����",
	"ia��",   "i����",   "i����",   "i����",   "i����",
	"ian��",  "i��n��",  "i��n��",  "i��n��",  "i��n��",
	"iang��", "i��ng��", "i��ng��", "i��ng��", "i��ng��",
	"iao��",  "i��o��",  "i��o��",  "i��o��",  "i��o��",
	"ie��",   "i����",   "i����",   "i����",   "i����",
	"in��",   "��n��",   "��n��",   "��n��",   "��n��",
	"ing��",  "��ng��",  "��ng��",  "��ng��",  "��ng��",
	"iong��", "i��ng��", "i��ng��", "i��ng��", "i��ng��",
	"iu��",   "i����",   "i����",   "i����",   "i����",
	"m��",    "m��",    "m��",    "m��",    "m��",
	"n��",    "n��",    "����",    "����",    "����",
	"ng��",   "ng��",   "ng��",   "ng��",   "ng��",
	"o��",    "����",    "����",    "����",    "����",
	"ong��",  "��ng��",  "��ng��",  "��ng��",  "��ng��",
	"ou��",   "��u��",   "��u��",   "��u��",   "��u��",
	"u��",    "����",    "����",    "����",    "����",
	"ua��",   "u����",   "u����",   "u����",   "u����",
	"uai��",  "u��i��",  "u��i��",  "u��i��",  "u��i��",
	"uan��",  "u��n��",  "u��n��",  "u��n��",  "u��n��",
	"uang��", "u��ng��", "u��ng��", "u��ng��", "u��ng��",
	"ue��",   "u����",   "u����",   "u����",   "u����",
	"ui��",   "u����",   "u����",   "u����",   "u����",
	"un��",   "��n��",   "��n��",   "��n��",   "��n��",
	"uo��",   "u����",   "u����",   "u����",   "u����",
	"����",    "����",    "����",    "����",    "����",
	"��e��",   "������",   "������",   "������",   "������",
	"0��",   "1��",   "2��",   "3��",   "4��",  /* for undefinited YunMu  */
	};


/* pinyin_tbl:  size is NUM_SHENGMU*NUM_YUNMU, including empty ShengMu */
/*     and empty YunMu , and undefinited YunMu'-' */

int  pinyin_tbl[PY_NUM_SHENGMU*PY_NUM_YUNMU] = {
					    	
  0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
  0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
  0,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,0,1,0,0,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,
  0,1,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,1,0,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,1,
  0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,0,0,0,0,1,1,1,0,0,1,0,0,0,0,1,1,1,1,
  0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,1,0,0,0,1,
  0,0,0,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,1,0,1,0,0,0,0,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,
  0,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,
  0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,1,0,0,0,1,
  0,1,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,1,0,1,0,1,0,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,
  0,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1
	};
#endif /* CHINESE */
