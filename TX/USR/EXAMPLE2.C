/* C program example2 */

/* This program run without the SPY interpreter.
 * We don't need to create a user_app procedure
 * because the SPY is not invoked
 */
umain(argc, argv)
int argc;
char *argv[];
{
        /* Start the hanoi tower program (defined in sample.c) */
        /*m_StopPreemption();*/
        wdemo();
}
user_app(){}
