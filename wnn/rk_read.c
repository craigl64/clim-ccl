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
			rk_read.c
						88. 6.11  �� ��

	rk_main.c �����ʡ��Ѵ��б�ɽ���ɤ߹��ߤ�ô����
***********************************************************************/
/*	Version 3.0
 */
#include "rk_header.h"
#include "rk_extvars.h"
#include <sys/types.h>
#include <sys/stat.h>

letter	onescan(), *rangekettei();
char	*ename();

extern	letter	*ltrgrow(), *ltrend(), letterpick(), *ltr1cut();
extern	char	*chrcat(), *strend();

static int termsscan(), evalandcpy(), eval1cpy(), partscan(),
  evlis(), atermscan(), serfun(), hensrc_tourk(), blankpass(),
  modsrcL(), chkL_get_int();
static void ERRLIN(), ERHOPN(), readhyo(),
  ltr1tostr(), ERRHYO(), vchk(), rangeset(), de_bcksla(), 
  listscan(), singleqscan(), doubleqscan();
extern void BUGreport(), choosehyo(), readmode();
extern int ltrcmp(), ltrstrcmp(), readfnm(), fixednamep();

#define IHENSU (1 << 24) /* ����ɽ���ξ�̣��Х��Ȥǡ��ѿ���ɽ�� */
#define IKANSU (2 << 24) /*	      ��	      �ؿ���ɽ�� */
#define IMODNM (3 << 24) /*	      ��	      �⡼��̾��ɽ�� */

#define ENDOF_NestFileArea ((FILE *)~0)
	 /** include�ե�����Υǥ�������ץ���Ǽ���ꥢ�Υ���ɥޡ���*/


 /**	����ɤ߹��ߡ�������ɤ߹��ޤʤ�������EOF�ˤĤ������ä���0���֤���
	â��include���θ���Ƥ��롣�б�ɽ�ˤ϶���ʸ���ʳ��Υ���ȥ���ʸ����
	���ǤϺ�����ʤ���ΤȤ��������äƤ���������å����롣*/
int
readln(buf)
uns_chr *buf;
{
	register uns_chr *p;
	register int	c;

	if(NULL == *base){
		*buf = '\0';
		return(0);
	}

	p = buf;
	while(1){
		while(EOF == (c = getc(*base))){
			fclose(*base);
			if(NULL == *--base){
				c = EOF;
				break;
			}
		}
		if(c == EOF || c == '\n') break;

		if(is_cntrl(c) && !isspace(c)){
			sprintf(hcurread, "\\%03o", c);
			ERRLIN(21);
		}
		*p++ = c;
	}
	*p = '\0';
	return(p != buf || c != EOF);
}

letter	*memptr, *ltrbufptr, *hensumei, *hen_iki, *term, **henptr;
letter	*dummy;
struct	dat	*datptr;
struct	hensuset
{
	unsigned	regdflg : 1;	/* ����Ͽ���ѿ���ɽ�� */
	unsigned	curlinflg : 1;	/* ���߹Ԥ˴��Ф��ѿ���ɽ�� */
	unsigned	constflg : 1; /* ����Ȥ���������줿���Ȥ�ɽ�� */
	letter		*name;
	letter		*range;
} *hentourkptr, *henorg;

 /** Ϳ����줿�ե����뤬�ǥ��쥯�ȥ�ʤ���0���֤� */
int
isdir(fnm)
char	*fnm;
{
	struct	stat	statbuf;
/*
	return(stat(fnm, &statbuf) == 0 && (statbuf.st_mode & S_IFDIR));
*/
	return(stat(fnm, &statbuf) == 0 && ((statbuf.st_mode & S_IFMT) == S_IFDIR));
}

 /**	name��Ϳ����줿̾���Ѵ��б�ɽ�򥪡��ץ󤹤롣����̾�ȸ��ʤ���ʤ�
	̾����fixednamep()���ȡˤ��Ф��Ƥϡ��������ѥ��򸫤롣���ξ�硢
	�����ץ�����������顢�ѥ�̾�� *dirnamptr��������֤롣����¾�ξ���
	*dirnamptr�˶�ʸ�������롣*errcod�ˤϼ��Υ��顼�����ɤ����롣
	Ϳ����줿�ե����뤬�ǥ��쥯�ȥ�λ�1�ʥ������ѥ���õ��������3�ˡ�
	�ե�����򥪡��ץ�Ǥ��ʤ���2��Ʊ4�ˡ�*/
FILE	*trytoopen(name, dirnamptr, errptr)
char	*name, **dirnamptr;
int	*errptr;
{
	char	**src, taio_fnm[REALFN];
	FILE	*opened;

	*dirnamptr = nulstr;
	*errptr = 0;

	if(fixednamep(name)){
		if(isdir(name)){
			*errptr = 1;
			return(NULL);
		}
		if(NULL == (opened = fopen(name, "r"))){
			*errptr = 2;
			return(NULL);
		}
		if(flags & RK_VERBOS){
			fprintf(stderr, "romkan: using Taio-hyo %s ...\r\n",
				name);
		}
		return(opened);
	} else {
		for(src = pathmeiorg; *src != NULL; src++){
			strcpy(taio_fnm, (*dirnamptr = *src));
			strcat(taio_fnm, name);

			if(isdir(taio_fnm)){
				*errptr = 3;
				return(NULL);
			}
			if(NULL == (opened = fopen(taio_fnm, "r"))) continue;

			 /* Now Taio-hyo found */
			if(flags & RK_VERBOS)
				fprintf(stderr,
					"romkan: using Taio-hyo %s ...\r\n",
					taio_fnm);
			return(opened);
		}
		if(flags & RK_VERBOS){ /* Taio-hyo not found */
			char	*p, *q;

			fprintf(stderr, "no %s in", name);
			for(src = pathmeiorg; *src != NULL; src++){
				fputc(' ', stderr);
				q = *src;
				if(*q == KUGIRI && *(q + 1) == '\0') q++;
				 else q = strend(q);
				for(p = *src; p < q; p++)
					fputc(*p, stderr);
			}
			fprintf(stderr, ".\n");
		}
		*dirnamptr = nulstr;
		*errptr = 4;
		return(NULL);
	}
}

 /** ɽ�������ɤ߹��� */
void
readdata(memory, data, hensudefhyo, modf)
letter	*memory;	/* �б�ɽ������ɽ����������������Ƭ���Ϥ��Ϥ���� */
struct	dat	*data;	/* �б�ɽ�ιԤ��ȤΥǡ������������������� */
letter	**hensudefhyo;	/* �ѿ�����Υǡ������������������� */
char	*modf;		/* �⡼�����ɽ��̾���Ϥ��Υѥ�̾�����äƤ��� */
{
	int	i, j;
	char	*hyomeibgn[HYOMAX];
	 /* ɽ��̾�ؤΥݥ��󥿤���������� */
	char	hyomeimem_[HYOMEI];
	 /* ɽ��̾�μºݤ�ʸ�������������� */
	char	*pathmeibgn[PTHMAX];
	 /* �������ѥ�̾�ؤΥݥ��󥿤���������� */
	char	pathmeimem_[PTHMEI];
	 /* �������ѥ�̾�μºݤ�ʸ�������������� */
	char	modfnm[REALFN];
	 /* �⡼��ɽ�Υե�����̾�򤳤��إ��Ԥ� �Τ��ˤϤ��Υѥ�̾������ */

	memptr = memory;
	datptr = data;
	henptr = hensudefhyo;
	*(pathmeiptr = pathmeiorg = pathmeibgn) = NULL;
	*(pathmeimem = pathmeimem_) = '\0';
	*(hyomeiptr = hyomeiorg = hyomeibgn) = NULL;
	*(hyomeimem = hyomeimem_) = '\0';
	*(modmeiptr = modmeibgn) = NULL;
	*(modmeimem = modmeimem_) = '\0';
	*(dspnamptr = dspnambgn) = NULL;
	*(dspcod = dspcod_) = '\0';
	*(naibu = naibu_) = '\0';

	strcpy(modfnm, modf);
	     /* modfnm��Ϳ�����ե�����̾���ѥ�̾����Ϳ����줿�顢�ǥե���Ȥ�
		�⡼��ɽ̾"mode"��Ĥʤ���KUGIRI��ǥ��쥯�ȥ�ζ��ڤ�ʸ�� */

	if(*modfnm == '\0' || *(strend(modfnm)) == KUGIRI){
		strcat(modfnm, "mode");
	} else if(isdir(modfnm)){
		chrcat(modfnm, KUGIRI);
		strcat(modfnm, "mode");
	}

	curdir = nulstr;
	readmode(curfnm = modfnm);

	for(i = 0; hyomeiorg[i] != NULL; i++){
		int	err;

		for(j = 0; j < FILNST; j++) nestfile[j] = NULL;
		nestfile[FILNST - 1] = ENDOF_NestFileArea;
		base = nestfile + 1;

		*base = trytoopen(curfnm = hyomeiorg[i], &curdir, &err);
		if(err){
			switch(err){
				case 1:
				case 3: ERHOPN(1);
				case 2:
				case 4: ERHOPN(0);
			}
		}
		readhyo(i);
	}

	hyo_n[i] . data = NULL;
	choosehyo();
}

 /** �ե�����̾����ѥ�̾���������ʬ����Ƭ���֤���*/
char	*ename(s)
char	*s;
{
	char	*p;

	p = strrchr(s, KUGIRI);
	return(p == NULL ? s : p + 1);
}

 /**	�ե�����̾�Υ����å�����Ƭ�ʥѥ�̾�Ͻ����ˤ�'1'��'3'�Ǥʤ��Ȥ����ʤ���
	�����ʤ�Τʤ�1��3�������ܡ������ɽ�ζ��̤�ɽ���ˤ��֤��������Ǥʤ���
	0���֤���*/
int
filnamchk(s)
char	*s;
{
	char	c;

	c = *(ename(s)) - '0';
	return((1 <= c && c <= 3) ? c : 0);
}

 /**	linbuf�����äƤ���unsigned char�����letter����ˤ���ltrbuf������롣
	����ɥޡ�����EOLTTR�ˤʤ롣flg����0�λ��ϡ���Ƭ�ζ���ʸ�������Ф���*/
void
ustrtoltr(linbuf, ltrbuf, flg)
uns_chr *linbuf;
int	flg;
register letter *ltrbuf;
{
	register letter l;

	if(flg){
		while(l = letterpick(&linbuf), is_eolsp(l)){
			if(l == EOLTTR){
				*ltrbuf = EOLTTR;
				return;
			}
		}
		*ltrbuf++ = l;
	}
	while((*ltrbuf++ = letterpick(&linbuf)) != EOLTTR);
}

 /**	letter�����ʸ����˥���С��ȡ���ĤΥݥ��󥿤�Ʊ�����ϤǤ��äƤ�
	ư����ݾڤ��뤳�ȡ�*/
void
ltrtostr(ltrbuf, linbuf)
char	*linbuf;
letter	*ltrbuf;
{
	letter	l;

	while((l = *ltrbuf++) != EOLTTR) ltr1tostr(l, &linbuf);
	*linbuf = '\0';
}

 /** letter��ʸ����˥���С��� */
static void
ltr1tostr(l, sptr)
char	**sptr;
letter	l;
{
	int	i;

	for(i = 0; i < 3 && 0 == (l & (0xff000000)); i++) l <<= 8;
	for(; i < 4; i++){
		*(*sptr)++ = (char)(l >> 24);
		l <<= 8;
	}
}

 /** �ѿ��Ρָ��߹Դ��Хե饰�פ����ƥ��ꥢ */
void
hen_useflgclr(hensu)
struct	hensuset	*hensu;
{
	for(; hensu -> name != NULL; hensu++)
		hensu -> curlinflg = 0; /* ���߹Ԥ�̤�� */
}

 /** �б�ɽ����ɤ߹��� */
static void
readhyo(n)
int	n;
{
	uns_chr linbuf[LINSIZ];
	letter	ltrbuf[LINSIZ], *lp;
	letter	termbuf[SIZALL]; /* ���顼���θ���ơ�ɽ�Υ�����ʬ��äƤ���*/
	letter	dummybuf[TRMSIZ], hensumei_[VARLEN], hen_iki_[SIZALL];
	struct	hensuset	hensu[VARMAX];
	int	m, hyosw, rsltyp[3];

	hcurread = (char *)linbuf; /* ���顼������ */
	ltrbufbgn = ltrbuf;

	*(hensumei = hensumei_) = EOLTTR;
	*(hen_iki = hen_iki_) = EOLTTR;
	(henorg = hentourkptr = hensu) -> name = NULL;
	dummy = dummybuf;

	hyo_n[n] . hensudef = henptr;
	hyo_n[n] . data = datptr;

	hyosw = hyoshu[n];
	while(readln(linbuf)){
		hen_useflgclr(henorg);
		ustrtoltr(linbuf, ltrbuf, 1);
		ltrbufptr = ltrbuf;

		for(m = 0; termsscan(&ltrbufptr, term = termbuf, 1); m++){
			 /* m�ϡ������ܤι��ܤ򸫤Ƥ��뤫��ɽ�� */
			if(*term == ';') break; /* ���� */
			if(m == 3) ERRLIN(15);
			if(m != 0 && rsltyp[0] == 4) ERRLIN(12);

			datptr -> code[m] = memptr;
			if((rsltyp[m] = evalandcpy(&term, m)) == 4){
				if(m) ERRLIN(14);
			 /* ����Ϻǽ�ι��ܤˤ������ʤ���
			    funstr[]��appear�ե饰�Ǥ�����å����Ƥ��뤬
			    ����Τ���˰�������ˤ�����å�������Ƥ�����*/
			} else {
			 /* ����λ��ˤ�����ɽ���ؤΥݥ��󥿤Ͽʤ�ʤ� */
				totail(memptr);
				memptr++;
			}
		}

		if(m != 0 && rsltyp[0] != 4){
			for( /* m=? */ ; m < 3; m++){
				datptr -> code[m] = nil;
				rsltyp[m] = -1; /* doesn't exist */
			}
			datptr++;

 /* rsltyp: 0=ʸ���� 1=ʸ����Ϣ 2=�ǡ���Ϣ 3=��ǽ�� 4=����� -1=¸�ߤ��� */
			switch(hyosw){
		   /* ����������ϡ�ɽ�����Ƥ����¤����롣����򸡺� */
				case 1:
					if(!(rsltyp[0] == 0 &&
					     rsltyp[1] == 0 && 
					     rsltyp[2] == -1
					    ))
						ERRLIN(17);
					break;
				case 2:
					if(rsltyp[1] == 3 && rsltyp[2] != -1)
						ERRLIN(19);
					break;
				case 3:
					if(!(rsltyp[0] == 0 &&
					     (rsltyp[1] == 0 ||
					      rsltyp[1] == 1) &&
					     rsltyp[2] == -1
					    ))
						ERRLIN(18);
					break;
				default:
					BUGreport(10);
			}

		}
	}

	(datptr++) -> code[0] = NULL;

		/* �ѿ���Ͽ�����ǤޤȤ�Ƥ��� */
	for(lp = hen_iki_; lp < hen_iki; ) *memptr++ = *lp++;
	for(hentourkptr = henorg; hentourkptr -> name != NULL;
	    hentourkptr++){
		if(hentourkptr -> regdflg == 0) ERRHYO(0);
		*henptr++ = memptr - (lp - hentourkptr -> range);
	}
	*henptr++ = NULL;

	/* ������ fclose(*base); �����ס�readln��Ǥ���Ǥ��� */
}

 /**	�Ѵ��б�ɽ�ι��ܰ�Ĥ򡢲�ᤷ�ơ����������Υǡ������ꥢ�˥��ԡ����롣
	�֤��ͤϡ���ᤷ�����ܤ�ʸ����ʤ�0������ʳ���ʸ����Ϣ�ʤ�1������ʳ�
	�ǥǡ���Ϣ�ʤ�2����ǽ��ʤ�3�������ʤ�4������ʳ��ϥ��顼��*/
static int
evalandcpy(socp, m)
register letter **socp; /* ���ܤؤΥݥ��󥿡ʤؤΥݥ��󥿡�*/
int	m;   /* �б�ɽ�β����ܤι��ܤ򸫤Ƥ��뤫��ɽ�������ϥ���������
		���Ƥ���Ȥ���0�����ϥ��������ʤ�1���Хåե��Ĥ����ʤ�2 */
{
#define TYPMAX	5

	char	exist[TYPMAX], total;
	 /* exist�ϡ����줾�쥿����0��4�ι�νи��Υե饰��eval1cpy()���ȡˡ�
	    total�����ΤȤ��Ƥνи��ե饰���ɤ���⡢1=̤�� 2=���� ����¾��
	    ���ϲ�2�ӥå�0 */
	int	type;

#define TIME_0(flag) ((flag) == 1)
#define TIME_1(flag) ((flag) == 2)
#define TIME_n(flag) (((flag) & 3) == 0)

	total = 1;
	for(type = 0; type < TYPMAX; type++) exist[type] = 1;

	while(!is_eolsp(**socp)){
		if(!(0 <= (type = eval1cpy(socp, m, 0)) && type < TYPMAX))
			BUGreport(3);
		exist[type] <<= 1;
		total <<= 1;
	}

	if(TIME_0(total))
		BUGreport(13); /* ���ܤ����ʤ�evalandcpy�ϼ¹Ԥ��ʤ�Ȧ */
	if(!TIME_0(exist[3]) || !TIME_0(exist[4])){
		if(TIME_n(total)) ERRLIN(9);
		return(type); /* 3����4��type���ͤ��ݻ����Ƥ���Ȧ */
	}
	if(TIME_1(total) && TIME_1(exist[0])) return(0);
	return(!TIME_0(exist[2]) ? 2 : 1);
}

 /**	�б�ɽ�ι��ܤ���ι��Ĥ��ᤷ�����������Υǡ������ꥢ�˥��ԡ�����
	����Υ����ס�ʸ����=0 ʸ�����=1 �ü�ؿ���=2 ��ǽ��=3 �����=4�ˤ�
	�֤���flg����0�ʤ顢�Ƶ�Ū�˸ƤФ줿���Ȥ��̣�������ξ�硢
	���߹Ԥ�̤�Ф��ѿ��򸡽Ф����饨�顼���ޤ���m����0�ΤȤ������ϥ�������
	�ʳ��ν�򸫤Ƥ�����ˤ⡢���߹Ԥ�̤�Ф��ѿ��򸡽Ф����饨�顼��*/
static int
eval1cpy(socp, m, flg)
letter	**socp; /* flg�ʳ��ΰ����ΰ�̣��evalandcpy��Ʊ�� */
int	m, flg;
{
	letter	t1buf[TRMSIZ], *t1bufp;

	t1bufp = t1buf;

	*memptr = EOLTTR;
	switch(partscan(socp, t1bufp)){
		case 1: /* ñʸ�� */
			memptr = ltrgrow(memptr, t1bufp);
			*memptr = EOLTTR;
			return(0);
		case 2: /* ����ʸ�� */
			t1bufp++;
			*memptr++ = onescan(&t1bufp, dummy);
			*memptr = EOLTTR;
			return(0);
		case 3: /* ����ʸ���� */
			t1bufp++;
			while(*t1bufp != '"'){
				*memptr++ = onescan(&t1bufp, dummy);
			}
			*memptr = EOLTTR;
			return(1);
		case 0: /* �ꥹ�� */
			return(evlis(m, &t1bufp, flg));
			 /* evlis��� *memptr = EOLTTR; �򤷤Ƥ��롣*/
		default:
			BUGreport(4);
			return(-1);
			 /*NOTREACHED*/
	}
	/*NOTREACHED*/
}

#define bitchk(x, n) ((x) & (1 << (n)))

#define get_ltr(lp) (*(lp)++)
#define unget_ltr(l, lp) (*--(lp) = (l))

 /** global�ʥݥ��󥿤���ؤ���Ƥ���letter�󤫤��ʸ����äƤ��롣*/
letter	get1ltr()
{
	return(get_ltr(lptr));
}

letter	unget1ltr(l)
letter	l;
{
	return(unget_ltr(l, lptr));
}

int int_get1ltr() {return((int)(get1ltr()));}
int int_unget1ltr(c) letter c; {return((int)(unget1ltr((letter) c)));}
 /** �������Ȥ��ξ�ʤ����ʤ��ؿ��η���cast���Ǥ��ʤ��Ρ�
    ��(int ()) get1ltr�פȽ񤭤����� */

 /**	include�ե�����̾��letter���string�˼��Ф���Ȥ�letter��ν����
	�ޤ�³���롣flg & 01����0�ʤ顢'/'�Ǥ⽪λ��*/
letter	getfrom_dblq(socp, destp, flg)
letter	**socp;
char	**destp;
int	flg;
{
	letter	l;

	while(**socp != EOLTTR && !(flg & 01 && **socp == KUGIRI)){
		if (**socp == '\\') *(*destp)++ = '\\';
		l = onescan(socp, dummy);
		ltr1tostr(l, destp);
	}
	*(*destp)++ = '\0';
	return(**socp);
}

int
getfrom_lptr(sptr, flg)
char	**sptr;
int	flg;
{
	return((int)getfrom_dblq(&lptr, sptr, flg));
}

 /**	�ꥹ�Ȥ��ᤷ������ɽ���ˤ��롣���ͤϡ����Υꥹ�ȤΥ����פ�ɽ������
	ʸ���ѿ�������ʸ���ؿ���:0 ʸ����ؿ���:1 �ü�ؿ���:2
	��ǽ��:3 �����:4 */
static int
evlis(m, socp, flg)
letter	**socp; /* �����ΰ�̣��eval1cpy�򻲾� */
int	m, flg;
{
	int	fnnum, hennum, i;
	letter	t1buf[TRMSIZ];

	(*socp)++; /* '('�򥹥��å� */
	atermscan(socp, t1buf, 3);

	fnnum = serfun(t1buf);
	if(fnnum != -1 && !bitchk(func[fnnum] . appear, m)) ERRLIN(14);
	 /* m���ͤˤ�äơ�����ƤϤ����ʤ���ؤνи����ɤ������Ƥ��롣*/

	switch(fnnum){ /* default�ν�ʳ��� func[fnnum].argnum��ȤäƤʤ� */
		case -1: /* �ѿ� */
			vchk(t1buf);
			atermscan(socp, dummy, 2); /* �����ERR */
			hennum = hensrc_tourk(t1buf, ((m==0 && !flg)? 0 : 1));
			*memptr++ = (henorg[hennum] . constflg ? 
				     *(henorg[hennum].range) : hennum|IHENSU);
			break;

		case 0: /* fn No.0 defvar */
			atermscan(socp, t1buf, 3);
			if(*t1buf == '('){
				letter	*soc2, t1buf2[TRMSIZ], t1buf3[TRMSIZ];
				letter	*common_hen;

				atermscan(socp, t1buf3, 3);

				soc2 = t1buf + 1; /* skip '(' */

				atermscan(&soc2, t1buf2, 3);
				vchk(t1buf2);
				if(-1 != serfun(t1buf2)) ERRLIN(11);
				hennum = hensrc_tourk(t1buf2, 2);
				common_hen = rangekettei(hennum, t1buf3);

				while(atermscan(&soc2, t1buf2, 0)){
					vchk(t1buf2);
					if(-1 != serfun(t1buf2)) ERRLIN(11);
					hennum = hensrc_tourk(t1buf2, 2);
					rangeset(hennum, common_hen);
				}
			} else {
				vchk(t1buf);
				if(-1 != serfun(t1buf)) ERRLIN(11);
				hennum = hensrc_tourk(t1buf, 2);
			  /* defvar��defconst���ѿ�̾�ν�ʣ���򤱤뤿�� */
				atermscan(socp, t1buf, 3);
				rangekettei(hennum, t1buf);
			}
			atermscan(socp, dummy, 2);
			break;
		case 36: /* fn No.36 defconst */
			atermscan(socp, t1buf, 3);
			vchk(t1buf);
			if(-1 != serfun(t1buf)) ERRLIN(11);

			hennum = hensrc_tourk(t1buf, 6);
			  /* defvar��defconst���ѿ�̾��ʣ���򤱤뤿�� */

			rangeset(hennum, hen_iki);

			blankpass(socp, 1);
			if(*(*socp)++ != '\'') ERRLIN(8);
			*hen_iki++ = onescan(socp, dummy);
			(*socp)++; /*��'�פ��Ĥ��Ƥ��뤳�Ȥθ����ϺѤ�Ǥ���*/
			*hen_iki++ = EOLTTR; /* needed? */
			*hen_iki = EOLTTR;

			atermscan(socp, dummy, 2);
			break;

		case 1: /* fn No.1 include */
			{
				char	fnmtmparea[REALFN], *s, *dirnamptr;
				int	dummyc = 0, err;

				blankpass(socp, 1);
				if(3 != partscan(socp, t1buf)) ERRLIN(22);
				atermscan(socp, dummy, 2);
				 /* ;ʬ�ˤ����ERR */

				ltr1cut(lptr = t1buf + 1);
				*(s = fnmtmparea) = '\0';
				err = readfnm(int_get1ltr, int_unget1ltr,
					      getfrom_lptr, &s, &dummyc);

				if(err){
					hcurread = s;
					switch(err){
						case 1:
						case 3: ERRLIN(25);
						case 2: ERRLIN(26);
						case 4: ERRLIN(27);
					}
				}
				de_bcksla(fnmtmparea, fnmtmparea);

				if(*++base == ENDOF_NestFileArea){
					base--;
					ERRLIN(23);
				}
				*base= trytoopen(fnmtmparea, &dirnamptr,&err);
				if(err){
					switch(err){
						case 1:
						case 3:
						case 2:
						case 4: base--;
							ERRLIN(24);
					}
				}
			}
			break;

		 /* �⡼��̾��Ĥ�����˼���� */
		case 4: /* fn No.4��6 off,on,switch */
		case 5:
		case 6:
		case 20: /* fn No.20,21 if,unless */
		case 21:
			*memptr++ = fnnum | IKANSU;
			atermscan(socp, t1buf, 3);
			*memptr++ = modsrcL(t1buf) | IMODNM;
			atermscan(socp, t1buf, 2);
			break;

		 /* �⡼��̾��ʸ�� ��Ĥ��Ĥ�����˼���� */
		case 37: /* fn No.37��43 setmode�ʤ� */
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
			{
				int	err, n;
				modetyp	stat;

				*memptr++ = fnnum | IKANSU;
				atermscan(socp, t1buf, 3);
				*memptr++ = (n = modsrcL(t1buf))| IMODNM;
				atermscan(socp, t1buf, 3);
				err = chkL_get_int(t1buf, &stat,
							 modesw[n] . moderng);
				if(err != 0) ERRLIN(29);
				*memptr++ = stat;
				atermscan(socp, t1buf, 2);
				break;
			}

		 /* ���̡ʰ�������ʤ���Τ�ޤ�� */
		default: /* toupper,tolower�� */
			*memptr++ = fnnum | IKANSU;
			*memptr = EOLTTR;
			for(i = 0; i < func[fnnum] . argnum; i++){
				blankpass(socp, 1);
				if(eval1cpy(socp, m, 1) != 0) ERRLIN(13);
			}
			atermscan(socp, dummy, 2); /* ;ʬ�ˤ����ERR */
			break;
	}
	*memptr = EOLTTR;
	return(fnnum == -1 ? 0 : func[fnnum] . fntype);
}

 /** ʸ������Ρ�\�פ�ȴ�� */
static void
de_bcksla(s, r)
char	*s, *r;
{
	for(; *s; *r++ = *s++) if(*s == '\\') s++;
	*r = '\0';
}

 /**	defvar�����������shiki������ˤ��ᤷ�ơ������ѿ����Ѱ����ꤹ�롣
	�ѿ���κǸ�˥���ɥޡ�������������롣���ͤϡ��Ѱ���Ǽ����
	��ؤΥݥ��󥿡�*/
letter	*rangekettei(num, shiki)
letter	*shiki;
int	num; /* �Ѱ����ꤷ�ĤĤ����ѿ��������ֹ� */
{
	letter	hyoukabuf[TRMSIZ];

	rangeset(num, hen_iki);
	*hen_iki = EOLTTR;

	if(*shiki++ != '(') ERRLIN(8);
	atermscan(&shiki, hyoukabuf, 1);

	if(!ltrstrcmp(hyoukabuf, "all")){
		*hen_iki++ = VARRNG;
		*hen_iki++ = 0;
		*hen_iki++ = LTRHUG; /* �Ѱ����ʸ�� */
		*hen_iki++ = EOLTTR;
		*hen_iki = EOLTTR;
		atermscan(&shiki, dummy, 2); /* ;ʬ�ˤ����ERR */
	} else 
	if(!ltrstrcmp(hyoukabuf, "between")){
		int	i;

		*hen_iki++ = VARRNG;
		while(blankpass(&shiki, 1), *shiki != ')'){
			for(i = 1; i <= 2; i++){
				switch(*shiki){
					case '\'':
						shiki++;
						*hen_iki++ =
						       onescan(&shiki, dummy);
						shiki++;
						break;
					case ')':
					case '"':
					case '(': ERRLIN(8);
					default:
						*hen_iki++ = *shiki++;
				}
				if(i < 2){
					if(!is_eolsp(*shiki)) ERRLIN(8);
					blankpass(&shiki, 1);
				}
			}
		}
		*hen_iki++ = EOLTTR;
		*hen_iki = EOLTTR;
	} else
	if(!ltrstrcmp(hyoukabuf, "list")){
		while(blankpass(&shiki, 1), *shiki != ')'){
			switch(*shiki){
				case '"':
				case '(': ERRLIN(8);
				case '\'':
					shiki++;
					*hen_iki++ = onescan(&shiki, dummy);
					shiki++; /* �����ˡ�'�פ��Ĥ��Ƥ��뤫
					 �ɤ����θ����Ϥ⤦�Ѥ�Ǥ��롣*/
					break;
				default:
					*hen_iki++ = *shiki++;
			}
			if(!is_eolsp(*shiki)) ERRLIN(8);
		}
		*hen_iki++ = EOLTTR;
		*hen_iki = EOLTTR;
	} else {
		ERRLIN(8); /* ����Ϥ���¾�ι�ʸ�����ͽ��Ǥ��ä� */
	}

	return(henorg[num] . range);
}

 /**	num���ܤ��ѿ����Ѱ��ؤ��٤��ݥ��󥿤λؤ������ꤷ�������ѿ���
	����Ͽ���֤ˤ��롣*/
static void
rangeset(num, range)
letter	*range; /* �Ѱ��������Υݥ��� */
int	num;
{
	henorg[num] . range = range;
	henorg[num] . regdflg = 1;
}

 /**	name�ǻ��ꤵ�줿̾���ѿ���õ�����ʤ������Ͽ���ѿ�̾�κǸ��
	����ɥޡ�������������롣
	flg & 01����0�λ�������̾���ѿ������߹Ԥ�̤�Фʤ饨�顼��const�Ȥ���
	����Ͽ�λ�������ˡ��ޤ� flg & 02����0�λ�������̾���ѿ���������ʤ�
	���顼��defvar�ν�ʣ�����å��ѡˡ�flg & 04����0�ʤ�const�Ȥ�����Ͽ��*/
static int
hensrc_tourk(name, flg)
letter	*name;
int	flg;
{
	int	i;

	for(i = 0; henorg[i] . name != NULL; i++){
		if(ltrcmp(henorg[i] . name, name)) continue;
		 /* found */
		if(flg & 04) ERRLIN(28);
		if(flg & 02 && henorg[i] . regdflg != 0) ERRLIN(10);
		if(flg & 01 && henorg[i] . curlinflg == 0 &&
				 henorg[i] . constflg == 0)
			ERRLIN(5);
		henorg[i] . curlinflg = 1;
		return(i);
	}
	if(henorg + i != hentourkptr) BUGreport(5);

	 /* �������褿�Ȥ������Ȥϡ���Ф��ѿ��Ȥ������ȡ�������flg & 01��
	    ��0�ʤ饨�顼��*/
	if(flg & 01) ERRLIN(5);
	hentourkptr -> name = hensumei;
	hentourkptr -> curlinflg = 1;
	hentourkptr -> regdflg = 0; /* ��Ф�����range��def��̤����Ȧ */
	hentourkptr -> constflg = ((flg & 04) != 0);
	(++hentourkptr) -> name = NULL;
	hensumei = ltrgrow(hensumei, name);
	*++hensumei = EOLTTR;

	return(i);
}

 /** �Ȥ߹��ߴؿ�����ǽ̾���Ф��ƤϤ����ֹ�򡢤����Ǥʤ���Τʤ�-1���֤� */
static int
serfun(lp)
register letter *lp; /* �ؿ�����ǽ̾�⤷�����ѿ�̾ */
{
	register int	i;

	for(i = 0; func[i] . fnname != NULL; i++){
		if(! ltrstrcmp(lp, func[i] . fnname)) return(i);
	}
	return(-1);
}

 /** �ѿ���̾���Υ����å� ���������ȥ��顼 */
static void
vchk(lp)
letter	*lp;
{
	if(is_digit(*lp)) ERRLIN(3);
	for(; *lp != EOLTTR; lp++){
	  /*	if(is_lower(*lp)) *lp = to_upper(*lp);	*/
		if(!is_alnum(*lp) && *lp != '_') ERRLIN(3);
	}
}

 /**	����ܤ���Ф������Ф������������1���֤���flg����0�λ��ϡ�')'��
	���Ĥ���ȥ��顼��';'�Ϥ�������ǰ���ܰ�����*/
static int
termsscan(socp, dest, flg)
register letter **socp, *dest;
	 /* socp�λؤ��Ƥ���ݥ��󥿤��ؤ��Ƥ���꤫����Ф���dest������롣
	    ���θ塢socp���ؤ��Ƥ���ݥ��󥿤�ʤ�롣���Υե������ **scan()
	    �Ȥ����ؿ������Ƥ����ʤäƤ롣*/
int	flg;
{
	letter	*bgn;

	bgn = dest;

	if(blankpass(socp, 0) == 0){
		if(flg && **socp == ';'){
			*dest++ = *(*socp)++;
		} else
		while(!is_eolsp(**socp)){
			if(**socp == ')'){
				if(flg) ERRLIN(0);
				break;
			} else {
				partscan(socp, dest);
				totail(dest);
			}
		}
	}

	*dest = EOLTTR;
	return(bgn != dest);
}

 /**	�ꥹ�Ȱ�Ĥ���ñ���ΰ�ޤȤޤ����Ф�������������1���֤���
	flg��1�ΤȤ���')'�����Ĥ���ȥ��顼��
	flg��2�ΤȤ������Ф������������饨�顼��
	flg��3�ΤȤ������Ф��˼��Ԥ����饨�顼��*/
static int
atermscan(socp, dest, flg)
register letter **socp, *dest;
int	flg;
{
	letter	*bgn;
	int	found;

	bgn = dest;

	if(blankpass(socp, 0) == 0){
		if(**socp == '('){
			listscan(socp, dest);
			totail(dest);
		} else {
			while(!is_eolsp(**socp) && **socp != '('){
				if(**socp == ')'){
					if(flg == 1) ERRLIN(0);
					break;
				} else {
					partscan(socp, dest);
					totail(dest);
				}
			}
		}
	}

	*dest = EOLTTR;

	found = (bgn != dest);
	if((!found && flg == 3) || (found && flg == 2)) ERRLIN(7);
	return(found);
}

 /**	���Ĥ���Ф������Ф�����Τ��ꥹ�Ȥʤ����ͤ�0��ñʸ���ʤ�1��
	����ʸ���ʤ�2������ʸ����ʤ�3��*/
static int
partscan(socp, dest)
register letter **socp, *dest;
{
	switch(**socp){
		case '(':
			listscan(socp, dest);
			return(0);
		case '\'':
			singleqscan(socp, dest);
			return(2);
		case '"':
			doubleqscan(socp, dest);
			return(3);
		default:
			*dest++ = *(*socp)++;
			*dest = EOLTTR;
			return(1);
	}
}

 /** ���󥰥륯������ɽ����Ĥ���Ф���*/
static void
singleqscan(socp, dest)
letter	**socp, *dest;
{
	*dest++ = *(*socp)++;
	onescan(socp, dest);
	totail(dest);
	if((*dest++ = *(*socp)++) != '\'') ERRLIN(1);

	*dest = EOLTTR;
}

 /** ���֥륯������ɽ����Ĥ���Ф���*/
static void
doubleqscan(socp, dest)
letter	**socp, *dest;
{
	*dest++ = *(*socp)++;
	while(**socp != '"'){
		if(**socp == EOLTTR) ERRLIN(1);
		onescan(socp, dest);
		totail(dest);
	}
	*dest++ = *(*socp)++;

	*dest = EOLTTR;
}

 /**	8��10��16�ʥ������ѥ���饯����ºݤΥ����ɤ�ľ�������ϤΥ����å���
	��������ʸ���ȿ����ʳ������Ϥ��Ф��Ƥ�ñ��0���֤���*/
int
ltov(l)
letter	l;
{
	if(is_upper(l)) return(l - 'A' + 10);
	if(is_lower(l)) return(l - 'a' + 10);
	if(is_digit(l)) return(l - '0'); else return(0);
}

 /** ltov�ε� */
letter	vtol(l)
letter	l;
{
	if(BASEMX <= l) return('*');
	return(l + (l < 10 ? '0' : 'A' - 10));
}

 /**	���󥰥롦���֥륯�����Ȥ���Ǥΰ�ʸ�����Ф���
	��^�פˤ�륳��ȥ��륳����ɽ������\�פˤ��8��10��16��ɽ���ˤ�
	�б�����\��ɽ���ϡ���\��o����d����x�Ͽ������¤ӡ�;�ϡפǤ��롣*/
letter	onescan(socp, dest)
letter	**socp, *dest;
{
	letter	l, realcode;
	int	digflg;

	switch(realcode = *dest++ = *(*socp)++){
		case '^':
			if(!(' ' <= (l = *(*socp)++) && l < '\177'))ERRLIN(2);
			realcode = ((*dest++ = l) == '?' ? '\177' : l & 0x1f);
			break;
		case '\\':
			digflg = 0;
			switch(**socp){
				case 'n':
					*dest++ = *(*socp)++; realcode = '\n';
					break;
				case 't':
					*dest++ = *(*socp)++; realcode = '\t';
					break;
				case 'b':
					*dest++ = *(*socp)++; realcode = '\b';
					break;
				case 'r':
					*dest++ = *(*socp)++; realcode = '\r';
					break;
				case 'f':
					*dest++ = *(*socp)++; realcode = '\f';
					break;
				case 'e':
				case 'E':
					*dest++ = *(*socp)++; realcode=ESCCHR;
					break;
				case 'o':
					*dest++ = *(*socp)++;
					for(realcode = 0; is_octal(**socp);){
						digflg = 1;
						realcode <<= 3;
						realcode += ltov
						       (*dest++ = *(*socp)++);
					}
					if(!digflg) ERRLIN(2);
					if(**socp== ';') *dest++ = *(*socp)++;
					break;
				case 'x':
					*dest++ = *(*socp)++;
					for(realcode = 0; is_xdigit(**socp);){
						digflg = 1;
						realcode <<= 4;
						realcode += ltov
						       (*dest++ = *(*socp)++);
					}
					if(!digflg) ERRLIN(2);
					if(**socp== ';') *dest++ = *(*socp)++;
					break;
				case 'd':
					*dest++ = *(*socp)++;
					for(realcode = 0; is_digit(**socp);){
						digflg = 1;
						realcode *= 10;
						realcode += ltov
						       (*dest++ = *(*socp)++);
					}
					if(!digflg) ERRLIN(2);
					if(**socp== ';') *dest++ = *(*socp)++;
					break;
				default:
					if(is_octal(**socp)){
						for(realcode = 0;
						    is_octal(**socp);){
							realcode <<= 3;
							realcode += ltov(
							*dest++ = *(*socp)++);
						}
						if(**socp == ';')
							*dest++ = *(*socp)++;
					} else {
						realcode= *dest++= *(*socp)++;
					}
			}
			break;
		default: ;
	}

	*dest = EOLTTR;
	return(realcode);
}
	
 /**	letter�������Ƭ�ˤ������򥹥��åפ��롣
	������ã�����顢flg��0�ΤȤ���1���֤��������Ǥʤ��ȥ��顼��*/
static int
blankpass(pptr, flg)
register letter **pptr;
	 /* letter����Υݥ��󥿤ؤΥݥ��󥿡����줬�ؤ��Ƥ����Τ�ʤ�� */
int	flg;
{
	while(is_eolsp(**pptr)){
		if(EOLTTR == **pptr){
			if(flg) ERRLIN(4);
			return(1);
		}
		(*pptr)++;
	}
	return(0);
}

 /** �ꥹ�Ȱ�ļ��Ф� */
static void
listscan(socp, dest)
register letter **socp, *dest;
{
	int	eofreach;

	*dest++ = *(*socp)++; /* = '(' */
	*dest++ = ' ';
	
	while(eofreach = blankpass(socp, 0), **socp != ')'){
		if(eofreach){
			if(! readln((uns_chr *)hcurread)) ERRLIN(20);
			ustrtoltr((uns_chr *)hcurread, (*socp= ltrbufbgn), 1);
			 /* list����ǹԤ��ڤ�Ƥ����硢����ɲ��ɤ߹��ߤ�
			    ���롣uns_chr�ѤΥХåե��⡢letter�ѤΤ�Τ⡢
			    ������ʪ����Ƭ��������Ѥ��Ƥ���Τ���ա��ޤ���
			    ���顼����������硢 ���顼�ΰ��֤ˤ�����餺��
			    ���߹ԤȤ���ɽ�������Τϡ��Ǹ���ɤޤ줿�ԤΤ�*/
		} else {
			termsscan(socp, dest, 0);
			totail(dest);
			*dest++ = ' ';
		}
	}
	*dest++ = *(*socp)++; /* = ')' */
	*dest = EOLTTR;
}

 /** lp�ǻ��ꤵ�줿�⡼��̾��õ�������Ĥ���ʤ��ȥ��顼 */
static int
modsrcL(lp)
letter	*lp;
{
	int	n;

	for(n = 0; modmeibgn[n] != NULL; n++)
		if(!ltrstrcmp(lp, modmeibgn[n])) return(n);

	ERRLIN(16);
	 /*NOTREACHED*/
	return(-1);
}

 /** chk_get_int��rk_modread.c�ˤ�letter�� */
static int
chkL_get_int(lp, ip, range)
letter	*lp;
modetyp	*ip;
modetyp range;
{
	int	sgn = 1;
	modetyp	out;

	if(*lp == '-'){
		lp++;
		sgn = -1;
	}
	for(out = 0; *lp != EOLTTR; lp++){
		if(!is_digit(*lp)) return(-1);
#if defined(UX386) || defined(sun386)
		out = out * 10;
#else
		out *= 10;
#endif
		out += ltov(*lp);
	}
	if(range != 0) out %= range;
	if(sgn == -1 && out != 0) out = range - out;
	*ip = out;
	return(0);
}

 /**	������ϡ�ɽ�ɤ߹��ߤǥ���ä����˷ٹ𤹤�롼����n�ϥ��顼�����ɡ�
	�ޤ����������ơ�romkan_init�Υ��顼�����ɤȤ��ơ�longjmp��ͳ��
	1���֤���*/

void
ERMOPN(n) /* �⡼�����ɽ�������ץ�Ǥ��ʤ� */
unsigned int	n;
{
	static	char	*ermes[] = {
 /*  0 */	"Can't open Mode-hyo",
		"Unprintable error"
	};

	if(n >= numberof(ermes)) n = 1;

	fprintf(stderr, "\r\nMode-hyo %s ---\r\n", curfnm);
	fprintf(stderr, "%d: %s.\r\n", n, ermes[n]);
	longjmp(env0, 1);
}

static void
ERHOPN(n) /* �Ѵ��б�ɽ�������ץ�Ǥ��ʤ� */
unsigned int	n;
{
	static	char	*ermes[] = {
 /*  0 */	"Can't open Taio-hyo",
		"Is a directory",
		"Unprintable error"
	};

	if(n >= numberof(ermes)) n = 2;

	fprintf(stderr, "\r\nTaio-hyo %s%s ---\r\n", curdir, curfnm);
	fprintf(stderr, "%d: %s.\r\n", n, ermes[n]);
	longjmp(env0, 1);
}

void
ERRMOD(n) /* �⡼�����ɽ�Υ��顼 */
unsigned int	n;
{
	static	char	*ermes[] = {
 /*  0 */	"Table incomplete",
		"')' mismatch",
		"Unprintable error",
		"Illegal filename",
		"Illegal modename",
 /*  5 */	"Undefined mode",
		"Illegal content(s) of list",
		"Illegal ^,\\o,\\x or \\d expression",
		"Illegal defmode",
		"Unrecognized keyword",
 /* 10 */	"Incomplete string",
		"Search path specified after filename",
		"Argument must be a string",
		"Can't get home directory",
		"Illegal @-keyword",
 /* 15 */	"User doesn't exist",
		"Illegal character",
		"Defmode or set-path placed wrong"
	};

	if(n >= numberof(ermes)) n = 2;

	fprintf(stderr, "\r\nMode-hyo %s%s ---\r\n%s\r\n",
		curdir, curfnm, mcurread);
	fprintf(stderr, "%d: %s.\r\n", n, ermes[n]);
	fclose(modefile);
	longjmp(env0, 1);
}

static void
ERRLIN(n) /* �Ѵ��б�ɽ�Υ��顼 */
unsigned int	n;
{
	static	char	*ermes[] = {
 /*  0 */	"')' mismatch",
		"Incomplete single-quote or double-quote expression",
		"Illegal ^,\\o,\\x or \\d expression",
		"Illegal variable name",
		"Incomplete line",
 /*  5 */	"Evaluation of unbound variable",
		"Unprintable error",
		"Too many or too few contents of list",
		"Illegal defvar/defconst",
		"Faculity or declaration joined other item(s)",
 /* 10 */	"Duplicate defvar/defconst",
		"Variable/constant name conflicts with Function name",
		"A line contains both declaration and other output item(s)",
		"Argument isn't a letter",
		"Function, faculity or declaration in illegal place",
 /* 15 */	"More than 3 items",
		"Undefined mode",
		"Against the restriction of pre-transform table",
		"Against the restriction of after-transform table",
		"Item comes after faculity",
 /* 20 */	"Incomplete list",
		"Illegal character",
		"Illegal include",
		"Too many levels of 'include' nest",
		"Can't open include file",
 /* 25 */	"Can't get home directory",
		"Illegal @-keyword",
		"User doesn't exist",
		"Constant must be defined before used",
		"Illegal mode status"
	};

	if(n >= numberof(ermes)) n = 6;

	fprintf(stderr, "\r\nTaio-hyo %s%s ---\r\n%s\r\n",
		curdir, curfnm, hcurread);
	fprintf(stderr, "%d: %s.\r\n", n, ermes[n]);

	while(NULL != *base) fclose(*base--);

	longjmp(env0, 1);
}

static void
ERRHYO(n) /* ERRLIN��Ʊ�͡��б�ɽ�Υ��顼��������Ԥ����θ��Ǥʤ�
 ���Τ򸫤ʤ��Ȥ狼��ʤ���ꡣ���νꡢ��̤����ѿ��νи��פΤߡ�*/
unsigned int	n;
{
	static	char	*ermes[] = {
 /*  0 */	"Undefined variable was found",
		"Unprintable error"
	};

	if(n >= numberof(ermes)) n = 1;

	fprintf(stderr, "\r\nTaio-hyo %s%s ---\r\n", curdir, curfnm);
	fprintf(stderr, "%d: %s.\r\n", n, ermes[n]);

	while(NULL != *base) fclose(*base--);

	longjmp(env0, 1);
}
