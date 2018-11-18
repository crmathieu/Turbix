/************************************************************
*  15:57:36  20/06/1988   cvideo.c                          *
*     1- clrscr(): clear screen                             *
*     2- gotoxy (X,Y)                                       *
*     3- setcursor() :                                      *
*     4- hide_cursor()                                      *
*     5- read_curs_pos(Page)                                *
*     6- window_up()                                        *
*     7- window_down()                                      *
*     8- fill_line(X,Y,Char,Times,Attrib)                   *
*     9- getkey()                                           *
*    10- get_mode()                                         *
*    11 - disk_err()                                        *
*    12 - key_delay()
************************************************************/

#include "xed.h"
#include "ext_var.h"

/*--------------------------------------------------------------------
 * clrscr  - Clear Screen
 *------------------------------------------------------------------
 */
clrscr()
{
       _clrwin(0, 0, 79, 24, F_WHITE|B_BLACK, 0x20);
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
 * key_delay - modifier le delay lecture clavier
 *------------------------------------------------------------------
 */
key_delay(delay, rate)
{ union REGS reg;
      reg.h.ah = 3;
      reg.h.al = 5;
      reg.h.bh = delay;   /* delais precedant le flot de repetition */
      reg.h.bl = rate;    /* # caracteres / sec */
      int86(0x16, &reg, &reg);
}

/*-------------------------------------------------------------------
 * setcursor  - Sets the shape of the cursor
 *              mode mono :lines 11 and 12 ; mode color :lines 6 and 7
 *-----------------------------------------------------------------
 */
setcursor(wp)
struct OBJ *wp;
{/* union REGS reg;
      reg.h.ah = 1;
      if (adapter == CGA)  reg.x.cx = (6 << 8) + 7;
      else                 reg.x.cx = (11 << 8) + 12;
      int86(0x10, &reg, &reg);*/
    wgotoxy(wp,wp->curX,wp->curY);
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

/*-------------------------------------------------------------------
 * get_mode - get current video mode
 *-------------------------------------------------------------------
 */
get_mode()
{union REGS ireg,oreg;
      ireg.h.ah = 15;
      int86(0X10, &ireg, &oreg);
      return(oreg.h.al);
}

/*-------------------------------------------------------------------
 * get_color - get typ of COLOR adapter (CGA or EGA) not DOS DOCUMENTED
 *-------------------------------------------------------------------
 */
get_color()
{union REGS ireg,oreg;
      ireg.h.ah = 0x12;
      ireg.x.cx = 0;
      ireg.h.bl = 0x10;
      int86(0X10, &ireg, &oreg);
      return(oreg.x.cx);
}

/*-------------------------------------------------------------------
 * window_up - scrolling vers le haut d'une window
 *-----------------------------------------------------------------
 */
_window_up(nl,xg,yg,xd,yd,fill_att)
int  nl,                 /* nb de ligne … scroller */
     xg,                 /* no colonne du plus haut coin gauche */
     yg,                 /* no ligne du plus haut coin gauche */
     xd,                 /* no colonne du plus bas coin droit */
     yd,                 /* no ligne du plus bas coin droit */
     fill_att;           /* attribut de la ligne de filler */
{union REGS ireg;

      hideMouse();
      ireg.h.al = nl;
      ireg.h.ah = 6;
      ireg.h.bh = fill_att;
      ireg.h.ch = yg;
      ireg.h.cl = xg;
      ireg.h.dh = yd;
      ireg.h.dl = xd;
      int86(0X10, &ireg, &ireg);
      showMouse();
}

/*-----------------------------------------------------------------
 * window_down - scrolling vers le bas d'une window
 *---------------------------------------------------------------
 */
_window_down(nl,xg,yg,xd,yd,fill_att)
int  nl,xg,yg,xd,yd,fill_att;
{
   union REGS ireg;

      hideMouse();
      ireg.h.al = nl;
      ireg.h.ah = 7;
      ireg.h.bh = fill_att;
      ireg.h.ch = yg;
      ireg.h.cl = xg;
      ireg.h.dh = yd;
      ireg.h.dl = xd;
      int86(0X10, &ireg, &ireg);
      showMouse();
}

/*---------------------------------------------------------------------
 * reset_floppy - reinitialise le controleur de floppy apres une erreur
 *---------------------------------------------------------------------
 */
reset_floppy()
{union REGS ireg;
      ireg.h.ah = 0;
      int86(0x13, &ireg, &ireg);
}

/*--------------------------------------------------------------------
 * getkey - lire un caractere et son attribut sur une page inactive
 *          retourne (TYPE | caractere)
 *------------------------------------------------------------------
 */
getkey()
{
   unsigned key, main, auxiliary;
   unsigned automate();
   int mouseEmulate = 0;

   /* Saisir soit un caractere du clavier, soit un caractere issu
    * de l'automate d'‚mulation clavier de la souris
    */
   for (;;) {
        /* examiner le status du clavier */
        if (look_kbd()) { /* caractere tap‚ au clavier */
             key = read_kbd();
             break;
         }
         if (M_in != M_out) {
             getMouseBuf(&key);
             return(key);
         }
         /* LA gestion souris est en commentaire */
         if (mouseEvent) { /* inserer ICI le code emulation KBD */
               automate();
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
          if (auxiliary == SHIFTTAB)
                return(NORMAL|TAB); /* REMARQUE : Shift TAB
                                     * est conditionn‚ en TAB
                                     */
          return(ALT | auxiliary);
       }
       else  /* command Key */
           switch(key) {
           case 0x1c0d :                               /* return      */
           case 0x0f09 :                               /* TAB         */
           case 0x0e08 : return(NORMAL|main);          /* back space  */
           case 0x1c0a : return(CTRL|RETURN);          /* Ctrl RETURN */
           default     : return(CTRL|(main+ 0x40));    /* ctrl i      */
           }
   }
}



/*---------------------------------------------------------------------------
 *  sasVector - "save and set interrupt vector"
 *---------------------------------------------------------------------------
 */
sasVector(vec_nbr, new_vector ,old_vector)
int vec_nbr;
int (*new_vector)();
long *old_vector;
{
    unsigned long *vector;
    vector = (unsigned long)vec_nbr * 4;
    *old_vector        = *((long *)vector);
    *( (long *) vector )= (long)new_vector;
}

/*---------------------------------------------------------------------------
 *  mouse_manager - Action principale sur RELEASE
 *---------------------------------------------------------------------------
 */
mouse_manager()
{
    int /*posX, posY,*/ buttonStatus;
    struct OBJ *wkObj;
    char work[20];
    struct OBJ *whereIsMouse();

    extern struct mouse_block_interface ib;
    mouseEvent--;
    memset(work, '\0', 20);

    /* recuperer la position du curseur
     * souris et le type d'evenement
     */
/*    m_getPosAndButtonStatus(&posX, &posY, &buttonStatus);*/

    /* convertir X et Y en # d'octets */
    posY = ib.vCursor / 8;
    posX = ib.hCursor / 8;
    buttonStatus = ib.buttonState;
/*
 *  le bit 0 represente le bouton de gauche
 *  le bit 1 represente le bouton de droite
 *  Pour Chacun des 2 bits :
 *  si … 1 -> enfonc‚,   0 -> relach‚
 */
    switch(mouse_state) {
    case M_REPOS :
                     if (buttonStatus & 1)
                        /* bouton enfonc‚ */
                        mouse_state = M_LEFT_PRESS;
                   break;

    case M_LEFT_PRESS :
                   strcpy(work,itoa(posX,NULL,10));
                   strcat(work, " ");
                   strcat(work,itoa(posY,NULL,10));
                   strcat(work, " ");
                   _wstringDB(60, 24, strlen(work), NEWATT, MENUink|MENUpaper, work);
                   if (!(buttonStatus & 1)) {
                        mouse_state = M_REPOS;
/*                        if (isMouseOnObj(posX, posY))
                                return(NORMAL|ESC);
                        else
                                return(0);*/
                        if (whereIsMouse(currObj, posX, posY) == currObj) {
                                emulateInt9(0x0100|ESC);
                                emulateInt9(F3<<8);
                        }
                        else
                                return(0);
                   }
                   break;
    default : return(0);
    }
    return(0);
}

/*---------------------------------------------------------------------------
 * setCurrObj - met a jour l'objet courant
 *------------------------------------------------------------------- ------
 */
setCurrObj(obj)
struct OBJ *obj;
{
   if (obj->prev != (struct OBJ *)NULL)
        obj->prev->next = obj->next;
   if (obj->next != (struct OBJ *)NULL)
        obj->next->prev = obj->prev;
   /* A FINIR */
}

/*---------------------------------------------------------------------------
 *  WhereIsMouse - Ou est la souris ?
 *                 Retourne l'objet sur lequel se trouve la souris
 *---------------------------------------------------------------------------
 */
struct OBJ *whereIsMouse(Obj)
struct OBJ * Obj;
{
   if (Obj == (struct OBJ *)NULL)
        return((struct OBJ *)NULL);

   if ((posX < Obj->ul_x) || (posX > Obj->ul_x + Obj->ncol+1))
        return(whereIsMouse(Obj->prev, posX, posY));
   if ((posY < Obj->ul_y) || (posY > Obj->ul_y + Obj->nline+1))
        return(whereIsMouse(Obj->prev));
   return(Obj);
}

/*---------------------------------------------------------------------------
 *  beep - emet un signal sonore
 *---------------------------------------------------------------------------
 */
beep(f)
{
    int x, k, f2;
    int ps;
    f2 = f * 2;
    f <<= 1;
    outp(TIMER3, 0xb6);
    outp(TIMER2, f& LOWBYTE);
    outp(TIMER2, (f >> 8) & LOWBYTE);
    x = inp(PORT_B);
    outp(PORT_B, x | 3);
    for(k = 0; k < B_TIME; k++) ;
    outp(PORT_B, x);
    f = f2;
    f <<= 1;
    outp(TIMER3, 0xb6);
    outp(TIMER2, f& LOWBYTE);
    outp(TIMER2, (f >> 8) & LOWBYTE);
    x = inp(PORT_B);
    outp(PORT_B, x | 3);
    for(k = 0; k < B_TIME; k++) ;
    outp(PORT_B, x);
}

unsigned automate()
{
    int buttonStatus;
    unsigned ret;
    struct OBJ *wkObj;
    struct OBJ *whereIsMouse();

    extern struct mouse_block_interface ib;
    mouseEvent--;
    ret = 0;

    /* recuperer la position du curseur
     * souris et le type d'evenement
     */
    m_getPosAndButtonStatus(&posX, &posY, &buttonStatus);

    /* convertir X et Y en # d'octets */
    posY = ib.vCursor / 8;
    posX = ib.hCursor / 8;
    buttonStatus = ib.buttonState;

/*
 *  le bit 0 represente le bouton de gauche
 *  le bit 1 represente le bouton de droite
 *  Pour Chacun des 2 bits :
 *  si … 1 -> enfonc‚,   0 -> relach‚
 */
    switch(mouse_state) {
    case M_REPOS :
                     if (buttonStatus & 1)
                        /* bouton gauche enfonc‚ */
                        (* currObj->M_Gpr)();
                     else
                        if (buttonStatus & 2)
                            /* bouton droit enfonc‚ */
                            (* currObj->M_Dpr)();
/*                      else
                            /* la souris a bouge */
/*                          (* currObj->M_Mov)();*/
                     break;

    case M_RIGHT_PRESS:
                     if (buttonStatus & 2)
                         /* bouton toujours enfonc‚ */
                         (* currObj->M_CDpr)();
                     else
                         /* bouton relach‚ */
                         (* currObj->M_Dre)();
                     break;
    case M_LEFT_PRESS :
                     if (buttonStatus & 1)
                         /* bouton toujours enfonc‚ */
                         (* currObj->M_CGpr)();
                     else
                         /* bouton relach‚ */
                         (* currObj->M_Gre)();
                     break;
    default : break;
    }
    return(ret);
}

/* <<<<<<<<<<<<<<<<<<< DEFAULT MOUSE TRANSITIONS >>>>>>>>>>>>>>>>>>> */

/*--------
 *  m_Gpr
 *--------
 */
m_Gpr()
{
        mouse_state = M_LEFT_PRESS;

        /* commentaires en exclusions avec m_Gre() */
/*        return(_m_localize(NUMPAD|MOUSE_LP));*/
}

/*--------
 *  m_Mov
 *--------
 */
m_Mov()
{
    char work[20];

    memset(work, '\0', 20);
    strcpy(work,itoa(posX,NULL,10));
    strcat(work, " ");
    strcat(work,itoa(posY,NULL,10));
    strcat(work, " ");
    _wstringDB(60, 24, strlen(work), NEWATT, MENUink|MENUpaper, work);
    setMouseBuf(NUMPAD|MOUSE_M);
}

/*--------
 *  m_Dpr
 *--------
 */
m_Dpr()
{
        mouse_state = M_RIGHT_PRESS;
        setmouseBuf(NUMPAD|MOUSE_RP);
}

/*--------
 *  m_Gre
 *--------
 */
m_Gre()
{
        mouse_state = M_REPOS;
        /* commentaires en exclusions avec m_Gpr() */
        return(_m_localize(NUMPAD|MOUSE_LR));

}

/*-------------
 *  _m_localize
 *-------------
 */
_m_localize(key)
unsigned key;
{
        struct OBJ *ob;
        int item;

        if ((ob = whereIsMouse(tailObj)) == currObj) {
             if (ob->objet == MENU) {
                if (getCurrItem(ob, &item)) {
                        if (item == ob->curY)
                                setmouseBuf(NORMAL|RETURN); /* declencher ouverture */
                        else
                                setMouseBuf(key);  /* generer la cle souris */
                }
             }
             else
                        setMouseBuf(key);
        }
        else { /* on a cliqu‚ sur un autre d'OBJET */
               if (ob != (struct OBJ *)NULL) {

                   currObj = ob;               /* c'est maintenant l'objet
                                                * courant
                                                */

                   setMouseBuf(ob->m_key);     /* ouvrir le nouvel objet
                                                * s'il n'est pas deja ouvert
                                                */


                   if (ob->objet == MENU) { /* A REVOIR */
                            setMouseBuf(key); /* a pour effet de declencher
                                                * une mise a jour de l'ITEM
                                                * courant
                                                */

                   }
               }
               /* position sur aucun objet */
               return(1);
        }
        return(0);
}

/*--------
 *  m_Dre
 *--------
 */
m_Dre()
{
        mouse_state = M_REPOS;
        setmouseBuf(NUMPAD|MOUSE_RR);
}

/*--------
 *  m_CDpr
 *--------
 */
m_CDpr()
{

}

/*--------
 *  m_CGpr
 *--------
 */
m_CGpr()
{
        struct OBJ *ob;
        int item;

        if ((ob = whereIsMouse(tailObj)) == currObj) {
             if (ob->objet == MENU) {
                if (getCurrItem(ob, &item))
                        if (item != ob->curY)
                                setMouseBuf(NUMPAD|MOUSE_L); /* generer la cle souris */
             }
        }
        else {
               if (ob->objet == MENU) {
                        if (ob != (struct OBJ *)NULL) {
                                if (ob == &mainMenu) {
                                        if (getCurrItem(ob, &item))
                                                if (item != ob->curY) {
                                                        setMouseBuf(NUMPAD|MOUSE_DRAG); /* pour sortir du SCANVMENU */
                                                        setMouseBuf(NUMPAD|MOUSE_L);
                                                }
                                }
                                else {
                                currObj = ob;           /* c'est maintenant l'objet courant */
                                setMouseBuf(ob->m_key); /* ouvrir le nouvel objet
                                                         * s'il n'est pas deja ouvert
                                                         */
                                setMouseBuf(NUMPAD|MOUSE_L); /* a pour effet de declencher
                                                        * une mise a jour de l'ITEM
                                                        * courant
                                                        */
                                }
                        }
               }
        }
}

/*---------------------------------------------------------------------------
 *  setCurrItem - met a jour l'ITEM courant dans un objet
 *---------------------------------------------------------------------------
 */
setCurrItem(ob)
struct OBJ *ob;
{
        int item;

        if (getCurrItem(ob, &item))
                /* verifier qu'on est pas sur une ligne de separation */
                if ((unsigned char)ITEM(ob,item)->str[0] != 'Ä')
                        ob->curY = item;
}

/*---------------------------------------------------------------------------
 *  getCurrItem - determine l'ITEM courant dans un objet
 *---------------------------------------------------------------------------
 */
getCurrItem(ob,item)
struct OBJ *ob;
int *item;
{
    int i, s, offset;

    i = 0;
    if (ob->objet == MENU) {
        if (ob->mov == HOR) { /* menu horizontal */
            while (i < ob->nline) {
                offset = xcoor(ob,i);
                if ((offset <= posX) &&
                    (posX <= (offset + (s = strlen(ITEM(ob,i)->str))))) {
                        *item = i;
                        return(1); /* done */
                }
                i++;
            }
            return(0);
        }
        /* menu vertical */
        *item = posY - (ob->ul_y + 1);
        if (*item < 0)
                *item = 0;
        if (*item >= ob->nline)
                *item = ob->nline - 1;
        return(1); /* done */
    }
    return(0);
}

/*---------------------------------------------------------------------------
 *  setMouseBuf - place une cl‚ dans le buffer Souris
 *---------------------------------------------------------------------------
 */
setMouseBuf(val)
unsigned val;
{
   mouseBuffer[M_in] = val;
   M_in = (M_in + 1) & 15;
}

/*---------------------------------------------------------------------------
 *  razMouseBuf - Raz Buffer souris
 *---------------------------------------------------------------------------
 */
razMouseBuf()
{
   M_in = M_out = 0;
}

/*---------------------------------------------------------------------------
 *  getMouseBuf - retire une cl‚ du buffer Souris
 *---------------------------------------------------------------------------
 */
getMouseBuf(val)
unsigned *val;
{
   *val = mouseBuffer[M_out];
   M_out = (M_out + 1) & 15;
}

/* <<<<<<<<<<<<<<<<<<< SPECIFIC EDITOR MOUSE TRANSITIONS >>>>>>>>>>>>>>>>>>> */

/*-----------
 *  m_editDpr
 *-----------
 */
m_editDpr()
{
        struct OBJ *ob;

        mouse_state = M_RIGHT_PRESS;
        if ((ob = whereIsMouse(tailObj)) == currObj) {
                setMouseBuf(NUMPAD|MOUSE_R);
                /* boucler sur la pression en emulant une IT mouse */
/*              ticks = 0;
                while (ticks < 1);
                mouseEvent++;*/
        }
}

/*-----------
 *  m_editGpr
 *-----------
 */
m_editGpr()
{
        /* sur pression gauche, enregistrer la position
         * de la souris
         */
        XMOUSE = posX;
        YMOUSE = posY;
        CMOUSE = current;
        LMOUSE = pedwin->leftCh;
        if (_m_localize(NUMPAD|MOUSE_LP) == 0)
                /* on est sur un objet
                 * (si 1, on est
                 * dans une zone non referencee,
                 * donc on ne fait rien)
                 */
                mouse_state = M_LEFT_PRESS; /* forcer l'etat */
        else
                mouse_state = M_REPOS;
}
/*-----------
 *  m_editGre
 *-----------
 */
m_editGre()
{
        mouse_state = M_REPOS;
        setMouseBuf(NUMPAD|MOUSE_LR);
}

/*--------
 *  m_editCDpr
 *--------
 */
m_editCDpr()
{
        m_editDpr();
}

/*--------
 *  m_CGpr
 *--------
 */
m_editCGpr()
{
        _m_localize(NUMPAD|MOUSE_L);
        mouse_state = M_LEFT_PRESS; /* forcer l'etat */

}

/*-----------
 *  it_clk
 *-----------
 */
interrupt it_clk()
{
    ticks++;  /* incrementer le nb de tics d'horloge */
}

/* <<<<<<<<<<<<<<<<<<< SPECIFIC RDIALWIN MOUSE TRANSITIONS >>>>>>>>>>>>>>>>>>> */
/*--------
 *  m_saisieGpr
 *--------
 */
m_saisieGpr()
{
        XMOUSE = posX;
        YMOUSE = posY;
        if (isOnField(currObj) >= 0)
                setMouseBuf(NORMAL|RETURN);
        else
                m_Gpr();
}
