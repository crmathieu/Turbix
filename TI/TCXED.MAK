#
# Program: TCXed
#
iturbo.obj : iturbo.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   iturbo.c;
var.obj : var.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   var.c;
cvideo.obj : cvideo.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   cvideo.c

config.obj : config.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   config.c

fterm.obj : fterm.c       xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   fterm.c

mouse.obj : mouse.c       xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   mouse.c

direct.obj : direct.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   direct.c

menu3.obj : menu3.c       xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   menu3.c

startup2.obj : startup2.c xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   startup2.c

bigbuf3.obj : bigbuf3.c   xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   bigbuf3.c

block5.obj : block5.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   block5.c

display.obj : display.c   xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   display.c

edmain2.obj : edmain2.c   xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   edmain2.c


help.obj : help.c     xed.h
   tcc -c -I\tc\include    -ml -DTURBO -v -y   help.c

xed.exe : bigbuf3.obj startup2.obj direct.obj fterm.obj \
                      cvideo.obj menu3.obj block5.obj  \
		      display.obj edmain2.obj edass.obj \
		      config.obj mouse.obj iturbo.obj var.obj
	tlink  /STACK:4096  @tcxed.lnk
