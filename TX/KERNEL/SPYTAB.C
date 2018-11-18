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
 c_sem(),
 c_pipe(),
 c_seml(),
 c_pipel(),
 c_pipec(),
 c_session(),
 c_proc(),
/* win(),*/
 user_app();


struct cmdslot Commandtab[] = {

                    "ps"         ,  c_ps         ,  FALSE ,
                    "kill"       ,  c_kill       ,  FALSE ,
                    "copy"       ,  c_copy       ,  FALSE ,
                    "type"       ,  c_type       ,  FALSE ,
                    "mem"        ,  c_mem        ,  TRUE  ,
                    "more"       ,  c_more       ,  FALSE ,
                    "exit"       ,  c_exit       ,  TRUE  ,
                    "delay"      ,  c_delay      ,  TRUE  ,
                    "?"          ,  c_help       ,  TRUE  ,
                    "help"       ,  c_help       ,  TRUE  ,
                    "cls"        ,  c_cls        ,  TRUE  ,
                    "session"    ,  c_session    ,  FALSE ,
                    "sem"        ,  c_sem        ,  FALSE ,
                    "seml"       ,  c_seml       ,  FALSE ,
                    "pipe"       ,  c_pipe       ,  FALSE ,
                    "pipel"      ,  c_pipel      ,  FALSE ,
                    "pipec"      ,  c_pipec      ,  FALSE ,
                    "proc"       ,  c_proc       ,  FALSE ,
/*                    "win"        ,  wdemo        ,  FALSE,*/
                    "user_app"   ,  user_app     ,  FALSE
};

nb_command()
{
    return(sizeof(Commandtab) / sizeof(struct cmdslot));
}
