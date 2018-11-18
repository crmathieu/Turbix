/* slu.h */

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

extern  struct  csr slutab[];


#define    C8259          0x20    /* registre du controleur 8259              */
#define    ITEND          0x20    /* indique fin d'it au controleur 8259      */
#define    IT_ATTENTE     0       /* indique qu'une interrup est en attente   */

/*
 * constantes relatives a la gestion du circuit serie ( 8250 )
 *
 */

#define    SLUENABLE      0x07    /* valider toutes les its             */
#define    SLUREADY       0x80
#define    SLUDISABLE     0x05    /* masque inhibition des it emission  */
#define    SLUTBREAK      0x01
#define    SLUERMASK      0xf000
#define    SLUCHMASK      0xff
#define    SLU1SETIT      0x3f9   /* @reg de mise en fnct des it du c1  */
#define    SLU1TYPIT      0x3fa   /* @reg d'identification des it du c1 */
#define    SLU1IOREG      0x3f8   /* @reg buffer e/s  du 8250 du c1     */
#define    SLU1CNTRL      0x3fb   /* @reg  control  ligne               */
#define    SLU1CNTRM      0x3fc   /* @reg  control  du modem            */
#define    SLU1ETLGN      0x3fd   /* @reg  etat     ligne               */
