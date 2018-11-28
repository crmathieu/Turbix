;****************************************************************************
;*  CXTSWP.ASM                                                              *
;*                                                                          *
;*  Stack switching management                                              *
;*                                                                          *
;*  Other ASSEMBLING modules:                                               *
;*                 VIDEO.ASM , SYSTEM.ASM , GR.ASM , IT_HARD.ASM            *
;*                                                                          *
;****************************************************************************

_TEXT SEGMENT  BYTE PUBLIC 'CODE'
_TEXT ENDS
CONST SEGMENT  WORD PUBLIC 'CONST'
CONST ENDS
_BSS SEGMENT  WORD PUBLIC 'BSS'
_BSS ENDS
_DATA SEGMENT  WORD PUBLIC 'DATA'
_DATA ENDS

DGROUP GROUP CONST, _BSS, _DATA
 ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

PUBLIC  __swapstk
PUBLIC  _mkdSetjmp
PUBLIC  _mkdLongjmp
PUBLIC  __UtoK
PUBLIC  __KtoU

EXTRN  __disable : FAR
EXTRN  __restore : FAR

_TEXT SEGMENT


;*
;*  SWAPSTK( @ current stack descriptor, new stack value)
;*                 bp+6 , bp+8            bp+10 , bp+12
;*        updates stack
;*
__swapstk PROC FAR
     push bp
     mov bp,sp

     push si
     push di
     pushf
     les bx,[bp+6]              ; es = seg desc  bx = offset desc
     mov es:[bx],sp             ; save current task stack offset
     mov es:[bx+2],ss           ; save current task stack seg

; Nouvelle pile

     mov ax,[bp+10]             ; bx = new task stack offset
     mov dx,[bp+12]             ; ss = new task stack segment
     mov sp,ax
     mov ss,dx
;     mov ax,DGROUP    ; to be removed: replace ca with (push ds/pop ds)
;     mov ds,ax        ;

     popf

     pop di
     pop si
     pop bp
     ret
__swapstk ENDP




;*
;*  SETJMP( @ stack context backup structure)
;*
_mkdSetjmp PROC FAR
     push bp
     mov bp,sp
     mov cx,es

     les bx,[bp+6]              ; es = seg structure  bx = offset
     pushf
     pop ax                     ; get flag
     mov es:[bx],ax
     mov es:[bx+2],sp
     mov es:[bx+4],ss
     mov es:[bx+6],bp
     mov es:[bx+8],si
     mov es:[bx+10],di
     mov es:[bx+12],ds
     mov es:[bx+14],cx   ; es
     mov ax,0            ; setjmp returned value

     ; set SETJUMP return value (IP) in structure

     mov dx,[bp+2]
     mov es:[bx+16],dx   ; ip
     mov dx,[bp+4]
     mov es:[bx+18],dx   ; cs
     mov es,cx
     pop bp
     ret
_mkdSetjmp ENDP





;*
;*  LONGJMP( @ structure , SETJMP returned value)
;*
_mkdLongjmp PROC FAR
     push bp
     mov bp,sp

     les bx,[bp+6]              ; es = seg structure  bx = offset
     push es:[bx]
     popf                       ; get flag
     mov ax,[bp+10]             ; read returned value
     mov sp,es:[bx+2]
     mov ss,es:[bx+4]
     mov bp,es:[bx+6]
     mov si,es:[bx+8]
     mov di,es:[bx+10]
     mov ds,es:[bx+12]

     ; placer le contenu de structure.IP at .CS sur la pile
     ; et le transferer dans IP et CS

     ; place structure.IP at .CS on stack 
     ; and transfer it in IP and CS

     push es:[bx+18]    ; get CS
     push es:[bx+16]    ; get IP
     mov cx,es:[bx+14]
     mov es,cx
     pop dx
     mov [bp+2],dx
     pop dx
     mov [bp+4],dx

     add sp,2           ; skip bp (embedded in SETJMP)
     ret
_mkdLongjmp ENDP




;*************** Stack Description when calling KERNELMODE ******************
;
;
;               USER                                         KERNEL
;  +     _____________________
;       |       argn          |
;       |_____________________|
;       |         :           |
;       |_____________________|
;       |       arg1          |
;       |_____________________|                       _____________________
;       |                     |                      |                     |
;       |__     return @    __|   CALL  primitive    |__     return @    __|
;       |   in calling task   |              =======>|    in system call   |
;       |_____________________|             |        |_____________________|
;       |        bp           |             |        |        bp           |
;       |_____________________|             |        |_____________________|
;       |                     |             |        |                     |
;       |   System call       |             |_______________________
;       |   local variables   |                                     |
;   +18 |_____________________|<----- @ to save to USP        _     |
;       |                     |                                |    |
;   +16 |__  return @ in    __|   CALL kernelMode              |    |
;       |     system call     |                                |____|
;   +14 |_____________________|                                |
;       |        bp           |<----- @ to save on             |
;   +12 |_____________________|       system stack            _|
;       |     KSP value       |
;   +10 |_____________________|
;       |                     |
;    +8 |__   @ USP backup  __|
;       |                     |
;    +6 |_____________________|
;       |                     |
;       |__  return @ in    __|   CALL  UtoK
;       |     kernelmode      |
;       |_____________________|
;       |        bp           |
;  -    |_____________________| <- sp , bp



;*
;*  UtoK( @ USP , KSP value)
;*
__UtoK PROC FAR
     push bp
     mov bp,sp
     mov cx,es

     les bx,[bp+6]              ; es = seg  bx = offset
     mov dx,bp
     add dx,18
     mov es:[bx],dx             ; backup stack context in USP
                                ; (holds local variables)

     ; switch to system stack 
     mov sp,[bp+10]             ; get KSP value

     ; we are now on the system stack
     push [bp+16]               ; place system call return @ on system stack 
     push [bp+14]               ; + parameters pointers of system call
     push [bp+12]               ; invoking kernelMode()
     mov es,cx
     pop bp
     ret                        ; we are now at the base of the system stack
__UtoK ENDP



;*************** Stack description when calling USERMODE  ******************
;
;
;               KERNEL                                        USER
;   +    _____________________                __
;       |                     |                 |    |_____________________|
;       |__   return @ in   __|  CALL userMode  |_   |  system call        |
;       |     system call     |                 | |  |  local variables    |
;   +10 |_____________________|                _| |  |_____________________|
;       |        bp           |<-- BP value       |  |                     |
;    +8 |_____________________|    pointing to    |_ |__   return @ in   __|
;       |    valeur  USP      |    parameters        |    system call      |
;    +6 |_____________________| (to be restituted )  |_____________________|
;       |                     |                      |         bp          |
;       |__  @ retour dans  __|   CALL  KtoU         |_____________________|
;       |     userMode        |
;       |_____________________|
;       |        bp           |
;  -    |_____________________| <- sp , bp


;*
;*  KtoU( USP value )
;*
__KtoU PROC FAR
     push bp
     mov bp,sp

     ; switch to user stack
     mov sp,[bp+6]

     ; place system call return @ on user stack
     push [bp+12]
     push [bp+10]
     push [bp+8]      ; bp

     pop  bp
     ret
__KtoU ENDP


_TEXT ENDS
END
