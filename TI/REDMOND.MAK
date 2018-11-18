#
# Program: MSxed
#

var.obj : var.c
   cl -c -Zp -Gs -AL -I\c500\include       var.c

iturbo.obj : iturbo.c
   cl -c -Zp -Gs -AL -I\c500\include       iturbo.c

cvideo.obj : cvideo.c
   cl -c -Zp -Gs -AL -I\c500\include       cvideo.c

config.obj : config.c
   cl -c -Zp -Gs -AL -I\c500\include       config.c

fterm.obj : fterm.c
   cl -c -Zp -AL -I\c500\include       fterm.c


findfunc.obj : findfunc.c
   cl -c -Zp -Gs -AL -I\c500\include       findfunc.c

mouse.obj : mouse.c
   cl -c -Zp -Gs -AL -I\c500\include       mouse.c

direct.obj : direct.c
   cl -c -Zp -Gs -AL -I\c500\include       direct.c

menu3.obj : menu3.c
   cl -c -Zp -Gs -AL -I\c500\include       menu3.c

startup2.obj : startup2.c
   cl -c -Zp -Gs -AL -I\c500\include       startup2.c

bigbuf3.obj : bigbuf3.c
   cl -c -Zp -Gs -AL -I\c500\include       bigbuf3.c

block5.obj : block5.c
   cl -c -Zp -Gs -AL -I\c500\include       block5.c

display.obj : display.c
   cl -c -Zp -Gs -AL -I\c500\include       display.c

edmain2.obj : edmain2.c
   cl -c -Zp -Gs -AL -I\c500\include       edmain2.c


help.obj : help.c
   cl -c -Zp -Gs -AL -I\c500\include       help.c

wlink.obj : wlink.c
   cl -c -Zp -Gs -AL -I\c500\include       wlink.c

wtools.obj : wtools.c
   cl -c -Zp -Gs -AL -I\c500\include       wtools.c

ledass.obj : ledass.asm
   masm ledass;

edass.obj : edass.asm
   masm edass;

amdi.obj : amdi.asm
   masm amdi;

mkded.exe : bigbuf3.obj startup2.obj direct.obj fterm.obj findfunc.obj \
                      cvideo.obj menu3.obj block5.obj wlink.obj \
                      display.obj edmain2.obj edass.obj wtools.obj \
                      config.obj mouse.obj iturbo.obj var.obj help.obj
        link  /STACK:4096  @msxed.lnk
