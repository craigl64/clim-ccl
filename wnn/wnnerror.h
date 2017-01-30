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
/* file �ط��Υ��顼 */
#define WNN_FILE_READ_ERROR	90 /* �ե�������ɤ߹��ळ�Ȥ��Ǥ��ޤ��� */
#define WNN_FILE_WRITE_ERROR	91 /* �ե������񤭽Ф����Ȥ��Ǥ��ޤ��� */
#define WNN_FID_ERROR		92 /* ���饤����Ȥ��ɤ߹�����ե�����ǤϤ���ޤ��� */
#define WNN_NO_MORE_FILE	93 /* ����ʾ�ե�������ɤ߹��ळ�Ȥ��Ǥ��ޤ���*/
#define WNN_INCORRECT_PASSWD    94 /* �ѥ���ɤ��ְ�äƤ��ޤ��� */
#define WNN_FILE_IN_USE    	95 /* �ե����뤬�ɤ߹��ޤ�Ƥ��ޤ��� */
#define WNN_UNLINK    		96 /* �ե����뤬����Ǥ��ޤ��� */
#define WNN_FILE_CREATE_ERROR	97 /* �ե����뤬��������ޤ���*/
#define WNN_NOT_A_FILE		98	/*�ף��Υե�����Ǥ���ޤ���*/
#define WNN_INODE_CHECK_ERROR   99 /* �ե������I-node��FILE_UNIQ����פ���������Ǥ��ޤ��� */

/*	V3	*/
/* �����ɲôط��Υ��顼 */
#define WNN_NO_EXIST		1	/*�ե����뤬¸�ߤ��ޤ���*/
#define WNN_MALLOC_ERR		3	/*����alloc�Ǽ��Ԥ��ޤ�����*/
#define WNN_NOT_A_DICT		5 	/*����ǤϤ���ޤ���*/
#define WNN_NOT_A_HINDO_FILE	6	/*���٥ե�����ǤϤ���ޤ���*/
#define WNN_NOT_A_FUZOKUGO_FILE	7	/*��°��ե�����ǤϤ���ޤ���*/
#define WNN_JISHOTABLE_FULL	9	/*����ơ��֥뤬���դǤ���*/
#define WNN_HINDO_NO_MATCH	10	/*���٥ե����뤬�����ꤵ�줿�����
					  ���٥ե�����ǤϤ���ޤ���*/
#define WNN_OPENF_ERR		16	/*�ե����뤬�����ץ�Ǥ��ޤ���*/
#define WNN_NOT_HINDO_FILE	17	/*���������٥ե�����Ǥ���ޤ���*/
#define WNN_NOT_FZK_FILE	18	/*��������°��ե�����Ǥ���ޤ���*/
#define WNN_FZK_TOO_DEF		19	/*��°��θĿ����٥���Ĺ���ʤɤ�
					  ¿�᤮�ޤ�*/
/* �������ط��Υ��顼 */
#define WNN_DICT_NOT_USED	20	/*�����ֹ�μ���ϡ��Ȥ��Ƥ��ޤ���*/

/* �Ѵ����Υ��顼 */
#define WNN_BAD_FZK_FILE  24      /*��°��ե���������Ƥ�����������ޤ���*/
#define WNN_GIJI_HINSI_ERR	25	/*�����ʻ��ֹ椬�۾�Ǥ�
					  hinsi.data������������ޤ���*/
#define WNN_NO_DFE_HINSI	26	/*̤������ʻ줬��ü�ʻ�Ȥ���
					  �������Ƥ��ޤ� Not Used*/
#define WNN_FZK_FILE_NO_LOAD	27	/*��°��ե����뤬�ɤ߹��ޤ�Ƥ��ޤ���*/

/* jishobiki.c */
#define WNN_DIC_ENTRY_FULL	30	/*����Υ�����ȥ꤬¿�᤮�ޤ���*/
#define WNN_LONG_MOJIRETSU	31	/*�Ѵ����褦�Ȥ���ʸ����Ĺ�᤮�ޤ���*/
#define WNN_WKAREA_FULL		32	/*��°������ΰ褬��­���Ƥ��ޤ���*/
#define WNN_JKTAREA_FULL 	34	/*�������ΰ褬��­���Ƥ��ޤ���*/
#define WNN_NO_KOUHO		35	/*���䤬 1 �Ĥ���ޤ���Ǥ���*/

/* ñ����Ͽ���Υ��顼 */
#define WNN_YOMI_LONG		40	/*�ɤߤ�Ĺ�᤮�ޤ���*/
#define WNN_KANJI_LONG		41	/*������Ĺ�᤮�ޤ���*/
#define WNN_NOT_A_UD		42 /*���ꤵ�줿����ϡ���Ͽ��ǽ�ǤϤ���ޤ���*/
#define WNN_NO_YOMI		43	/*�ɤߤ�Ĺ����0�Ǥ���*/
#define WNN_NOT_A_REV		44 /*���ꤵ�줿����ϡ��հ�����ǽ�ǤϤ���ޤ���*/
#define WNN_RDONLY		45	/*�꡼�ɥ���꡼�μ������Ͽ���褦�Ȥ��ޤ�����*/
#define WNN_DICT_NOT_IN_ENV	46	/*�Ķ���¸�ߤ��ʤ��������Ͽ���褦�Ȥ��ޤ�����*/

/* ���ٹ����Υ��顼 */
#define WNN_RDONLY_HINDO	49 /* �꡼�ɥ���꡼�����٤��ѹ����褦�Ȥ��ޤ����� */

/* ñ���������ʻ������Υ��顼 */
/*
WNN_RDONLY
*/
#define WNN_WORD_NO_EXIST	50	/*���ꤵ�줿ñ�줬¸�ߤ��ޤ���*/

/* ������λ��Υ��顼 */
#define WNN_MALLOC_INITIALIZE	60	/*����alloc�Ǽ��Ԥ��ޤ�����*/

#define WNN_SOME_ERROR		68	/* �����Υ��顼��������ޤ�����*/
#define WNN_SONOTA		69	/*�Х���ȯ�����Ƥ������ͤǤ���Not Used*/
#define WNN_JSERVER_DEAD	70	/*�����Ф����Ǥ��ޤ���(jlib)*/
#define WNN_ALLOC_FAIL		71	/*alloc�˼���(jlib)*/
#define WNN_SOCK_OPEN_FAIL	72	/*jd_begin��socket��open�˼���(jlib)*/
#define WNN_BAD_VERSION		73	/*�̿��ץ�ȥ���ΥС�����󤬹�äƤ��ޤ���*/
#define WNN_BAD_ENV		74      /*���饤����Ȥ����������Ķ��ǤϤ���ޤ���*/

#define WNN_MKDIR_FAIL		80	/* �ǥ��쥯�ȥ����»�ʤä� */


/*�ʻ�ե����� */
#define WNN_TOO_BIG_HINSI_FILE 100 	/*�ʻ�ե����뤬�礭�᤮�ޤ���*/
#define WNN_TOO_LONG_HINSI_FILE_LINE 101		/*�ʻ�ե����뤬�礭�᤮�ޤ���*/
#define WNN_NO_HINSI_DATA_FILE 102	/*�ʻ�ե����뤬¸�ߤ��ޤ��� */
#define WNN_BAD_HINSI_FILE 103	       /*�ʻ�ե���������Ƥ��ְ�äƤ��ޤ���*/

#define WNN_HINSI_NOT_LOADED 105 	/* �ʻ�ե����뤬�ɤ߹��ޤ�Ƥ��ޤ���*/
#define WNN_BAD_HINSI_NAME 106		/* �ʻ�̾���ְ�äƤ��ޤ� */
#define WNN_BAD_HINSI_NO 107		/* �ʻ��ֹ椬�ְ�äƤ��ޤ� */

#define NOT_SUPPORTED_OPERATION 109 /*�������ϥ��ݡ��Ȥ���Ƥ��ޤ���Not Used*/

/* 
 *
 *  jl �ʹ���饤�֥��ˤΥ��顼
 * 
 */

#define WNN_CANT_OPEN_PASSWD_FILE 110  /* �ѥ���ɤ����äƤ���ե����뤬
					 �����ץ�Ǥ��ޤ���(jl) */
/* ��������Υ��顼  */
#define WNN_RC_FILE_NO_EXIST 111 /* uumrc�ե����뤬¸�ߤ��ޤ���(jl) Not Used*/
#define WNN_RC_FILE_BAD_FORMAT 112 /* uumrc�ե�����η�������äƤ��ޤ�(jl)Not Used */
#define WNN_NO_MORE_ENVS  113 /* ����ʾ�Ķ����뤳�ȤϽ���ޤ���*/
#define WNN_FILE_NOT_READ_FROM_CLIENT 114 /* ���Υ��饤����Ȥ��ɤ߹�����ե�����Ǥ���ޤ���*/
#define WNN_NO_HINDO_FILE 115 /* ��������٥ե����뤬�Ĥ��Ƥ��ޤ���*/

#define WNN_CANT_CREATE_PASSWD_FILE 116 /*�ѥ���ɤΥե����뤬��������ޤ���*/
/*
  Local Variables:
  kanji-flag: t
  End:
*/
