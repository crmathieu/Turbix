#include "cmd.h"
#include "kernel.h"

extern int
 c_help(),
 c_cls(),
 c_ps(),
 c_kill(),
 c_exit(),
 c_sleep(),
 c_more(),
 c_mem(),
 c_resx(),
 c_video(),
 c_dchk(),
 c_dir(),
 c_voir(),
 c_copy(),
 c_amount(),
 c_bmount(),
 c_cmount(),
 c_dmount(),
 c_umount(),
 c_cd(),
 c_md(),
 chrono(),
 ignoredoc(),
 tach2(),
 tach3(),
 tach4(),
 sigctrlc(),
 recep(),
 emett(),
 toto(),
 toto2(),
 pipex(),
 shell();

struct cmdslot Commandtab[] = {

                    "ps"         ,  c_ps         ,  FALSE ,
                    "resx"       ,  c_resx       ,  TRUE  ,
                    "video"      ,  c_video      ,  TRUE  ,
                    "more"       ,  c_more       ,  FALSE ,
                    "mem"        ,  c_mem        ,  FALSE ,
                    "voir"       ,  c_voir       ,  FALSE ,
                    "copy"       ,  c_copy       ,  FALSE ,
                    "a:"         ,  c_amount     ,  TRUE  ,
                    "b:"         ,  c_bmount     ,  TRUE  ,
                    "c:"         ,  c_cmount     ,  TRUE  ,
                    "d:"         ,  c_dmount     ,  TRUE  ,
                    "umount"     ,  c_umount     ,  FALSE ,
                    "cd"         ,  c_cd         ,  TRUE  ,
                    "md"         ,  c_md         ,  TRUE  ,
                    "kill"       ,  c_kill       ,  TRUE  ,
                    "dchk"       ,  c_dchk       ,  TRUE  ,
                    "dir"        ,  c_dir        ,  FALSE ,
                    "exit"       ,  c_exit       ,  TRUE  ,
                    "sleep"      ,  c_sleep      ,  TRUE  ,
                    "?"          ,  c_help       ,  TRUE  ,
                    "cls"        ,  c_cls        ,  TRUE  ,
                    "chrono"     ,  chrono       ,  FALSE ,
                    "pipess"     ,  ignoredoc    ,  FALSE ,
                    "tach2"      ,  tach2        ,  FALSE ,
                    "tach3"      ,  tach3        ,  FALSE ,
                    "tach4"      ,  tach4        ,  FALSE ,
                    "sigctrlc"   ,  sigctrlc     ,  FALSE ,
                    "recep"      ,  recep        ,  FALSE ,
                    "emett"      ,  emett        ,  FALSE ,
                    "toto"       ,  toto         ,  FALSE ,
                    "toto2"      ,  toto2        ,  FALSE ,
                    "pipex"      ,  pipex        ,  FALSE ,
                    "shell"      ,  shell        ,  FALSE
};

nb_command()
{
    return(sizeof(Commandtab) / sizeof(struct cmdslot));
}