;****************************************************************************
;*   IT_DOS.ASM                                                             *
;*   Other ASSEMBLER modules:                                               *
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

;* I/O disk management *
PUBLIC __ioopen
PUBLIC __iocreat
PUBLIC __ioread
PUBLIC __iowrite
PUBLIC __ioclose
PUBLIC __setHandleCount
PUBLIC __getcwdir
PUBLIC __iomkdir
PUBLIC __iogetfatt
PUBLIC __iosetfatt

PUBLIC __ioremove
PUBLIC __iomove
EXTRN  __errflag:WORD

_DATA SEGMENT
_sseg     dw 0
_soff     dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*  setHandleCount(handles)
;*
__setHandleCount PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf

     mov bx,6[bp]               ; bx = # handles
     mov ah,67h

     int 21h                    ; call BIOS
     jc  setHc_err
     xor ax,ax
     jmp setHc_end

setHc_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

setHc_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__setHandleCount ENDP

;*
;*  getcwdir(pstring, dev)
;*
__getcwdir PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf

     lds si,6[bp]
     mov dl,10[bp]              ; drive
     mov ah,47h

     int 21h                    ; call BIOS
     jc  gwd_err
     xor ax,ax
     jmp gwd_end

gwd_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

gwd_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__getcwdir ENDP

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

     int 21h                    ; call BIOS
     jc  open_err
     jmp open_end

open_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
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

     int 21h                    ; call BIOS
     jc  creat_err
     jmp creat_end

creat_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
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

     int 21h                    ; call BIOS
     jc  read_err
     jmp read_end

read_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
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

     int 21h                    ; call BIOS
     jc  write_err
     jmp write_end

write_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
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

     int 21h                    ; call BIOS
     jc  close_err
     xor ax,ax
     jmp close_end

close_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
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
;*  IOREMOVE(filename)
;*
__ioremove PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     lds dx,6[bp]               ; filename (ZSTRING)  

     mov ah,41h

     int 21h                    ; call BIOS
     jc  remove_err
     xor ax,ax
     jmp remove_end

remove_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

remove_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__ioremove ENDP

;*
;*  IOMOVE(handle, pos, orig)
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

     mov al,12[bp]              ; orig

     mov ah,42h

     int 21h                    ; call BIOS
     jc  move_err
     jmp move_end

move_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

move_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iomove ENDP

;*
;*  IOSETFATT(filename, att)
;*
__iosetfatt PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     lds dx,6[bp]               ; filename (ZSTRING)
     mov cx,10[bp]

     mov ax,4301h

     int 21h                    ; call DOS
     jc  setfatt_err
     xor ax,ax
     jmp setfatt_end

setfatt_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

setfatt_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iosetfatt ENDP

;*
;*  IOGETFATT(filename)
;*
__iogetfatt PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     lds dx,6[bp]               ; filename (ZSTRING)

     mov ax,4300h

     int 21h                    ; call DOS
     jc  getfatt_err
     mov ax,cx                  ; attributs in cx
     jmp getfatt_end

getfatt_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

getfatt_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iogetfatt ENDP

;*
;*  IOMKDIR(Path)
;*
__iomkdir PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf

     lds dx,6[bp]               ; @ path

     mov ah,39h

     int 21h                    ; call BIOS
     jc  mkd_err
     xor ax,ax
     jmp mkd_end

mkd_err:
     mov bx, DGROUP
     mov es, bx
     mov es:__errflag, ax
     mov ax,-1

mkd_end:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__iomkdir ENDP

_TEXT ENDS
END
