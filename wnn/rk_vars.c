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
			rk_vars.c
						88. 6.16  �� ��

	�ץ���������İʾ�Υե�����ˤޤ����äƻȤ��ѿ���
	�ޤȤ��������Ƥ��롣
***********************************************************************/
/*	Version 3.0
 */
#include "rk_header.h"
#include "rk_extvars.h"

char	rk_errstat = 0; /* (error)�ˤ�äƥ��顼�������������줿��1��â��
			   (eofflg)��Ω�äƤ������������ޤǤζ������Ͻ�����
			   �����ä�����2�ˤˤʤ롣romkan_henkan()�¹Ԥ��Ȥ�
			   ��������롣����ϥ桼���ˤⳫ����*/

int	flags = 0;
 /* �ʲ��Υե饰��OR��
	RK_CHMOUT �⡼�ɥ����󥸤��Τ餻�뤫�Υե饰 
	RK_KEYACK ����������Ф�ɬ�������֤����Υե饰 
	RK_DSPNIL romkan_disp(off)mode�����ͤΥǥե���Ȥ���ʸ����Ǥ��뤫��
		  �ե饰��Ω���ʤ���Хǥե���Ȥ�NULL���ߴ����Τ���romkan_
		  init2�ޤǤϤ��줬Ω�ġ�
	RK_NONISE �������ɤ�Ф��ʤ��褦�ˤ��뤫�Υե饰
	RK_REDRAW Wnn�ǻȤ��ü�ʥե饰�����줬Ω�äƤ����硢romkan_henkan
		  �η�̤Ȥ���disout��nisedl�ʵ�ʪ��del�˰ʸ夬�ü쥳���ɤΤ�
		  �ǽ����ʸ�����֤äƤ����顢���θ���REDRAW��Ĥʤ���Wnn
		  ���Ѵ��Ԥ�redraw�򤹤뤿���ɬ�פ����֡�
	RK_SIMPLD delete��ư���ñ��ˤ��뤫�Υե饰
	RK_VERBOS verbose�ǵ��������Υե饰
 */

jmp_buf env0;

FILE	*modefile; /* �⡼�����ɽ�Υե����� */
char	nulstr[1] = {'\0'};

char	*hcurread, *mcurread; /* �Ѵ�ɽ���⡼��ɽ�θ��߹�buf�ؤΥݥ��� */
char	*curfnm, *curdir; /* �����ɤ�Ǥ��Ѵ�ɽ���⡼��ɽ��̾�ȥǥ��쥯�ȥꡣ
     â��curdir�ϡ��������ѥ����ܤ��Ƥ���ɽ�Υ����ץ�������������Τ�ͭ�� */
letter	*ltrbufbgn; /* �������letter�ΥХåե�����Ƭ��ؤ� */
  /* �����ޤĤϡ�readdata()�ʤɤδؿ���ǥ��������������������Ƭ���
     �������äơ����δؿ���ȴ������Ǥ������ͤ�Ȥ�ʤ��褦��ʬ��ա��äˡ�
     ���顼�ٹ�롼����ʤɤǤϡ�longjmp����������ˤ�����print��Ԥ�����
     �ʥ��顼�����Ѥ˻ȤäƤ���Τϡ��夫��͸Ĥޤǡˡ�*/
FILE	*nestfile[FILNST], **base; /* �Ѵ��б�ɽ�Υե����� */

struct	modestat modesw[MODMAX];

char	hyoshu[HYOMAX]; /* ɽ�μ��̡������ܡ������ɽ�ζ��̡ˤ���������� */
char	**modmeiptr, *modmeibgn[MODMAX]; /* �⡼��̾���Ф�����Ʊ�ͤΥǡ���*/
char	*modmeimem, modmeimem_[MODMEI]; /*		��		*/
char	**dspnamptr, *dspnambgn[DMDMAX];
				/* �⡼��ɽ��ʸ������Ф���Ʊ�ͤΥǡ��� */
char	*dspcod, dspcod_[DMDCHR]; /*		��		*/
char	*dspmod[2][2] = {NULL, NULL, NULL, NULL};
	 /* ���ߵڤӰ�����Ρ�romkan�򤽤줾��on��off���Ƥ�����Υ⡼��ɽ��
	    ʸ����ؤΥݥ��󥿡�romkan_dispmode()���֤��ͤ�dspmod[0][0]��*/

char	**hyomeiorg, **hyomeiptr;
	 /* ɽ��̾�ؤΥݥ��󥿤���������� *hyomeibgn[HYOMAX] �� readdata() ��
	    �Ȥ�����������Ƭ�ڤ���������ǤؤΥݥ��� */
char	*hyomeimem;
	 /* ɽ��̾�μºݤ�ʸ�������������� hyomeimem_[HYOMEI] �� readdata()
	    �ǻȤ�����������������ǤؤΥݥ��� */
char	**pathmeiorg, **pathmeiptr;
	 /* �б�ɽ�Υ������ѥ�̾�ؤΥݥ��󥿤���������� *pathmeibgn[PTHMAX]��
	    readdata() �ǻȤ�����������Ƭ�ڤ���������ǤؤΥݥ��� */
char	*pathareaorg, *pathmeimem;
	 /* �嵭�������ѥ�̾�μºݤ�ʸ�������������� pathmeimem_[PTHMEI] ��
	    readdata() �ǻȤ�����������Ƭ�ڤ���������ǤؤΥݥ��󥿡�
	    â��pathmeimem_����Ƭ�ˤϥ⡼��ɽ�Τ���ǥ��쥯�ȥ��̾�����ꡢ��
	    �ǻȤ��Τǡ�pathareaorg�ˤϤ���³���Υ��ꥢ����Ƭ�����Ϥ����롣*/

int	usemaehyo[HYOMAX], usehyo[HYOMAX], useatohyo[HYOMAX];
	 /* �����ܡ������ɽ�Τɤ줬���򤵤�Ƥ��뤫�Υǡ��� */
int	naibu_[NAIBMX], *naibu; /* �⡼��ɽ������ɽ������������� */

letter	*lptr; /* letter�ؤ����ѥݥ��� */

letter	rk_input; /* ���ϡ�3�Х��ȤޤǤ��ѹ��ʤ����б��� */
letter	disout[OUTSIZ]; /* �ܽ�������ν��ϤΥХåե� */
letter	rk_output[OUTSIZ]; /* ���������ν��ϡʺǽ����ϡˤΥХåե� */
letter	keybuf[KBFSIZ], urabuf[KBFSIZ]; /* �ܽ����Хåե��Ȥ��������ΰ� */
int	lastoutlen, lastkbflen;
letter	oneletter[2] = {EOLTTR, EOLTTR}, nil[1] = {EOLTTR};
int	hyonum;

#define bit3(x, y, z) ((x) | ((y) << 1) | ((z) << 2))

struct	funstr	func[] = /* �ؿ����ɲäˤϤ����� mchevl(), evlis() �򤤤��� */
{
/*			�����ο���-1�Ĥ�����̤���ѡ�
	  ̾		    �и�     ��	 �����ס�0:ʸ���ؿ� 1:ʸ����ؿ�
	  ��		     ��	     ��	 ��	 2:�ü�ؿ� 3:��ǽ 4:��� */
	"defvar",	bit3(1,0,0), -1, 4, /*	0 */
	"include",	bit3(1,0,0), -1, 4,
	"toupper",	bit3(1,1,1),  1, 0,
	"tolower",	bit3(1,1,1),  1, 0,
	"off",		bit3(0,1,0), -1, 2,
	"on",		bit3(0,1,0), -1, 2, /*	5 */
	"switch",	bit3(0,1,0), -1, 2,
	"toupdown",	bit3(1,1,1),  1, 0,
	"tozenalpha",	bit3(1,1,1),  1, 0,
	"tohira",	bit3(1,1,1),  1, 0,
	"tokata",	bit3(1,1,1),  1, 0, /* 10 */
	"tohankata",	bit3(1,1,1),  1, 1,
	"tozenhira",	bit3(1,1,1),  1, 0,
	"tozenkata",	bit3(1,1,1),  1, 0,
	"+",		bit3(1,1,1),  2, 0,
	"-",		bit3(1,1,1),  2, 0, /* 15 */
	"*",		bit3(1,1,1),  2, 0,
	"/",		bit3(1,1,1),  2, 0,
	"%",		bit3(1,1,1),  2, 0,
	"last=",	bit3(1,0,0),  1, 2,  /* ľ���Υޥå�������==������? */
	"if",		bit3(1,0,0), -1, 2, /* 20 */
	"unless",	bit3(1,0,0), -1, 2,
	"restart",	bit3(0,1,0),  0, 3,
	"delchr",	bit3(1,1,1),  0, 0,  /* delchr���֤����������ޥ�� */
	"alloff",	bit3(0,1,0),  0, 2,  /* ���⡼�ɤ򤤤äڤ��OFF */
	"allon",	bit3(0,1,0),  0, 2, /* 25 */
	"bitand",	bit3(1,1,1),  2, 0,
	"bitor",	bit3(1,1,1),  2, 0,
	"bitnot",	bit3(1,1,1),  1, 0,
	"!",		bit3(0,1,0),  0, 2,  /* ΢�Хåե�������Ū���ꥢ */
	"atEOF",	bit3(1,0,0),  0, 2, /* 30 */
	"todigit",	bit3(1,1,1),  2, 1,
	"dakuadd",	bit3(1,1,1),  1, 1,  /* �������ղ� */
	"handakuadd",	bit3(1,1,1),  1, 1,  /* Ⱦ�������ղ� */
	"value",	bit3(1,1,1),  1, 0,
	"error",	bit3(0,1,0),  0, 3, /* 35 */
	"defconst",	bit3(1,0,0), -1, 4,
	"setmode",	bit3(0,1,0), -1, 2,
	"mode+",	bit3(0,1,0), -1, 2,
	"mode-",	bit3(0,1,0), -1, 2,
	"mode=",	bit3(1,0,0), -1, 2, /* 40 */
	"mode!=",	bit3(1,0,0), -1, 2,
	"mode<",	bit3(1,0,0), -1, 2,
	"mode>",	bit3(1,0,0), -1, 2,
	"send",		bit3(0,1,0),  1, 1,  /* ��� */
	NULL
};
	/* last=��unless�� ���ϥ����ɤΰ��֤ˤ����֤�����ʸ����ؿ������ʤΤ�
			�ܽ���ɽ�ˤ����񤱤ʤ���*/
	/* +,-,*,/,bitand,bitor,bitnot�� 3�Х���ʬ�α黻��Ԥ���*/
	/* atEOF�� �ե����������Ǥ�����ư�����ꤹ�뤿��Τ�Ρ�â��������
		ư����Դ�����*/

struct	hyo	hyo_n[HYOMAX]; /* ɽ���Ȥ��Ѱ衦�б��ǡ����γ������� */
