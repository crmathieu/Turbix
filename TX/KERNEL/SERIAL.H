/* serial.h */

/* registers addresses for a given COM */

struct  csr  {
           portaddr       csetit;     /* set interrupt          */
           portaddr       ctypit;     /* interrupt type         */
           portaddr       crtbuf;     /* receiving register     */
           portaddr ccntrl;           /* line control register  */
           portaddr ccntrm;           /* modem control register */
           portaddr       cetlgn;     /* line state register    */
};

extern  struct  csr Serialtab[];


#define    C8259          0x20    /* 8259 controller register                        */
#define    ITEND          0x20    /* indicate end of int to 8259 controller          */
#define    IT_ATTENTE     0       /* indicate an interrupt is waiting for processing */

/* constants specific to COM 8250 controller management */

#define    S_ENABLE      0x07    /* validate all interrupts            */
#define    S_READY       0x80
#define    S_DISABLE     0x05    /* mask to disable emit interrupt     */
#define    S_TBREAK      0x01
#define    S_ERMASK      0xf000
#define    S_CHMASK      0xff
#define    S_1SETIT      0x3f9   /* @reg of interrupt setting for com1 */
#define    S_1TYPIT      0x3fa   /* @reg of interrupt type for com1    */
#define    S_1IOREG      0x3f8   /* @reg of 8250 buffer i/o for com1   */
#define    S_1CNTRL      0x3fb   /* @reg of line control               */
#define    S_1CNTRM      0x3fc   /* @reg of modem control              */
#define    S_1ETLGN      0x3fd   /* @reg of line state                 */
