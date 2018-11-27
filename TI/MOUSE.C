#include "xed.h"
#include "ext_var.h"

/* fonctions a utiliser si le driver souris est install- */

mmain()
{
 extern int far prolog_it_mouse();
 int i;
 unsigned nb;
 unsigned X,Y, status;

    if (!m_installed(&nb))  {
        exit(0);
    }
    ibad = &ib;
    m_getPosAndButtonStatus(&XMOUSE, &YMOUSE, &status);

    /* positionner l 'interruption utilisateur */
    m_callMaskAndAddress(2, prolog_it_mouse);
    showMouse();
    for (i=0; i<10;i++) {
         getkbd();
    }
    m_callMaskAndAddress(0, 0L);
    hideMouse();

}

/*--------------------------------------------------------------
 * m_installed - teste si le driver souris est present. si oui,
 *               retourne le nombre de boutons
 *--------------------------------------------------------------
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

/*--------------------------------------------------------------
 * showMouse -
 *
 *--------------------------------------------------------------
 */
showMouse2()
{
 union  REGS  rin,rout;

/* rin.x.ax = 01;
 int86(0x33, &rin, &rout);*/
}

/*--------------------------------------------------------------
 * hideMouse -
 *
 *--------------------------------------------------------------
 */
hideMouse2()
{
 union  REGS  rin,rout;

/* rin.x.ax = 0x02;
 int86(0x33, &rin, &rout);*/
}

/*--------------------------------------------------------------
 * m_getPosAndButtonStatus - retourne la position et le status:
 *                        le bit 0 represente celui de droite
 *                        le bit 1 represente celui de gauche
 *                        Pour Chacun des 2 bits :
 *                        si - 1 -> enfonc-,   0 -> relach-
 *--------------------------------------------------------------
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

/*--------------------------------------------------------------
 * m_setCursorPos -
 *
 *--------------------------------------------------------------
 */
m_setCursorPos(newPosX, newPosY)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x04;
        rin.x.cx  = XMOUSE = newPosX;
        rin.x.dx  = YMOUSE = newPosY;
        int86(0x33, &rin, &rout);
}

/*--------------------------------------------------------------
 * m_getButtonPressInfo - retourne la position a la derniere pression,
 *                        le nombre de pressions et le status:
 *                        STATUS:
 *                        si - 1 -> enfonc-,   0 -> relach-
 *                        NB DE PRESSIONS: de 0 - 32767; remis - 0
 *                        apres l'appel.
 *--------------------------------------------------------------
 */
m_getbuttonPressInfo(buttonToTest, posX, posY, buttonStatus, count)
int *posX, *posY;
unsigned *buttonStatus, *count;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x05;
        rin.x.bx  = buttonToTest;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
        *buttonStatus = rout.x.ax;
        *count = rout.x.bx;
}

/*--------------------------------------------------------------
 * m_getButtonReleaseInfo - retourne la position au dernier relachement,
 *                        le nombre de relachements et le status:
 *                        STATUS:
 *                        si - 1 -> enfonc-,   0 -> relach-
 *                        NB DE RELACHEMENTS: de 0 - 32767; remis - 0
 *                        apres l'appel.
 *--------------------------------------------------------------
 */
m_getButtonReleaseInfo(buttonToTest, posX, posY, buttonStatus, count)
int *posX, *posY;
unsigned *buttonStatus, *count;
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x06;
        rin.x.bx  = buttonToTest;
        int86(0x33, &rin, &rout);
        *posX = rout.x.cx;
        *posY = rout.x.dx;
        *buttonStatus = rout.x.ax;
        *count = rout.x.bx;
}
/*--------------------------------------------------------------
 * m_setMinMaxHorCursorPos -
 *
 *--------------------------------------------------------------
 */
m_setMinMaxHorCursorPos(MinPos, MaxPos)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x07;
        rin.x.cx  = MinPos;
        rin.x.dx  = MaxPos;
        int86(0x33, &rin, &rout);
}
/*--------------------------------------------------------------
 * m_setMinMaxVerCursorPos -
 *
 *--------------------------------------------------------------
 */
m_setMinMaxVerCursorPos(MinPos, MaxPos)
{
 union  REGS  rin,rout;

        rin.x.ax  = 0x08;
        rin.x.cx  = MinPos;
        rin.x.dx  = MaxPos;
        int86(0x33, &rin, &rout);
}

/*--------------------------------------------------------------
 * m_callMaskAndAddress - chaine - l'IT souris, le traitement
 *                       utilisateur a appeler si les conditions
 *                       exprim-es dans le masque sont reunies:
 *                              - bit 0 : changement de position
 *                              - bit 1 : bouton Gauche press-
 *                              - bit 2 : bouton Gauche relach-
 *                              - bit 3 : bouton Droit press-
 *                              - bit 4 : bouton Droit relach-
 *--------------------------------------------------------------
 */
m_callMaskAndAddress(callMask, subroutine)
int callMask;
int (far * subroutine)();
{
 union  REGS  rin,rout;
 struct SREGS segreg;

        rin.x.ax  = 0x0c;
        rin.x.cx  = callMask;
        rin.x.dx  = FP_OFF(subroutine) ;
        segreg.es = FP_SEG(subroutine);
        int86x(0x33, &rin, &rout,&segreg);
}

/*--------------------------------------------------------------
 * m_setGraphCursorBlock -
 *
 *--------------------------------------------------------------
 */
/*m_setGraphCursorBlock()
 union  REGS  rin,rout;
 struct SREGS segreg;*/

 /* recuperer le parameter block */
/*      rin.h.ah  = 0x44;
      rin.h.al  = 0x0d;
      rin.h.bl  = dev;
      rin.h.ch  = 8;
      rin.h.cl  = 0x60;
      rin.x.dx  = FP_OFF(ppb) ;
      segreg.ds = FP_SEG(ppb);
      int86x(0x21, &rin, &rout,&segreg);
}*/

/* union  REGS  rin,rout;
 struct SREGS segreg;*/

 /* recuperer le parameter block */
/*      rin.h.ah  = 0x44;
      rin.h.al  = 0x0d;
      rin.h.bl  = dev;
      rin.h.ch  = 8;
      rin.h.cl  = 0x60;
      rin.x.dx  = FP_OFF(ppb) ;
      segreg.ds = FP_SEG(ppb);
      int86x(0x21, &rin, &rout,&segreg);
}*/
flush_kbd2()
{
    unsigned *pOut, *pIn, *pIn2, *pBuf, *ksop, *keop;
    unsigned save, *pbiot, PINv;

    pOut  = HEADptr;
    pIn   = TAILptr;
    *pOut  = *pIn;
}

emulateInt9(scanChar)
unsigned scanChar;
{
    unsigned *pOut, *pIn, *pIn2, *pBuf, *ksop, *keop;
    unsigned save, *pbiot, PINv;

    pbiot = BIOSTAB;
    pOut  = HEADptr;
    pIn   = TAILptr;
    pBuf  = KEYbuf;
    ksop  = KSOP;
    keop  = KEOP;
    PINv  = *pIn;
    save  = *(pbiot + (PINv/2));
    *(pbiot + (PINv/2)) = scanChar;
    *pIn               += 2;
    if (*pIn >= *keop)
        *pIn = *ksop;
    if (*pIn == *pOut) {
        *(pbiot + (PINv/2)) = save;
        *pIn = PINv;
    }
    mouseEmulate++;
}
getkbd()
{
  static int flagStatus = 0;
  unsigned code;
  int status, count;

  while (1) {
         if (look_kbd()) {
               code = read_kbd();
               break;
         }
/*       m_getPosAndButtonStatus(&XMOUSE, &YMOUSE, &status);
         if ((status > 0) && (status != flagStatus)) break;
         if ((status == 0) && (status != flagStatus))
              flagStatus = 0;*/
         if ((code = mouseClic) != 0)
               break;

  }
  flagStatus = status;
  mouseClic = 0;
  return(code);
}

look_kbd()
{
/* union  REGS  rin,rout;
 struct SREGS segreg;

 rin.h.ah  = 01;
 int86(0x16, &rin, &rout);
 if (rout.x.cflag) /* Hit Key */
/*     return(1);
 return(0);*/
 return(_look_kbd());
}

read_kbd()
{
/* union  REGS  rin,rout;

 rin.h.ah  = 0;
 int86(0x16, &rin, &rout);*/
 _read_kbd();
}

/*--------------------------------------------------------------
 * getSelectedItemByMouse - Retourne le numero de l'article
 *                          selectionn- par la souris sinon -1
 *--------------------------------------------------------------
 */
getSelectedItemByMouse(mp)
struct Menu *mp;
{

}
int mouseEvent;

it_mouse()
{
    mouseEvent++;
}
