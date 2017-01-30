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
			rk_main.c
						88. 6.10  �� ��

	���޻������Ѵ�������¾�����ϥ����ɤ��Ѵ�����ץ���ࡣ
	ASCII�����ɻ��Ѥ�����Ȥ��Ƥ��롣
	���Υե�����ϡ��Ѵ��Υᥤ��롼����
***********************************************************************/
/*	Version 3.0
 */
#include "rk_header.h"
#include "rk_extvars.h"
#include "rk_fundecl.h"

struct	matchpair /*	���ߥޥå�����Ƥ����ѿ����ֹ�ȡ��ޥå�����ʸ����
			�Ȥ��ݻ���*/
{
	int	hennum;
	letter	ltrmch;
};

letter	memory[SIZALL]; /* �Ѵ��б�ɽ�䡢�ѿ����Ѱ�ʤɤ�������ΰ� */
struct	dat	data[LINALL]; /* �б�ɽ�ΰ�Ԥ��ȤΥǡ����ؤΥݥ��� */
letter	*hensudefhyo[VARTOT]; /* �ѿ����Ȥ��Ѱ�ǡ����ؤΥݥ��� */
struct	matchpair	henmatch[VARTOT]; /* �ѿ��Υޥå������򤿤�Ƥ��� */

#ifdef KDSP
#  ifdef MVUX
letter	displine[DSPLIN]; /* �ǥХå���	 ɽ���ΰ��ʬ�򵭲� */
#  endif
#endif

letter	*curdis;
int	codein_len;
letter	*codeout, *remainkbf;
letter	ungetc_buf = EOLTTR; /* romkan_ungetc����ʸ�����򤷤Ƥ����Хåե� */
letter	unnext_buf = EOLTTR; /* romkan_unnext��		   ��		  */

letter	(*keyin_method)(); /* ��������ؿ��Υ��ɥ쥹 */
int	(*bytcnt_method)(); /* �Х��ȥ�����ȴؿ��Υ��ɥ쥹 */
int	(*kbytcnt_method)(); /* �������ϲ���ѥХ��ȥ�����ȴؿ��Υ��ɥ쥹 */
char	prv_modfnm[REALFN] = "\0"; /* �⡼��ɽ̾�����Ϥ��Υѥ�̾�ˤ���¸ */
	    /* ������꤬�����ø�! */

#define DISOUT ((flags & RK_NONISE) ? rk_output : disout)
 /* �ե饰���ͤ�romkan_init3()��������ꤵ�졢����RK_NONISE�ΥӥåȤ��ͤ�
    ɸ���0�����ä�DISOUT��ɸ���ͤ�disout�������rk_output���Ѥ���ȡ���������
    �ʽ���LTREOF�ˤ���ڽФ��ʤ���â�����������Ϥ������ɬ���������֤��褦��
    ���Ƥ�����˸¤ꡢEOLTTR����֤��ˡ�*/

char	eofflg; /* romkan_next()����LTREOF���褿�Ȥ��ˡ�romkan_henkan()���1
		   �ˤ��롣������ͤ���0�ʤ��Ѵ��б�ɽ��(error)��̵�뤷�ơ�
		   �ܽ����Хåե����̤���ꥳ���ɤ⡢����Ū���Ѵ������롣����
		   LTREOF���褿�������̤ˤ�����Ϥ�Ф��褦�˻��ꤵ��Ƥ����
		   ���ν�����Ԥ���â�������ν����ϵޤ����館���Դ����ˡ�����
		   �������eofflg��2��*/

letter	evalbuf[2][2][OUTSIZ];
char	ebf_sw = 0;
letter	saishu_out[OUTSIZ];
letter	delchr, delchr2 = EOLTTR, nisedl; /* DEL�Υ���饯������Ĥޤǻ��Ƥ�*/

modetyp	chgmod(), incmod(), decmod();


/* letter���֤��ؿ�������ڤ�����ʰ����ˤ�char�Τ�Τ⺮���äƤ���� */

static letter	mchedsrc();
letter	romkan_next(), romkan_unnext();
letter	romkan_getc(), romkan_ungetc(), *romkan_henkan();
letter	to_zenalpha(), to_zenhira(), to_zenkata();
 /* to_zenhira;kata �����������ʸ�����ޤȤ�ˤ��ʤ���*/

void BUGreport();
static void maeato_henkan(), add_at_eof(), rk_delete(),
  set_rubout(), mchevl(), codeout_chg(), ltrevlcpy();
int match();
static int p_eq(), hen_ikisrc(), henkan_ok(), rk_rst(),
  head_bytecount();
extern void readdata(), romkan_reset(), hank_setup(), to_hankata(),
  to_digit(), dakuadd(), handakuadd(), allchgmod();
extern int ltov();

static
letter	*ltrcpy(lp1, lp2)
letter	*lp1, *lp2;
{
	letter	*org;

	org = lp1;
	while((*lp1++ = *lp2++) != EOLTTR);
	return(org);
}

#ifdef notuse
letter	*ltrncpy(lp1, lp2, n) /* ������'EOLTTR'�򥻥å� */
letter	*lp1, *lp2;
int	n;
{
	letter	*org;

	org = lp1;
	for(; n; --n) if(EOLTTR == (*lp1++ = *lp2++)) return(org);
	*lp1 = EOLTTR;
	return(org);
}
#endif

 /** letter����������˰�ʸ���Ĥʤ��� */
static
letter	*ltr1cat(lp, l)
letter	*lp, l;
{
	letter	*org;

	org = lp;
	totail(lp);
	*lp++ = l;
	*lp = EOLTTR;
	return(org);
}

 /** ʸ����������˰�ʸ���Ĥʤ��롣ltr1cat��char�� */
char	*chrcat(s, c)
char	*s, c;
{
	char	*org;

	org = s;
	strtail(s);
	*s++ = c;
	*s = '\0';
	return(org);
}

static
letter	*ltrcat(lp1, lp2)
letter	*lp1, *lp2;
{
	letter	*org;

	org = lp1;
	totail(lp1);
	ltrcpy(lp1, lp2);
	return(org);
}

 /** letter����lp2�γ����ǤκǾ�̥ӥåȤ�Ω�ƤƤ��顢lp1�θ��ˤĤʤ��롣*/
static
letter	*bitup_ltrcat(lp1, lp2)
letter	*lp1, *lp2;
{
	letter	*org;

	org = lp1;
	totail(lp1);

	while((*lp1 = *lp2++) != EOLTTR) *lp1++ |= HG1BIT;
	 /** lp2 ���������ƤκǾ�̥ӥåȤ�Ω�Ƥ롣*lp2��NISEBP(rk_spclval.h
	     ��define)�ʤɤΤȤ��Ϻǽ餫��Ǿ�̤�Ω�äƤ뤫��褤 */

	return(org);
}

 /** ltrcat�������ȡ���̤�ʸ����������˥ݥ��󥿤��äƤ�����*/
letter	*ltrgrow(lp1, lp2)
letter	*lp1, *lp2;
{
	totail(lp1);
	while((*lp1 = *lp2++) != EOLTTR) lp1++;
	return(lp1);
}

static int
ltrlen(lp)
letter	*lp;
{
	letter	*org;

	for(org = lp; *lp != EOLTTR; lp++);
	return(lp - org);
}

 /**	letter����κǸ��ʸ���ؤΥݥ��󥿤��֤���â����ʸ������Ф��Ƥ�
	���Τޤ��֤���*/
letter	*ltrend(lp)
letter	*lp;
{
	return((*lp != EOLTTR) ? (lp + ltrlen(lp) - 1) : lp);
}

 /**	ʸ����κǸ��ʸ���ؤΥݥ��󥿤��֤���ltrend()��char�ǡ�ltrend()��
	Ʊ�͡���ʸ����ΤȤ������㤬���롣*/
char	*strend(s)
char	*s;
{
	return(*s ? (s + strlen(s) - 1) : s);
}

int
ltrcmp(lp1, lp2)
letter	*lp1, *lp2;
{
	for(; *lp1 == *lp2; lp1++, lp2++) if(*lp1 == EOLTTR) return(0);
	return(*lp1 > *lp2 ? 1 : -1);
}

#ifdef notuse
int
ltrncmp(lp1, lp2, n)
letter	*lp1, *lp2;
int	n;
{
	for(; n && *lp1 == *lp2; n--, lp1++, lp2++)
		if(*lp1 == EOLTTR) return(0);
	return(n == 0 ? 0 : (*lp1 > *lp2 ? 1 : -1));
}

letter	*ltr_index(lp, l)
letter	l, *lp;
{
	for(; *lp != EOLTTR; lp++) if(*lp == l) return(lp);
	return(NULL);
}
#endif

static
letter	*ltr_rindex(lp, l)
letter	l, *lp;
{
	letter	*lp2;

	for(lp += ltrlen(lp2 = lp); lp2 != lp; ) if(*--lp == l) return(lp);
	return(NULL);
}

 /** ʸ�����������ʸ���򥫥åȤ��롣*/
letter	*ltr1cut(lp)
letter	*lp;
{
	int	l;

	if(0 == (l = ltrlen(lp))) BUGreport(0); /* Illegal POP */
	*(lp + --l) = EOLTTR;
	return(lp);
}

 /** letter�ΰ�ʸ����Ĺ������ʸ������Ѵ����� */
static
letter	*ltr_to_ltrseq(lp, l)
letter	*lp, l;
{
	*lp++ = l;
	*lp-- = EOLTTR;
	return(lp);
}

 /** char���󤫤�letter���ļ��Ф���â��'\0'��EOLTTR���Ѥ��� */
letter	letterpick(lbfpptr)
uns_chr **lbfpptr;
{
	letter	l = 0;
	register int	i;

	for(i = (*bytcnt_method)(*lbfpptr); i; i--)
		l = (l << 8) + *(*lbfpptr)++;
	return(l == 0 ? EOLTTR : l);
}

 /**	���Ϥ��줿�����ɤ�ʸ��ñ�̤ˤޤȤ�롣â��EOF��LTREOF��rk_spclval.h
	�ˤ�����ˤ��Ѥ��롣*/
letter	romkan_next()
{
	letter	in;
	int	i, n;
	uns_chr c;

	  /* unnext����Ƥ�����ϡ��������Ф���*/
	if(unnext_buf != EOLTTR)
		return(in = unnext_buf, unnext_buf = EOLTTR, in);

	if( (letter)EOF == (in = (*keyin_method)() )) return(LTREOF);
	c = (in &= 0xff);
	n = (*kbytcnt_method)(&c);
	for(i = 1; i < n; i++) in = (in << 8) + ((*keyin_method)() & 0xff);
	return(in);
}

 /*	letter��ʸ�� l ��char��ʸ�� c ����ӡ�����ɥޡ���Ʊ�Ρ�letter���
	����ɥޡ�����EOLTTR��char��Τ�'\0'�ˤ���פȤߤʤ���*/
#define ltrchreq(l, c) ((l) == ((c) != 0 ? (c) : EOLTTR))

 /** letter��ʸ�����char��ʸ�������� â���羮��Ӥ����ʤ��Ȥ��Ƥ�� */
int
ltrstrcmp(l, s)
register letter *l;
register char	*s;
{
	for(; ltrchreq(*l, *s); l++, s++) if(*s == 0) return(0);
	return((*l < (uns_chr)*s || *l == EOLTTR)? -1 : 1);
}

 /** �Ǹ�˥ޥå�����ʸ�� �Ĥޤ�urabuf�κǸ塣urabuf��������EOLTTR���֤� */
static
letter	lastmch()
{
	return(*(ltrend(urabuf)));
}

/* end of 'letter' functions */


#ifdef	OMAKE
 /** ��	 ��  ��
	�ԣ֥��˥�Τ�����ʰ����û���ޤ�ˡ����������������λ��Ϥɤ줫��ġ�
	-1�ʤ����Ƥ���Ϥ�������ʳ��ʤ鲿����Ϥ��ʤ���������ξ��⡢�ǡ���
	�θĿ����֤���*/
takeoka(i)
int	i;
{
#define OPIMAX (numberof(opinion))

    static char *opinion[] = {
	"����ˡ�Υ����ɥ� �ѥ��ƥ�桼�ߡפ��ɤ����Ȥ���by�ݲ�",
	"�ֵ�ư��Υ�����দ���פ��ϵ��Ԥ��ɤ����Ȥ���by����",
	"����ˡ�Υ����� �ޥ����륨�ߡפ��ɤ����Ȥ���by�ݲ�",
	"�ʻ��Ͱո��ˡ֥ޥ����륨�ߡ�������͡פ��ɤ����˥����by����",
	"����ˡ��ŷ�� ���ꥣ�ߡ��ޥߡפ��ɤ����Ȥ���by�ݲ�",
	"�֥���������פ��ɤ����Ȥ���by����",
	"�֥ࡼ�ߥ�פ��ɤ����Ȥ���by����",
	"�֥ޥ������פΥ��ɥ��ʡ����꡼�����ɤ����Ȥ���by����",
	"�֥��ס��󤪤Ф���פ��ɤ����Ȥ���by��",
	"�֤Ϥ���ʹ֥��㡼�ȥ륺�פ��ɤ����Ȥ���by�ٱ�",
	"�ֱ���ε��Υƥå��ޥ�פ��ɤ����Ȥ���by����",
	"���Ų��ʹ֥٥�פ��ɤ����Ȥ���by��",
	"�֥���ץ��ξ����ϥ����פ��ɤ����Ȥ���by�ٱ�",
	"�֤Ȥ��᤭�ȥ��ʥ��ȡפ��ɤ����Ȥ���by����",
	"�֤������Υ��硼�פ��ɤ����Ȥ��ä��͡�byΩ�ڡʤĤ�����",
	"�֤��ޤ��쥪��󥸡����ɡפ��ɤ����Ȥ���by����������",
	"�֥��å�����ܡסʡ֥��å�����ܣǡפ�ޤ�ˤ��ɤ����Ȥ���by����",
	"�֥ƥä����סʡ֤�����ҥ����פΤ��ȡˤ��ɤ����Ȥ���by����",
	"�֥��������ޥ����ԥ졼���פ��ɤ����Ȥ���by����",
	"�־�����������פ��ɤ����Ȥ���by����",
	"����̩���⥴��󥸥㡼�פ��ɤ����Ȥ���by����",
	"���������Ρˡ�Ŵ��28��פ��ɤ����Ȥ���by����"
    };

	if(i >= 0) printf("%s\n", opinion[i % OPIMAX]);
	 else if(i == -1) for(i= 0; i< OPIMAX; i++) printf("%s\n",opinion[i]);
	return(OPIMAX);
}
#endif	/* OMAKE */


 /** �Ѵ��Хåե��Υ��ꥢ */
void	romkan_clear()
{
#ifdef KDSP
#  ifdef MVUX
	*displine = EOLTTR;
#  endif
#endif
	ungetc_buf = *keybuf = *urabuf = *disout = *rk_output = EOLTTR;
	curdis = DISOUT;
	lastoutlen = lastkbflen = 0;
}

 /**	ɽ���ɤ߹�����Ѵ��ν������򤹤롣��part 1��*/
int
romkan_init(modhyo, delchr_, chmoutf, keyinfn, bytcntfn)
char	*modhyo, chmoutf;
letter	delchr_;
letter	(*keyinfn)();
int	(*bytcntfn)(); /* �����ξܺ٤�romkan_init3�򻲾� */
{
	return(romkan_init2(modhyo, delchr_, chmoutf, keyinfn, bytcntfn,
			    0, 0, 0));
}

#define ifflg(a, b) ((a) ? (b) : 0)

 /**	ɽ���ɤ߹�����Ѵ��ν������򤹤롣��part 2: �������Ϥ��Ф�ɬ������
	�֤��褦�ˤ��뤫�������Хåե��򥯥ꥢ���뤫�ɤ����ʤɤ����ġ�*/
int
romkan_init2
(modhyo, delchr_, chmoutf, keyinfn, bytcntfn, keyackf, restartf, nonisecodf)
char	*modhyo, chmoutf, keyackf, restartf, nonisecodf;
letter	delchr_;
letter	(*keyinfn)();
int	(*bytcntfn)(); /* �����ξܺ٤�romkan_init3�򻲾� */
{
	return(romkan_init3(modhyo, delchr_, toNISE(delchr_), EOLTTR,
			    keyinfn, bytcntfn, (int (*)())NULL,
			    restartf,
			    ifflg(chmoutf, RK_CHMOUT) |
			    ifflg(keyackf, RK_KEYACK) |
			    ifflg(nonisecodf, RK_NONISE) |
			    RK_DSPNIL));
}

 /**	ɽ���ɤ߹�����Ѵ��ν������򤹤롣��part 3��*/
int
romkan_init3
(modhyo, delchr_, nisedl_, delchr2_, keyinfn, bytcntfn, kbytcntfn, restartf,
 flags_)
char	*modhyo;	/* �⡼�����ɽ��̾���Ϥ��Υѥ�̾ */
letter	delchr_;	/* DEL�Ȥ��ƻȤ������� */
letter	nisedl_;	/* ��DEL�Ȥ��ƻȤ������� */
letter	delchr2_;	/* DEL�Ȥ��ƻȤ������ɤ���Ĥ����礽�Τ⤦���� */
letter	(*keyinfn)();	/* ��������ؿ� */
int	(*bytcntfn)();	/* �Х��ȥ�����ȴؿ� */
int	(*kbytcntfn)(); /* ��������β���ѥХ��ȥ�����ȴؿ���NULL����λ���
			   bytcntfn��Ʊ����Τ��Ȥ��� */
char	restartf;
 /*	rk_rst()�⤫��ƤФ줿����Ω�ĥե饰�����줬Ω�Ĥȡ��Хåե��򥯥ꥢ
	�����˺Ѥޤ������λ��ϥ��顼���л��Υ�å������⾯���ۤʤ� */
int	flags_;
 /* �ʲ��Υե饰�����Ѳġ�������rk_spclval.h�ˤ������
	RK_CHMOUT:�⡼�ɥ����󥸤��Τ餻�륳���ɤ��֤�����
	RK_KEYACK:����������Ф�ɬ���������֤���
	RK_DSPNIL:romkan_disp(off)mode�Υǥե���Ȥ��ʸ����ˤ��뤫
	RK_NONISE:�������ɤ�Ф��ʤ��褦�ˤ��뤫
	RK_REDRAW:Wnn���ü�ե饰��redraw�ѤΥե饰��Ф����ɤ�����
	RK_SIMPLD:delete��ư���ñ��ˤ��뤫
	RK_VERBOS:verbose�⡼�ɤǵ�������
 */
{
	int	errcod; /* ���ν�1�Τ� */

	 /* �������ɤ�Ф����������romkan_clear�ǻȤ��Τǡ��ե饰�������
	    ��ˤ�äƤ����ʤ��Ȥ����ʤ���*/
	if((flags = flags_) & RK_VERBOS){
		fprintf(stderr, "romkan_init invoked.\r\n");
		fflush(stderr);
	}

#ifdef	OMAKE
	if(modhyo == NULL) takeoka(-1); /* ���ޤ����̾�ϼ¹Ԥ���ʤ� */
#endif	/* OMAKE */

	keyin_method = keyinfn;
	bytcnt_method = (bytcntfn == NULL ? head_bytecount : bytcntfn);
	kbytcnt_method = (kbytcntfn == NULL ? bytcnt_method : kbytcntfn);
	strcpy(prv_modfnm, modhyo);

	delchr = delchr_;
	nisedl = nisedl_;
	delchr2 = delchr2_;
 /**	�¤�DEL�Υ���饯������Ĥޤǻ��Ƥ롣����ܡ�delchr2�ϡ�̤�����
	�Ȥ����ͤ�EOLTTR�ˤ�̵�뤵��롣����ʳ��ϡ�delchr2�����Ϥ�����
	delchr�����Ϥ��줿�Τ�Ʊ�������񤤤򤹤롣*/

	if((errcod = setjmp(env0)) != 0){
		fprintf(stderr,"romkan_init failed.");
		if(!restartf){
			romkan_reset();
		 /*	fprintf(stderr,
			      "No conversion is currently performed."); */
			fprintf(stderr, "\r\n");
		} else {
			fprintf(stderr,
				"Conversion method was unchanged.\r\n");
		}
	} else {
		readdata(memory, data, hensudefhyo, modhyo);
		if(!restartf){
			romkan_clear();
			hank_setup();
		}
	}

	if(!errcod && (flags & RK_VERBOS)){
		fprintf(stderr, "romkan_init finished.\r\n");
		fflush(stderr);
	}
	return(errcod); /* ���ｪλ�ʤ�0 */
}

 /**	�Х��ȥ�����ȴؿ��Υǥե���ȡ�s�λؤ��Ƥ��������äƤ���Τ�
	���Х��ȥ����ɤ����֤���
	romkan_next����ա�������Ǥϡ��Х��ȥ�����ȴؿ����ͤ�
	�����ΰ�ʸ���ܤˤΤ߰�¸����Ȳ��ꤷ�Ƥ��롣*/
static int
head_bytecount(s)
uns_chr *s;
{
#ifdef IKIS
	return((*s <= 0xa0 || *s == 0xff)? 1 : 2);
#else
	return(((*s <= 0xa0 && *s != HNKAK1) || *s == 0xff)? 1 : 2);
#endif
}

 /** �Ѵ����줿ʸ����缡�֤� */
letter	romkan_getc()
{
	letter	l;

	  /* ungetc����Ƥ�����ϡ��������Ф���*/
	if(ungetc_buf != EOLTTR)
		return(l = ungetc_buf, ungetc_buf = EOLTTR, l);

	while(*curdis == EOLTTR){
		 /* romkan_next()���ͤ�LTREOF�λ��⡢���Τޤ�������ɤ���*/
		curdis = romkan_henkan(romkan_next());

		 /* keyackflg����0�ʤ顢�������󤬤����ɬ�������֤�������
		    ���ᡢ�֤��٤���Τ��ʤ�����EOLTTR���֤����Ȥˤ��롣*/
		if(flags & RK_KEYACK) break;
	}

	if(EOLTTR != (l = *curdis)) curdis++;
	return(l);
	 /* ��ʪ��ʸ���ʤ顢HG1BIT��Ω�äƤ��롣*/
	 /* ���̤ʥ�����(�㤨�е���BEEP�Ȥ���NISEBP)���֤��Ȥ������롣������
	    rk_spclval.h���������Ƥ��롣*/
}

 /**	romkan_getc�β��̴ؿ��Ȥ��ơ����Ϥ��ʸ��������ä�
	�Ѵ���̤�ʸ�������Ϥ��롣*/
letter	*romkan_henkan(mae_in)
letter	mae_in;
{
	letter	mae_out[2], *p;

	/* if(*curdis != EOLTTR){p = curdis; curdis = nil; return(p);} */
	/* ������Ǥ�mae_in��̵�뤵��� */
	curdis = nil;

	  /* ����������ؤ����ϤϾ�ˤ�����ʸ�� */
	mae_out[0] = mae_out[1] = EOLTTR;

	eofflg = rk_errstat = 0;
	*rk_output = *disout = EOLTTR;

	maeato_henkan(mae_in, mae_out, usemaehyo);
	rk_input = *mae_out;

	if(rk_input == LTREOF){
		 /* LTREOF���褿��硢(error)��̵�뤷���ܽ����Хåե���������
		    �����Ѵ����롣���Τ����eofflg��1�ˤ��롣���θ塢��̤�
		    �����ˡ�LTREOF���褿�������̥����ɡʻ��ꤵ��Ƥ���Сˤȡ�
		    LTREOF��Ĥʤ���*/
		eofflg = 1;
		match();

		eofflg = 2;
		add_at_eof(); /* codeout�ˡ�LTREOF���褿���Ф������ɤ����롣*/
		ltr1cat(codeout, LTREOF);

		ltrcat(disout, codeout);
		ltrcat(rk_output, codeout);
	} else if(rk_input == EOLTTR){
		 /* EOLTTR���褿���⡢���Ʊ�ͤν�����Ԥ�����LTREOF��
		    �Ĥʤ��ʤ����ʤ��������romkan_getc()��Ƥ�Ǥ������
		    ������ʤ���romkan_next()��EOLTTR���֤��ʤ�����ˡ�*/
		eofflg = 1;
		match();
	} else if(rk_input == delchr || rk_input == delchr2){
	    /* delchr2��̤����ʤ餽���ͤ�EOLTTR�ʤΤ�rk_input���������ʤ���*/
		rk_delete(rk_input);
	} else {
		ltr1cat(keybuf, rk_input);
		ltr1cat(disout, toNISE(rk_input));
		match();
	}

	if(!(flags & RK_CHMOUT)){
	   /* chmoutflg��0�λ��ϡ�CHMSIG��Ф��ʤ���*/
		for(p = DISOUT; ; p++){
			while(*p == CHMSIG) ltrcpy(p, p + 1);
			if(*p == EOLTTR) break;
		}
	}

	if((flags & RK_REDRAW) && NULL != (p = ltr_rindex(disout, nisedl))){
		for(p++; *p != EOLTTR || (ltr1cat(disout, REDRAW), 0); p++)
			if(!isSPCL(*p)) break;
	}
	 /* wnn��redraw�Υե饰��Ω�äƤ��ơ�disout��nisedl�ʸ��ü쥳���ɤΤ�
	    �ǽ���äƤ����顢REDRAW��Ф��ơ�Wnn���Ѵ��Ԥ�redraw�򤵤��롣*/

	return(DISOUT);
}


 /* �ǥХå��Ѵؿ� */
#ifdef KDSP
#  ifdef MVUX

void
pridbg2(a, b, c)
char	*a, *c;
letter	*b;
{
	printf("%s", a); dump_fordbg(b); printf("%s", c);
}

int
ltr_displen(l)
letter	l;
{
	while(l >= 256) l >>= 8;

	if(l == 0) return(0);
	if(l < ' ') return(0);
	if(168 <= l && l <= 170) return(1);
	return(l > 160 ? 2 : 1);
}

void
dump_fordbg(lp)
letter	*lp;
{
	while(*lp != EOLTTR) printf("%x/", *lp++);
}

void
print_fordbg(lp)
letter	*lp;
{
	while(*lp != EOLTTR) putletter(*lp++ & ~HG1BIT);
}

void
print_ltrseq(lp)
letter	*lp;
{
	while(*lp != EOLTTR) print_ltr(*lp++ & ~HG1BIT);
}

void
print_ltr(l)
letter	l;
{
	letter	*disptail;
	int	i;

	if(!isSPCL(l)) l &= ~HG1BIT;

	if(l == CHMSIG)
#ifdef CHMDSP
		printf("...mode=%s\n",romkan_dispmode())
#endif
		;
	else
#define BEEPCH	'\007'
#define NEWLIN	'\n'
	if(l == BEEPCH || l == NISEBP) putchar(BEEPCH);
	else
	if(l == NEWLIN){
		*displine = *keybuf = *urabuf = EOLTTR;
		lastoutlen = lastkbflen = 0;
		putchar(l);
	}
	else
	if(l == delchr || l == nisedl){
		if(*displine == EOLTTR) putchar(BEEPCH);
		 else {
			disptail = ltrend(displine);
			for(i = ltr_displen(*disptail); i; i--)
				printf("\031 \031");
			*disptail = EOLTTR;
		}
	}
	else {
		ltr1cat(displine, l);
		putletter(l);
	}
}

#  endif /* of #ifdef MVUX */
#endif
/* �ǥХå��Ѵؿ������ */


 /** DEL�����Ϥ��줿�Ȥ��ν����򤹤� */
static void
rk_delete(input_del)
letter	input_del;
{
	if(ltrlen(keybuf) > ((flags & RK_SIMPLD) ? 0 : lastkbflen)){
		ltr1cut(keybuf);
		set_rubout(disout, 1, nisedl);
	} else {
		if(*urabuf != EOLTTR && !(flags & RK_SIMPLD)){
			ltr1cut(ltrcpy(keybuf, urabuf));
			*urabuf = EOLTTR;

			set_rubout(rk_output, lastoutlen, input_del);

			set_rubout(disout, lastkbflen, nisedl);
			bitup_ltrcat(ltrcat(disout, rk_output), keybuf);

			lastkbflen = lastoutlen = 0;
		} else {
			set_rubout(disout, 1, input_del);
			set_rubout(rk_output, 1, input_del);
		}
	}
}

 /**	letter���� lp1 �� lp2 �Υޥå����ߤ롣���ͤϡ�lp1��lp2��Ƭ����
	���פλ� -1��lp1�ޤ��Ϥ���Ƭ����lp2�����פΤȤ��ϰ���Ĺ��lp2��
	��ʸ����λ���ޤࡣ���ξ�����ͤ�0�ˡ�����ʳ��� -2��
	lp2¦�˼����ޤޤ�Ƥ�����ɾ���򤹤롣lp1¦�ˤϼ���ޤ�ǤϤ��� */
static int
prefixp(lp1, lp2)
letter	*lp1, *lp2;
{
	/* 1�ԥޥå������뤴�Ȥˡ��ޤ�match����򥯥ꥢ���Ƥ��顣
	   �Ĥޤꡢhenmatch[0] . ltrmch = EOLTTR; �Ȥ��Ƥ�����*/

	register int	mch_len = 0, d_len;

	for(;;){
		if(*lp2 == EOLTTR) return(mch_len);
		if(*lp1 == EOLTTR) return(-1);
		if((d_len = p_eq(&lp2, &lp1)) < 0) return(d_len);
		mch_len += d_len;
	}
}

 /** num���ܤ��ѿ������ˤ���ʸ���ȥޥå����Ƥ���Ȳ��ꤷ�ơ�����ʸ�����֤� */
static
letter	mchedsrc(num)
int	num;
{
	struct	matchpair	*pairptr;

	for(pairptr = henmatch; pairptr -> ltrmch != EOLTTR; pairptr++){
		if(num == pairptr -> hennum)
			return(pairptr -> ltrmch);
	}
	return(BUGreport(8), 0);
}

 /**	num���ܤ��ѿ���ʸ�� l �ȥޥå����뤫Ĵ�٤롣�����ѿ���unbound���ä�
	���� l ��bind���롣�ޥå�������(bind�λ���ޤ�)1�����ʤ���0���֤� */
static int
mchsrc(num, l)
int	num;
letter	l;
{
	struct	matchpair	*pairptr;

	if(hen_ikisrc(num, l) == 0) return(0);
	for(pairptr = henmatch; pairptr -> ltrmch != EOLTTR; pairptr++){
		if(num == pairptr -> hennum)
			return(pairptr -> ltrmch == l);
	}

	pairptr -> ltrmch = l;
	pairptr -> hennum = num;
	(++pairptr) -> ltrmch = EOLTTR;
	return(1);
}

 /**	l1p�����ñ�̤��ä�ɾ������ʸ��l2�Ȱ��פ��뤫�ɤ������֤���ɾ������
	��̤���ʸ���ˤʤ�ʤ��ä��顢�������פ��ʤ���*/
static int
l_eq(l1p, l2)
letter	*l1p, l2;
{
	letter	evlrsl[RSLMAX];

	switch(SHUBET(*l1p)){
		case 0: return(*l1p == l2);
		case 1: return(mchsrc((int)LWRMSK(*l1p), l2));
		case 2:
			mchevl(&l1p, evlrsl);
			return(evlrsl[0] == l2 && evlrsl[1] == EOLTTR);
		default:
			return(BUGreport(1), 0);
	}
}

 /**	prefixp��ǻ���	 â�������ν���ϵա����ʤ�������ޤޤ줦��Τ�l1pp¦
	�Τߡ�l1pp�����ñ�̤֤��ä�ɾ��������Τ�l2pp�Υޥå����ߤ롣����
	��l2pp�ޤ��Ϥ���Ƭ���ȥޥå�����а���Ĺ���֤���l1pp��ɾ����̤���ʸ��
	��λ���ޤࡣ���ξ�����ͤ�0�ˡ��դ�l1pp��ɾ����̤�Ƭ����l2pp���ޥ�
	���������� -1���֤����ޥå������Ԥ��������ͤ� -2��*/
static int
p_eq(l1pp, l2pp)
register letter **l1pp, **l2pp;
{
	int	num;
	letter	evlrsl[RSLMAX], *rslptr;
	register int	retval = -2;

	 /*  l2pp¦�ˤϼ���ޤޤʤ�Ȧ */
	if(!is_HON(**l2pp)){
	    /*	if(is_HON(**l1pp)) retval = p_eq(l2pp, l1pp); else  */
		BUGreport(9);
	} else {
		switch(SHUBET(**l1pp)){
			case 0: /* ʸ��Ʊ�� */
				retval = (*(*l1pp)++ == *(*l2pp)++ ? 1 : -2);
				break;
			case 1: /* �ѿ���ʸ�� */
				num = LWRMSK(*(*l1pp)++);
				retval = (mchsrc(num, *(*l2pp)++) ? 1 : -2);
				break;
			case 2: /* ����ʸ�� */
				mchevl(l1pp, rslptr = evlrsl);
				for(retval = 0; *rslptr != EOLTTR; retval++){
					if(**l2pp == EOLTTR){
						retval = -1; break;
					} else if(*rslptr++ != *(*l2pp)++){
						retval = -2; break;
					}
				}
				break;
			default:
				BUGreport(2);
		}
	}

	return(retval);
}

 /** l1p�����ñ��ɾ������l2p������� */
static void
mchevl(l1pp, l2p)
letter	**l1pp, *l2p;
{
	letter	*l1p, tmpevl[RSLMAX];

	l1p = *l1pp;

	switch(SHUBET(*l1p)){
		case 0: *l2p++ = *l1p++; break;
		case 1: *l2p++ = mchedsrc((int)LWRMSK(*l1p++)); break;
		case 2:	 /* toupper, tolower, error, �� */
			switch(LWRMSK(*l1p++)){
				case 2: mchevl(&l1p, tmpevl);
					*l2p++ = to_upper(*tmpevl);
					break;
				case 3: mchevl(&l1p, tmpevl);
					*l2p++ = to_lower(*tmpevl);
					break;
				case 4:
					*l2p++ = CHMSIG;
					*l2p++ = LWRMSK(*l1p++);
					*l2p++ = 0; break; /* EOL�ǤϤʤ� */
				case 5:
					*l2p++ = CHMSIG;
					*l2p++ = LWRMSK(*l1p++);
					*l2p++ = 1; break;
				case 6:
					*l2p++ = CHMSIG;
					*l2p++ = XY2INT(2, LWRMSK(*l1p++));
					*l2p++ = 1; break;
				case 7:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_updown(*tmpevl);
					break;
				case 8:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_zenalpha(*tmpevl);
					break;
				case 9:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_hira(*tmpevl);
					break;
				case 10:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_kata(*tmpevl);
					break;
				case 11:
					mchevl(&l1p, tmpevl);
					to_hankata(*tmpevl, &l2p);
					break; /* �ü� */
				case 12:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_zenhira(*tmpevl);
					break;
				case 13:
					mchevl(&l1p, tmpevl);
					*l2p++ = to_zenkata(*tmpevl);
					break;
				case 14:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					*l2p += *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 15:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					*l2p -= *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 16:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					*l2p *= *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 17:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					if(! *tmpevl) *l2p = LTRHUG;
					 else *l2p /= *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 18:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					if(! *tmpevl) *l2p = LTRHUG;
					 else *l2p %= *tmpevl;
					LWRCUT(*l2p++);
					break;
				 /* 19��21��30�ϡ������������ȶ�ʸ����
				    �����Ǥʤ���UNUSDC��ʸ����Ȥ����֤���*/
				case 19:
					mchevl(&l1p, tmpevl);
					if(lastmch() != *tmpevl)
						*l2p++ = UNUSDC;
					break;
				case 20:
					if(! modesw[LWRMSK(*l1p++)] . curmode)
						*l2p++ = UNUSDC;
					break;
				case 21:
					if(modesw[LWRMSK(*l1p++)] . curmode)
						*l2p++ = UNUSDC;
					break;
				case 22:
					*l2p++ = REASIG;
					break;
				case 23:
					*l2p++ = delchr;
					break;
				case 24:
					*l2p++ = CHMSIG;
					*l2p++ = XY2INT(1, 0);
					 /* ����ǡ�all�פ�ɽ�� */
					*l2p++ = 0; break;
				case 25:
					*l2p++ = CHMSIG;
					*l2p++ = XY2INT(1, 0);
					*l2p++ = 1; break;
				case 26:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					*l2p &= *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 27:
					mchevl(&l1p, tmpevl);
					*l2p = *tmpevl;
					mchevl(&l1p, tmpevl);
					*l2p |= *tmpevl;
					LWRCUT(*l2p++);
					break;
				case 28:
					mchevl(&l1p, tmpevl);
					*l2p = ~(*tmpevl);
					LWRCUT(*l2p++);
					break;
				case 29:
					*l2p++ = URBFCL;
					break;
				case 30:
					if(eofflg != 2 || *keybuf != EOLTTR)
						*l2p++ = UNUSDC;
					break;
				case 31:
					{
						letter	code, basenum;

						mchevl(&l1p, tmpevl);
						code = *tmpevl;
						mchevl(&l1p, tmpevl);
						if((basenum = *tmpevl) <= 1 ||
						   BASEMX < basenum)
							basenum = 10;
						to_digit(code, basenum, &l2p);
					}
					break;
				case 32:
					mchevl(&l1p, tmpevl);
					dakuadd(*tmpevl, &l2p);
					break; /* �ü� */
				case 33:
					mchevl(&l1p, tmpevl);
					handakuadd(*tmpevl, &l2p);
					break; /* �ü� */
				case 34:
					mchevl(&l1p, tmpevl);
					*l2p++ = ltov(*tmpevl);
					break;
				case 35:
					*l2p++ = ERRCOD;
					break;
			     /*	case 36: omitted */
				case 37:
					*l2p++ = CHMSIG;
					*l2p++ = LWRMSK(*l1p++);
					*l2p++ = *l1p++; break;
				case 38:
					*l2p++ = CHMSIG;
					*l2p++ = XY2INT(2, LWRMSK(*l1p++));
					*l2p++ = *l1p++; break;
				case 39:
					*l2p++ = CHMSIG;
					*l2p++ = XY2INT(3, LWRMSK(*l1p++));
					*l2p++ = *l1p++; break;
				case 40:
					{
						letter	modnum;

						modnum = LWRMSK(*l1p++);
						if(modesw[modnum] . curmode !=
						   *l1p++)
							*l2p++ = UNUSDC;
						break;
					}
				case 41:
					{
						letter	modnum;

						modnum = LWRMSK(*l1p++);
						if(modesw[modnum] . curmode ==
						   *l1p++)
							*l2p++ = UNUSDC;
						break;
					}
				case 42:
					{
						letter	modnum;

						modnum = LWRMSK(*l1p++);
						if(modesw[modnum] . curmode >=
						   *l1p++)
							*l2p++ = UNUSDC;
						break;
					}
				case 43:
					{
						letter	modnum;

						modnum = LWRMSK(*l1p++);
						if(modesw[modnum] . curmode <=
						   *l1p++)
							*l2p++ = UNUSDC;
						break;
					}
				case 44:
					mchevl(&l1p, tmpevl);
					*l2p++ = SENDCH;
					*l2p++ = *tmpevl;
					break;
				default:/* case 0�ڤӾ嵭�ʳ� */;
					BUGreport(7);
			}
	}

	*l2p = EOLTTR;
	*l1pp = l1p;
}

 /** num���ܤ��ѿ����Ѱ褬ʸ�� l ��ޤफ�ɤ������֤� */
static int
hen_ikisrc(num, l)
int	num;
letter	l;
{
	letter	*defptr;

	defptr = hyo_n[hyonum] . hensudef[num];
	if(*defptr == VARRNG){
		for(defptr++; *defptr != EOLTTR; )
			if(*defptr++ <= l && l <= *defptr++) return(1);
		return(0);
	}

	for(; *defptr != EOLTTR; defptr++){
		if(l == *defptr) return(1);
	}
	return(0);
}

 /**	�Ѵ��Υᥤ��롼�����ܽ�����Ԥ������Ĥ��Ǥ˸�������äƤ��롣
	���ʤߤ��������ϡ�romkan_getc�β��̴ؿ�romkan_henkan����ǡ�
	���δؿ���Ƥ����ˤ�äƤ��롣
	���δؿ��ϡ�romkan_next�����ʸ������٤˸ƤФ�롣�ƤӽФ��줿ľ���
	output�϶�ʸ����disout�ˤ����ϥ����ɰ�ʸ�������äƤ��롣
	���δؿ���������ʸ�����󤬡�romkan_henkan���Ϥꡢ
	romkan_getc�ϡ������ʸ�����Ȥ�ʬ�򤷤��֤���
	(error)�ǥ��顼�������������줿����0���֤������ｪλ����1���֤� */
int
match()
{
	int	henkanflg = 0, okcode = 0, chm_exist;
	letter	*p;
	letter	urabufcreate[KBFSIZ], orgkeybuf[KBFSIZ], kbftail[KBFSIZ];
	letter	*urabufjunbi, *outcutptr, *dis_end;

	if(*keybuf == EOLTTR){
		*urabuf = EOLTTR;
		return(1);
	}

	ltrcpy(urabufjunbi = orgkeybuf, keybuf);
	outcutptr = rk_output;

	while((okcode = henkan_ok()) > 0){
		henkanflg = 1;

		codeout_chg();
		ltrcat(rk_output, codeout);

		ltrcpy(kbftail, keybuf + codein_len);
		ltrcat(ltrcpy(keybuf, remainkbf), kbftail);

		if(okcode == 2){
			ltrcpy(urabufjunbi = urabufcreate, keybuf);
			totail(outcutptr);
		}
	}

	if(okcode == 0){
		ltr1cut(ltrcpy(keybuf, orgkeybuf));
		ltr_to_ltrseq(disout, NISEBP);
		*rk_output = EOLTTR;
		return(0);
	}
	if(henkanflg){
		ltrcpy(urabuf, urabufjunbi);

		set_rubout(disout, ltrlen(orgkeybuf) - 1, nisedl);

		dis_end = disout;
		totail(dis_end);
		ltrcpy(dis_end, rk_output);

		 /* �⡼�ɥ����󥸤�ľ�����Τ餻�뤿�� CHMSIG�����
		    ��flags���RK_CHMOUT��Ω�äƤʤ��ȡ����Ȥǰ�ʸ�����Ĥ�
		    ʬ�򤹤�����ǡ�CHMSIG�򥫥åȡˡ�
		    â����rk_output����ϡ�CHMSIG��ȴ����
		    �ޤ���CHMSIG�������ˣ��󤷤����Ϥ��ʤ�
		    �ʣ���ʾ夢�äƤ⡢����ˤޤȤ�ơ��������֤��ˡ�*/
		for(chm_exist = 0, p = rk_output; ; p++){
			while(*p == CHMSIG){
				chm_exist = 1;
				if(ltrcpy(p, p + 1) < outcutptr) outcutptr--;
			}
			if(*p == EOLTTR) break;
		}
		if(chm_exist){
		 /* CHMSIG�򣱤ĤˤޤȤ᤿��Τ�dis_end�ˤĤʤ�ľ����
		    ����ifʸ�򥫥åȤ���С�CHMSIG�ΤȤ�ޤȤ�Ϥ��ʤ� */
			ltr1cat(ltrcpy(dis_end, rk_output), CHMSIG);
		}

		bitup_ltrcat(disout, keybuf);
		lastoutlen = ltrlen(outcutptr);
		lastkbflen = ltrlen(keybuf);
	}
	return(1);
}

 /**	LTREOF�����ä��Ȥ��ˡ������Ф��褦�˻��ꤵ��Ƥ��뤫Ĵ�٤ơ�
	codeout�򤽤η�̤�ʸ����ʻ��꤬�ʤ��ä����������ˤ˥ݥ���Ȥ��롣
	Ķ�ޤ����館�ǡ��ü쥳�������ϰ���̵�뤹�롣*/
static void
add_at_eof()
{
	register struct dat	*datptr;
	register int	i;
	int	hyoseq;
	letter	evlrsl[RSLMAX], *p;

	for(hyoseq = 0; (hyonum = usehyo[hyoseq]) != -1; hyoseq++){
		for(i = 0, datptr = hyo_n[hyonum] . data;
		    NULL != (p = datptr[i] . code[0]); i++){
			henmatch[0] . ltrmch = EOLTTR;
			while(*p != EOLTTR){
				switch(SHUBET(*p)){
					case 0: /* ʸ�� */
					case 1: /* �ѿ� */
					 /* ����餬������ϡ�NULL�Ȥ�
					    �ޥå������ʤ���*/
						goto Pass;
					case 2: /* �� */
						mchevl(&p, evlrsl);
						if(*evlrsl != EOLTTR)
							goto Pass;
					 /* ���ϥ��������ˡ�ɾ�������
					    ��ʸ����ˤʤ��Τ���¾�ˤϤʤ�
					    ���Ȥ�����*/
				}
			}

			ltrevlcpy(codeout = p = evalbuf[0][0],
				  datptr[i] . code[1]);
			while(*p != EOLTTR){
				if(isSPCL(*p)) ltrcpy(p, p + 1);
				 else p++;
			}
			codeout_chg();
			return;

Pass:			;
		}
	}
	codeout = nil;
}

 /**	���ޥå����ߤ롣���ͤϡ��ޥå����Ƴ��ꤷ�����1�ʥ⡼�ɥ����󥸤�
	�����äƤ������2�ˡ��ޥå�������̤����λ�-1���ޥå����ʤ��ä���0��
	�¹���ϡ��ѿ� l �ˡ�����ޤǤ˰��פ���Ĺ���κǹ⵭Ͽ������Ƥ��ꡢ
	���Ĺ�����פ����Τ����Ĥ��뤴�Ȥˡ�����򹹿����롣l���ͤϡ��ޥå�
	���Ƥ��Ƥ�0�ˤʤ뤳�Ȥ⤢�롣p_eq() �ڤ� prefixp() �����ʸ�򻲾ȡ�*/
static int
henkan_ok()
{
	register struct dat	*datptr;
	register int	i, k;
	int	l, j, hyoseq;
	char	urabuf_clrf; /* �⡼�ɥ����󥸤ʤɤǡ�urabuf�򥯥ꥢ����ɬ�פ�
				���������Ϥ��줬Ω�������η�̡�henkan_ok()
				��1���֤��٤��Ȥ����2���֤�������򸫤ơ�
				match()��urabuf�ʤɤ�Ĵ���򤹤롣*/
	register letter *p;

	if(*keybuf == EOLTTR) return(-1);

	for(l = -1, hyoseq = 0; (hyonum = usehyo[hyoseq]) != -1; hyoseq++){
		for(i = 0, datptr = hyo_n[hyonum] . data;
		    NULL != (p = datptr[i] . code[0]); i++){
			henmatch[0] . ltrmch = EOLTTR;
			switch(k = prefixp(keybuf, p)){
				case -2: break;
				case -1:
					if(eofflg != 0) break;
				 /* eofflg��Ω�äƤ����顢̤����β�ǽ����
				    �ΤƤ롣*/

					return(-1);
				default:
					if(k > l){
						ebf_sw = !ebf_sw;
						for(j = 1; j <= 2; j++){
							ltrevlcpy(
							 evalbuf[ebf_sw][j-1],
							 datptr[i] . code[j]);
						}

						l = k;
					}
			}
		}
	}

	if(l >= 0){
		codein_len = l;
		codeout = evalbuf[ebf_sw][0];
		remainkbf = evalbuf[ebf_sw][1];

		for(urabuf_clrf = 0, p = codeout; *p != EOLTTR;){
			switch(*p){
				case CHMSIG:
					p++;
			 /* codeout��1�Х����ܤ˴ؿ��Υ����å��ȥ⡼���ֹ桢
			    2�Х����ܤ˥⡼�ɤο����֤����äƤ� */
					switch(SHUBET(*p)){
						case 0:
							chgmod((int)*p,
							   (modetyp)*(p + 1));
							break;
						case 1:
							allchgmod((modetyp)
								    *(p + 1));
							break;
						case 2:
							incmod((int)LWRMSK(*p)
							  ,(modetyp)*(p + 1));
							break;
						case 3:
							decmod((int)LWRMSK(*p)
							  ,(modetyp)*(p + 1));
							break;
					}
					ltrcpy(p, p + 2);
					 /* CHMSIG�����Ĥ���1,2�Х�����cut */

					urabuf_clrf = 1;
					break;
				case URBFCL:
				 /* urabuf�Υ��ꥢ������Ū�˻��ꤹ�� */
					urabuf_clrf = 1;
					ltrcpy(p, p + 1);
					break;
				default:
					p++;
			}
		}

		if(*codeout == ERRCOD){
			if(eofflg == 0){
				rk_errstat = 1;
				return(0);
			}

			 /* (error)�Ǥ��äơ�������eofflg��Ω�äƤ��顢keybuf
			    �������ޤǡ����Τޤ޽Ф���*/
			codein_len = ltrlen(keybuf);
			codeout = ltrcpy(evalbuf[ebf_sw][0], keybuf);
			remainkbf = nil;
			rk_errstat = 2;

			return(1);
		}
	   /* (error)��ñ�ȤǤ����񤱤ʤ��Τǡ����顼���ФϤ���ǽ�ʬ��*/

		if(*codeout == REASIG){
			*codeout = (rk_rst() != 0 ? EOLTTR : CHMSIG);
			 /* ��read�ǥ���ä���⡼�ɥ����󥸤����ΤϤ��ʤ� */

			urabuf_clrf = 1;
		}
	   /* ɽ�κ�read��â�������줬�����ä����Ȥ򳰤��Τ餻��Τ�CHMSIG��
	      ���Υ����ɤ��Τ�Τϳ��ؽФʤ���(restart)�ϡ�(error)Ʊ�͡�
	      ñ�ȤǤ����񤱤ʤ��Τǡ����ФϤ���ǽ�ʬ��*/

		return(urabuf_clrf ? 2 : 1);
	}

	 /* ɽ�˸���Ƥ��ʤ������ɤϤ��Τޤ��֤� */

	codein_len = 1;
	*(codeout = oneletter) = *keybuf;
	remainkbf = nil;
	return(1);
}

 /* rk_rst��ǻȤ��ޥ��� */
#define taihi(X, Y, N) {for(i = 0; i < N; i++) X[i] = Y[i];}
#define recov(X, Y, N) taihi(Y, X, N)

 /**	ɽ��ưŪ���ɤ߹��ߤ򤹤롣���ߤ�����ɽ�����������򤷡�����Ʊ��
	�ǥ��쥯�ȥ�ʤˡ����ߤΤȤ������ˤ���ɽ���ɤ߹��ࡣ�⤷��
	�ɤ߹�����˥��顼�򸡽Ф���С���Ȥ������ǡ��������褷��0���֤���*/
static int
rk_rst()
{
	register int	i;
	int	j;

	letter	memoryt[SIZALL];
	struct	dat	datat[LINALL];
	struct	hyo	hyo_nt[HYOMAX];
	letter	*hensudefhyot[VARTOT];
	struct	modestat	modeswt[MODMAX];
	char	hyoshut[HYOMAX];
	char	*modmeibgnt[MODMAX], modmeimem_t[MODMEI];
	char	*dspnambgnt[DMDMAX], dspcod_t[DMDCHR];
	int	usemaehyot[HYOMAX], usehyot[HYOMAX], useatohyot[HYOMAX];
	int	naibu_t[NAIBMX];
	char	*dspmodt[2][2];

	taihi(memoryt,		memory,		SIZALL);
	taihi(datat,		data,		LINALL);
	taihi(hyo_nt,		hyo_n,		HYOMAX);
	taihi(hensudefhyot,	hensudefhyo,	VARTOT);
	taihi(modeswt,		modesw,		MODMAX);
	taihi(hyoshut,		hyoshu,		HYOMAX);
	taihi(modmeibgnt,	modmeibgn,	MODMAX);
	taihi(modmeimem_t,	modmeimem_,	MODMEI);
	taihi(dspnambgnt,	dspnambgn,	DMDMAX);
	taihi(dspcod_t,		dspcod_,	DMDCHR);
	taihi(usemaehyot,	usemaehyo,	HYOMAX);
	taihi(usehyot,		usehyo,		HYOMAX);
	taihi(useatohyot,	useatohyo,	HYOMAX);
	taihi(naibu_t,		naibu_,		NAIBMX);
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)	dspmodt[i][j] = dspmod[i][j];

	if(0 == romkan_init3(prv_modfnm, delchr, nisedl, delchr2,
			     keyin_method, bytcnt_method, kbytcnt_method, 1,
			     flags & ~RK_VERBOS))
		return(0); /* ���ｪλ */

	recov(memoryt,		memory,		SIZALL);
	recov(datat,		data,		LINALL);
	recov(hyo_nt,		hyo_n,		HYOMAX);
	recov(hensudefhyot,	hensudefhyo,	VARTOT);
	recov(modeswt,		modesw,		MODMAX);
	recov(hyoshut,		hyoshu,		HYOMAX);
	recov(modmeibgnt,	modmeibgn,	MODMAX);
	recov(modmeimem_t,	modmeimem_,	MODMEI);
	recov(dspnambgnt,	dspnambgn,	DMDMAX);
	recov(dspcod_t,		dspcod_,	DMDCHR);
	recov(usemaehyot,	usemaehyo,	HYOMAX);
	recov(usehyot,		usehyo,		HYOMAX);
	recov(useatohyot,	useatohyo,	HYOMAX);
	recov(naibu_t,		naibu_,		NAIBMX);
	for(i = 0; i < 2; i++)
		for(j = 0; j < 2; j++)	dspmod[i][j] = dspmodt[i][j];
	return(1);
}

 /** lp2����ɾ����������ʸ�����lp1�˥��ԡ� */
static void
ltrevlcpy(lp1, lp2)
letter	*lp1, *lp2;
{
	while(*lp2 != EOLTTR){
		mchevl(&lp2, lp1);
		totail(lp1);
	}
	*lp1 = EOLTTR;
}

static void
set_rubout(lp, n, del)
 /** lp�� ��del��n�Ĥ���򥻥åȤ��롣������ del �� 'delchr'��'nisedl' */
letter	*lp, del;
int	n;
{
	for(; n; n--) *lp++ = del;
	*lp = EOLTTR;
}

 /** ���줬�¹Ԥ��줿��Х���â���¹ԤϤ��Τޤ�³�� */
void
BUGreport(n)
int	n;
{
	fprintf(stderr, "\r\nromkan-Bug%d!!\r\n", n);
}

 /** ������(mae_in��mae_out)���ϸ����(ato_in��ato_out)��Ԥ���*/
static void
maeato_henkan(in, outp, m_a_hyo)
letter	in;		/* ���Ϥΰ�ʸ�� */
letter	*outp;		/* ���ϤϤ��������� */
int	*m_a_hyo;	/* �ɤ����������ɽ�����򤵤�Ƥ��뤫�ξ��� */
{
	struct	dat	*datptr;
	int	i, hyoseq;
	letter	*curdat;

	if(isSPCL(in)){
	 /* LTREOF��CHMSIG���褦��Τǡ��ü쥳���ɤϤ��Τޤ��֤��褦��
	    �ٹ����Ƥ����ʤ��Ȥ����ʤ���*/
		ltr_to_ltrseq(outp, in);
		return;
	}

	for(hyoseq = 0; (hyonum = m_a_hyo[hyoseq]) != -1; hyoseq++){
		for(i = 0, datptr = hyo_n[hyonum] . data;
		    NULL != (curdat = datptr[i] . code[0]); i++){
			henmatch[0] . ltrmch = EOLTTR;
			if(!l_eq(curdat, in)) continue;

			ltrevlcpy(outp, datptr[i] . code[1]);
			return;
		}
	}
	ltr_to_ltrseq(outp, in);
}

 /** ����� */
static void
codeout_chg()
{
	letter	*saishu_outp;

	*(saishu_outp = saishu_out) = EOLTTR;

	for(; *codeout != EOLTTR; codeout++){
		maeato_henkan(*codeout, saishu_outp, useatohyo);
		totail(saishu_outp);
	}

	codeout = saishu_out;
}

 /** ��ʸ���ץå��塦�Хå� */
letter	romkan_ungetc(l)
letter	l;
{
	return(ungetc_buf = l);
}

 /** romkan_next���Ф���ʸ���ץå��塦�Хå� */
letter	romkan_unnext(l)
letter	l;
{
	return(unnext_buf = l);
}

 /**	delete�Ȥ��ƻȤ�����饯��������ʵ�delete��ˡ������¹Ը��
	romkan_clear��¹Ԥ��Ƥ����ʤ��Ⱥ���Τ�Ȥˤʤ�ޤ���*/
/*	���ѻߡ�
void
romkan_setdel(delchr_, nisedl_)
letter	delchr_, nisedl_;
{
	delchr = delchr_;
	nisedl = nisedl_;
}
*/
