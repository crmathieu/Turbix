/* setjmp */

typedef struct {
    unsigned  j_flag;
    unsigned  j_sp;
    unsigned  j_ss;
    unsigned  j_bp;
    unsigned  j_si;
    unsigned  j_di;
    unsigned  j_ds;
    unsigned  j_es;
    unsigned  j_ip;
    unsigned  j_cs;
 } jmp_buf[1];
