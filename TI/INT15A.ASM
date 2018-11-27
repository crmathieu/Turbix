arg1 equ  6[bp]
arg2 equ  8[bp]
arg3 equ 10[bp]
arg4 equ 12[bp]

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

EXTRN   __checkINT15  : FAR
EXTRN   _msdosv_int15 : DWORD

;* gestion des E/S disque *
PUBLIC  __int13         ; READ / WRITE operations
PUBLIC  __int15         ; gestion preemption durant l'entree sortie

_DATA SEGMENT
_DATA ENDS

_TEXT SEGMENT



;*
;*  INT13(codop & nbsec,head & nolec,cyl & sec,@buffer)
;*     BP +       6            8           10       12
;*               ax           dx           cx       bx/es
;*          renvoie un code retour dans AX
;*
__int13 PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf
     mov ax,6[bp]               ; code op & nbsec

     les bx,12[bp]              ; @ buffer

     mov dx,8[bp]               ; head & nolec

     mov cx,10[bp]              ;  cyl & sec

     int 13h                    ; appel routine BIOS
     jc  int131
     mov ax,0

int131:
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
__int13 ENDP

;*
;*  INT15 - filtre l'interruption 15h pour capturer les sous fonctions
;*          90h (device wait)  et 91h (device post)
;*
__int15_2 PROC FAR
;    sauvegarder les parametres pass-s a l'INT 15h
     push ds
     push ax
     push bx
     push cx
     push dx
     push si
     push di
     push es
     push bp

;    verifier le numero de sous fonction
     push ax
     mov ax,DGROUP
     mov ds,ax
     call far ptr __checkINT15
     add sp,2
     pop bp
     pop es
     pop di
     pop si
     pop dx
     pop cx
     pop bx
     or ax,ax
     jz DOSint15    ; appeler l'ancienne IT si 0
     cmp ax,2

     pop ax
     pop ds
     iret

DOSint15:
     pop ax
     pop ds
     int 61h
     iret
__int15_2 ENDP
;*
;*  INT15 - filtre l'interruption 15h pour capturer les sous fonctions
;*          90h (device wait)  et 91h (device post)
;*
__int15 PROC FAR
;    sauvegarder les parametres pass-s a l'INT 15h
     push ds
     push ax
     push bx
     push cx
     push dx
     push si
     push di
     push es
     push bp

;    verifier le numero de sous fonction
     push ax
     mov ax,DGROUP
     mov ds,ax
     call far ptr __checkINT15
     add sp,2
     pop bp
     pop es
     pop di
     pop si
     pop dx
     pop cx
     pop bx
     or ax,ax
     jz BIOSint15    ; appeler l'ancienne IT si 0
     cmp ax,2
     jnz MYINT15
     push bx
     mov bx, sp
     or byte ptr ss:[bx+10],1     ; positionner CARRY FLAG
     pop bx

MYINT15:
     pop ax
     pop ds
     iret

BIOSint15:
     pop ax
     pop ds
     int 61h  ; perform BIOS INT15
     iret
__int15 ENDP


_TEXT ENDS
END
