mkd_ref.obj : mkd_ref.c
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO mkd_ref.c

refcall.obj : refcall.c
   tcc -c -ml -I\tc\include -L\tc\lib -DTURBO refcall.c

mkd_ref.exe : mkd_ref.obj ledass.obj emsg_us.obj refcall.obj
   tlink \tc\lib\c0l+mkd_ref+ledass+refcall+emsg_us,mkd_ref,,\tc\lib\emu  \tc\lib\MATHL \tc\lib\CL
