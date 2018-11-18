                /*****************************
                 *     MODULE INITIAL MDI    *
                 *****************************/


#include <process.h>
#include <stdio.h>
#include <dir.h>
#include "xed.h"
#ifdef TURBO
#define _dos_setvect setvect
#define _dos_getvect getvect
#endif
struct varLink VL, *pvl;

int retCode;
char wkfile[80];
char currDir[80], overlayStr[80];
char *ARGV[5], *pt;

#define  NULL  (char *)0
#define  SIGNATURE      0x0412

unsigned dosbuf[2000];
int dosX, dosY, adapter;
unsigned SEGvideo;

extern char retFromInt[];
extern char loadEdit[];
extern char resid[];

/* MOUSE */
unsigned long BiosInt33;
int mouse_present;
/*extern int prolog_it_mouse();*/
void interrupt Dummy_interrupt();

char homedir[80];

main(argc, argv)
int argc;
char *argv[];
{
        int ret;
        int i = 0;


        /* copier arguments */
        while (argv[i] && i < argc) {
               ARGV[i] = argv[i];
               i++;
        }

        /* verifier le vecteur de lien */
        if ((pvl = (unsigned long)_dos_getvect(0xf2))->signature == SIGNATURE) {
             set_mode_base();
             read_curs_pos(&dosX, &dosY);
             gotoxy(dosX, dosY-1);
             print_at(dosX, dosY-1, resid, OLDATT);
             gotoxy(dosX, dosY);
             return(0);
        }

        /* initialiser le vecteur de communication */
        _dos_setvect(0xf2, (void interrupt(* )())&VL);
        VL.signature = SIGNATURE;

        /* init buffer vid‚o DOS */
        set_mode_base();
        _save_screen(0, 0, 79, 24, dosbuf);
        read_curs_pos(&dosX, &dosY);
        VL.segVideo = SEGvideo;
        VL.adapter  = adapter;

        /* init SOURIS */
        init_mouse();

        /* init ligne de commande */
        memset(VL.lineCom, '\0', 80);

        /* initialement, declencher l'editeur */
        retCode = VL.retCode = 0;
        strcpy(VL.fError, "error.msg");

        /* recuperer la directory courante */
        getcwd(homedir, 79);

        strcpy(currDir,argv[0]);
        if ((pt = strrchr(currDir, '\\')) != NULL)
                *(pt+1) = '\0';
        while (1) {
                switch(retCode) {

                case 0 :

                                /* lancer l'editeur */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txed.ovl");
                                VL.retCode = spawnvp(P_WAIT, overlayStr, ARGV);
                                switch(retCode = VL.retCode) {
                                case 1 :
                                case 2 :
                                case 3 : break;
                                case 4 : inhibe_mouse();break;
                                case 5 : inhibe_mouse();
                                         setExe();
                                         break;
                                case 6 :
                                case 7 :
                                case 8 : 
                                case 12: break;
                                default: /* sortir */
                                        inhibe_mouse();
                                        _refresh(0, 0, 79, 24, dosbuf);
                                        gotoxy(dosX, dosY-1);
                                        print_at(dosX, dosY-1, loadEdit, OLDATT);
                                        gotoxy(dosX, dosY);
                                        retCode = -1;
                                        break;
                                }
                                break;

                case 1 :        /* Compile / Assemble */
                case 2 :        /* Assemble */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txcomp.ovl");

                                VL.spawnRet = spawnvp(P_WAIT, overlayStr, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/c";
                                ARGV[2] = NULL;
                                break;

                case 3 :        /* lancer le Make */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txmake.ovl");

                                VL.spawnRet = spawnvp(P_WAIT, overlayStr, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/m";
                                ARGV[2] = NULL;
                                break;


                case 4 :        /* shell */
                                _refresh(0, 0, 79, 24, dosbuf);
                                gotoxy(dosX, dosY-1);
                                print_at(dosX, dosY-1, retFromInt, NEWATT, F_LWHITE);
                                spawnlp(P_WAIT, "command.com", NULL);
                                VL.spawnRet = 0;
                                _save_screen(0, 0, 79, 24, dosbuf);
                                read_curs_pos(&dosX, &dosY);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/s";
                                ARGV[2] = NULL;
                                init_mouse();
                                break;

                case 5 :        /* lancer NOYAU */
                                VL.spawnRet = spawnvp(P_WAIT, wkfile, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/k";
                                ARGV[2] = NULL;
                                /* RAZ ligne de commande noyau */
                                /*VL.lineCom[0] = '\0';*/
                                init_mouse();
                                break;

                case 6 :        /* lancer le Link */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txlink.ovl");

                                VL.spawnRet = spawnvp(P_WAIT, overlayStr, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/l";
                                ARGV[2] = NULL;
                                break;


                case 7 :        /* Sortir */
                                inhibe_mouse();
                                _refresh(0, 0, 79, 24, dosbuf);
                                gotoxy(dosX, dosY-1);
                                retCode = -1;
                                break;

                case 8 :        /* Browser */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txref.ovl");

                                VL.spawnRet = spawnvp(P_WAIT, overlayStr, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/b";
                                ARGV[2] = NULL;
                                break;

                case 12 :        /* CHK */
                                strcpy(overlayStr, currDir);
                                strcat(overlayStr, "txchk.ovl");

                                VL.spawnRet = 0;
                                spawnvp(P_WAIT, overlayStr, ARGV);
                                retCode = 0;
                                ARGV[0] = "txed.ovl";
                                ARGV[1] = "/b";
                                ARGV[2] = NULL;
                                break;

                default:        /* restaurer ancien vecteur */
                                _dos_setvect(0xf2, (void interrupt(* )())pvl);

                                /* replacer dir courante */
                                chdir(homedir);
                                return(0);

                }
        }
}

/*-------------------------------------------------------------------
 * setExe - fabrique le nom de l'executable
 *-----------------------------------------------------------------
 */
setExe()
{
        char *pt;
        strcpy(wkfile,VL.exename);
        if ((pt = strrchr(wkfile, '.')) != NULL) {
                *pt = '\0';
                strcat(wkfile,".exe");
        }
        ARGV[0] = wkfile;
        ARGV[1] = VL.lineCom;
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

/*-------------------------------------------------------------------
 * read_curs_pos -
 *-----------------------------------------------------------------
 */
read_curs_pos(x,y)
int *x,*y;            /* adress of th variables to contain the x,y positions */
{union REGS ireg,oreg;
      ireg.h.ah = 3;
      ireg.h.bh = 0; /* PAGE ZERO of the screen memory */
      int86(0X10, &ireg, &oreg);
      *x=oreg.h.dl;
      *y=oreg.h.dh;
}
/*----------------------------------------------------------------
 * 9- set_mode_base : set the video address for mono and color modes
 *---------------------------------------------------------------
 */
set_mode_base()
{
     if ((adapter = _init_video()) == MONO)
         SEGvideo  = 0xB000; /*MONOBASE;*/
     else
         SEGvideo  = 0xB800; /*COLORBASE;*/
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
        else
                if (mode == 15) {       /* MONO graphique: repasser en carac */
                        adapt = MONO;
                        _set_mode(7);         /* Set to 7, standard MONO mode */
                }
                else { /* couleur */
                        adapt = _is_ega();         /* Test for CGA vs. EGA */
                        if (mode >= 8 && mode <=14)
                                _set_mode(3);
                        else
                                switch (mode) {
                                case 1 :               /* Color */
                                case 4 : _set_mode(3); /* 3 is standard color mode */
                                         break;
                                case 0 :               /* B & W */
                                case 2 :
                                case 5 :
                                case 6 : _set_mode(2); /* 2 is standard B & W mode */
                                default: break;
                                }
                }

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
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 * print_at - ecrire la string str aux coordonnees x, y, dans
 *            la window wp
 *ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
 */
print_at(x, y, str, isnewatt, att)
char *str;
{
  int i;
  if ((i = strlen(str)) > 0)
       _wstringDB(x, y, i, isnewatt, att, str);
}
/*>>>>>>>>>>>>> MOUSE
/*
 * Stopper la souris
 *
 */
inhibe_mouse()
{
   int i;

   /* replacer les anciens vecteurs et cacher la souris */
   _cli();
   if (mouse_present) {
        m_getPosAndButtonStatus(&VL.mouX, &VL.mouY, &i);
   }
   else
       _dos_setvect(0x33, BiosInt33);
   _sti();
   hideMouse();
}
/*
 * Demarrer la souris
 *
 */
init_mouse()
{
        /*--------------------------------------------------------------
         * m_callMaskAndAddress - chaine … l'IT souris, le traitement
         *                       utilisateur a appeler si les conditions
         *                       exprim‚es dans le masque sont reunies:
         *                              - bit 0 : changement de position
         *                              - bit 1 : bouton Gauche press‚
         *                              - bit 2 : bouton Gauche relach‚
         *                              - bit 3 : bouton Droit press‚
         *                              - bit 4 : bouton Droit relach‚
         *--------------------------------------------------------------
         */
   unsigned bidon;

   /* check mouse */
   if (!m_installed(&bidon)) {
        /* pas de souris, installer
         * l'interruption DUMMY
         */
        BiosInt33 = (unsigned long)_dos_getvect(0x33);
        _dos_setvect(0x33, Dummy_interrupt);
        VL.mouse = mouse_present = FALSE;
   }
   else {
        /* positionner l 'interruption souris editeur */
        VL.mouse = mouse_present = TRUE;
        showMouse();

   }
}
/*
 * teste si mouse presente
 *
 */
m_installed(nbutton)
unsigned *nbutton;
{
 union  REGS  rin,rout;

 rin.x.ax  = 0;
 int86(0x33, &rin, &rout);
 *nbutton = rin.x.bx;
 return(rout.x.ax);
}
/*
 * positionner la souris
 *
 */
m_setPos(newPosX, newPosY)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x04;
        rin.x.cx  = newPosX;
        rin.x.dx  = newPosY;
        int86(0x33, &rin, &rout);
}
/*
 * recupere les coordonnees souris
 *
 */
m_getPosAndButtonStatus(posX, posY, buttonStatus)
int *posX, *posY;
unsigned   *buttonStatus;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x03;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
        *buttonStatus = rout.x.bx;
}
void interrupt Dummy_interrupt() {};
