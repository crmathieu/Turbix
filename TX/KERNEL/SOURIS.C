#include <stdio.h>
#include <dos.h>
main()
{
        union REGS ireg, oreg;
        void (interrupt far *int_handler)();
        long vector;
        unsigned char first_byte;

        /* get interrupt vector */
        int_handler = getvect(0x33);
        first_byte = * (unsigned char far *)int_handler;
        vector = (long) int_handler;

        /* check installation of int33 */
        if ((vector == 0) || (first_byte == 0xcf)) {
                printf("Mouse driver NOT installed\n");
                exit(0);
        }

        /* reset mouse */
        ireg.x.ax = 0;
        int86(0x33, &ireg, &oreg);

        /* check mouse connection */
        if (oreg.x.ax == -1)
                printf("Mouse connected ...\n");
        else
                printf("Mouse NOT connected\n");

}

