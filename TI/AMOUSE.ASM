;****************************************************************************
;*   AMOUSE.ASM                                                             *
;****************************************************************************

arg1 equ [bp+6]
arg2 equ [bp+8]
arg3 equ [bp+10]
arg4 equ [bp+12]
arg5 equ [bp+14]
arg6 equ [bp+16]


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

EXTRN _ib:DWORD
EXTRN _ibad:DWORD
PUBLIC _getinfo
PUBLIC __look_kbd
PUBLIC __read_kbd
PUBLIC _flush_kbd
PUBLIC __cli
PUBLIC __sti
EXTRN _user_mouse_routine:FAR
PUBLIC _it_mouse_user

_DATA SEGMENT
savbx dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*  MOUSE_INTERFACE (@ descripteur interface block)
;*
_getinfo PROC FAR
     push es
     push ax
     push bx
     mov savbx,bx
     mov bx,seg _ib
     mov es,bx
     mov bx,offset _ib
     mov es:[bx],ax
     mov ax,savbx
     mov es:[bx+2],ax
     mov es:[bx+4],cx
     mov es:[bx+6],dx
     mov es:[bx+8],di
     mov es:[bx+10],si
     pop bx
     pop ax
     pop es
     ret
_getinfo ENDP

_it_mouse_user PROC FAR
     push ds
     push ax
     mov ax, DGROUP
     mov ds,ax
     pop ax
     call FAR PTR _getinfo
     call FAR PTR _user_mouse_routine
     pop ds
     ret
_it_mouse_user ENDP

_flush_kbd PROC FAR
     mov ax,0c00h
     int 21h
     ret
_flush_kbd ENDP

__look_kbd PROC FAR
     mov ah,01
     int 16h
     jnz code_available
     mov ax,0

code_available:
     ret
__look_kbd ENDP

__read_kbd PROC FAR
     mov ah,0
     int 16h
     ret
__read_kbd ENDP

__cli PROC FAR
     cli
     ret
__cli ENDP

__sti PROC FAR
     sti
     ret
__sti ENDP

_TEXT ENDS
END
