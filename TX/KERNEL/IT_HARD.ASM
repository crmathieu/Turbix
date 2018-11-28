;****************************************************************************
;*  IT_HARD.ASM                                                             *
;*                                                                          *
;*  Hardware interrupts management : clock, keyboard, serial com            *
;*                                                                          *
;*  Other ASSEMBLER modules:                                                *
;*                 VIDEO.ASM , GR.ASM , SYSTEM.ASM , CXTSWP.ASM             *
;*                                                                          *
;****************************************************************************

QUANTUM  equ      3
CLOCKFQ  equ      2
arg1     equ  6[bp]
arg2     equ  8[bp]
arg3     equ 10[bp]
arg4     equ 12[bp]

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



EXTRN  __clock       : FAR
EXTRN  __keyboard    : FAR
EXTRN  __ttydisp     : FAR
EXTRN  __flush_kbd    : FAR
EXTRN  __get_char     : FAR
EXTRN  __int24       : FAR
;EXTRN  __handler_disk : FAR

EXTRN  _msdosv_clk : DWORD
EXTRN  _msdosv_kbd : DWORD
;EXTRN  _msdosv_flp : DWORD
;EXTRN  _msdosv_hd  : DWORD

;EXTRN  _curD : WORD

; interrupts handling
PUBLIC  __itcom1
PUBLIC __itclock
PUBLIC __itkbd
;PUBLIC __ithd
;PUBLIC __itflp
PUBLIC __call_it
PUBLIC __dositkbd
PUBLIC __itcritical

;PUBLIC _itdisk
;PUBLIC _diskess

; mask interrupt handling
PUBLIC __activwait
PUBLIC __disable
PUBLIC __enable
PUBLIC __restore


_DATA SEGMENT
_quantum   dw QUANTUM
_clockfreq dw CLOCKFQ
_itcritical_ret db 0
_DATA ENDS

_TEXT SEGMENT


;*
;*  ACTIVWAIT - set CPU in active wait 
;*
__activwait PROC FAR
    sti
P2: wait
    jmp P2
__activwait ENDP

;*
;*   DISABLE - blocks all interrupts and returns previous CCR value
;*
__disable PROC FAR
    pushf
    cli
    pop ax
    ret
__disable ENDP

;*
;*   ENABLE - enables all interrupts
;*
__enable PROC FAR
    sti
    ret
__enable ENDP

;*
;*   RESTORE(ps) - set the CCR with the parameter value (in ps)
;*   int ps
;*
__restore PROC FAR
    push bp
    mov bp,sp
    mov ax,arg1
    push ax
    popf
    pop bp
    ret
__restore ENDP

;*
;*   ITCLOCK - clock interrupt (18 times / second)
;*
__itclock PROC FAR
     push ax
     push bx
     push cx
     push dx
     push bp
     push di
     push si
     push es
     push ds
;     mov ax,20h                ; unlock 8259
;     out 20h,ax
     mov ax,DGROUP
     mov ds,ax
     push word ptr _msdosv_clk+2
     push word ptr _msdosv_clk
     call far ptr __call_it
     add sp,4
     dec _quantum
     jnz itclk10

; 18 / QUANTUM  calls / second
     mov ds:_quantum,QUANTUM
     call FAR PTR __clock

itclk10:
     pop ds
     pop es
     pop si
     pop di
     pop bp
     pop dx
     pop cx
     pop bx
     pop ax
     iret
__itclock ENDP

;*
;*   ITKBD - keyboard interrupt
;*
__itkbd PROC FAR
     cli
     push ax
     mov ax,20h
     out 20h,ax
     push bx
     push cx
     push dx
     push bp
     push di
     push si
     push ds
     push es

     mov ax,DGROUP
     mov ds,ax

     call FAR PTR __keyboard
     call FAR PTR __dositkbd
;     call FAR PTR __flush_kbd
     call FAR PTR __get_char


_itkbd_fin:
     pop es
     pop ds
     pop si
     pop di
     pop bp
     pop dx
     pop cx
     pop bx
     pop ax
     iret
__itkbd  ENDP

;*
;*   DOSITKBD - DOS keyboard interrupt
;*
__dositkbd PROC FAR
     push word ptr _msdosv_kbd+2  ;
     push word ptr _msdosv_kbd    ;
     call far ptr __call_it       ;
     add sp,4                     ;
     ret
__dositkbd  ENDP

;*
;*  ITCOM1 - communication circuit 1 interrupt
;*
__itcom1 PROC FAR
      push ax
;      cli
      mov al,20h
      out 20h,al
      push bx
      push cx
      push dx
      push bp
      push di
      push si
      push ds
      push es
      mov ax,DGROUP
      mov ds,ax
      call FAR PTR __ttydisp
      pop es
      pop ds
      pop si
      pop di
      pop bp
      pop dx
      pop cx
      pop bx
      pop ax
      iret
__itcom1 ENDP

;*
;*  ITCRITICAL   valeur retour dans AL
;*
__itcritical PROC FAR
      push bx
      push cx
      push dx
      push bp
      push di
      push si
      push ds
      push es
      push ax

;     set params on stack
      push ax
      push di
      push bp
      push si

      mov ax,DGROUP
      mov ds,ax
      call FAR PTR __int24
      add sp, 8

      mov  _itcritical_ret, al
      pop ax
      mov  al, _itcritical_ret
      pop es
      pop ds
      pop si
      pop di
      pop bp
      pop dx
      pop cx
      pop bx
      iret
__itcritical ENDP

;*
;*   CALL_IT - calls the interrupt handler provided as a parameter
;*
__call_it PROC FAR
     push bp
     mov bp,sp
     push es
     push bx
     sub sp,4       ; reserver space for necessary local variables
     les bx,arg1
     mov [bp-2],es
     mov [bp-4],bx
     pushf
     call dword ptr [bp-4]
     add sp,4
     pop bx
     pop es
     pop bp
     ret
__call_it  ENDP

;*
;*   ITDISK - disk and floppy interrupts
;*
;_itdisk2 PROC FAR
;     push ax
;     push bx
;     push cx
;     push dx
;     push di
;     push si
;     push ds
;     push es
;     push bp
;     mov bp,sp
;     sub sp,4       ; reserver space for necessary local variables
;     mov ax,DGROUP
;     mov ds,ax
;     cmp _curD,2
;     jge _dcod
;
;     mov dx,word ptr _msdosv_floppy+2
;     mov ax,word ptr _msdosv_floppy
;     mov [bp-2],dx
;     mov [bp-4],ax
;     jmp _diskend
;
;_dcod:
;     mov dx,word ptr _msdosv_disk+2
;     mov ax,word ptr _msdosv_disk
;     mov [bp-2],dx
;     mov [bp-4],ax
;
;_diskend:
;     pushf
;     call dword ptr [bp-4]
;     add sp,4
;     pop bp
;     pop es
;     pop ds
;     pop si
;     pop di
;     pop dx
;     pop cx
;     pop bx
;     pop ax
;     iret
;_itdisk2  ENDP

;__ithd PROC FAR
;     push ax
;     push bx
;     push cx
;     push dx
;     push di
;     push si
;     push ds
;     push es
;     push bp

;     mov ax,DGROUP
;     mov ds,ax
;
;     push word ptr _msdosv_hd+2
;     push word ptr _msdosv_hd
;     call far ptr __call_it
;     add sp,4
;
;     call far ptr _handler_disk
;     pop bp
;     pop es
;     pop ds
;     pop si
;     pop di
;     pop dx
;     pop cx
;     pop bx
;     pop ax
;     iret
;__ithd  ENDP
;
;__itflp PROC FAR
;     push ax
;     push bx
;     push cx
;     push dx
;     push di
;     push si
;     push ds
;     push es
;     push bp
;
;     mov ax,DGROUP
;     mov ds,ax

;     push word ptr _msdosv_flp+2
;     push word ptr _msdosv_flp
;     call far ptr __call_it
;     add sp,4
;
;     call far ptr _handler_disk
;     pop bp
;     pop es
;     pop ds
;     pop si
;     pop di
;     pop dx
;     pop cx
;     pop bx
;     pop ax
;     iret
;__itflp  ENDP


;*
;*   DISKESS - ???
;*
;_diskess PROC FAR
;     push bp
;     mov bp,sp
;     mov dx,word ptr _msdosv_floppy+2
;     mov ax,word ptr _msdosv_floppy
;     pop bp
;     ret
;_diskess  ENDP


_TEXT ENDS
END
