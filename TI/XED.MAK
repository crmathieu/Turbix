#
# Program: Xed
#

edass.obj : edass.asm
   masm edass;

iturbo.obj : iturbo.c     xed.h
   cl -c -Od    -AL  -Zp iturbo.c

help.obj : help.c     xed.h
   cl -c -Od    -AL  -Zp help.c

cvideo.obj : cvideo.c   xed.h
   cl -c -Od    -AL  -Zp cvideo.c

config.obj : config.c   xed.h
   cl -c -Od    -AL  -Zp config.c

fterm.obj : fterm.c     xed.h
   cl -c -Od    -AL -Zp fterm.c

mouse.obj : mouse.c     xed.h
   cl -c -Od    -AL -Zp mouse.c

direct.obj : direct.c   xed.h
   cl -c -Od    -AL -Zp direct.c

menu3.obj : menu3.c     xed.h
   cl -c -Od    -AL  -Zp menu3.c

startup2.obj : startup2.c       xed.h
   cl -c -Od    -AL  -Zip startup2.c

bigbuf3.obj : bigbuf3.c         xed.h
   cl -c -Od    -AL  -Zp bigbuf3.c

block5.obj : block5.c   xed.h
   cl -c -Od    -AL  -Zp block5.c

display.obj : display.c         xed.h
   cl -c -Od    -AL -Zp display.c

edmain2.obj : edmain2.c         xed.h
   cl -c -Od    -AL -Zip edmain2.c

xed.exe : bigbuf3.obj startup2.obj direct.obj fterm.obj \
                      cvideo.obj menu3.obj block5.obj  \
		      display.obj edmain2.obj edass.obj \
		      config.obj mouse.obj help.obj
	link  /STACK:4096 /CO @Xed.lnk  ;
