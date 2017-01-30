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
/**********************************************************************
			rk_modread.c
						88. 6.16  �� ��

	�⡼�����ɽ���ɤ߹��ߤ�ô������ץ���ࡣ
***********************************************************************/
/*	Version 3.0
 */
#include "rk_header.h"
#include "rk_extvars.h"
#ifdef WNNDEFAULT
#  include "config.h"
 /* �ޥ���LIBDIR������ʤΤ�������ˡ�����ѥ�����ϡ��إå��ե������
    �������ѥ��ˡ�Wnn�Υ��󥯥롼�ɥե�����Τ��꤫�����ꤷ�Ƥ������ȡ�*/
#endif
#include <pwd.h>

#define Terminator 0 /* int�����naibu[]�ˤν��ߥ����� */

char	*getenv();
#if defined(uniosu)
extern	struct	passwd	*getpwnam();
#endif
extern	char	*chrcat(), *strend(), *ename();
extern	void	romkan_clear();
char	*modhyopath;

static void cond_evl(), mystrcpy(),
  rd_bcksla(), rd_ctrl(), hyouse(), look_choose();
void choosehyo();
static int mystrcmp(), read1tm(), mod_evl(), fnmsrc_tourk(), dspnamsrc_tourk(),
  scan1tm(), modsrc_tourk(), chk_get_int(), pathsrc_tourk(),
  modnamchk(), ctov(), look_cond(), evlcond(), chkchar_getc(); 
static char codeeval();
extern void ERRMOD(), ERMOPN(), BUGreport();
extern int filnamchk();

struct	kwdpair {
	 /* ������ɤȤ�������ɽ�����б���Ϳ���빽¤�Ρ�����ɽ����
	    �����ʤ���Τ��Ф��Ƥ�0��Ϳ�����Ƥ��롣*/
	char	*name;
	int	code;
} modfn[] = {
	"defmode",	0,
	"if",		XY2INT(2, 0),
	"when",		XY2INT(2, 1),
	"path",		0,
	"search",	0,
	"on_dispmode",	XY2INT(5, 0),
	"off_dispmode", XY2INT(5, 1),
	"on_unchg",	XY2INT(6, 0),
	"off_unchg",	XY2INT(6, 1),
	NULL
}; /* ���򸫤� ����������ɽ�Ϥޤ�������Ĥ���Τ� */

struct	kwdpair modcond[] = {
	"not",	XY2INT(3, 0),
	"and",	XY2INT(3, 1),
	"or",	XY2INT(3, 2),
	"true", XY2INT(3, 3),
	"false",XY2INT(3, 4),
	"=",	XY2INT(3, 5),
	"!=",	XY2INT(3, 6),
	"<",	XY2INT(3, 7),
	">",	XY2INT(3, 8),
	NULL
};
int	condarg[] = {1, 2, 2, 0, 0, 2, 2, 2, 2}; /* ���Ƚ�Ǵؿ��ΰ����Ŀ� */

struct	kwdpair swstat[] = {
	"on",	0,
	"off",	0,
	NULL
};
 /* 1��2^24��ϥ⡼��̾��4��2^24��ϥ⡼��ɽ��ʸ����7��2^24��ϥ⡼�ɾ�����
    ��������ɽ���˻ȤäƤ��� */

 /** ������ɡ�if, and�ʤɡˤ������ʤ�Τ������å����������ֹ���֤� */
static int
kwdsrc(hyo, wd)
struct	kwdpair *hyo;	/* �ɤΥ������ɽ��Ȥ��� */
char	*wd;		/* �����å�����륭����� */
{
	int	i;

	for(i = 0; hyo[i] . name != NULL; i++)
		if(!mystrcmp(hyo[i] . name, wd)) return(i);
	ERRMOD(9);
	 /*NOTREACHED*/
}

 /** �⡼��ɽ���ɤ߹��� */
void
readmode(modfname)
char	*modfname; /* �⡼��ɽ��̾ */
{
	char	buf[MDHMAX], *bufp;
#ifdef RKMODPATH
	char	*genv, *pathenv, *pathp;
#endif

	mcurread = buf; /* ���顼������ */

#ifdef RKMODPATH
	if(!fixednamep(modfname) &&
	   NULL != (pathenv = genv = getenv(RKMODPATH)) && *genv != '\0'){
	 /* PATH�˾��ʤ��Ȥ��ĤΥ������ѥ��������� */
		for(;;){
			 /* �������ѥ��γơ���������ΰ�ˤϡ�pathmeimem�����
			    ���Ƥ��롣��ǡ��������ѥ�����Ƭ�ˡ��⡼��ɽ�Τ���
			    �ǥ��쥯�ȥ�����ꤹ��Τǡ������ص��Τ���⤢��*/
			for(pathp = pathmeimem; *genv != ':' && *genv; genv++)
				*pathp++ = *genv;
			*pathp = '\0';

			if(*(strend(pathmeimem)) != KUGIRI) *pathp++ = KUGIRI;
			 /* path�ζ��ڤ��DG��MV�ˤǤ��äƤ�'/' */

			strcpy(pathp, modfname);
			if(NULL != (modefile = fopen(pathmeimem, "r"))){
				 /* Now Mode-hyo found */
				if(flags & RK_VERBOS)
					fprintf(stderr,
					"romkan: using Mode-hyo %s ...\r\n",
						pathmeimem);
				curdir = pathmeimem; /* ���λ����Ǥϥե�����̾
					���ߤ��������Ȥǥѥ�̾�����ˤʤ� */
				curfnm = ename(modfname);
				break;
			}

			if(*genv != ':'){ /* Mode-hyo not found */
				if(flags & RK_VERBOS){
					fprintf(stderr, "no %s in ",modfname);
					for(genv = pathenv; *genv; genv++){
						fputc((*genv == ':' ?
						      ' ' : *genv), stderr);
					}
					fprintf(stderr, ".\n");
				}
				ERMOPN(0);
			} else genv++; /* coutinues searching Mode-hyo */
		}
	} else
#endif
	{
		if(NULL == (modefile = fopen(modfname, "r"))) ERMOPN(0);
		if(flags & RK_VERBOS)
			fprintf(stderr, "romkan: using Mode-hyo %s ...\r\n",
				modfname);
		strcpy(pathmeimem, modfname);
	}

	 /* �������ѥ�����Ƭ�ˡ��⡼��ɽ�Τ���ǥ��쥯�ȥ�����ꤷ�Ƥ��롣*/
	*(ename(pathmeimem)) = '\0';
	modhyopath = *pathmeiptr++ = pathmeimem;
	*pathmeiptr = NULL;
	strtail(pathmeimem);
	*(pathareaorg = ++pathmeimem) = '\0';
	 /* pathareaorg�ϡ�pathmeimem_[]�Τ����⡼��ɽ�Τ���ǥ��쥯�ȥ�̾��
	    ��Ǽ�����Ĥ����ʬ����Ƭ��ؤ���*/

	while(bufp = buf, read1tm(&bufp, 0)) mod_evl(buf);
	fclose(modefile);
}

 /**	���ꤵ�줿�ʥ������ѥ��򸫤�ɬ�פΤʤ��˥ե�����̾���Ф��Ƥ���0��
	�֤������ߤΤȤ��� / ./ ../ �Τɤ줫�ǻϤޤ��ΤȤ��Ƥ��뤬�ʺ�Ԥ�
	���ǡˡ�Ŭ�����Ѥ��Ƥ褤��strchr(s,'/')!=NULL �Ȥ��������ɤ����� */
int
fixednamep(s)
char	*s;
{
	return(!strncmp("/",s,1)|| !strncmp("./",s,2)|| !strncmp("../",s,3));
}

 /**	�⡼��ɽ�ΰ줫���ޤ�ʥꥹ�ȡ��ե�����̾���⡼��ɽ��ʸ����ˤ�
	��᤹�롣�֤��ͤϡ�defmode,search�ڤ�path�λ�0������ʳ��ʤ�1��*/
static int
mod_evl(s)
char	*s; /* �⡼��ɽ������ɽ������ؤΥݥ��� */
{
	char	md1[MDT1LN], *bgn, *end;
	int	num, retval = 1;

	if(*s != '('){
		if(*s != '"'){
			num = fnmsrc_tourk(s);
			*naibu++ = XY2INT(4, num);
		} else {
			s++;
			if(*(end = strend(s)) != '"') ERRMOD(10);
			*end = '\0';
			num = dspnamsrc_tourk(s);
			*naibu++ = XY2INT(5, 0);
			*naibu++ = num;
		}
	} else {
		s++;
		scan1tm(&s, md1, 1);
		switch(num = kwdsrc(modfn, md1)){
			case 0: /* defmode */
				retval = 0;
				scan1tm(&s, md1, 1); /* modename */
				num = modsrc_tourk(md1, 0);
				if(scan1tm(&s, md1, 0) == 0){
				 /* ���on-off�ˤĤ��Ʋ���񤤤Ƥʤ�����
				    default��off */
					modesw[num] . moderng = 2;
					modesw[num] . curmode = 0;
					break;
				}

				if(*md1 == '('){
					char	tmp[MDT1LN], *s;
					unsigned int	i, j;

					s = md1 + 1;

					scan1tm(&s, tmp, 1);
					if(chk_get_int(tmp, &i, 0) != 0)
						ERRMOD(8);
					modesw[num] . moderng = i;
					scan1tm(&s, tmp, 1);
					if(chk_get_int(tmp, &j,
						  modesw[num] . moderng) != 0)
						ERRMOD(8);
					modesw[num] . curmode = j;
					if(
#ifdef ModeNotInt
					   modesw[num] . moderng != i ||
					   modesw[num] . curmode != j ||
#endif
					   i == 1 || (i != 0 && j >= i)){
					   	ERRMOD(8);
					}
					scan1tm(&s, tmp, 2);
				} else {
					switch(kwdsrc(swstat, md1)){
						case 0: modesw[num] . curmode
								= 1; break;
						case 1: modesw[num] . curmode
								= 0; break;
					}
					modesw[num] . moderng = 2;
				}
				scan1tm(&s, md1, 2); /* �����err */
				break;
			case 1: /* if */
			case 2: /* when */
				*naibu++ = modfn[num] . code;
				scan1tm(&s, md1, 1); /* condition */
				cond_evl(md1);
				while(scan1tm(&s, md1, 0)){
					if(mod_evl(md1) == 0) ERRMOD(17);
				}
				*naibu++ = Terminator;
				break;
			case 3: /* path */
				*(pathmeimem = pathareaorg) = '\0';
				*(pathmeiptr = pathmeiorg) = NULL;
			case 4: /* search */
				retval = 0;
				if(hyomeiptr != hyomeiorg) ERRMOD(11);
				 /* �������ѥ��λ���ϥե�����̾�νи����
				    ��Ԥ��ʤ���Фʤ�ʤ��Ȥ��Ƥ�����*/

				while(scan1tm(&s, md1, 0)){/* find pathname */
					pathsrc_tourk(md1);
				}
				break;
			case 5: /* on_dispmode */
			case 6: /* off_dispmode */
				*naibu++ = modfn[num] . code;
				scan1tm(&s, md1, 1); /* dispmode string */

				if(*(bgn = md1) != '"') ERRMOD(12);
				bgn++;
				if(*(end = strend(bgn)) != '"') ERRMOD(10);
				*end = '\0';
				*naibu++ = dspnamsrc_tourk(bgn);
				scan1tm(&s, md1, 2); /* �����err */
				break;
			case 7: /* on_unchg */
			case 8: /* off_unchg */
				*naibu++ = modfn[num] . code;
				scan1tm(&s, md1, 2); /* �����err */
				break;
		}

	}
	*naibu = Terminator;
	return(retval);
}

 /** ��Ｐ�ʥ⡼��̾ ����not,and�ʤɤμ��˰�Ĥ��� */
static void
cond_evl(cod)
char	*cod; /* ��Ｐ������ɽ������ؤΥݥ��� */
{
	char	md1[MDT1LN];
	unsigned int	num;
	int	i; 

	if(is_digit(*cod) || *cod == '-'){
		*naibu++ = XY2INT(7, 0);
		if(0 != chk_get_int(cod, &num, 0)) ERRMOD(4);
		*naibu++ = num;
	} else if(*cod != '('){
		num = modsrc_tourk(cod, 1);
		*naibu++ = XY2INT(1, num);
	} else {
		cod++;
		scan1tm(&cod, md1, 1); /* not;and;or */
		num = kwdsrc(modcond, md1);
		*naibu++ = XY2INT(3, num);
		for(i = condarg[num]; i; i--){
			scan1tm(&cod, md1, 0);
			cond_evl(md1);
		}
		scan1tm(&cod, md1, 2);
	}
	*naibu = Terminator;
}

 /**	s�ǻ��ꤵ�줿�ե�����̾������Ͽ��õ�����ʤ������Ͽ��â��������Ͽ��
	�ɤ����Υ����å��ϸ�̩�ǤϤʤ������㤨�С�Ʊ���ե�����Ǥ⡢
	�ѥ�̾�դ���̵���ȤǤϡ�Ʊ���ȸ��ʤ��ˡ��ե�����̾������Ͽ���ɤ���
	�����å�����Τϡ���������Τ����Ʊ��ɽ���ɤ߹���Τ��ɤ�����
	�����ʤΤǡ�����ʳ��ˤ��̤˺������Ϥʤ���*/
static int
fnmsrc_tourk(s)
char	*s;
{
	int	n;

	for(n = 0; hyomeiorg[n] != NULL; n++)
		if(!mystrcmp(hyomeiorg[n], s)) return(n);

	if(hyomeiorg + n != hyomeiptr) BUGreport(101);

	*hyomeiptr++ = hyomeimem;
	*hyomeiptr = NULL;
	mystrcpy(hyomeimem, s);
	if(!(hyoshu[n] = filnamchk(hyomeimem))) ERRMOD(3);
	strtail(hyomeimem);
	*++hyomeimem = '\0';
	return(n);
}

 /**	s�ǻ��ꤵ�줿�������ѥ�̾������Ͽ��õ�����ʤ������Ͽ��â����fnmsrc_
	tourk()Ʊ�͡�����Ͽ���ɤ����Υ����å��ϸ�̩�ǤϤʤ�������ʤ���*/
static int
pathsrc_tourk(s)
char	*s;
{
	int	n;
	char	fnm_addsla[MDT1LN];

	mystrcpy(fnm_addsla, s);
	if( !(*fnm_addsla == '\0' || *(strend(fnm_addsla)) == KUGIRI))
		chrcat(fnm_addsla, KUGIRI);
	 /* �ѥ�̾��'/'�ǽ���äƤʤ���С�������ղä��롣*/

	for(n = 0; pathmeiorg[n] != NULL; n++)
		if(!strcmp(pathmeiorg[n], fnm_addsla)) return(n);

	if(pathmeiorg + n != pathmeiptr) BUGreport(104);

	*pathmeiptr++ = pathmeimem;
	*pathmeiptr = NULL;
	strcpy(pathmeimem, fnm_addsla);

	strtail(pathmeimem);

	*++pathmeimem = '\0';
	return(n);
}

 /** s�ǻ��ꤵ�줿�⡼��ɽ��ʸ���󤬴���Ͽ��õ�����ʤ������Ͽ */
static int
dspnamsrc_tourk(s)
char	*s;
{
	int	n;

	for(n = 0; dspnambgn[n] != NULL; n++)
		if(!mystrcmp(dspnambgn[n], s)) return(n);

	if(dspnambgn + n != dspnamptr) BUGreport(103);

	*dspnamptr++ = dspcod;
	*dspnamptr = NULL;
	mystrcpy(dspcod, s);
	strtail(dspcod);
	*++dspcod = '\0';
	return(n);
}

 /**	��Ͽ����Ƥ���⡼��̾���椫�顢s�ǻ��ꤵ�줿�⡼��̾��õ����*np ��
	�⡼���ֹ椬���롣���Ĥ���ʤ��ȸ��⡼��̾����������롣���ξ��
	���ͤ�0��*/
static int
modnam_src(s, np)
char	*s;
int	*np;
{
	for(*np = 0; modmeibgn[*np] != NULL; (*np)++ )
		if(!mystrcmp(modmeibgn[*np], s)) return(1);
	return(0);
}

 /**	s�ǻ��ꤵ�줿�⡼��̾��õ�����ʤ������Ͽ��â����flg����0�ʤ顢
	���Ĥ���ʤ���Х��顼 */
static int
modsrc_tourk(s, flg)
char	*s;
int	flg;
{
	int	n;

	if(modnam_src(s, &n)) return(n);

	if(flg) ERRMOD(5);

	if(modmeibgn + n != modmeiptr) BUGreport(102);

	*modmeiptr++ = modmeimem;
	*modmeiptr = NULL;
	mystrcpy(modmeimem, s);
	if(!modnamchk(modmeimem)) ERRMOD(4);
	strtail(modmeimem);
	*++modmeimem = '\0';
	return(n);
}

 /** �ե����뤫���ʸ���ɤ�ʶ���ʸ�������Ф��ˡ��ɤ��ʸ����EOF�ʤ�0���֤� */
static char
fspcpass()
{
	register int	c;

	while(EOF != (c = chkchar_getc(modefile)) && is_nulsp(c));
	return(c == EOF ? '\0' : c);
}

 /**	�⡼��ɽ�ˤ϶���ʸ���ʳ��Υ���ȥ���ʸ�������ǤϺ�����ʤ���Τ�
	���롣�����äƤ������ϥ����å����Ĥġ�getc��Ԥ���*/
static int
chkchar_getc(f)
FILE	*f;
{
	register int	c;

	c = getc(f);
	if(is_cntrl(c) && !isspace(c)){
		sprintf(mcurread, "\\%03o", c);
		ERRMOD(16);
	}
	return(c);
}	

static int
modehyo_getc()
{
	return(chkchar_getc(modefile));
}

static int
modehyo_ungetc(c)
register int	c;
{
	return(ungetc(c, modefile));
}

 /**	soc��̾�Υ桼���Υ����󡦥ǥ��쥯�ȥ�̾��dest�����졢*dest�ˤ���
	������ؤ����롣â��soc������ʤ鼫ʬ�Υ����󡦥ǥ��쥯�ȥ�̾��
	NULL�ʤ鼫ʬ�Υۡ��ࡦ�ǥ��쥯�ȥ�̾��������ξ��⡢����������
	���⤷�ʤ������ͤϡ���������-1��getenv("HOME")���Ի�������-2�ˡ�*/
static int
get_hmdir(dest, soc)
char	**dest, *soc;
{
	struct	passwd	*usr;
	char	*p;

	if(soc == NULL){
		if(NULL == (p = getenv("HOME"))) return(-2);
	} else {
		if(NULL == (usr = (*soc? getpwnam(soc) : getpwuid(getuid()))))
			return(-1);
		p = usr -> pw_dir;
	}
	strcpy(*dest, p);
	strtail(*dest);
	return(0);
}

 /**	�⡼��ɽ���б�ɽ��Ρ��ե�����̾����ʬ���ɤ߹��ߡ���Ƭ�� @ ���� ~ ��
	���ϡ��ü������Ԥ��������ϡ�����ɤ߹��ߡ�����ᤷ��ʸ������Ф���
	�ؿ��ȡ���̤�����륨�ꥢ�����ϤؤΥݥ��󥿡������ɤޤ��ʸ���������
	�ݥ��󥿡����ͤϡ����ｪλ��0��@HOME�ǥۡ��ࡦ�ǥ��쥯�ȥ꤬���ʤ���
	1��@�Τ��Ȥ��Ѥʤ�Τ��褿��2��~�Ǽ�ʬ�Υۡ��ࡦ�ǥ��쥯�ȥ꤬���ʤ�
	��3��~�Τ��Ȥ�¸�ߤ��ʤ��桼��̾���褿��4��*/
int
readfnm(readchar_func, unreadc_func, readstr_func, areap, lastcptr)
register int	(*readchar_func)(), (*unreadc_func)(), (*readstr_func)();
char	**areap;
int	*lastcptr;
{
	char	*head;
	register int	c;

	c = (*readchar_func)();
	if(c == '@'){ /* @HOME, @MODEDIR, @LIBDIR */
		*(*areap)++ = c;
		head = *areap;
		(*readstr_func)(areap, 1);

		if(mystrcmp("HOME", head) == 0){
			*areap = --head;
			if(get_hmdir(areap, (char *)NULL) != 0){
				*areap = head;
				return(1);
			}
		} else
		if(mystrcmp("MODEDIR", head) == 0){
			strcpy(*areap = --head, modhyopath);
			if(KUGIRI== *(*areap= strend(*areap))) **areap = '\0';
		} else
#ifdef WNNDEFAULT
		if(mystrcmp("LIBDIR", head) == 0){
			strcpy(*areap = --head, LIBDIR);
			strtail(*areap);
		} else
#endif
		{
			*areap = --head;
			return(2);
		}

	} else
	if(c == '~'){ /* ~user */
		int	err;
			
		*(*areap)++ = c;
		head = *areap;
		(*readstr_func)(areap, 1);

		mystrcpy(head, head);
		*areap = head - 1;
		if((err = get_hmdir(areap, (*head ? head : NULL)))!= 0){
			*areap = --head;
			return(err == -2 ? 3 : 4);
		}

	} else {
		(*unreadc_func)(c);
	}

	*lastcptr = (*readstr_func)(areap, 0);
	return(0);
}

 /**	�⡼��ɽ�����ʸ��ʬ���Ф���Ȥ򡢶��򡦳�̤Τɤ줫
	����EOF�����ޤ�³���롣flg & 01����0�ʤ顢'/'����Ƥ�
	����롣���ͤϡ������ɤޤ��ʸ����*/
static int
rd_string(readfile, sptr, flg)
register FILE	*readfile;
char	**sptr;
int	flg;
{
	int	c;

	while(EOF != (c = chkchar_getc(readfile)) &&
	      !(is_nulsp(c) || c == '(' || c == ')') &&
	      !(flg & 01 && c == KUGIRI)){
		switch(c){
			case '\\': rd_bcksla(readfile, sptr); break;
			case '^': rd_ctrl(readfile, sptr); break;
			default: *(*sptr)++ = c;
		}
	}
	**sptr = '\0';
	return(ungetc(c, readfile));
}

static int
rd_str_from_modefile(sptr, flg)
char	**sptr;
int	flg;
{
	return(rd_string(modefile, sptr, flg));
}


 /**	�⡼��ɽ����Хå�����å�������ΰ�ʸ��ʬ����Ф���'\��8�ʡ�;'
	�η���ľ����â������Ƭ��'\\'�ϴ����ɤޤ줿���ȡ�*/
static void
rd_bcksla(readfile, sptr)
register FILE	*readfile;
char	**sptr;
{
	int	c, code = 0, digflg = 0;

	switch(c = chkchar_getc(readfile)){
		case 'n':
			code = '\n'; digflg = 1; break;
		case 't':
			code = '\t'; digflg = 1; break;
		case 'b':
			code = '\b'; digflg = 1; break;
		case 'r':
			code = '\r'; digflg = 1; break;
		case 'f':
			code = '\f'; digflg = 1; break;
		case 'e':
		case 'E':
			code = ESCCHR; digflg = 1; break;
		case 'o':
			while(c = chkchar_getc(readfile), is_octal(c)){
				code <<= 3;
				code += ctov(c);
				digflg = 1;
			}
			if(c != ';') ungetc(c, readfile);
			break;
		case 'd':
			while(c = chkchar_getc(readfile), is_digit(c)){
				code *= 10;
				code += ctov(c);
				digflg = 1;
			}
			if(c != ';') ungetc(c, readfile);
			break;
		case 'x':
			while(c = chkchar_getc(readfile), is_xdigit(c)){
				code <<= 4;
				code += ctov(c);
				digflg = 1;
			}
			if(c != ';') ungetc(c, readfile);
			break;
		default:
			if(is_octal(c)){
				digflg = 1;
				code = ctov(c);
				while(c= chkchar_getc(readfile), is_octal(c)){
					code <<= 3;
					code += ctov(c);
				}
				if(c != ';') ungetc(c, readfile);
			} else {
				code = c;
				digflg = 1;
			}
	}

	if(digflg == 0) ERRMOD(7);
	sprintf(*sptr, "\\%o;", code);
	strtail(*sptr);
}

 /**	�⡼��ɽ���饳��ȥ��륳���ɷ����ΰ�ʸ��ʬ����Ф���
	'\��8�ʡ�;' �η���ľ����â������Ƭ��'^'�ϴ����ɤޤ줿���ȡ�*/
static void
rd_ctrl(readfile, sptr)
register FILE	*readfile;
char	**sptr;
{
	int	c;

	if(!(' ' <= (c = chkchar_getc(readfile)) && c < '\177')) ERRMOD(7);
	if(c == '?') c = '\177'; else c &= 0x1f;

	sprintf(*sptr, "\\%o;", c);
	strtail(*sptr);
}

 /**	�⡼��ɽ�ΰ줫���ޤ�ʥꥹ�ȡ��ե�����̾���⡼��ɽ��ʸ����ˤ�
	�ڤ�Ф������κݡ��ü��ɽ����'^','\'�ˤ��ˤϡ�'\��8�ʡ�;' ��
	����ľ����flg����0�ʤ顢EOF�ǥ��顼�򵯤�����')'��0���֤���*/
static int
read1tm(sptr, flg)
char	**sptr; /* �⡼��ɽ������ɽ������ؤΥݥ��󥿤ؤΥݥ��󥿡�
		   rd_bcksla()��rd_ctrl()��codeeval()�Ǥ�Ʊ�� */
int	flg;
{
	int	c, err, retval = 1;
	char	*s;

	s = *sptr;

	while((c = fspcpass()) == ';'){
	  /* ���ʸ�򸡽Ф����顢�����ޤǤȤФ��ƺƻ�ԡ�*/
		while((c = chkchar_getc(modefile)) != '\n' && c != EOF);
	}

	switch(c){
		case '\0': /* EOF��ɽ�� */
			if(flg) ERRMOD(0);
			 else retval = 0;
			break;
		case ')':
			if(flg) retval = 0;
			 else ERRMOD(1);
			break;
		case '(':
			*s++ = c;
			*s++ = ' ';
			while(read1tm(&s, 1)) *s++ = ' ';
			*s++ = ')';
			break;
		case '"':
			*s++ = c;
			while((c = chkchar_getc(modefile)) != '"'){
				switch(c){
					case EOF : ERRMOD(0);
					case '\\': rd_bcksla(modefile, &s);
						   break;
					case '^' : rd_ctrl(modefile, &s);
						   break;
					default	 : *s++ = c;
				}
			}
			*s++ = '"';
			break;
		default:
			ungetc(c, modefile);
		 /* ��Ƭ�� @ ���� ~ �λ��ϡ��ü������*/
			err = readfnm(modehyo_getc, modehyo_ungetc,
				      rd_str_from_modefile, &s, &c);
			if(err){
				mcurread = s;
				switch(err){
					case 1:
					case 3: ERRMOD(13);
					case 2: ERRMOD(14);
					case 4: ERRMOD(15);
				}
			}

			if(c == EOF && flg) ERRMOD(0);
			if(c == ')' && !flg) ERRMOD(1);
	}

	*s = '\0';
	*sptr = s;
	return(retval);
}

 /**	8��10��16�ʥ������ѤΥ���饯����ºݤΥ����ɤ�ľ�������ϤΥ����å���
	���ʤ���*/
static int
ctov(c)
char	c;
{
	if(is_upper(c)) return(c - 'A' + 10);
	if(is_lower(c)) return(c - 'a' + 10);
	return(c - '0');
}

 /**	�ꥹ�Ȥ���Ȥ�scan�����ѡ�')'��0���֤���EOL����ʤ��Ϥ���
	flg == 1 �ΤȤ������Ф��˼��Ԥ����饨�顼��
	flg == 2 �ΤȤ������Ф������������饨�顼��
	�ü�ʥ�����ɽ���ϴ������� '\��8�ʡ�;' �η���ľ�äƤ���Ȧ��*/
static int
scan1tm(socp, dest, flg)
char	**socp, *dest;
	 /* socp�λؤ��Ƥ���ݥ��󥿤��ؤ��Ƥ���꤫����Ф���dest������롣
	    ���θ塢socp���ؤ��Ƥ���ݥ��󥿤�ʤ�롣*/
int	flg;
{
	char	c;
	int	retval = 1;

	while(c = *(*socp)++, is_nulsp(c)) if(c == '\0') ERRMOD(6);
	switch(c){
		case ')':
			retval = 0;
			break;
		case '(':
			*dest++ = c;
			*dest++ = ' ';
			while(scan1tm(socp, dest, 0)){
				strtail(dest);
				*dest++ = ' ';
			}
			*dest++ = ')';
			break;
		case '"':
			*dest++ = c;
			while((c = *dest++ = *(*socp)++) != '"'){
				if(c == '\\'){ /* '\��8�ʡ�;'�β�� */
					while(c = *dest++ = *(*socp)++,
					      is_octal(c));
				}
			}
			break;
		default:
			*dest++ = c;
			while(!is_nulsp(**socp)) *dest++ = *(*socp)++;
	}

	*dest = '\0';
	if((flg == 1 && retval == 0) || (flg == 2 && retval == 1)) ERRMOD(6);
	return(retval);
}

 /** �⡼��̾�Ȥ��������������å����ѿ�������ʤäƤ���Ф��� */
static int
modnamchk(s)
char	*s;
{
	if(is_digit(*s)) return(0);
	for(; *s; s++) if(!is_alnum(*s) && *s != '_') return(0);
	return(1);
}

#define modu1(a, b) ((b) ? ((a) % (b)) : (a))
#define curmod(num) (modesw[num] . curmode)
#define modrng(num) (modesw[num] . moderng)

 /**	num���ܤΥ⡼�ɤ�����󥸤����Ѵ�ɽ������ľ�������� mode ���ͤ�0�ʤ�
	�⡼�ɤ�off��1�ʤ�on���뤳�Ȥˤʤ롣�ʤ�����mode���ͤ��֤���*/
modetyp	chgmod(num, mode)
int	num;
modetyp mode;
{
	modetyp oldmod;

	oldmod = curmod(num);
	curmod(num) = modu1(mode, modrng(num));
	choosehyo();
	return(oldmod);
}

 /** ���⡼�ɤ�ޤȤ���ڤ��ؤ��� */
void
allchgmod(mode)
modetyp	mode;
{
	int	i;

	for(i = 0; modmeibgn[i] != NULL; i++){
		curmod(i) = modu1(mode, modrng(i));
	}
	choosehyo();
}

 /**	num���ܤΥ⡼�ɤ���ꤷ�����������󥯥���Ȥ�����mode���ͤ��֤���*/
modetyp	incmod(num, dmode)
int	num;
modetyp	dmode;
{
	modetyp	oldmod, newmod;

	newmod = oldmod = curmod(num);
	newmod += dmode;
	if(oldmod > newmod) newmod -= modrng(num);
	return(chgmod(num, newmod));
}

 /**	num���ܤΥ⡼�ɤ���ꤷ���������ǥ�����Ȥ�����mode���ͤ��֤����Թ�
	�ˤ�ꡢincmod�Ȥ��̤��Ѱդ��ʤ��ƤϤʤ�ʤ���*/
modetyp	decmod(num, dmode)
int	num;
modetyp	dmode;
{
	modetyp	oldmod, newmod;

	newmod = oldmod = curmod(num);
	newmod -= dmode;
	if(oldmod < newmod) newmod += modrng(num);
	return(chgmod(num, newmod));
}

 /**	name��̾�Υ⡼�ɤ��ʤ������0���֤�������Ф��Υ⡼���ֹ桦�ڤӤ���
	���֤κ����͡ܣ��ȸ��ߤξ��֤��äƤ��� */
int
romkan_getmode(name, nump, modep, moderngp)
char	*name;
int	*nump;
modetyp	*modep, *moderngp;
{
	if(!modnam_src(name, nump)) return(-1);
	*modep = curmod(*nump);
	*moderngp = modrng(*nump);
	return(0);
}

 /**	name��̾�Υ⡼�ɤ��ʤ������0���֤�������Ф��ξ��֤򥻥åȤ���
	�Ѵ�ɽ�������θ塢����֤�������0���֤���*/
int
romkan_setmode(name, modep)
char	*name;
modetyp	*modep;
{
	modetyp	oldmode, moderng;
	int	modenum;

	if(romkan_getmode(name, &modenum, &oldmode, &moderng)!= 0) return(-1);
	chgmod(modenum, *modep);
	*modep = oldmode;
	return(0);
}

 /** �Ѵ�ɽ�Υ��ꥢ */
void
romkan_reset()
{
	naibu_[0] = Terminator;
	choosehyo();
	romkan_clear();
}

 /** �Ѵ��б�ɽ�������Ԥ� */
void
choosehyo()
{
	int	*naibup, i;

	naibup = naibu_;
	usemaehyo[0] = usehyo[0] = useatohyo[0] = -1;
	for(i = 0; i < 2; i++){
		dspmod[1][i] = dspmod[0][i];
		dspmod[0][i] = NULL;
	}

	look_choose(&naibup, 1);
}

 /**	�⡼��ɽ������������缡���Ƥ���������ɽ������ڤӥ⡼��ɽ��ʸ�����
	�����ԤäƤ�����â��flg��0�ʤ饹���åפ������ */
static void
look_choose(naibupp, flg)
int	**naibupp;   /* �⡼��ɽ������ɽ������ؤΥݥ��󥿤ؤΥݥ��󥿡�
			look_cond()��evlcond()�Ǥ�Ʊ�� */
int	flg;
{
	int	*naibup, naibu1, naibu2, branch, lcrsl;

	naibup = *naibupp;

	while((naibu1 = *naibup++) != Terminator){
		switch(SHUBET(naibu1)){
			case 4: /* ɽ̾ */
				if(flg) hyouse(LWRMSK(naibu1));
				break;
			case 2: /* ��Ｐ */
				branch = LWRMSK(naibu1); /* if;when */
				lcrsl = look_cond(&naibup, flg);
				if(branch == 0 && lcrsl) flg = 0;
				break;
			case 5: /* romkan��on��off�����줾���
				   �⡼��ɽ��ʸ���� */
				naibu2 = *naibup++;
				if(flg) dspmod[0][LWRMSK(naibu1)] =
							    dspnambgn[naibu2];
				break;
			case 6: /* romkan�����줾��on��off���Υ⡼��ɽ��
				   ʸ��������Τޤޤ� */
				if(flg) dspmod[0][LWRMSK(naibu1)] = 
						    dspmod[1][LWRMSK(naibu1)];
				break;
			default:
				BUGreport(6);
		}
	}

	*naibupp = naibup;
}

 /**	*naibupp ��������ɽ������Ǿ�Ｐ��ɽ���Ȥ����ؤ��Ƥ���Ȧ�ʤΤǡ�
	�����ɾ���������ʤ餽��³�����ᤷ�ˤ��������ʤ��ɤ����Ф���
	���ͤϡ��ǽ��ɾ��������Ｐ�ο����͡�*/
static int
look_cond(naibupp, flg)
int	**naibupp, flg;
{
	int	*naibup, condrsl;

	naibup = *naibupp;

	condrsl = evlcond(&naibup); /* ɬ��ɾ�����ʤ��Ȥ����ʤ����� */
	flg = flg && condrsl;
	look_choose(&naibup, flg);

	*naibupp = naibup;
	return(flg);
}

 /** ��Ｐ�ο����ͤ�ɾ��  ���ͤ�0��1�Ȥϸ¤�� */
static int
evlcond(naibupp)
int	**naibupp;
{
	int	*naibup, naibu1, retval = -1, tmpval[ARGMAX], i, imax;

	naibup = *naibupp;

	naibu1 = *naibup++;
	switch(SHUBET(naibu1)){
		case 7: /* ���� */
			retval = *naibup++; break;
		case 1: /* �⡼��̾ */
			retval = modesw[LWRMSK(naibu1)] . curmode; break;
		case 3: /* and�ʤ� */
			imax =	condarg[LWRMSK(naibu1)];
			for(i = 0; i < imax; i++)
				tmpval[i] = evlcond(&naibup);
			switch(LWRMSK(naibu1)){
			 /* �夫����true,false,not,and,or */
				case 0: retval = !tmpval[0]; break;
				case 1: retval = tmpval[0]&& tmpval[1]; break;
				case 2: retval = tmpval[0]|| tmpval[1]; break;
				case 3: retval = 1; break;
				case 4: retval = 0; break;
				case 5: retval = (tmpval[0] == tmpval[1]);
					break;
				case 6: retval = (tmpval[0] != tmpval[1]);
					break;
				case 7: retval = ((unsigned int)tmpval[0] <
						  (unsigned int)tmpval[1]);
					break;
				case 8: retval = ((unsigned int)tmpval[0] >
						  (unsigned int)tmpval[1]);
					break;
			}
			break;
	}

	*naibupp = naibup;
	return(retval);
}

 /** num���ܤ�ɽ�򡢻��Ѥ����ΤȤ�����Ͽ���롣�����ܡ�������ζ��̤⤹�� */
static void
hyouse(num)
int	num;
{
	int	*ptr;

	switch(hyoshu[num]){
		case 1: ptr = usemaehyo; break;
		case 2: ptr = usehyo; break;
		case 3: ptr = useatohyo; break;
		default: BUGreport(11); return;
	}
	for(; *ptr != -1; ptr++) if(*ptr == num) return;
	*ptr = num;
	*++ptr = -1;
}

 /** strcmp��Ʊ��  â����'\��8�ʡ�;'���᤹�롣*/
static int
mystrcmp(s1, s2)
char	*s1, *s2;
{
	char	c1, c2;

	while((c1 = codeeval(&s1)) == (c2 = codeeval(&s2)))
		if(c1 == '\0') return(0);
	return(c1 > c2 ? 1 : -1);
}

 /** strcpy��Ʊ�� â��'\��8�ʡ�;'���᤹�롣s1 <= s2�ʤ�����ư���Ϥ� */
static void
mystrcpy(s1, s2)
char	*s1, *s2;
{
	while(*s1++ = codeeval(&s2));
}

 /**	��ʸ���β���Ԥ������̤�ʸ���Ϥ��Τޤޡ�'\��8�ʡ�;'�ϼºݤΥ����ɤ�
	ľ�������θ塢ʸ����ؤΥݥ��󥿤��ʸ��ʬ�ʤ�Ƥ����ʾ��ʤ��Ȥ�
	���Х���ʬ�ʤळ�Ȥ��ݾڤ����Ϥ��ˡ�*/
static char
codeeval(sptr)
register char	**sptr;
{
	register char	c;
	char	code = 0;

	if((c = *(*sptr)++) != '\\') return(c);
	while(c = *(*sptr)++, is_octal(c)){
		code <<= 3;
		code += ctov(c);
	}
	if(c != ';') BUGreport(12);
	return(code);
}

 /** romkan��on���Υ⡼��ɽ��ʸ������֤��ؿ���̵����Ǥ��ä�RK_DSPNIL�ե饰��
     Ω�äƤ�����϶�ʸ������֤���*/

char	*romkan_dispmode()
{
	return(dspmod[0][0] == NULL && (flags & RK_DSPNIL) ?
	       nulstr : dspmod[0][0]);
}

 /** romkan��off���Υ⡼��ɽ��ʸ������֤��ؿ���̵����Ǥ��ä�RK_DSPNIL�ե饰
     ��Ω�äƤ�����϶�ʸ������֤���*/
char	*romkan_offmode()
{
	return(dspmod[0][1] == NULL && (flags & RK_DSPNIL) ?
	       nulstr : dspmod[0][1]);
}

 /** ʸ����10�������ʤ餽�β��򤷡������Ǥʤ������0���֤� */
static int
chk_get_int(p, ip, range)
char	*p;
unsigned int	*ip;
modetyp range;
{
	int	sgn = 1;
	modetyp	out;

	if(*p == '-'){
		p++;
		sgn = -1;
	}
	for(out = 0; *p; p++){
		if(!is_digit(*p)) return(-1);
#if defined(UX386) || defined(sun386)
		out = out * 10;
#else
		out *= 10;
#endif
		out += ctov(*p);
	}
	if(range != 0) out %= range;
	if(sgn == -1 && out != 0) out = range - out;
	*ip = out;
	return(0);
}
