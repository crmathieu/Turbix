#include <stdio.h>
#include <dos.h>
#include <dir.h>

/* <<<<<<<<<<<<<<<<<<<<<  VARIABLES D'IMPORTATION >>>>>>>>>>>>>>>>>>>>>> */
#include "xed.h"
#include "emsg.h"

#ifdef TURBO
#include <fcntl.h>
#include <process.h>
#define _dos_setvect setvect
#define _dos_getvect getvect
#endif

struct OBJ *messwin;
int Xmess, Ymess;
#define nullstr ""

unsigned SEGvideo;
int adapter;

#define ERRMSG 4
#define NMXMSG 3

#define HBYTE  0xff00
#define LBYTE  0x00ff

#define TRUE          1
#define FALSE         0

struct configuration conf;

/*  zone de reecriture des commandes si le fichier
 *  de configuration existe
 */

struct stat entry;

struct OBJ *wm, wmake;
char work[80];
unsigned char boxtype[6][11] = {{'Ú','¿','À','Ù','³','Ä','Ã','´','Â','Á','Å'},   /* type 1 */
                                {'Õ','¸','Ô','¾','³','Í','Æ','µ','Ñ','Ï','Ø'},   /* type 2 */
                                {'Ö','·','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'},   /* type 3 */
                                {'É','»','È','¼','º','Í','Ì','¹','Ë','Ê','Î'},   /* type 4 */
                                {'°','°','°','°','°','°','°','°','°','°','°'},   /* type 5 */
                                {'Ç','¶','Ó','½','º','Ä','Ç','¶','Ò','Ğ','×'}};  /* type 6 : STATUS */



/* <<<<<<<<<<<<  Partie propre a l'installation >>>>>>>>>>>>>> */

char *sourceLabel[] = {"a:\\*.*",
		       "b:\\*.*"
};

unsigned char repNormale[] = {'\x0d', 'O', 'o', 0};
unsigned char repValide[]  = {'\x0d', 'O', 'o', 0};

char okStr[]   = " VALIDER ....... ";

char *source[]   = {"[A]", "[B]"};
char *target[]   = {"[A]", "[B]", "[C]", "[D]"};
char *targetR[]  = {"A", "B", "C", "D"};
char deuxPoints[] = ":\\";

int   srcDrv = 0;
int   tarDrv = 2;
int   yesNo  = 0;

char  *yesNoStr[] = {"[O]", "[N]"};

/***********************************************************************\
*  1ere disquette : label 	:	NOYAU                     	*
*		    directorie  :	nmx\noyau                 	*
*		    			fichiers :			*
*							nmx_sml.lib	*
*							nmx_win.lib	*
*							nmx_shx.lib  	*
*							*.h		*
*                                                                 	*
*                                                                 	*
*  2eme disquette : label	:	NDI                       	*
*		    directories :	nmx\ndi                   	*
*					fichiers :                	*
*							ndi.exe   	*
*							nmxed.exe 	*
*							nmxmke.exe	*
*							nmxcmp.exe	*
*							nmxlnk.exe	*
*                                                                 	*
*					nmx\ndi\aide              	*
*					fichiers :                	*
*							*.hlp     	*
*                                                                 	*
\***********************************************************************/


/*
 *  Pour le transfer des fichiers (step3) , prendre une string sur 2
 */
char *tarDir[]  =    {":\\mkd",		     /* directorie nmx */
		      ":\\mkd\\noyau",       /* directorie des librairies & include */
		      ":\\mkd\\mdi",         /* directorie de l'editeur */
		      ":\\mkd\\mdi\\aide",   /* help editeur */
		      "",
		      ""
};

char *disquettesLabel[] = {"NOYAU",
			   "MDI",
			   ""
};

char razline[]    = "                                           ";
char step0Msg[]   = " Installation de N.M.X. -- Appuyer sur une touche ... ";
char step1Msg[]   = " D‚finition des Drives source et destination ";
char step2Msg[]   = " Cr‚ation des r‚pertoires N.M.X. ";
char step3Msg[]   = " Copie des fichiers ... ";
char stepfMsg[]   = " Installation termin‚e Normalement - appuyer sur une touche ";
char stepfErrMsg[]= " Fin anormale de l'installation ... ";
char chgOpt[]     = " Pressez return pour valider l'option ";
char defMsg1[]    = " Source ........ ";
char defMsg2[]    = " Destination ... ";
char creaDir[]    = " Cr‚ation r‚pertoire    ";
char impCreaDir[] = " Impossible de cr‚er ce r‚pertoire ... ";
char impCopy[]    = " Echec - Ajouter le chemin d'acc‚s aux commandes DOS dans PATH ";
char badLabelStr[]= " Label incorrect - ins‚rer la disquette ";
char insDiskStr[] = " Ins‚rer la disquette : ";

char work[80];

struct OBJ *WP, wint10;
char *ARGV[5];
unsigned char int10Buffer[80];
int M_in = 0;

/* <<<<<<<<<<<<<<<<   Fonctions de l'installation  >>>>>>>>>>>>>>>>>>>>>> */

/*---------------
 * get_string
 *---------------
 */
get_string(str)
char *str;
{

}

/*---------------
 * getlabel
 *---------------
 */
getlabel(lab)
char *lab;
{
 int          i, ret;
 union  REGS  rin,rout;
 struct SREGS segreg;
 struct DTA   dta;
 struct DTA  *currdta, *pdta;

 pdta = &dta;

 /* recuperer la DTA courante */
 rin.h.ah  = 0x2f ;
 int86x(0x21, &rin, &rout,&segreg);
 (unsigned long)currdta = segreg.es;
 (unsigned long)currdta = (unsigned long)currdta << 16;
 (unsigned long)currdta = (unsigned long)currdta | rout.x.bx;

 /* initialiser la DTA */
 rin.h.ah  = 0x1a ;
 rin.x.dx  = FP_OFF(pdta) ;
 segreg.ds = FP_SEG(pdta);
 int86x(0x21, &rin, &rout,&segreg);

 /* amorcer la recherche */
 ret = 0;
 rin.h.ah  = 0x4e;
 rin.x.cx  = 0x08;    /* attribut LABEL */
 rin.x.dx  = FP_OFF(sourceLabel[srcDrv]);
 segreg.ds = FP_SEG(sourceLabel[srcDrv]);
 int86x(0x21, &rin , &rout,&segreg) ;
 if (rout.x.cflag)
	ret = -1; /* erreur */
 else
	if (strncmpi(dta.fname, lab, 11) != 0)
		ret = -1;

 /* remettre la DTA par defaut */
 rin.h.ah  = 0x1a ;
 rin.x.dx  = FP_OFF(currdta) ;
 segreg.ds = FP_SEG(currdta);
 int86x(0x21, &rin, &rout,&segreg);
 return(ret);
}

/* <<<<<<<<<<<<<<  STEPS >>>>>>>>>>>>>>> */

/*---------------
 * STEP0
 *---------------
 */
step0() /* banniere */
{
   hide_cursor();
   _clrwin(0, 0, 79, 24, F_LYELLOW|B_BLUE, '°');
   pushPop_mess(step0Msg, 10, NMXMSG);
}

/*---------------
 * STEP1
 *---------------
 */
step1() /* definir drive source et destination */
{
	char *pt;
	struct OBJ ob, *wp;
	int size;
	unsigned inkAtt, paperAtt, borderAtt, ATT;


   yesNo = 1;
   wp  = &ob;
   size = strlen(okStr)+3;
   inkAtt = F_LYELLOW;
   paperAtt = B_BLUE;
   borderAtt = inkAtt|paperAtt;
   ATT = 0;
   initWin(wp, step1Msg, 10, 8, 60, 12, TRUE, FALSE, inkAtt, paperAtt, borderAtt, ATT, W_WIN);
   winPopUp(wp, W_OPEN, NMXMSG);
   while (yesNo) {
	clrwin(wp, 0x20);
	print_at(wp, 9, 1, chgOpt, OLDATT);

	while (getYesNo(wp, defMsg1, source[srcDrv], 5, 3))
		srcDrv = (srcDrv + 1) % 2;

	while (getYesNo(wp, defMsg2, target[tarDrv], 5, 5))
		tarDrv = (tarDrv + 1) % 4;

	while (getYesNo(wp, okStr, yesNoStr[yesNo], (60-size)/2, 8))
		yesNo = (yesNo + 1) % 2;

   }
   winPopUp(wp, W_CLOSE);
   hide_cursor();
}

/*---------------
 * STEP2
 *---------------
 */
step2() /* cr‚er les directories NMX */
{
	char w[3];
	struct OBJ ob, ob2, *wp, *wp2;
	unsigned inkAtt, paperAtt, borderAtt, ATT;
	int ret, i, j;
	char aux[80];


   wp  = &ob;
   wp2 = &ob2;

   inkAtt = F_WHITE;
   paperAtt = B_BLUE;
   borderAtt = inkAtt|paperAtt;
   ATT = 0;
   initWin(wp, step2Msg, 10, 8, 60, 10, TRUE, FALSE, inkAtt, paperAtt, borderAtt, ATT, W_WIN);
   winPopUp(wp, W_OPEN, NMXMSG);

   ret = 0;
   strcpy(w, targetR[tarDrv]);

   for (j=0, i=2; strlen(tarDir[j]) > 0; j++, i++) {
	strcpy(work, w);
	strcat(work, tarDir[j]);
	strupr(work);
	strcpy(aux, creaDir);
	strcat(aux, work);
	print_at(wp, 3, i, aux, OLDATT);
	if (mkdir(work)) {
		ret = -1;
		pushPop_mess(impCreaDir, 3, ERRMSG);
		break;
	}
   }
   return(ret);
}

/*---------------
 * STEP3
 *---------------
 */
step3() /* transfert des fichiers */
{
	char w[3];
	unsigned inkAtt, paperAtt, borderAtt, ATT;
	int ret, j;
	extern void interrupt it_10();
	unsigned long DosInt10;
	char aux[80];


   inkAtt = F_WHITE;
   paperAtt = B_BLUE;
   borderAtt = inkAtt|paperAtt;
   ATT = 0;
   initWin(WP, step3Msg, 10, 8, 60, 3, TRUE, FALSE, inkAtt, paperAtt, borderAtt, ATT, W_WIN);


   ret = 0;
   strcpy(w, targetR[tarDrv]);

   for (j=0;strlen(disquettesLabel[j]) > 0; j++) {
	strcpy(work, w);
	strcat(work, deuxPoints);
	strupr(work);             /* work = drive cible */

	strcpy(aux, insDiskStr);
	strcat(aux, disquettesLabel[j]);
	strcat(aux, " ");

	/* inserer disquette xxx */
	pushPop_mess(aux, 3, NMXMSG);

	while (getlabel(disquettesLabel[j])) { /* label incorrect */
		/* disquette xxx : label incorrect */
		pushPop_mess(badLabelStr, 3, ERRMSG);
		continue;
	}

	winPopUp(WP, W_OPEN, NMXMSG);
	hide_cursor();  /* 25 ieme ligne */

	ARGV[0] = "xcopy.exe";
	ARGV[1] = sourceLabel[srcDrv];
	ARGV[2] = work;
	ARGV[3] = "/s";

	/* redefinir l'int10 */
	DosInt10 = _dos_getvect(0x10);
	_dos_setvect(0x10, it_10);
	if (spawnvp(P_WAIT, "xcopy.exe", ARGV)) {
		pushPop_mess(impCopy, 3, ERRMSG);
		_dos_setvect(0x10, DosInt10);
		winPopUp(WP, W_CLOSE);
		break;
	}
	/* restore int10 */
	_dos_setvect(0x10, DosInt10);
	winPopUp(WP, W_CLOSE);
/*	return; /* Fin provisoire (normalement enlever le RETURN) */
   }
   return(ret);

}

/*---------------
 * STEP_final
 *---------------
 */
step_final()
{
   hide_cursor();
   pushPop_mess(stepfMsg, 10, NMXMSG);
   clrscr();
   gotoxy(0,0);
}

/*---------------
 * STEP_final_erreur
 *---------------
 */
step_final_erreur()
{
   hide_cursor();
   pushPop_mess(stepfErrMsg, 10, ERRMSG);
   clrscr();
   gotoxy(0,0);
}

/*----------------------------
 * getYesNo  si oui retourne 0
 *	     si non retourne 1
 *----------------------------
 */
getYesNo(wp, msg, param, x, y)
struct OBJ *wp;
char *msg, *param;
{
   int reponse;
   strcpy(work, msg);
   strcat(work, param);
   print_at(wp, x, y, work, OLDATT);
   wgotoxy(wp, x + strlen(work) - 1, y);
   reponse = (getkey() & 0xff);
   if ((char)reponse == 0x0d || reponse == 'O' || reponse == 'o')
	return(0);
   return(1);
}

/**********************************************************************/


/*-------------------------------------------------------
 *      M       M      MM      M   N     N
 *      M M   M M     M  M     M   N N   N
 *      M   M   M    M MM M    M   N   N N
 *      M       M   M      M   M   N     N
 *-------------------------------------------------------
 */
main()
{
   void interrupt Dummy_interrupt();
   struct OBJ errwin, *wmm, message, msgwin;

   int bidon;

   WP = &wint10;

   /* init video mode */
   set_mode_base();

   /* inhiber le ^C */
   _dos_setvect(0x23, Dummy_interrupt);


   wm           = &message;
   wmm          = &msgwin;

   step0();
   step1();
   if (step2()) {
	step_final_erreur();
	exit(0);
   }
   step3();
   step_final();

}

/*----------------------------------------------------
 * affiche -
 *----------------------------------------------------
 */
affiche(str, msgtyp)
char *str;
{
    struct OBJ errwin;

    pushPop_MESS(str, 10, msgtyp);
    winPopUp(wm, W_CLOSE);  /* supprimer cadre compilation */
}


/*----------------------------------------------------
 * dummy_interrupt
 *----------------------------------------------------
 */
void interrupt Dummy_interrupt()
{
}

it_mouse()
{
}


/**********************
 * IMPORTATIONS
 **********************/

/*---------------------------------------------------------------------------
 * verify_window - gestion apparition des messages systemes
 *---------------------------------------------------------------------------
 */
verify_window(wp, y, str, titre, type)
struct OBJ *wp;
char *str, *titre;         /* message a ecrire et titre */
{
    int size;
    unsigned inkAtt, paperAtt, borderAtt, messAtt, ATT;

        if (type == ERRMSG) {
		inkAtt    = F_LWHITE;
		paperAtt  = B_RED;
		borderAtt = inkAtt|paperAtt;

        }
        else {
		inkAtt    = F_BLACK;
		paperAtt  = B_WHITE;
		borderAtt = inkAtt|paperAtt;
        }

	messAtt   = inkAtt|paperAtt;
	ATT	  = 0;
        if ((size = strlen(str)) > 78)
                size = 76;
	initWin(wp, titre, (80 - size)/2, y, size, 3, TRUE, FALSE, inkAtt, paperAtt, borderAtt, ATT, W_WIN);
	winPopUp(wp, W_OPEN, 1);
        messwin = wp;
        Xmess = 0;
        Ymess = 1;
        print_at(messwin, Xmess, Ymess, str, NEWATT, messAtt);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * winPopUp - ouvrir / fermer une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
winPopUp(wp, mode, borderType)
struct OBJ *wp;
{
    if (mode == W_OPEN) {
        /* allouer buffer de sauvegarde des lignes ecrasees par le sous menu */
        if (wp->save == NULL) /* check presence tampon */
            if ((wp->save = malloc(2 * (wp->ncol + 4) * (wp->nline + 3))) == NULL)
                 exit(0);  /* !!!!!!!!!!!!!!! ?????????? */

        wp->pushed = TRUE;
        saveScreen(wp, WINDOW);
        write_wbox(wp, borderType);
        clrwin(wp, BLANK);
    }
    else {
        wp->pushed = FALSE;
        if (wp->save != NULL) {
            restoreScreen(wp, WINDOW);
            free(wp->save);
            wp->save = NULL;
        }
    }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * clrwin Ä clear window
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION clrwin(wp, Ch)
struct OBJ *wp;
unsigned char Ch;
{
       int i;
       _clrwin(   wp->ul_x+1,
                  wp->ul_y+1,
                  wp->ul_x + wp->ncol,
                  wp->ul_y + wp->nline,
                  wp->ink|wp->paper,
                  Ch);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_ombrage - dessiner l'ombrage
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
write_ombrage(x,y,l,h)
{
   int offy, i;
   unsigned char hide_att;

   /* ombrage horizontal */
   _brighten(x + 1, y + h + 2, l + 2, 0x07);

   /* ombrage vertical */
   for (offy = 0; offy <= h + 1; offy++)
        _brighten(x + 2 + l,
                  y + 1 + offy,
                  2,
                  0x07);

}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_mbox - dessiner le contour d'un menu
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_mbox(mp)
struct OBJ *mp;
{
  /* type 'Ä' */
  write_box(mp->title, mp->ul_x, mp->ul_y, mp->ncol, mp->nline, mp->border, 0);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_box - dessiner le contour d'une BOXE
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_box(title,x,y,l,h,att,typ)
char *title;
unsigned char att;
{
   int ps,i,j,tl;
   unsigned char ulc, llc, urc, lrc, Vchar, Hchar;
   char Ch;

   ulc   = boxtype[typ][0];
   urc   = boxtype[typ][1];
   llc   = boxtype[typ][2];
   lrc   = boxtype[typ][3];
   Vchar = boxtype[typ][4];
   Hchar = boxtype[typ][5];


   /* COINS */
   _wstringDB(x        , y        , 1, NEWATT, att, &ulc);
   _wstringDB(x + l + 1, y        , 1, NEWATT, att, &urc);
   _wstringDB(x        , y + h + 1, 1, NEWATT, att, &llc);
   _wstringDB(x + l + 1, y + h + 1, 1, NEWATT, att, &lrc);


   /* HORIZONTALES */
   for (i = x+1; i <= x+l; i++) {
        _wstringDB(i, y        , 1, NEWATT, att, &Hchar);
        _wstringDB(i, y + h + 1, 1, NEWATT, att, &Hchar);
   }

   /* DETERMINER SI LE TITRE PEUT RENTRER DANS LA LARGEUR */
   tl = strlen(title);
   if ((tl > 0) && (l - 4 >= tl)) {
        i = x + (l - tl)/2;
        Ch = BLANK;
        _wstringDB(i++   , y, 1 , NEWATT, att, &Ch);
        _wstringDB(i     , y, tl, NEWATT, att, title);
        _wstringDB(i + tl, y, 1 , NEWATT, att, &Ch);
   }


   /* VERTICALES */
   for (i = y+1; i <= y+h; i++ ) {
        _wstringDB(x        , i, 1, NEWATT, att, &Vchar);
        _wstringDB(x + l + 1, i, 1, NEWATT, att, &Vchar);
   }
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * write_wbox - dessiner le contour de la fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION write_wbox(wp, type)
struct OBJ *wp;
{
      write_box(wp->title, wp->ul_x, wp->ul_y, wp->ncol, wp->nline, wp->border, type);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreScreen - RESTITUER la partie de l'ecran ecras‚ par le SOUS MENU
 *                 ou la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU)
        restoreFromMenu(mp,mp->save);
    else
        _refresh( mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol + 3,
                  mp->ul_y + mp->nline + 2,
                  mp->save);

}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * restoreFromMenu- ecriture PHYSIQUE d'un tampon sur l'ecran
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
restoreFromMenu(mp,buf)
struct OBJ *mp;
int            *buf;
{
   if (mp->mov == VER)
       _refresh(  mp->ul_x,
                  mp->ul_y,
                  mp->ul_x + mp->ncol+3,
                  mp->ul_y + mp->nline+2,
                  buf);
   else
       _wstringDW(mp->ul_x,
                  mp->ul_y,
                  mp->ncol,
                  buf);
}
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * saveScreen - SAUVEGARDER la partie de l'ecran correspondant au SOUS MENU
 *              ou a la WINDOW
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
saveScreen(mp)
struct OBJ *mp;
{
    if (mp->objet == MENU) {
        if (mp->mov == VER)
            _save_screen(mp->ul_x,
                     mp->ul_y,
                     mp->ul_x + mp->ncol+3,
                     mp->ul_y + mp->nline+2,
                     mp->save);
        else
            _rstringDW(  mp->ul_x,
                     mp->ul_y,
                     mp->ncol,
                     mp->save);

    }
    else
        _save_screen( mp->ul_x,
                      mp->ul_y,
                      mp->ul_x + mp->ncol + 3,
                      mp->ul_y + mp->nline + 2,
                      mp->save);
    if (mp->f_ombrage)
        write_ombrage(mp->ul_x, mp->ul_y, mp->ncol, mp->nline);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * initWin : initialiser une fenetre
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
FUNCTION initWin(wp,title,ul_x,ul_y,ncols,nlines,ombrage,bar,ink,paper,border,batt)
struct OBJ *wp;
char *title;
{
   wp->nline      = nlines;
   wp->leftCh     = 0;
   wp->curX       = wp->curY = 0;
   wp->border    = border;
   wp->ink        = ink;
   wp->paper      = paper;
   wp->ul_x       = ul_x;
   wp->ul_y       = ul_y;
   wp->ncol       = ncols;
   wp->pushed    = FALSE;
   wp->title      = title;
   wp->save       = NULL;
   wp->f_ombrage  = ombrage;
   wp->f_bar      = bar;
/*   fillCh         = 0;*/

}

/*ÄÄÄÄÄÄÄÄ
 * pushPop_MESS
 *ÄÄÄÄÄÄÄÄ
 */
pushPop_MESS(str, y, msgtyp)
char *str;
{
   struct OBJ errwin;
   char aux[80];
   strcpy(aux, str);
   verify_window(&errwin, y, aux, nullstr, msgtyp);
   getkey();
   winPopUp(messwin, W_CLOSE);
}

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at - ecrire la string str aux coordonnees x, y, dans
 *            la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_at(wp, x, y, str, isnewatt, att)
struct OBJ *wp;
char *str;
{
  int i;
  if ((i = strlen(str)) > 0)
       _wstringDB(wp->ul_x + 1 + x,
                  wp->ul_y + 1 + y, i, isnewatt, att, str);
}


/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at_nb - ecrire nb carac aux coordonnees x, y, dans
 *               la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_nb_at(wp, x, y, str, nb, isnewatt, att)
struct OBJ *wp;
char *str;
{
   if (nb > 0) {
       if (isnewatt)
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, att, str);
       else
           _wstringDB(wp->ul_x + 1 + x,
                      wp->ul_y + 1 + y, nb, NEWATT, wp->ink|wp->paper, str);
   }
}

/*--------------------------------------------------------------------
 * getkey - lire un caractere et son attribut sur une page inactive
 *          retourne (TYPE | caractere)
 *------------------------------------------------------------------
 */
getkey()
{
   unsigned key, main, auxiliary;

   /* Saisir soit un caractere du clavier, soit un caractere issu
    * de l'automate d'‚mulation clavier de la souris
    */
   for (;;) {
        /* examiner le status du clavier */
	if (_look_kbd()) { /* caractere tap‚ au clavier */
	     key = _read_kbd();
             break;
	 }
   }
   main      = key & 0xff;
   auxiliary = key >> 8;

   if (main > CMDKEY)        /* c'est un caractere Normal */
       return(NORMAL | main);
   else
   {
       if (main == 0) {  /* special Key */
           if ((auxiliary >= HOME) && (auxiliary <= DELETE))
                return(NUMPAD | auxiliary);
           if ((auxiliary >= F1) && (auxiliary <= F10))
                 return(FUNC | auxiliary);
           if ((auxiliary >= SHIFT_F1) && (auxiliary <= SHIFT_F10))
                 return(SHIFTFUNC | auxiliary);
           if ((auxiliary >= CTRL_F1) && (auxiliary <= CTRL_F10))
                 return(CTRLFUNC | auxiliary);
           if ((auxiliary >= ALT_F1) && (auxiliary <= ALT_F10))
                 return(ALTFUNC | auxiliary);
          if (((auxiliary >= CTRL_ARRL) && (auxiliary <= CTRL_HOME)) ||
               (auxiliary == CTRL_PGUP))
                return(CTRLNUMPAD | auxiliary);
          return(ALT | auxiliary);  /* REMARQUE : Shift TAB
                                     * est conditionn‚ en ALT TAB
                                     */
       }
       else  /* command Key */
           switch(key) {
           case 0x1c0d :                               /* return      */
           case 0x0f09 :                               /* TAB         */
           case 0x0e08 : return(NORMAL|main);          /* back space  */
           default     : return(CTRL|(main+ 0x40));    /* ctrl i      */
           }
   }
}


/*---------------------------------------------------------------------------
 *  _init_video - determine le type de carte video
 *---------------------------------------------------------------------------
 */
_init_video()
{
        int adapt;
        int mode;                 /* Value returned by BIOS call */
        union REGS regs;

        regs.h.ah = 0xF;
        int86(0x10, &regs, &regs);   /* Get video mode, place in AL */
        mode = regs.h.al;
        if (mode == 7)               /* 7 and 15 are MONO modes */
                adapt = MONO;
        else if (mode == 15) {
                adapt = MONO;
                _set_mode(7);         /* Set to 7, standard MONO mode */
        } else {
                adapt = _is_ega();         /* Test for CGA vs. EGA */
                if (mode >= 8 && mode <=14)
                        _set_mode(3);
                else switch (mode) {
                case 1:                     /* Color */
                case 3:
                case 4: _set_mode(3);       /* 3 is standard color mode */
                        break;
                case 0:                     /* B & W */
                case 2:
                case 5:
                case 6: _set_mode(2);        /* 2 is standard B & W mode */
                        break;
                } /* end switch */
        } /* end else */
        return(adapt);
}


/*---------------------------------------------------------------------------
 *  _is_ega - determine si carte EGA ou CGA
 *---------------------------------------------------------------------------
 */
_is_ega()
{
        union REGS regs;
        char far *ega_byte = (char far *) 0x487;
        int  ega_inactive;

        regs.h.ah = 0x12;
        regs.x.cx = 0;
        regs.h.bl = 0x10;
        int86(0x10, &regs, &regs);
        if (regs.x.cx == 0)
                return (CGA);
        ega_inactive = *ega_byte & 0x8;
        if (ega_inactive)
                return (CGA);
        return (EGA);
}

/*---------------------------------------------------------------------------
 *  set_mode -
 *---------------------------------------------------------------------------
 */
_set_mode(mode)
int     mode;
{
        union REGS regs;

        regs.h.al = (char) mode;
        regs.h.ah = 0;
        int86(0x10, &regs, &regs);
        regs.h.al = 0;
        regs.h.ah = 5;
        int86(0x10, &regs, &regs);
}

/*----------------------------------------------------------------
 * set_mode_base : set the video address for mono and color modes
 *---------------------------------------------------------------
 */
set_mode_base()
{
     if ((adapter = _init_video()) == MONO) {
         SEGvideo  = MONOBASE;
/*	 Pvideo    =(char far *) VIDMONO ;*/
     }
     else {
         SEGvideo  = COLORBASE;
/*	 Pvideo    = (char far *)VIDCOLOR ; */
     }

}
/*--------------------------------------------------------------------
 * clrscr  - Clear Screen
 *------------------------------------------------------------------
 */
clrscr()
{
       _clrwin(0, 0, 80, 25, F_WHITE|B_BLACK, 0x20);
}

/*--------------------------------------------------------------------
 * gotoxy -  Moves the cursor to a specific column and row
 *------------------------------------------------------------------
 */
gotoxy(col,row)
int col,row;
{ union REGS reg;
      reg.h.ah = 2;
      reg.h.bh = 0;
      reg.x.dx = (row << 8)|col;
      int86(0x10, &reg, &reg);
}

/*--------------------------------------------------------------------
 * wgotoxy -  Moves the cursor to a specific column and row
 *------------------------------------------------------------------
 */
wgotoxy(wp, col, row)
struct OBJ *wp;
int col,row;
{
	gotoxy(wp->ul_x+col, wp->ul_y+row+1);
}

/*-------------------------------------------------------------------
 * hide_cursor - cache le curseur
 *-----------------------------------------------------------------
 */
hide_cursor()
{
   gotoxy(0,25);
}


/*-------------------------------------------------------------------
 * new_int10h - nouvelle INT 10h
 *-----------------------------------------------------------------
 */
new_int10h(ch)      /* al=cara bh=page bl=att cx=nb */
int ch;
{
	ch = ch & 0xff;
	switch(ch) {
	case 0x0A:
			int10Buffer[M_in] = '\0';
			print_at(WP, 3, 1, razline, OLDATT);
			print_at(WP, 3, 1, int10Buffer, OLDATT);
			M_in = 0;

	case 0x0D:	break;
	default  :      if (ch < 0x20 || ch > 0x7E)
				break;
			int10Buffer[M_in] = (unsigned char)ch;
			M_in++;
	}
}

