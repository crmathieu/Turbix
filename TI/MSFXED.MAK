#
# Program: msFrenchXed
#

fvar.obj : fvar.c     xed.h
   cl -c -Zp -Gs -AL -I\c500\include       fvar.c

iturbo.obj : iturbo.c     xed.h
   cl -c -Zp -Gs -AL -I\c500\include       iturbo.c

cvideo.obj : cvideo.c     xed.h
   cl -c -Zip -Gs -AL -I\c500\include       cvideo.c

config.obj : config.c     xed.h
   cl -c -Zp -Gs -AL -I\c500\include       config.c

fterm.obj : fterm.c       xed.h
   cl -c -Zip -Gs -AL -I\c500\include       fterm.c

findfunc.obj : findfunc.c
   cl -c -Zp -Gs -AL -I\c500\include       findfunc.c

mouse.obj : mouse.c       xed.h
   cl -c -Zp -Gs -AL -I\c500\include       mouse.c

direct.obj : direct.c     xed.h
   cl -c -Zp -Gs -AL -I\c500\include       direct.c

menu3.obj : menu3.c       xed.h
   cl -c -Zp -Gs -AL -I\c500\include       menu3.c

startup2.obj : startup2.c xed.h
   cl -c -Zp -Gs -AL -I\c500\include       startup2.c

bigbuf3.obj : bigbuf3.c   xed.h
   cl -c -Zp -Gs -AL -I\c500\include       bigbuf3.c

block5.obj : block5.c     xed.h
   cl -c -Zip -Gs -AL -I\c500\include       block5.c

display.obj : display.c   xed.h
   cl -c -Zp -Gs -AL -I\c500\include       display.c

edmain2.obj : edmain2.c   xed.h
   cl -c -Zp -Gs -AL -I\c500\include       edmain2.c


help.obj : help.c     xed.h
   cl -c -Zip -Gs -AL -I\c500\include       help.c

wlink.obj : wlink.c     xed.h
   cl -c -Zp -Gs -AL -I\c500\include       wlink.c

wtools.obj : wtools.c     xed.h
   cl -c -Zip -Gs -AL -I\c500\include       wtools.c

ledass.obj : ledass.asm
   tasm ledass;

amdi.obj : amdi.asm
   tasm amdi;

edass.obj : edass.asm
   tasm edass;

#
# Partie TURBO
#

mdi.obj : mdi.c xed.h
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mdi.c

mmsg_fr.obj : mmsg_fr.c
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mmsg_fr.c

mkd_make.obj : mkd_make.c xed.h
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mkd_make.c

emsg_fr.obj : emsg_fr.c xed.h
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO emsg_fr.c

mkd_comp.obj : mkd_comp.c xed.h
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mkd_comp.c

mkd_link.obj : mkd_link.c xed.h
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mkd_link.c

mdi.exe : mdi.obj mmsg_fr.obj amdi.obj
   tlink \tc\lib\c0l+mdi+amdi+mmsg_fr,mdi,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL

mkd_make.exe : mkd_make.obj ledass.obj emsg_fr.obj
   tlink \tc\lib\c0l+mkd_make+ledass+emsg_fr,mkd_make,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL

mkd_comp.exe : mkd_comp.obj ledass.obj emsg_fr.obj
   tlink \tc\lib\c0l+mkd_comp+ledass+emsg_fr,mkd_comp,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL

mkd_link.exe : mkd_link.obj ledass.obj emsg_fr.obj
   tlink \tc\lib\c0l+mkd_link+ledass+emsg_fr,mkd_link,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL

mkd_ref.obj : mkd_ref.c
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mkd_ref.c

refcall.obj : refcall.c
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO refcall.c

mkd_ref.exe : mkd_ref.obj ledass.obj emsg_fr.obj refcall.obj
   tlink \tc\lib\c0l+mkd_ref+ledass+refcall+emsg_fr,mkd_ref,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL


mkded.exe : bigbuf3.obj startup2.obj direct.obj fterm.obj findfunc.obj \
                      cvideo.obj menu3.obj block5.obj  wtools.obj \
                      display.obj edmain2.obj edass.obj wlink.obj \
                      config.obj mouse.obj iturbo.obj fvar.obj help.obj
        link  /CO /STACK:4096  @msfxed.lnk
