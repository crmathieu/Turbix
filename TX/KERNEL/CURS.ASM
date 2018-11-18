;****************************************************************************
;*  CURS.ASM                                                                *
;*                                                                          *
;*  Gestion  fenetre  systeme                                               *
;*                                                                          *
;*  Autres Modules ASSEMBLEUR :                                             *
;*           GR.ASM , SYSTEM.ASM , IT_HARD.ASM , CXTSWP.ASM , VIDEO.ASM     *
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
CONST SEGMENT  WORD PUBLIC 'CONST'
;MASKW  db  80h
;      db  40h
;      db  20h
;      db  10h
;      db  08h
;      db  04h
;      db  02h
;      db  01h
CONST ENDS
_BSS SEGMENT  WORD PUBLIC 'BSS'
_BSS ENDS
_DATA SEGMENT  WORD PUBLIC 'DATA'
_bpsav   dw 0
_lmargin dw 0
_savnbit dw 0
_DATA ENDS
DGROUP GROUP CONST, _BSS, _DATA
 ASSUME  CS: CURS_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP


PUBLIC __refresh     ; copier buffer tampon fenetre sur fenetre physique
PUBLIC __scrollbuff  ; scrolling dans un buffer

;PUBLIC __refreshM
;EXTRN   _maskbit:BYTE

_DATA SEGMENT

_DATA ENDS

CURS_TEXT SEGMENT


;*
;*  REFRESH(xg,yg,xd,yd,vs_base,buf)
;*   arg    1   2  3  4    5     6
;*
__refresh PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push es
    push ds

    mov ax,arg5
    mov es,ax
    mov ax,arg6     ; assume buf du type SEG:0000
    mov ds,ax
    mov si,0

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

;*
;*  REFRESHM(begx+1,begy+1,xd,yd,vs_base,seg_buf,@_mask) (PAS AU POINT)
;*   arg       1      2    3  4    5        6       7&8
;*
;__refreshM PROC FAR
;    push bp
;    mov bp,sp
;    push si
;    push di
;    push es
;    push ds
;    push ax
;    push bx
;    push cx
;    push dx
;
;    mov ax,arg6     ; assume buf du type SEG:0000
;    mov ds,ax
;    mov si,0
;
;    ; calcul offset DI
;    mov ax,arg2
;    mov cx,80 * 2
;    mul cx
;    mov di,ax
;    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
;    shl ax,1
;    add di,ax                ; di = offset du point  (xg,yg)
;
;    ; calcul offset octet dans masque  (yg * 10) + (xg/8)
;    mov ax,arg2
;    mov cx,10
;    mul cx
;    mov bx,ax
;    mov ax,arg1
;    shr ax,1
;    shr ax,1
;    shr ax,1
;    add bx,ax
;
;    ; calcul nb de lignes
;    mov cx,arg4
;    inc cx                   ; effet de bord
;    sub cx,arg2              ; cx = nb de lignes dans la fenetre
;
;    ; calcul taille ligne
;    mov dx,arg3
;    inc dx                   ; effet de bord
;    sub dx,arg1              ; dx = nb de words dans une ligne
;    cld
;    mov ax,arg8  ;;
;    mov es,ax
;    mov ax,arg7  ;;
;    add bx,ax    ;;

;;    xor bx,bx                ; es:bx pointe sur la BITMAP de masque
;;    xor ax,ax                ; init refresh : ax = 0
;    mov ax,arg1
;    and al,07h                ; al = xg % 8 offset bit / gauche dans
;                              ; octet courant
;    mov DGROUP:_bpsav,bp
;
;BB1:                          ; cx = #lignes           - dx = #car/ligne
;                              ; bx = offset OCTET masq - ax = offset BIT masq
;    push cx
;    mov cx,dx
;
;TEST:
;
;    push ax                  ; ax = no du bit a tester dans le masque
;    mov bp,ax
;    and bp,07h
;    mov al,DGROUP:[_maskbit+bp]  ; al = valeur du masque pour ce bit
;;    mov al,0ffh ;;;;;;
;    and al,es:[bx]           ; si != 0 -> afficher le caractere
;    cmp al,0
;    jz  NOAFF
;    mov bp,DGROUP:_bpsav
;    push es
;    mov ax,arg5
;    mov es,ax
;    movsw                    ; caractere affich‚
;    pop es
;    jmp CONTINUE
;
;NOAFF:                       ; passer au caractere suivant
;    add si,2
;    add di,2
;
;CONTINUE:
;    pop ax
;    inc ax                   ; prochain caractere
;    cmp ax,08h
;    jl  C1
;    mov ax,0                ; si ax >= 8 remettre ax … 0
;    inc bx                   ; et incrementer BX
;
;C1:
;    loop TEST
;;    push ax
;    mov DGROUP:_savnbit,ax
;
;    ; repositionner DI a la prochaine ligne
;    mov ax,80
;    sub ax,dx
;    shl ax,1
;    add di,ax
;
;    ; repositionner bx a la prochaine ligne
;    mov ax,80
;    sub ax,dx
;    shr ax,1 ;
;    shr ax,1 ;
;    shr ax,1 ;               ; ax = (80 - #car/ligne)/8
;    add bx,ax
;    mov ax,80
;    sub ax,dx
;    and ax,07h
;    add ax,DGROUP:_savnbit
;    cmp ax,08h
;    jl  C31
;    inc bx
;
;C31:
;    sub ax,DGROUP:_savnbit
;;    pop ax
;;    cmp ax,0;
;;    jz C12
;;    jmp C14;
;
;C12:
;;    dec bx
;C14:
;;    mov bp,DGROUP:_bpsav
;;    mov ax,arg1 ;
;;    and al,07h ;
;
;    pop cx
;    loop BB1
;
;    pop ax
;    pop cx
;    pop bx
;    pop ax
;    pop ds
;    pop es
;    pop di
;    pop si
;    pop bp
;    ret
;__refreshM ENDP

;*
;*  REFRESHM(begx+1,begy+1,xd,yd,vs_base,seg_buf,seg_mask)
;*   arg       1      2    3  4    5        6       7
;*
;__refreshM2 PROC FAR
;    push bp
;    mov bp,sp
;    push si
;    push di
;    push es
;    push ds
;    push ax
;    push bx
;    push cx
;    push dx
;
;    mov ax,arg6     ; assume buf du type SEG:0000
;    mov ds,ax
;    mov si,0
;
;    ; calcul offset DI
;    mov ax,arg2
;    mov cx,80 * 2
;    mul cx
;    mov di,ax
;    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
;    shl ax,1
;    add di,ax                ; di = offset du point  (xg,yg)
;
;    ; calcul offset octet dans masque  (yg * 10) + (xg/8)
;    mov ax,arg2
;    mov cx,10
;    mul cx
;    mov bx,ax
;    mov ax,arg1
;    shr ax,1
;    shr ax,1
;    shr ax,1
;    add bx,ax
;
;    ; calcul nb de lignes
;    mov cx,arg4
;    inc cx                   ; effet de bord
;    sub cx,arg2              ; cx = nb de lignes dans la fenetre
;
;    ; calcul taille ligne
;    mov dx,arg3
;    inc dx                   ; effet de bord
;    sub dx,arg1              ; dx = nb de words dans une ligne
;    cld
;    mov ax,arg7
;    mov es,ax
;;    xor bx,bx                ; es:bx pointe sur la BITMAP de masque
;;    xor ax,ax                ; init refresh : ax = 0
;    mov ax,arg1
;    and al,07h                ; al = xg % 8 offset bit / gauche dans
;                              ; octet courant
;    mov DGROUP:_bpsav,bp
;
;BB13:                          ; cx = #lignes           - dx = #car/ligne
;                              ; bx = offset OCTET masq - ax = offset BIT masq
;    push cx
;    mov cx,dx
;
;TEST3:
;
;    push ax                  ; ax = no du bit a tester dans le masque
;    mov bp,ax
;    and bp,07h
;    mov al,DGROUP:[_maskbit+bp]  ; al = valeur du masque pour ce bit
;;    mov al,0ffh ;;;;;;
;    and al,es:[bx]           ; si != 0 -> afficher le caractere
;    cmp al,0
;    jz  NOAFF3
;    mov bp,DGROUP:_bpsav
;    push es
;    mov ax,arg5
;    mov es,ax
;    movsw                    ; caractere affich‚
;    pop es
;    jmp CONTINUE3
;
;NOAFF3:                       ; passer au caractere suivanT
;    add si,2
;    add di,2
;
;CONTINUE3:
;    pop ax
;    inc ax                   ; prochain caractere
;    cmp ax,08h
;    jl  C3
;    mov ax,0                ; si ax >= 8 remettre ax … 0
;    inc bx                   ; et incrementer BX
;
;C3:
;    loop TEST3
;;    push ax
;    mov DGROUP:_savnbit,ax
;
;    ; repositionner DI a la prochaine ligne
;    mov ax,80
;    sub ax,dx
;    shl ax,1
;    add di,ax
;
;    ; repositionner bx a la prochaine ligne
;    mov ax,80
;    sub ax,dx
;    shr ax,1 ;
;    shr ax,1 ;
;    shr ax,1 ;               ; ah = (80 - #car/ligne)/8
;    add bx,ax
;    mov ax,80
;    sub ax,dx
;    and ax,07h
;    add ax,DGROUP:_savnbit
;    cmp ax,08h
;    jl  C313
;    and al,07h
;    inc bx
;
;C313:
;
;;    pop ax
;;    cmp ax,0;
;;    jz C12
;;    jmp C14;
;
;C123:
;;    dec bx
;C143:
;;    mov bp,DGROUP:_bpsav
;;    mov ax,arg1 ;
;;    and al,07h ;
;
;    pop cx
;    loop BB13
;
;    pop ax
;    pop cx
;    pop bx
;    pop ax
;    pop ds
;    pop es
;    pop di
;    pop si
;    pop bp
;    ret
;__refreshM2 ENDP

CURS_TEXT ENDS
END
