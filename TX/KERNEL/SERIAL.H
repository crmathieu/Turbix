/* serial.h */

/*
 * adresses des registres pour un circuit donn‚
 *
 */

struct     csr            {
           portaddr       csetit;     /* mise en fonction des it */
           portaddr       ctypit;     /* identification de l'it  */
           portaddr       crtbuf;     /* reg de reception        */
           portaddr       ccntrl;     /* reg control  ligne      */
           portaddr       ccntrm;     /* reg control  modem      */
           portaddr       cetlgn;     /* reg etat     ligne      */
};

extern  struct  csr Serialtab[];


#define    C8259          0x20    /* registre du controleur 8259              */
#define    ITEND          0x20    /* indique fin d'it au controleur 8259      */
#define    IT_ATTENTE     0       /* indique qu'une interrup est en attente   */

/*
 * constantes relatives a la gestion du circuit serie ( 8250 )
 *
 */

#define    S_ENABLE      0x07    /* valider toutes les its             */
#define    S_READY       0x80
#define    S_DISABLE     0x05    /* masque inhibition des it emission  */
#define    S_TBREAK      0x01
#define    S_ERMASK      0xf000
#define    S_CHMASK      0xff
#define    S_1SETIT      0x3f9   /* @reg de mise en fnct des it du c1  */
#define    S_1TYPIT      0x3fa   /* @reg d'identification des it du c1 */
#define    S_1IOREG      0x3f8   /* @reg buffer e/s  du 8250 du c1     */
#define    S_1CNTRL      0x3fb   /* @reg  control  ligne               */
#define    S_1CNTRM      0x3fc   /* @reg  control  du modem            */
#define    S_1ETLGN      0x3fd   /* @reg  etat     ligne               */
