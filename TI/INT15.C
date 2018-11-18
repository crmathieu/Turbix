#include <dos.h>
#include <stdio.h>
extern void  _int15();

unsigned long msdosv_int15 = 0;
char buff[1024];


main()
{

  msdosv_int15 = (unsigned long)_dos_getvect(0x15);
  h = _int15;
  _dos_setvect(0x15, h);
  _dos_setvect(0x61, msdosv_int15);
  _int13( ( 02  << 8)  | 1 ,
          ( 00  << 8)  | 0 ,
          ( 00  << 8)  | 1 ,
           buff);

  _dos_setvect(0x15, msdosv_int15);
}

_checkINT15(subfunc)
{
    switch(subfunc & 0xff00) {
    case 0x9000 : /* se suspendre pendant la duree de l'E/S */
         if ((subfunc & 0x00ff) == 0xfd)  {/* time out motor */
              /*sdelay(3);*/
              return(2);
         }
         return(1);
    case 0x9100 : /* arrive lors de l'interruption disque */
         return(1);
    }
    return(0); /* appeler l'INT15 DOS Normalement */
}




