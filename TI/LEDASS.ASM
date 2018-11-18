;****************************************************************************
;*  CURS.ASM                                                                *
;*                                                                          *
;*  Gestion  fenetre  systeme                                               *
;*                                                                          *
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
;MASKW  db  80h
;      db  40h
;      db  20h
;      db  10h
;      db  08h
;      db  04h
;      db  02h
;      db  01h
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
PUBLIC __scrollbuff  ; scrolling dans un buffer
PUBLIC __wstringDW   ; copier buffer ligne (att + char)
PUBLIC __save_screen ; copier la fenetre physique dans un tampon
PUBLIC __brighten    ; changer d'attribut sur une ligne
PUBLIC __clrwin      ; RAZ window
PUBLIC __wstringDB   ; copier buffer ligne (char)
PUBLIC __rstringDW   ; copier ligne physique dans tampon (att + char)
PUBLIC __read_kbd
PUBLIC _prolog_it_mouse ; traitement utilisateur it souris
PUBLIC __look_kbd
PUBLIC __sti
PUBLIC __cli
PUBLIC _flush_kbd
PUBLIC _flush_buffer
PUBLIC _showMouse
PUBLIC _hideMouse
PUBLIC __ioread
PUBLIC __iowrite

EXTRN  _SEGvideo : WORD
EXTRN  _it_mouse:FAR
EXTRN  _lig:BYTE
EXTRN  _DosInt10 : DWORD
CURS_TEXT SEGMENT

;*
;*  _flush_buffer(hd)
;*
_flush_buffer PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

    mov bx, arg1
    mov ah, 68h
    int 21h
    jc  fb_err
    xor ax,ax
    jmp fb_end

fb_err:
    mov ax,-1

fb_end:

    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
_flush_buffer ENDP

;*
;*  RSTRING(x,y,nb,buf)
;*   arg    1 2 3  4
;*
__rstringDW PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

;    mov ax,0B800h   ; pour le moment
    mov ax,_SEGvideo
    mov ds,ax
    xor si,si

    les di,arg4


    ; calcul offset SI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov si,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add si,ax                ; si = offset du point  (xg,yg)

    mov cx,arg3
    rep movsw

    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__rstringDW ENDP

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
    call FAR PTR _hideMouse
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

    call FAR PTR _showMouse
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__refresh ENDP
;*
;*  CLRWIN(xg,yg,xd,yd,att,fillCh)
;*   arg   1  2  3  4   5    6
;*
__clrwin PROC FAR
    push bp
    mov bp,sp
    push di
    push es

;    mov ax,0B800h
    call FAR PTR _hideMouse
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

    call FAR PTR _showMouse
    pop es
    pop di
    pop bp
    ret
__clrwin ENDP

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

    call FAR PTR _hideMouse
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

    call FAR PTR _showMouse
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__save_screen ENDP

;*
;*  WSTRINGDW(x,y,nb,buf)
;*   arg    1 2 3  4
;*
__wstringDW PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

;    mov ax,0B800h   ; pour le moment
    call FAR PTR _hideMouse
    mov ax,_SEGvideo
    mov es,ax
    xor di,di

    lds si,arg4


    ; calcul offset DI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov di,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add di,ax                ; di = offset du point  (xg,yg)

    mov cx,arg3
    rep movsw

    call FAR PTR _showMouse
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__wstringDW ENDP
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

    call FAR PTR _hideMouse
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
    call FAR PTR _showMouse
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__wstringDB ENDP

;*
;*  BRIGHTEN(x,y,nb,att)
;*   arg     1 2 3  4
;*
__brighten PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

;    mov ax,0B800h   ; pour le moment
    call FAR PTR _hideMouse
    mov ax,_SEGvideo
    mov es,ax
    xor di,di

    mov ds,ax
    xor si,si

    ; calcul offset DI & SI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov di,ax
    mov ax,arg1              ; rajouter les caracteres a gauche
    shl ax,1
    add di,ax                ; di = offset du point  (xg,yg)
    mov si,di
    mov bx,arg4
    mov cx,arg3

EE:
    lodsw
    mov ah,bl
    stosw
    loop EE

    call FAR PTR _showMouse
    pop ds
    pop es
    pop di
    pop si
    pop bp
    ret
__brighten ENDP

;*
;*  SCROLLBUFF(segbuf,size,lsize,nbline,attribut) - scrolling en memoire
;*  char * segbuf;    /* adresse buffer prend arg1 et arg2
;*  int size;         /* taille du buffer (en word)
;*  int lsize;        /* taille d'une ligne (en word)
;*  int nline;        /* nombre de lignes a scroller
;*  int attribut;     /* video attribut de la nouvelle ligne
;*
__scrollbuff PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push ds
    lds si,arg1 ;;
    les di,arg1 ;;

;    mov ds,arg1
;    mov es,arg1
;    mov di,0
;    mov si,di

    mov ax,arg5   ;arg4       ; ax = nb de lignes a scroller
    or ax,ax
    js  _bscrolldwn           ; si < 0

    mov cx,arg4  ;arg3              ; taille d'une ligne
    shl cx,1
    mul cx                   ; ax = nb de bytes a scroller
    add si,ax                ; si = @ debut scrolling

    mov cx, arg3 ;arg2             ; nb total de words dans le buffer
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer
    cld

    rep movsw                ; transferer
    mov ax,arg5  ;arg4              ; nline
    mov cx,arg4  ;arg3              ; lsize
    mul cx
    mov cx,ax                ; cx = nb de words a mettre a blanc
    jmp _bscrollblank

;  SCROLLING INVERSE
_bscrolldwn:
    neg ax
    mov bx,arg3  ;arg2              ; se positionner en fin de plan memoire
    dec bx
    shl bx,1
;    mov di,bx
    add di,bx ;;
;    mov si,di
    add si,bx ;;
    mov cx,arg4  ;arg3
    shl cx,1
    mul cx
    sub si,ax

    mov cx, arg3  ;arg2          ; nb total de words dans un ecran
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer

    std                      ; progresser en decrementant

    rep movsw                ; transferer
    mov ax,arg5  ;arg4              ; nline a mettre a blanc
    neg ax
    mov cx,arg4  ;arg3              ; lsize
    mul cx
    cld
    les di,arg1

;  RAJOUTER LA OU LES NOUVELLES LIGNES
_bscrollblank:
    mov ax,arg6  ;arg5
    mov ah,al
    xor al,al                ; rajouter  nrlines
    rep stosw
    pop ds
    pop di
    pop si
    pop bp
    ret
__scrollbuff ENDP


__read_kbd PROC FAR
     mov ah,0
     int 16h
     ret
__read_kbd ENDP

_prolog_it_mouse PROC FAR
     push ds
     push ax
     mov ax, DGROUP
     mov ds,ax
     pop ax
     call FAR PTR _it_mouse
     pop ds
     ret
_prolog_it_mouse ENDP

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

__cli PROC FAR
     cli
     ret
__cli ENDP

__sti PROC FAR
     sti
     ret
__sti ENDP

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

CURS_TEXT ENDS
END
