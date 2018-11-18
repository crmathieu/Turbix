#include "cmd.h"
#include "kernel.h"

extern int
 c_help(),
 c_cls(),
 c_type(),
 c_copy(),
 c_ps(),
 c_kill(),
 c_exit(),
 c_delay(),
 c_more(),
 c_mem(),
 c_resx(),
 sigctrlc(),
 recep(),
 emett(),
 loop(),
 setlongjmp(),
 wdemo(),
 mkdSpy(),
 newsession();


struct cmdslot Commandtab[] = {

                    "ps"         ,  c_ps         ,  FALSE ,
                    "resx"       ,  c_resx       ,  TRUE  ,
                    "kill"       ,  c_kill       ,  TRUE  ,
                    "copy"       ,  c_copy       ,  FALSE ,
                    "type"       ,  c_type       ,  FALSE ,
                    "mem"        ,  c_mem        ,  TRUE  ,
                    "more"       ,  c_more       ,  TRUE  ,
                    "exit"       ,  c_exit       ,  TRUE  ,
                    "delay"      ,  c_delay      ,  TRUE  ,
                    "?"          ,  c_help       ,  TRUE  ,
                    "cls"        ,  c_cls        ,  TRUE  ,
                    "sigctrlc"   ,  sigctrlc     ,  FALSE ,
                    "recep"      ,  recep        ,  FALSE ,
                    "emett"      ,  emett        ,  FALSE ,
                    "loop"       ,  loop         ,  FALSE ,
                    "setlongjmp" ,  setlongjmp   ,  FALSE ,
                    "win"        ,  wdemo        ,  FALSE ,
                    "sh"         ,  mkdSpy       ,  FALSE ,
                    "session"    ,  newsession   ,  FALSE
};

nb_command()
{
    return(sizeof(Commandtab) / sizeof(struct cmdslot));
}