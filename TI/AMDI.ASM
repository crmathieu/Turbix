;****************************************************************************
;*  AMDI.ASM                                                                *
;****************************************************************************


arg1       equ  6[bp]
arg2       equ  8[bp]
arg3       equ 10[bp]
arg4       equ 12[bp]
arg5       equ 14[bp]
arg6       equ 16[bp]
arg7       equ 18[bp]
arg8       equ 20[bp]

VIDRETRACE equ    730

CURS_TEXT SEGMENT  BYTE PUBLIC 'CODE'
CURS_TEXT ENDS
BCONST SEGMENT  WORD PUBLIC 'CONST'
BCONST ENDS
_BSS SEGMENT  WORD PUBLIC 'BSS'
_BSS ENDS
_DATA SEGMENT  WORD PUBLIC 'DATA'
_bpsav   dw 0
_lmargin dw 0
_savnbit dw 0
savbx dw 0

_DATA ENDS

DGROUP GROUP BCONST, _BSS, _DATA
 ASSUME  CS: CURS_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

PUBLIC __refresh     ; copier buffer tampon fenetre sur fenetre physique
PUBLIC __save_screen ; copier la fenetre physique dans un tampon
PUBLIC __read_curs_pos
PUBLIC __wstringDB
PUBLIC __clrwin
PUBLIC _showMouse
PUBLIC _hideMouse
PUBLIC __sti
PUBLIC __cli

EXTRN  _SEGvideo : WORD
EXTRN  _lig:BYTE

CURS_TEXT SEGMENT


;*
;*  REFRESH(xg,yg,xd,yd,buf)
;*   arg    1   2  3  4  5
;*
__refresh PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

;    mov ax,arg5
    mov ax,_SEGvideo

;modif 2 lig
    mov es,ax
    lds si,arg5    ;arg6


    ; calcul offset DI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov di,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add di,ax                ; di = offset du point  (xg,yg)

    mov cx,arg4
    inc cx                   ; effet de bord
    sub cx,arg2              ; cx = nb de lignes dans la fenetre

    mov dx,arg3
    inc dx                   ; effet de bord
    sub dx,arg1              ; dx = nb de words dans une ligne
    cld

BB:
    push cx
    mov cx,dx

    rep movsw                ; transferer
    mov bx,80                ; repositionner DI a la prochaine ligne
    sub bx,dx
    shl bx,1
    add di,bx
    pop cx
    loop BB

    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__refresh ENDP

;*
;*  SAVE_SCREEN(xg,yg,xd,yd,buf)
;*       arg    1   2  3  4  5
;*
__save_screen PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

    les di,arg5        ;arg6      ; destination
;    mov ax,arg5
    mov ax,_SEGvideo
    mov ds,ax
    xor si,si

    ; calcul offset SI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov si,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add si,ax                ; si = offset du point  (xg,yg)

    mov cx,arg4
    inc cx                   ; effet de bord
    sub cx,arg2              ; cx = nb de lignes dans la fenetre

    mov dx,arg3
    inc dx                   ; effet de bord
    sub dx,arg1              ; dx = nb de words dans une ligne
    cld

CC:
    push cx
    mov cx,dx

    rep movsw                ; transferer
    mov bx,80                ; repositionner DI a la prochaine ligne
    sub bx,dx
    shl bx,1
    add si,bx
    pop cx
    loop CC

    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__save_screen ENDP

;*
;*  READ_CURS_POS( x , y )
;*       arg      1&2 3&4
;*
__read_curs_pos PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

    mov ah, 3
    mov bh, 0
    int 10h
    les di,arg1
    mov es:[di], dl
    les di,arg3
    mov es:[di], dh

    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__read_curs_pos ENDP
;*
;*  WSTRINGDB(x,y,nb,newatt,att,buf)
;*   arg      1 2 3    4     5   6
;*
__wstringDB PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

    mov ax,_SEGvideo
    mov es,ax
    xor di,di

    lds si,arg6


    ; calcul offset DI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov di,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add di,ax                ; di = offset du point  (xg,yg)

    mov cx,arg3
    cmp  word ptr arg4,0     ; newatt = 0 signifie "ne pas modifier attribut"
    jnz  ModifATT

;    ne pas modifier l'attribut
JJ0:
    lodsb
    stosb
    inc di
    loop JJ0
    jmp JJ3

;   modifier l'attribut
ModifATT:
    mov ax,arg5              ; attribut
    mov ah,al

JJ1:
    lodsb
    stosw
    loop JJ1

JJ3:
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__wstringDB ENDP

;*
;*  CLRWIN(xg,yg,xd,yd,att,fillCh)
;*   arg   1  2  3  4   5    6
;*
__clrwin PROC FAR
    push bp
    mov bp,sp
    push di
    push es

    mov ax,_SEGvideo
    mov es,ax

    ; calcul offset DI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov di,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add di,ax                ; di = offset du point  (xg,yg)

    mov cx,arg4
    inc cx                   ; effet de bord
    sub cx,arg2              ; cx = nb de lignes dans la fenetre

    mov dx,arg3
    inc dx                   ; effet de bord
    sub dx,arg1              ; dx = nb de words dans une ligne
    cld
    mov ax,arg5              ; att
    mov ah,al
    mov al,arg6              ; caractere de remplissage

GG:
    push cx
    mov cx,dx

    rep stosw                ; transferer
    mov bx,80                ; repositionner DI a la prochaine ligne
    sub bx,dx
    shl bx,1
    add di,bx
    pop cx
    loop GG

    pop es
    pop di
    pop bp
    ret
__clrwin ENDP

_showMouse PROC FAR
     mov ax,1
     int 33h
     ret
_showMouse ENDP

_hideMouse PROC FAR
     mov ax,2
     int 33h
     ret
_hideMouse ENDP

__cli PROC FAR
     cli
     ret
__cli ENDP

__sti PROC FAR
     sti
     ret
__sti ENDP

CURS_TEXT ENDS
END
