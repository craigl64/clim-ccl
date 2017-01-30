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
/***********************************************************************
			rk_spclval.h
						88. 6. 9  �� ��

	���Ѵ��ǻȤäƤ����ü���ͤ�����ʳ����ˤ�Ф��ˡ��ޥ���
	�ؿ��⡢ctype.h��Ȥ���ΰʳ��Ϥ�������������Ƥ��롣
	rk_macros.h ��include����Ƥ��롣
***********************************************************************/
/*	Version 3.0
 */
#ifndef EOLTTR

typedef unsigned int	letter; /* ʸ���ϰ��̤ˤ��η���ɽ�� */
typedef unsigned char	uns_chr;

typedef unsigned char	modetyp;
#define	ModeNotInt /* ���modetyp��unsigned int��꾮��������typedef�����
			�����餳���define���Ʋ�������*/
/* #define ModeHasBuf */

#define LTRHUG	(~(0xff000000))
	/* ����ɽ�� �ڤӵ�ʸ���������letter�μ����������� */


	/* �ʲ�����������ͤϡ��Ǿ�̥Х��Ȥ�255�Ǥʤ���Фʤ�ʤ���
	   �����Τ��������˽Ф�Τ� EOLTTR��CHMSIG��NISEBP��LTREOF��REDRAW��
	   SENDCH�ʵ��ǥ꡼�ȤȤ���NISEDL��Ȥ�����NISEDL��ˡ�*/

#define EOLTTR	((letter)-1) /* ʸ����ν�ü��ɽ�������� */
			/* �������Ϥ����ä���ɬ�����������ɤ��֤����֤ˤ���
			   �����ȡ��ä��֤��٤���Τ��ʤ����Ϥ�����֤���*/
#define ERRCOD	((letter)-2) /* (error)��ɽ������������ */
#define CHMSIG	((letter)-3) /* �⡼�ɥ����󥸤�ɽ�������� */
#define NISEBP	((letter)-4) /* ���顼�ٹ��Ѥε�BEL������ */
#define VARRNG	((letter)-5) /* �Ѱ褬��Ĥ�ʸ���δ֤Ǥ���Τ򼨤�����������*/
#define UNUSDC	((letter)-6) /* �ޥå����Ԥ����뤿������������� */
#define REASIG	((letter)-7) /* ɽ�κ�������׵᤹�뤿������������� */
#define URBFCL	((letter)-8) /* ΢�Хåե�������Ū���ꥢ���׵᤹������������*/
#define LTREOF	((letter)-9) /* romkan_next()��Ʊgetc()��EOF��������֤����
				�ʤǤ����EOF��Ʊ���ˤ��������ġ�*/
#define REDRAW	((letter)-10)/* Wnn���Ѵ��Ԥ�redraw���׵᤹���ü쥳���� */
#define NISEDL	((letter)-11)/* ���ǥ꡼�ȤȤ��������Ȥ������ͤΤ���˽���
				���줿�����â�����Ȥ������ʤ���лȤ�ʤ��Ƥ�
				�����ˡ��ѿ�nisedl�˥��åȤ��ƻȤ� */
#define SENDCH	((letter)-12)/* �Ѵ��Хåե������ʤ顢����ʸ����̵����
				�������褦Wnn���׵᤹�륨�������ץ����ɡ�*/


	/* romkan_init3()�ΰ����Υե饰�˻Ȥ��� */

#define RK_CHMOUT 0x1	/* �⡼�ɥ����󥸤��Τ餻�륳���ɤ��֤����� */
#define RK_KEYACK 0x2	/* ����������Ф�ɬ���������֤��� */
#define RK_DSPNIL 0x4	/* �⡼��ɽ��ʸ����̵����λ��˶�ʸ������֤����ʥǥ�
			   ����Ȥ�NULL�ˡ��ߴ����ݻ��Τ��� */
#define RK_NONISE 0x8	/* �������ɤ�Ф��ʤ��褦�ˤ��뤫 */
#define RK_REDRAW 0x10	/* Wnn���ü�ե饰��redraw�ѤΥե饰��Ф����ɤ�����*/
#define RK_SIMPLD 0x20	/* delete��ư���ñ��ˤ��뤫 */
#define RK_VERBOS 0x40	/* ���Ѥ���ɽ�ΰ������ݡ��Ȥ��뤫 */


	/* �����ɤζ��̤˻Ȥ��ޥ��� */

#define HG1BIT	(0x80000000) /* �Ǿ�̥ӥåȤ��� */
#define SHUBET(X) ((letter)(X) >> (int)24)
			      /* ����ɽ���ǡ��壱�Х��Ȥ����ɽ���˻ȤäƤ� */
#define LWRMSK(X) ((X) & ~(0xff000000)) /* �壱�Х��Ȥ������ */
#define LWRCUT(X) ((X) &= ~(0xff000000)) /* �壱�Х��Ȥ������ */
#define XY2INT(X, Y) (((X) << (int)24) | (Y)) /* ���줬���󤷤�Y��ɾ�����ʤ����Ȥ�
	��¸���ƽ񤤤Ƥ���ս꤬����  ��ա� */

#define is_HON(X) (SHUBET(X) == 0) /* ��ʪ��ʸ���� */
#define NAIBEX(X) (0 < SHUBET(X) && SHUBET(X) < 0x80) /* ����ɽ���򼨤��ͤ� */
#define isNISE(X) (SHUBET(X) == 0x80) /* ��ʪ��ʸ�����ʺǾ�̥ӥåȤ�Ω�ġ� */
#define isSPCL(X) (SHUBET(X) == 0xff) /* rk_spclval.h�����������ͤ��ɤ���*/
 /* NISEDL�ʤɤ�ޤ᤿����ʸ���Ǥ��뤫�ɤ�����Ƚ�ꤹ��ˤϡ�~is_HON(X) ����
    isNISE(X) || isSPCL(X) �Ȥ���Ƚ�ꤷ�ʤ��Ȥ����ʤ���*/

#define toNISE(X) ((X) | HG1BIT)
#define to_HON(X) ((X) & ~HG1BIT)

 /* �ߴ����Τ����Ѱդ��Ƥ�����̾ */
#define REALCD(X) is_HON(X)
#define NISECD(X) isNISE(X)
#define SPCLCD(X) isSPCL(X)


	 /** Ⱦ��ʸ���Υ����ɤ�define */

#ifndef HNKAK1
#  ifdef IKIS
#    define HNKAK1 0xA8
#  else
#    define HNKAK1 0x8E
#  endif
#endif

	/** rk_bltinfn.c ���䴰�Τ���Υޥ���
	    �ʰ�����ʣ����ɾ�������Τ�¿���Τ���ա�*/

#define HKKBGN	(HNKAK1 * 0x100 + 0xA1) /* Ⱦ�ѥ��ʤζ��� */
#define HKKEND	(HNKAK1 * 0x100 + 0xDF) /*     ��    Ⱦ���� */
#define HIRBGN	(0xA4A1) /* �� */
#define HIREND	(0xA4F3) /* �� */  /* �Ҥ餬�ʡ�"��"��"��" */
#define KATBGN	(0xA5A1) /* �� */
#define KATEND	(0xA5F6) /* �� */  /* �������ʡ�"��"��"��"��"��" */

#define _to_kata(l) ((l) + (KATBGN - HIRBGN)) /** �������ʤء���������¡�*/
#define _to_hira(l) ((l) - (KATBGN - HIRBGN)) /** �Ҥ餬�ʤء���������¡�*/
#define is_hira(l) (HIRBGN <= (l) && (l) <= HIREND) /** �Ҥ餬�ʤ��� */
#define is_kata(l) (KATBGN <= (l) && (l) <= KATEND) /** �������ʤ��� */
#define is_kata2(l) (_to_kata(HIRBGN) <= (l) && (l) <= _to_kata(HIREND))
				/** �б�����Ҥ餬�ʤΤ��륫�����ʤ��� */
#define to_kata(l) (is_hira(l) ? _to_kata(l) : (l)) /** �������ʤ� */
#define to_hira(l) (is_kata2(l) ? _to_hira(l) : (l))
			/** �Ҥ餬�ʤء��֥������פϥ������ʤΤޤ޻Ĥ롣*/
#define is_hankata(l) (HKKBGN <= (l) && (l) <= HKKEND)
				/** Ⱦ�ѥ��ʡʶ����ʤɤ�ޤ�ˤ��� */



	/* ����¾�Υޥ���ؿ����ʰ�����ʣ����ɾ�������Τ�¿���Τ���ա�*/

#define numberof(array) (sizeof(array) / sizeof(*array))

 /* �ݥ��󥿤�letter����������ؤ�äƤ�����letter *lp */
#define totail(lp) {while(*(lp) != EOLTTR) (lp)++;}

 /* ʸ����ؤΥݥ��󥿤򤽤�ʸ����κǸ����ء�totail��char�ǡ�char *sp */
#define strtail(sp) {while(*(sp)) (sp)++;}

	/* ������romkan_init3 */
#define romkan_init4(pathname, delchr, nisedl, keyin, bytecount, flg) \
	romkan_init3(pathname, delchr, nisedl, EOLTTR, \
		     keyin, bytecount, bytecount, 0, flg)

#define romkan_init5(pathname, delchr, keyin, bytecount, flg) \
	romkan_init4(pathname, delchr, toNISE(delchr), keyin, bytecount, flg)


#endif	/* of ifndef EOLTTR */
