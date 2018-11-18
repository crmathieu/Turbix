/*#include "include\cmd.h"
#include "include\const.h"*/
#include "cmd.h"
#include "kernel.h"

extern int
 c_help(),
 c_cls(),
 c_ps(),
 c_kill(),
 c_exit(),
 c_delay(),

 c_mem(),
 c_resx(),
 chrono(),
 interro(),
 sigctrlc(),
 recep(),
 emett(),
 toto(),
 setlongjmp(),
 helpexit(),
 wdemo(),
 ioctl(),
 shmkd(),
 essai(),
 dlook(),
 newsession(),
 c_partition(),
 
 ipc(),
 ipc2(),
 ipc3(),
 MouOn(),
 MouOff();


struct cmdslot Commandtab[] = {

                    "ps"         ,  c_ps         ,  FALSE ,
                    "resx"       ,  c_resx       ,  TRUE  ,
                    "mem"        ,  c_mem        ,  TRUE ,
                    "kill"       ,  c_kill       ,  TRUE  ,
                    "exit"       ,  c_exit       ,  TRUE  ,
                    "delay"      ,  c_delay      ,  TRUE  ,
                    "?"          ,  c_help       ,  TRUE  ,
                    "cls"        ,  c_cls        ,  TRUE  ,
                    "chrono"     ,  chrono       ,  FALSE ,
                    "interro"    ,  interro      ,  FALSE ,
                    "sigctrlc"   ,  sigctrlc     ,  FALSE ,
                    "recep"      ,  recep        ,  FALSE ,
                    "emett"      ,  emett        ,  FALSE ,
                    "toto"       ,  toto         ,  FALSE ,
                    "setlongjmp" ,  setlongjmp   ,  FALSE ,
                    "helpexit"   ,  helpexit     ,  FALSE ,
                    "win"        ,  wdemo        ,  FALSE ,
                    "sh"         ,  shmkd        ,  FALSE ,
                    "ioctl"      ,  ioctl        ,  FALSE ,
                    "essai"      ,  essai        ,  FALSE ,
                    "ipc"        ,  ipc          ,  FALSE ,
                    "ipc3"       ,  ipc3         ,  FALSE ,
                    "ipc2"       ,  ipc2         ,  FALSE ,
                    "session"    ,  newsession   ,  FALSE ,
                    "mou_on"     ,  MouOn        ,  FALSE ,
                    "mou_off"    ,  MouOff       ,  FALSE ,
};

nb_command()
{
    return(sizeof(Commandtab) / sizeof(struct cmdslot));
}