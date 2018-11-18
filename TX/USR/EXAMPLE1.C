/* C program example1 */

/* This program run with the SPY interpreter */
#include "const.h"
char *argv[4];
umain(argc, argv)
int argc;
char *argv[];
{
        int m_Spy(), ret;

        argv[0] = "Spy";
        argv[1] = "-i";
        argv[3] = NULL;

        /* execute the command interpreter in interactive mode */
        if (m_Fork() == 0)
                m_Exec(m_Spy, argv);
        m_Wait(&ret);
}

/* Actual user application called through the SPY */
user_app()
{
        /* start a procedure defined in sample.c */
        cxtswitch();
}