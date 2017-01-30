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
/*
	Nihongo	Henkan	Library Header File
*/
#ifndef	JSLIB
#ifndef	_WNN_SETJMP
#define	_WNN_SETJMP
#include <setjmp.h>
#endif

#define	JLIB_VERSION	0x4003

#ifndef	w_char
#define	w_char	unsigned short
#endif	/* w_char */

#define JSLIB

/* ��ü�٥���(�ʻ��ֹ�)����� */
#define	WNN_BUN_SENTOU	-1	/* ��ʸ�����Ƭ�ˤʤ���� */
#define	WNN_ALL_HINSI	-2	/* �ʤ�Ǥ⤨���� */
/* ��ü�٥�������� */
#define	WNN_VECT_KANREN	0	/* Ϣʸ���Ѵ��ν�ü�٥����� */
#define	WNN_VECT_KANTAN	1	/* ñʸ���Ѵ��ν�ü�٥����� */
#define	WNN_VECT_KANZEN	1	/* �������Ф��ν�ü�٥����� 
				   ñʸ���Ѵ����Τ�Τ���������*/ 
#define WNN_VECT_BUNSETSU 2     /* Ϣʸ���Ѵ����Ρ���ʸ��ν�ü�٥�����*/
#define	WNN_VECT_NO	-1	/* ��ü�٥���1 ̵�� */
/* ʸ��ν�ü����³���� */
#define	WNN_CONNECT_BK	1	/* ���ꤵ�줿��ü�٥���������³�Ǥ�����*/
#define	WNN_NOT_CONNECT_BK	0	/* */
/* ʸ�����ü����³���� */
#define	WNN_CONNECT	1	/* ���ꤵ�줿(�ʻ졢��°��)����³�Ǥ�����*/
#define	WNN_SENTOU	2	/* ��ʸ�����Ƭ */
#define	WNN_NOT_CONNECT	3	/* ��Ƭ�ˤʤ�ʤ�����������³�Ǥ��ʤ� */
#define	WNN_GIJI	4	/* ����ʸ����ä���*/

/* ����ʸ���ľ�������򤷤����� */
#define WNN_HIRAGANA	-1	/* �Ҥ餬�� *//* �ɤߤΤޤ� */
#define WNN_KATAKANA	-11	/* �������� */
/* ���� */
#define	WNN_NUM_HAN	-2	/* Ⱦ�ѿ��� *//* �ɤߤΤޤ� */
#define	WNN_NUM_ZEN	-12	/* ���ѿ��� *//* ������ */
#define	WNN_NUM_KAN	-13	/* ������ *//* ���� */
#define	WNN_NUM_KANSUUJI -15	/* ������ *//* ɴ�󽽻� */
#define	WNN_NUM_KANOLD	-16	/* ������ *//* ��ɴ������ */
#define	WNN_NUM_HANCAN	-17	/* Ⱦ�ѿ��� *//* 1,234 */
#define	WNN_NUM_ZENCAN	-18	/* ���ѿ��� *//* ���������� */
/* �ѿ� */
#define	WNN_ALP_HAN	-4	/* Ⱦ�� *//* �ɤߤΤޤ� */
#define	WNN_ALP_ZEN	-30	/* ���� */
/* ���� */
#define	WNN_KIG_HAN	-5	/* Ⱦ�� *//* �ɤߤΤޤ� */
#define	WNN_KIG_JIS	-40	/* ����(JIS) */
#define	WNN_KIG_ASC	-41	/* ����(ASC) */

#define WNN_FT_DICT_FILE 1
#define WNN_FT_HINDO_FILE 2
#define WNN_FT_FUZOKUGO_FILE 3

#define WNN_STATIC_DICT 1		/* static dict */
#define WNN_UD_DICT 2			/* updatable dict */
#define WNN_REV_DICT 3		/* Updatable Dictonary with Reverse Index */
#define WNN_REG_DICT 4		/* Regular Expressino Dictonary */

/* for CHINESE PIN-IN : with Si Sheng (����) */
#define CWNN_REV_DICT		0x103	/* Updatable with Reverse Index */

/* for CHINESE: Bi Xing (ɮ��) */
#define BWNN_REV_DICT		0x203	/* Updatable with Reverse Index */

#define	WNN_DIC_RDONLY	1
#define	WNN_DIC_RW	0

#define	WNN_DIC_ADD_REV 1	/* ���Ѵ� */
#define	WNN_DIC_ADD_NOR 0	/* �̾���Ѵ� */

/* header file for dic_syurui */
#define WNN_FILE_NOT_EXIST -1
#define WNN_NOT_A_JISHO 0
/* KURI
#define WNN_USER_DIC 1
#define WNN_SYSTEM_DIC 3
*/
#define WNN_HINDO_FILE 4
#define WNN_FUZOKUGO_FILE 5

/* set_hindo operation */
#define WNN_HINDO_NOP -2
#define WNN_IMA_ON -3
#define WNN_IMA_OFF -4
#define WNN_HINDO_INC -3
#define WNN_HINDO_DECL -4
#define WNN_ENTRY_NO_USE -1

/*	Wnn constant
*/
#define WNN_HOSTLEN 16
#define WNN_ENVNAME_LEN 32

/*	js_who	*/
#define WNN_MAX_ENV_OF_A_CLIENT 32

struct wnn_jwho {
	int sd;   /* jserver ��Υ����åȥǥ�������ץ�*/
	char user_name[64];   /* �桼��̾ */
	char host_name[64];   /* �ۥ���̾ */
	int  env[WNN_MAX_ENV_OF_A_CLIENT];  /* ���Υ��饤����Ȥ����Ѥ��Ƥ���
				�Ķ��ֹ���󡣶����Ƥ���Ȥ���ˤϡ�-1 ������ */
};
typedef struct wnn_jwho WNN_JWHO;

/*	js_env_list	*/
#define WNN_MAX_JISHO_OF_AN_ENV 30
#define WNN_MAX_FILE_OF_AN_ENV 60

 struct wnn_env_info {
        int	env_id;		/* �Ķ��ֹ� */
	char	env_name[WNN_ENVNAME_LEN]; /* �Ķ�̾ */
	int	ref_count;	/* ���ȿ� */
	/* struct wnn_param; */
	int	fzk_fid;	/* ��°��Υե������ֹ� */
	int jishomax;		/* ���Ѥ��Ƥ��뼭��θĿ� */
	int	jisho[WNN_MAX_JISHO_OF_AN_ENV];	/* ���Ѥ��Ƥ��뼭��μ����ֹ档
					   �ǽ��jishomax�Ĥ�����̣������ */
	int	file[WNN_MAX_FILE_OF_AN_ENV]; /* ���δĶ��ˤĤʤ��äƤ���
				 �ե������ֹ�(�����Ƥ���Ȥ���ˤϡ�-1 ������) */
};
typedef struct wnn_env_info WNN_ENV_INFO;

#define WNN_COMMENT_LEN 512     /* jisho no comment no nagasa */
#define WNN_F_NAMELEN 100
#define WNN_PASSWD_LEN 16  /* File Passwd Length */


struct wnn_dic_info {
	int dic_no;		/* �����ֹ� */
	int body;		/* �������ΤΥե������ֹ� */
	int hindo;		/* ���٤Υե������ֹ� */
	int rw;			/* ������Ͽ��ǽ���ɤ���
				   (WNN_DIC_RW, WNN_DIC_RDONLY) */
	int hindo_rw;		/* ���٤�������ǽ���ɤ���
				   (WNN_DIC_RW, WNN_DIC_RDONLY) */
	int enablef;		/* ���񤬻����椫�ɤ���
				   (1 = ������, 0 = ��������) */
	int nice;		/* ������Ѵ�����ͥ���� */
	int rev;		/* ���Ѵ������ɤ���(1 = ���Ѵ�, 0 = ���Ѵ�) */
/* added H.T */
	w_char comment[WNN_COMMENT_LEN];	/* ����Υ����� */
	char fname[WNN_F_NAMELEN]; /* ����Υե�����̾ */
	char hfname[WNN_F_NAMELEN]; /* ���٤Υե�����̾ */
	char passwd[WNN_PASSWD_LEN]; /* ����Υѥ���� */
	char hpasswd[WNN_PASSWD_LEN]; /* ���٤Υѥ���� */
	int type;		/* ����μ���(WNN_UD_DICT,WNN_STATIC_DICT) */
	int gosuu;		/* ����θ�� */
	int localf;
	int hlocalf;
};

typedef struct wnn_dic_info WNN_DIC_INFO;

struct wnn_file_stat {
    int type;			/* �ե�����μ���
		WNN_STATIC_DICT		�����������
		WNN_UD_DICT		��Ͽ��ǽ��������
		WNN_HINDO_FILE		���٥ե�����
		WNN_FUZOKUGO_FILE	��°��ե�����
		WNN_NOT_A_JISHO		����¾   */
};

typedef struct wnn_file_stat WNN_FILE_STAT;

/*	*/
extern	int	wnn_errorno;		/* Wnn�Υ��顼�Ϥ����ѿ�����𤵤�� */

extern char *wnn_dic_types[];	/* "����","��Ͽ","���Ѵ�","����" */

extern char *cwnn_dic_types[];	/* "����","��Ͽ","���Ѵ�","����" */
extern char *bwnn_dic_types[];	/* "����","��Ͽ","���Ѵ�","����" */

#define FILE_ALREADY_READ -2

/* ���ι�¤�Τϡ��饤�֥���������Ѥ����� */
struct wnn_jserver_id {
	int	sd;
	char	js_name[40];
	int	js_dead;
	jmp_buf js_dead_env;	/* �����Ф�������������Ǥ���env */
	int	js_dead_env_flg; /* jd_server_dead_env��ͭ�����ݤ�  */
};

typedef struct wnn_jserver_id WNN_JSERVER_ID;

/* ���ι�¤�Τϡ��饤�֥���������Ѥ����� */
struct wnn_env {
	int		env_id;
	WNN_JSERVER_ID	*js_id;
	char		lang[16];	/* for exsample "ja_JP" */
};

typedef struct wnn_env WNN_ENV;

struct wnn_param {
	int	n;	/* ��(��)ʸ����ϤΣ� */
	int	nsho;	/* ��ʸ����ξ�ʸ��κ���� */
	int 	p1;	/* ��Ω������٤Υѥ�᡼�� */
	int 	p2;	/* ��ʸ��Ĺ�Υѥ�᡼�� */
	int 	p3;	/* ��Ω��Ĺ�Υѥ�᡼�� */
	int 	p4;	/* ���Ȥä���ӥåȤΥѥ�᡼�� */
	int 	p5;	/* ����Υѥ�᡼�� */
	int	p6;	/* ��ʸ���ɾ���ͤΥѥ�᡼�� */
	int	p7;	/* ��ʸ��Ĺ�Υѥ�᡼�� */
	int	p8;	/* ��ʸ����Υѥ�᡼�� */
	int 	p9;	/* �����ʻ� ���������� */
	int 	p10;	/* �����ʻ� ���ʤ����� *//* CWNN:�ѿ������� */
	int 	p11;	/* �����ʻ� �ѿ������� *//* CWNN:��������� */
	int 	p12;	/* �����ʻ� ��������� *//* CWNN:����̤����� */
	int 	p13;	/* �����ʻ� �ĳ�̤����� *//* CWNN:�ĳ�̤����� */
	int 	p14;	/* �����ʻ� ��°������� *//* BWNN:No of koho */
	int 	p15;	/* �����ʻ� ����̤����� *//* CWNN:Not used */
};

#ifdef nodef
struct wnn_dic_info_struct {
	int	body_fid;
	int	hindo_fid;
	int	enablef;
	int	rw;
	int     hindo_rw;
	int	nice;
};

typedef struct wnn_dic_info_struct WNN_DIC_INFO_STRUCT;
#endif

struct wnn_file_info_struct {
	int	fid;		/* �ե������ֹ� */
	char	name[WNN_F_NAMELEN]; /* �ե�����̾ */
	int	localf;		/* �����С������ȤΥե����뤫�ɤ���
				   1: �����С�������
				   0: ���饤����ȡ�������   */
	int	type;		/* �ե�����μ��� */
	int	ref_count;	/* (�Ķ������)���ȿ� */
};

typedef struct wnn_file_info_struct WNN_FILE_INFO_STRUCT;

#define WNN_VECT_L	((256+8*4-1)/(8*4) + 5)	/***** !!!!! ****/

struct	wnn_sho_bunsetsu {
	int	end;		/* ����ʸ��� end char index */
	int	start;		/* ����ʸ��� top char index */
	int	jiriend;	/* ����ʸ��μ�Ω�� end char index */
	int	dic_no;		/* ��Ω��μ�����Υ���ȥ��ֹ� */
	int	entry;		/* ����ʸ��μ�Ω�켭�� entry */

	int	hinsi;		/* ��Ω���ʻ� */
	int     status;		/* ��ʸ�����Ƭ�� */
	int	status_bkwd;	/* usiro ��ʸ�����³�Ǥ��뤫 */
	int 	hindo;		/* ���伫Ω������� */ 
	int 	ima;		/* ���伫Ω��κ��Ȥä���ӥå� */ 
	int	kangovect;	/* ��³�٥��ȥ�ơ��֥�ؤΥݥ��� */
	int 	hyoka;		/* ��ʸ��ɾ���� */
	w_char	*kanji;		/* ��Ω��ʸ���� */
	w_char	*yomi;		/* ��Ω����ɤ�ʸ���� */
	w_char	*fuzoku;	/* ��°��ʸ���� */
/*
 *���٤ˤĤ��Ƥϡ����٥ե����뤬���ꤵ��Ƥ�����ϡ�
 *hindo = ���٥ե������(��)������ + �������(��)������
 *ima = ���٥ե�����κ��Ȥä���ӥå�
 *
 *���٥ե����뤬���ꤵ��Ƥ��ʤ����ˤϡ�
 *hindo = �������(��)�����͡�ima = ������κ��Ȥä���ӥå�
 *�Ǥ��롣�����ǡ��������ͤȤϡ��׻������7�ӥåȤ˰��̤��줿�ͤǤ��롣
 *���������ͤǤϤʤ��������ͤ��֤��Τϡ��Ѵ���̤ΥǥХå��Τ���Ǥ��롣
 */
};

struct	wnn_dai_bunsetsu {
	int	end;		/* ����ʸ��� end char index */
	int	start;		/* ����ʸ��� top char index */
	struct	wnn_sho_bunsetsu	*sbn;	/* ��ʸ����Ϸ�̤ؤΥݥ��� */
	int 	hyoka;		/* ��ʸ��ɾ���� */
	int	sbncnt;		/* ��ʸ��� (������ξ��ϡ��������)
				   DSD_SBN�ϡ�*sbn ���� sbncnt �������� */
};

struct	wnn_jdata {
	int	dic_no;		/* �����ֹ� */
	int	serial;		/* ������Υ���ȥ��ֹ� */
	int	hinshi;		/* �ʻ��ֹ�(�ʻ��ֹ���ʻ�̾���б��ϡ�manual/etc
				   �β��򻲾�) */
	int	hindo;		/* ���� */
	int	ima;		/* ���Ȥä���ӥå� */
	int	int_hindo;	/* ���������� */
	int	int_ima;	/* �����⡢���Ȥä���ӥå� */
	w_char   *yomi;		/* �ɤߤ�ʸ���� */
	w_char	*kanji;		/* ����ʸ���� */
	w_char	*com;		/* ����ȥ�Υ�����ʸ���� */
/*
 *���٤ˤĤ��Ƥϡ����٥ե����뤬���ꤵ��Ƥ�����ϡ�
 *hindo = ���٥ե������(����)�����͡�ima = ���٥ե�����κ��Ȥä���ӥå�
 *int_hindo = �������(����)�����͡�int_ima = ������κ��Ȥä���ӥå�
 *
 *���٥ե����뤬���ꤵ��Ƥ��ʤ����ˤϡ�
 *hindo = �������(����)�����͡�ima = ������κ��Ȥä���ӥå�
 *int_hindo = -1��int_ima = -1
 *
 *���������ɤ���ξ��Ǥ⡢����ȥ꤬������ߤξ��֤λ��ˤϡ�
 *hindo = -1, ima = 0 ���뤤�ϡ�
 *int_hindo = -1, int_ima = 0 �Ȥʤ롣
 *�����ǡ�(����)�����ͤȤϡ��׻������7�ӥåȤ˰��̤��줿�����ͤ��顢
 *�ºݤ��ͤ����ꤷ���ͤǤ��롣
 */
};

struct	wnn_ret_buf {
	int	size;		/* buf ���� alloc ����Ƥ����礭�� */
	char	*buf;		/* malloc �ʤɤǤȤ�줿�ΰ� */
};


#define	WNN_F_UNIQ_LEN	(sizeof(struct wnn_file_uniq))

/* ���ι�¤�Τϡ��饤�֥���������Ѥ����� */
struct wnn_file_head {
  struct wnn_file_uniq{
    int time;
    int dev;
    int inode;
    char createhost[WNN_HOSTLEN];
  } file_uniq;
  struct wnn_file_uniq file_uniq_org;
  int file_type;
  char file_passwd[WNN_PASSWD_LEN];
};

#define WNN_HINSI_NAME_LEN 64

#define	WNN_FILE_HEADER_LEN	(WNN_PASSWD_LEN + 8 + WNN_FILE_STRING_LEN + \
				 sizeof(struct wnn_file_uniq) * 2 + 4)
				/*
				  8 is for future use 
				  4 is for file_type.
				 */

#ifndef	JSERVER
/*
  JSLIB function declaration
*/
#define	js_open(server, timeout)	js_open_lang(server, "ja_JP", timeout)
#define	js_connect(server,env_name)	js_connect_lang(server, env_name, "ja_JP")

extern	WNN_JSERVER_ID *js_open_lang();
extern	int		js_close();
extern	WNN_JSERVER_ID *js_change_current_jserver();
extern	struct wnn_env *js_connect_lang();
extern	int		js_disconnect();
extern	int		js_env_list();
extern	int		js_param_set();
extern	int		js_param_get();
extern	char	       *js_get_lang();
/**************************************/
extern int js_access();
extern int js_dic_add();
extern int js_dic_delete();
extern int js_dic_file_create();
extern int js_dic_file_create_client();
extern int js_dic_info();
extern int js_dic_list();
extern int js_dic_list_all();
extern int js_dic_use();
extern int js_env_exist();
extern int js_env_sticky();
extern int js_env_un_sticky();
extern int js_file_comment_set();
extern int js_file_discard();
extern int js_file_info();
extern int js_file_list();
extern int js_file_list_all();
extern int js_file_loaded();
extern int js_file_loaded_local();
extern int js_file_password_set();
extern int js_file_read();
extern int js_file_receive();
extern int js_file_remove();
extern int js_file_remove_client();
extern int js_file_send();
extern int js_file_stat();
extern int js_file_write();
extern void js_flush();
extern int js_fuzokugo_get();
extern int js_fuzokugo_set();
extern int js_hindo_file_create();
extern int js_hindo_file_create_client();
extern int js_hindo_set();
extern int js_hinsi_dicts();
extern int js_hinsi_list();
extern int js_hinsi_name();
extern int js_hinsi_number();
extern int js_hinsi_table_set();
extern int js_isconnect();
extern int js_kanren();
extern int js_kantan_dai();
extern int js_kantan_sho();
extern int js_kanzen_dai();
extern int js_kanzen_sho();
extern int js_kill();
extern int js_mkdir();
extern int js_version();
extern int js_who();
extern int js_word_add();
extern int js_word_comment_set();
extern int js_word_delete();
extern int js_word_info();
extern int js_word_search();
extern int js_word_search_by_env();

#endif
#endif	/* JSLIB */
