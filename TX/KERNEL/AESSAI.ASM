;****************************************************************************
;*   IT_DOS.ASM                                                             *
;*   Autres modules ASSEMBLEUR :                                            *
;*                  VIDEO.ASM , GR.ASM , IT_HARD.ASM , CXTSWP.ASM           *
;*                                                                          *
;****************************************************************************

arg1 equ  6[bp]
arg2 equ  8[bp]
arg3 equ 10[bp]
arg4 equ 12[bp]
arg5 equ 14[bp]

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

;* gestion des E/S disque *
PUBLIC __ioopen
PUBLIC __iocreat
PUBLIC __ioread
PUBLIC __iowrite
PUBLIC __ioclose
EXTRN __errflag:WORD
PUBLIC __ioremove
PUBLIC __iomove

_DATA SEGMENT
_sseg  dw 0
_soff  dw 0
;__errflag dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*  IOOPEN(access, buffer)
;*
__ioopen PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov al,6[bp]               ; al = access

     lds dx,8[bp]               ; @ Zbuffer

     mov ah,3Dh

     int 21h                    ; appel routine BIOS
     jc  open_err
     jmp open_end

open_err:
     mov __errflag, ax
     mov ax,-1

open_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__ioopen ENDP

;*
;*  IOCREAT(access, buffer)
;*
__iocreat PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov cl,6[bp]               ; cl = access

     lds dx,8[bp]               ; @ Zbuffer

     mov ah,3Ch

     int 21h                    ; appel routine BIOS
     jc  creat_err
     jmp creat_end

creat_err:
     mov __errflag, ax
     mov ax,-1

creat_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iocreat ENDP

;*
;*  IOREAD(handle, buffer, count)
;*
__ioread PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov bx,6[bp]               ; handle

     lds dx,8[bp]               ; @ Zbuffer

     mov cx,12[bp]              ; count

     mov ah,3Fh

     int 21h                    ; appel routine BIOS
     jc  read_err
     jmp read_end

read_err:
     mov __errflag, ax
     mov ax,-1

read_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__ioread ENDP

;*
;*  IOWRITE(handle, buffer, count)
;*
__iowrite PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov bx,6[bp]               ; handle

     lds dx,8[bp]               ; @ Zbuffer

     mov cx,12[bp]              ; count

     mov ah,40h

     int 21h                    ; appel routine BIOS
     jc  write_err
     jmp write_end

write_err:
     mov __errflag, ax
     mov ax,-1

write_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iowrite ENDP

;*
;*  IOCLOSE(handle)
;*
__ioclose PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov bx,6[bp]               ; handle

     mov ah,3Eh

     int 21h                    ; appel routine BIOS
     jc  close_err
     jmp close_end

close_err:
     mov __errflag, ax
     mov ax,-1

close_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__ioclose ENDP

;*
;*  IOMOVE(handle, pos, org)
;*
__iomove PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov bx,6[bp]               ; handle

     mov dx,8[bp]               ; Low pos
     mov cx,10[bp]              ; High pos

     mov al,12[bp]              ; org

     mov ah,42h

     int 21h                    ; appel routine BIOS
     jc  move_err
     jmp move_end

move_err:
     mov __errflag, ax
     mov ax,0   ;-1

move_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iomove ENDP

_TEXT ENDS
END
