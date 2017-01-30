/*
 */
/*
 * Copyright Kyoto University Research Institute for Mathematical Sciences
 *                 1987, 1988, 1989, 1990, 1991
 * Copyright OMRON Corporation. 1987, 1988, 1989, 1990, 1991
 * Copyright ASTEC, Inc. 1987, 1988, 1989, 1990, 1991
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that all of the following conditions are satisfied:
 *
 * 1) The above copyright notices appear in all copies
 * 2) Both those copyright notices and this permission notice appear
 *    in supporting documentation
 * 3) The name of "Wnn" isn't changed unless substantial modifications
 *    are made, or
 * 3') Following words followed by the above copyright notices appear
 *    in all supporting documentation of software based on "Wnn":
 *
 *   "This software is based on the original version of Wnn developed by
 *    Kyoto University Research Institute for Mathematical Sciences (KURIMS),
 *    OMRON Corporation and ASTEC Inc."
 *
 * 4) The names KURIMS, OMRON and ASTEC not be used in advertising or
 *    publicity pertaining to distribution of the software without
 *    specific, written prior permission
 *
 * KURIMS, OMRON and ASTEC make no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * Wnn consortium is one of distributors of the official Wnn source code
 * release.  Wnn consortium also makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * KURIMS, OMRON, ASTEC AND WNN CONSORTIUM DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL KURIMS, OMRON, ASTEC OR
 * WNN CONSORTIUM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*	Version 4.0
 */
#include <stdio.h>
#include "jslib.h"
#include "commonhd.h"
#include "wnn_os.h"
#include "msg.h"

#ifdef	not_use
char *wnn_errormsg[] ={
"no_error 0",
"�ե����뤬¸�ߤ��ޤ���",		/*WNN_NO_EXIST	1 */
"no_error 2",
"����alloc�Ǽ��Ԥ��ޤ�����",		/*WNN_MALLOC_ERR	3 */
"no_error 4",
"����ǤϤ���ޤ���",		/*WNN_NOT_A_DICT	5  */
"���٥ե�����ǤϤ���ޤ���",	/*WNN_NOT_A_HINDO_FILE	6  */
"��°��ե�����ǤϤ���ޤ���",	/*WNN_NOT_A_FUZOKUGO_FILE 7  */
"no_error 8",
"����ơ��֥뤬���դǤ���",	/*WNN_JISHOTABLE_FULL	9*/
"���٥ե����뤬�����ꤵ�줿��������٥ե�����ǤϤ���ޤ���",
				 /*WNN_HINDO_NO_MATCH	10 */
"no_error 11",
"no_error 12",
"no_error 13",
"no_error 14",
"no_error 15",
"�ե����뤬�����ץ�Ǥ��ޤ���",	/*WNN_OPENF_ERR		16*/
"���������٥ե�����ǤϤ���ޤ���",		/* WNN_NOT_HINDO_FILE	17 */
"��������°��ե�����ǤϤ���ޤ���",	/* WNN_NOT_FZK_FILE 18 */
"��°��θĿ����٥���Ĺ���ʤɤ�¿�᤮�ޤ���",	/* WNN_FZK_TOO_DEF 19 */
"�����ֹ�μ���ϡ��Ȥ��Ƥ��ޤ���",	/*WNN_DICT_NOT_USED	20 */
"no_error 21",
"no_error 22",
"no_error 23",
"��°��ե���������Ƥ�����������ޤ���", /* WNN_BAD_FZK_FILE  24     */
"�����ʻ��ֹ椬�۾�Ǥ�hinsi.data������������ޤ���", /*WNN_GIJI_HINSI_ERR 25 */
"̤������ʻ줬��ü�ʻ�Ȥ����������Ƥ��ޤ���", /*WNN_NO_DFE_HINSI 26*/
"��°��ե����뤬�ɤ߹��ޤ�Ƥ��ޤ���", /*WNN_FZK_FILE_NO_LOAD	27 */
"no_error 28",
"no_error 29",
"����Υ�����ȥ꤬¿�᤮�ޤ���",	/*WNN_DIC_ENTRY_FULL	30*/
"�Ѵ����褦�Ȥ���ʸ����Ĺ�᤮�ޤ���",	/*WNN_LONG_MOJIRETSU	31 */
"��°������ΰ褬��­���Ƥ��ޤ���",	/*WNN_WKAREA_FULL	32 */
"no_error 33",
"�������ΰ褬��­���Ƥ��ޤ���",		/* WNN_JKTAREA_FULL 34 */
"���䤬 1 �Ĥ���ޤ���Ǥ��� ",	/* WNN_NO_KOUHO 35 */
"no_error 36",
"no_error 37",
"no_error 38",
"no_error 39",
"�ɤߤ�Ĺ�᤮�ޤ���",			/*WNN_YOMI_LONG	40 */
"������Ĺ�᤮�ޤ���",			/*WNN_KANJI_LONG	41 */
"���ꤵ�줿����ϡ���Ͽ��ǽ�ǤϤ���ޤ���", /*WNN_NOT_A_UD	42 */
"�ɤߤ�Ĺ����0�Ǥ���",			/*WNN_NO_YOMI	43 */
"���ꤵ�줿����ϡ��հ�����ǽ�ǤϤ���ޤ���", /* WNN_NOT_A_REV	44 */
"�꡼�ɥ���꡼�μ������Ͽ/������褦�Ȥ��ޤ�����",	/*WNN_RDONLY	45 */
"�Ķ���¸�ߤ��ʤ��������Ͽ���褦�Ȥ��ޤ�����", /* WNN_DICT_NOT_IN_ENV 46 */
"no_error 47",
"no_error 48",
"�꡼�ɥ���꡼�����٤��ѹ����褦�Ȥ��ޤ�����", /* WNN_RDONLY_HINDO 49 */
"���ꤵ�줿ñ�줬¸�ߤ��ޤ���",	/*WNN_WORD_NO_EXIST	50 */
"no_error 51",
"no_error 52",
"no_error 53",
"no_error 54",
"no_error 55",
"no_error 56",
"no_error 57",
"no_error 58",
"no_error 59",
"����alloc�Ǽ��Ԥ��ޤ�����",		/*WNN_MALLOC_INITIALIZE	60 */
"no_error 61",
"no_error 62",
"no_error 63",
"no_error 64",
"no_error 65",
"no_error 66",
"no_error 67",
"�����Υ��顼��������ޤ�����",		/* WNN_SOME_ERROR 68 */
"�Х���ȯ�����Ƥ������ͤǤ���",		/*WNN_SONOTA 69 */
"�����Ф����Ǥ��ޤ���",		/*WNN_JSERVER_DEAD 70 */
"alloc�˼��Ԥ��ޤ�����",	/*WNN_ALLOC_FAIL	71 */
"�����Ф���³�Ǥ��ޤ���Ǥ�����",	/*WNN_SOCK_OPEN_FAIL	72 */
"�̿��ץ�ȥ���ΥС�����󤬹�äƤ��ޤ���", /* WNN_BAD_VERSION	73 */
"���饤����Ȥ����������Ķ��ǤϤ���ޤ���", /* WNN_BAD_ENV 74 */
"no_error 75",
"no_error 76",
"no_error 77",
"no_error 78",
"no_error 79",
"�ǥ��쥯�ȥ���뤳�Ȥ��Ǥ��ޤ���",	/* WNN_MKDIR_FAIL	80	*/
"no_error 81",
"no_error 82",
"no_error 83",
"no_error 84",
"no_error 85",
"no_error 86",
"no_error 87",
"no_error 88",
"no_error 89",
"�ե�������ɤ߹��ळ�Ȥ��Ǥ��ޤ���",		/* WNN_FILE_READ_ERROR	90*/
"�ե������񤭽Ф����Ȥ��Ǥ��ޤ���",		/* WNN_FILE_WRITE_ERROR	91*/
"���饤����Ȥ��ɤ߹�����ե�����ǤϤ���ޤ���",	/* WNN_FID_ERROR 92*/
"����ʾ�ե�������ɤ߹��ळ�Ȥ��Ǥ��ޤ���",	/* WNN_NO_MORE_FILE	93*/
"�ѥ���ɤ��ְ�äƤ��ޤ���",		/* WNN_INCORRECT_PASSWD    94   */
"�ե����뤬�ɤ߹��ޤ�Ƥ��ޤ��� ", /*WNN_FILE_IN_USE    	95 */
"�ե����뤬����Ǥ��ޤ��� ", /*WNN_UNLINK    		96 */
"�ե����뤬��������ޤ���", /*WNN_FILE_CREATE_ERROR	97 */
"�ף��Υե�����Ǥ���ޤ���", /*WNN_NOT_A_FILE		98	*/
"�ե������I-node��FILE_UNIQ����פ���������Ǥ��ޤ���", /*WNN_INODE_CHECK_ERROR   99 */

"�ʻ�ե����뤬�礭�᤮�ޤ���",		/* WNN_TOO_BIG_HINSI_FILE 100 */
"�ʻ�ե����뤬�礭�᤮�ޤ���",		/* WNN_TOO_LONG_HINSI_FILE_LINE 101  */
"�ʻ�ե����뤬¸�ߤ��ޤ���",		/* WNN_NO_HINSI_DATA_FILE 102 */
"�ʻ�ե���������Ƥ��ְ�äƤ��ޤ���",	/* WNN_BAD_HINSI_FILE 103 */
"no_error 104",
"�ʻ�ե����뤬�ɤ߹��ޤ�Ƥ��ޤ���",	/* WNN_HINSI_NOT_LOADED 105*/
"�ʻ�̾���ְ�äƤ��ޤ���",		/* WNN_BAD_HINSI_NAME 106 */
"�ʻ��ֹ椬�ְ�äƤ��ޤ���",		/* WNN_BAD_HINSI_NO 107 */
"no_error 108",
"�������ϥ��ݡ��Ȥ���Ƥ��ޤ���", /*NOT_SUPPORTED_OPERATION 109 Not Used*/
"�ѥ���ɤ����äƤ���ե����뤬�����ץ�Ǥ��ޤ���", /*WNN_CANT_OPEN_PASSWD_FILE 110  */
/* ��������Υ��顼  */
"uumrc�ե����뤬¸�ߤ��ޤ���", /*WNN_RC_FILE_NO_EXIST    111 Not Used*/
"uumrc�ե�����η�������äƤ��ޤ���", /* WNN_RC_FILE_BAD_FORMAT  112 Not Used*/
"����ʾ�Ķ����뤳�ȤϤǤ��ޤ���", /* WNN_NO_MORE_ENVS  113 */
"���Υ��饤����Ȥ��ɤ߹�����ե�����Ǥ���ޤ���", /* WNN_FILE_NOT_READ_FROM_CLIENT 114 */
"��������٥ե����뤬�Ĥ��Ƥ��ޤ���", /*WNN_NO_HINDO_FILE 115 */
"�ѥ���ɤΥե����뤬��������ޤ���" /*WNN_CANT_CREATE_PASSWD_FILE 116*/
};

int wnn_errormsg_len = sizeof(wnn_errormsg) / sizeof(char *);

extern	int	wnn_errorno;
static char msg[] = ":BAD ERRORNO!!! ";

char *wnn_perror()
{
static char msgarea[100];

  if((wnn_errorno < 0) || (wnn_errorno > sizeof(wnn_errormsg) / sizeof(char *))){
    sprintf(msgarea , "%d"  , wnn_errorno);
    strcat(msgarea , msg);
    return(msgarea);
  }
  return(wnn_errormsg[wnn_errorno]);
}
#endif	/* not_use */

extern	int	wnn_errorno;
extern struct msg_cat *wnn_msg_cat;
static char msg[] = ":BAD ERRORNO!!! ";

char *
wnn_perror_lang(lang)
char *lang;
{
    static char msgarea[100];

    sprintf(msgarea , "%d"  , wnn_errorno);
    strcat(msgarea , msg); 
    return(msg_get(wnn_msg_cat, wnn_errorno, msgarea, lang));
}

char *
wnn_perror()
{
    return(wnn_perror_lang(NULL));
}

/*
  Local Variables:
  eval: (setq kanji-flag t)
  eval: (setq kanji-fileio-code 0)
  eval: (mode-line-kanji-code-update)
  End:
*/
