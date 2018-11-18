;****************************************************************************
;*   GR.ASM                                                                 *
;*                                                                          *
;*   Fonctions graphiques de tracage de traits verticaux et horizontaux     *
;*                                                                          *
;*         ( COULEUR CGA ET MONOCHROME HERCULES )                           *
;*                                                                          *
;*   Autres modules ASSEMBLEUR :                                            *
;*          VIDEO.ASM , SYSTEM.ASM , IT_HARD.ASM , CXTSWP.ASM               *
;*                                                                          *
;****************************************************************************

arg1 equ 6[bp]
arg2 equ 8[bp]
arg3 equ 10[bp]
arg4 equ 12[bp]
octet_ligne equ 90

; port address
index equ 03b4h
cntrl equ 03b8h

; control codes
scrn_on equ 8
grph equ 2
text equ 20h

MONOCHROME_BASE  equ 0B800h
COLOR_BASE       equ 0B800h
XMAXMONO         equ 719
XMAXCOLOR        equ 639
YMAXMONO         equ 347
YMAXCOLOR        equ 199
XMIN             equ 0
YMIN             equ 0
XBYTEMAXMONO     equ 89
XBYTEMAXCOLOR    equ 79

_TEXT   SEGMENT  BYTE PUBLIC 'CODE'
_TEXT   ENDS
CONST   SEGMENT  BYTE PUBLIC 'CONST'
masqV   db 80h
        db 40h
        db 20h
        db 10h
        db 08h
        db 04h
        db 02h
        db 01h

masqHd  db 80h
        db 0c0h
        db 0e0h
        db 0f0h
        db 0f8h
        db 0fch
        db 0feh
        db 0ffh

masqHg  db 0ffh
        db 7fh
        db 3fh
        db 1fh
        db 0fh
        db 07h
        db 03h
        db 01h

gtable db 35h,2dh,2eh,07h
       db 5bh,02h,58h,58h
       db 02h,03h,00h,00h,00h,00h

CONST   ENDS
_BSS   SEGMENT  WORD PUBLIC 'BSS'
_BSS   ENDS
_DATA   SEGMENT  WORD PUBLIC 'DATA'
_DATA   ENDS
DGROUP   GROUP   CONST,   _BSS,   _DATA
   ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP


; fonctions a utiliser en MONOCHROME
PUBLIC  _mwpixel     ; afficher un pixel
PUBLIC  _mwbyte      ; afficher un octet
PUBLIC  _mwlh        ; tracer une ligne horizontale
PUBLIC  _mwlv        ; trace une ligne verticale
PUBLIC  _invertm     ; inverser une fenetre graphique
PUBLIC  _mrbyte      ; lit un octet depuis l'ecran

; fonctions a utiliser en COULEUR
PUBLIC  _cwpixel
PUBLIC  _cwbyte
PUBLIC  _cwlh
PUBLIC  _cwlv
PUBLIC  _invertc
PUBLIC  _crbyte

_TEXT SEGMENT


;*
;* INVERTM(x1,y1,x2,y2)  - inversion video d'une fenetre graphique
;*         int x1, y1, x2, y2;
;*                             0 <= x <= 79
;*                             0 <= y <= 199
;*
_invertm PROC FAR
     push bp
     mov bp,sp
     push es
     push di
     push si

     mov ax,MONOCHROME_BASE
     mov es,ax
     mov ax,arg2 ; coord y1
     dec ax
     mov arg2,ax

     mov dx,arg1 ; coord x1

     mov cx,arg4 ; coord y2
     sub cx,arg2  ; coord y1
     ; inc cx

_invertm12:
     push cx
     mov ax,arg2
     add ax,cx
     shr ax,1
     jc _invertm_imp
     mov di,0
     jmp _invertm4

_invertm_imp:
     mov di,2000h

_invertm4:
     mov bl,80
     mul bl
     add di,ax
     add di,dx

     mov cx,arg3 ; coord x2
     sub cx,arg1  ; coord x1
     inc cx

_invertm2:
     not BYTE PTR es:[di]
     inc di
     loop _invertm2
     pop cx
     loopne _invertm12

     pop si
     pop di
     pop es
     mov sp,bp
     pop bp
     ret
_invertm endp

;*
;*  MWPIXEL( x , y , color ) - affichage d'un pixel en mode hercules
;*                             0 < x < 719     0 < y < 347
;*  le calcul est le suivant :
;*        l'offset ds la page graphique est donne par
;*                 2000h * ( y mod 4 ) + ( 90 * ( y div 4 )) + ( x div 8 )
;*        et la position du bit dans l'octet est donnee par
;*                 7 - ( x mod 8 )
;*  test de debordement inclus
;*
_mwpixel PROC FAR
     push bp
     mov bp,sp
     push es
     push di
     push si

;    tester si x et y ont des valeurs correctes
     mov ax,arg1
     cmp ax,XMAXMONO
     jnc _mwpixelFIN            ; x out of range
     cmp ax,XMIN
     jc  _mwpixelFIN            ; x < 0

     mov cx,ax                  ; sauvegarder x
     mov ax,arg2
     cmp ax,YMAXMONO
     jnc _mwpixelFIN            ; y out of range
     cmp ax,YMIN
     jc  _mwpixelFIN            ; y < 0

     mov dx,ax                  ; sauvegarder y
     mov ax,MONOCHROME_BASE
     mov es,ax
     mov ax,dx                  ; recuperer y

;    determiner sur quel plan memoire on doit ecrire le pixel
     and ax,03h
     cmp ax,0
     jnz _mwpixel_plan1
     mov di,0
     jmp _mwpixel4

_mwpixel_plan1:
     cmp ax,1
     jnz _mwpixel_plan2
     mov di,2000h
     jmp _mwpixel4

_mwpixel_plan2:
     cmp ax,2
     jnz _mwpixel_plan3
     mov di,4000h
     jmp _mwpixel4

_mwpixel_plan3:
     mov di,6000h

_mwpixel4:
     mov ax,dx                  ; recuperer y
     shr ax,1
     shr ax,1
     mov bl,octet_ligne
     mul bl
     add di,ax
     mov si,cx                  ; si = x
     shr cx,1
     shr cx,1
     shr cx,1

     add di,cx

     and si,07h
     mov al,[masqV+si]
     mov bx,arg3                ; recupere la couleur du point
     cmp bl,0
     jz _mwpixel6
     or es:[di],al
     jmp _mwpixelFIN

_mwpixel6:
     not al
     and es:[di],al

_mwpixelFIN:
    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_mwpixel endp


;*
;*   MWBYTE( x , y , byte ) - affichage d'un octet en mode hercules
;*                            0 < x < 89     0 < y < 347
;*   int x,y;
;*   char byte;
;*        le calcul est le suivant :
;*        l'offset ds la page graphique est donne par
;*                 2000h * ( y mod 4 ) + ( 90 * ( y div 4 )) + ( x div 8 )
;*
_mwbyte PROC FAR ;
    push bp
    mov bp,sp
    push es
    push di

;   tester si x et y ont des valeurs correctes
    mov ax,arg1
    cmp ax,XBYTEMAXMONO
    jnc _mwpixelFIN             ; x out of range
    cmp ax,XMIN
    jc  _mwpixelFIN             ; x < 0

    mov dx,ax                   ; sauvegarder x
    mov ax,arg2
    cmp ax,YMAXMONO
    jnc _mwpixelFIN             ; y out of range
    cmp ax,YMIN
    jc  _mwpixelFIN             ; y < 0

    mov bx,ax                   ; sauvegarder y

    mov ax,MONOCHROME_BASE
    mov es,ax

    mov ax,bx                   ; recuperer y
    and ax,03h                  ; y mod 4
    cmp ax,0
    jnz _mwbyte_plan1
    mov di,0
    jmp _mwbyte4

_mwbyte_plan1:
    cmp ax,1
    jnz _mwbyte_plan2
    mov di,2000h
    jmp _mwbyte4

_mwbyte_plan2:
    cmp ax,2
    jnz _mwbyte_plan3
    mov di,4000h
    jmp _mwbyte4

_mwbyte_plan3:
    mov di,6000h

_mwbyte4:
    mov ax,bx                 ; recuperer y
    shr ax,1
    shr ax,1                  ; y mod 4

    mov bl,octet_ligne
    mul bl
    add di,ax
    add di,dx                  ; rajouter x
    mov al,arg3
    mov es:[di],al

_mwbyteFIN:
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_mwbyte endp



;*
;* MWLH( x1, x2, y,color) - ecrire une ligne horizontale
;* int x1, x2, y, color;
;*                             x et y sont en pixels
;*   x1    : arg1
;*   x2    : arg2
;*   y     : arg3
;*   color : arg4         test de debordement integr‚
;*
_mwlh PROC FAR
    push bp
    mov bp,sp
    push es
    push di
    push si

;   tester si x et y ont des valeurs correctes
;    mov ax,arg1
;    cmp ax,XMAXMONO
;    jc _mwlhcntrl1
;    mov arg1,XMAXMONO            ; x out of range

;_mwlhcntrl1:
;    cmp ax,XMIN
;    jnl _mwlhcntrl2
;    mov WORD PTR arg1,XMIN       ; x < 0

;_mwlhcntrl2:
;    mov ax,arg2
;    cmp ax,XMAXMONO
;    jc  _mwlhcntrl3
;    mov arg2,XMAXMONO            ; x out of range

;_mwlhcntrl3:
;    cmp ax,XMIN
;    jnl _mwlhcntrl4
;    mov WORD PTR arg2,XMIN       ; x < 0

;_mwlhcntrl4:
;    mov ax,arg3
;    cmp ax,YMAXMONO
;    jc  _mwlhcntrl5
;    mov arg3,YMAXMONO            ; y out of range

;_mwlhcntrl5:
;    cmp ax,YMIN
;    jnl _mwlhcntrlFIN
;    mov WORD PTR arg3,YMIN       ; y < 0

;_mwlhcntrlFIN:
    mov ax,MONOCHROME_BASE
    mov es,ax
    mov ax,arg3                  ; coord y
    push ax

    and ax,03h                   ; determiner sur quel plan est la ligne
    cmp ax,0
    jnz _mwlh_plan1
    mov di,0
    jmp _mwlh4

_mwlh_plan1:
    cmp ax,1
    jnz _mwlh_plan2
    mov di,2000h
    jmp _mwlh4

_mwlh_plan2:
    cmp ax,2
    jnz _mwlh_plan3
    mov di,4000h
    jmp _mwlh4

_mwlh_plan3:
    mov di,6000h

_mwlh4:
    pop ax
    shr ax,1
    shr ax,1                ; y div 4
    mov bl,octet_ligne
    mul bl
    add di,ax               ; offset dans le plan

    ; determiner la patterne droite a ecrire : @(masqHd+(x div 8))
    mov dx,arg2             ; x2 en pixels
    mov si,dx               ;
    shr dx,1                ; dx = x2 div 8
    shr dx,1
    shr dx,1
    and si,07h              ; si = x2 mod 8
    mov al,[masqHd+si]      ; si x2 a droite

    push ax                 ; sauvegarder pattern droite
    push dx

    ; determiner la patterne gauche a ecrire : @(masqHg+(x div 8))
    mov dx,arg1             ; x1 en pixels
    mov si,dx               ;
    shr dx,1                ; dx = x1 div 8
    shr dx,1
    shr dx,1
    and si,07h              ; si = x1 mod 8
    mov al,[masqHg+si]      ; si x1 a gauche

    add di,dx               ; di est sur le 1er byte a ecrire

    mov bx,dx
    pop dx
    cmp dx,bx
    jnz _mwlhsuite

    ; cas tordu ou x1 et x2 sont dans le meme byte
    pop bx                  ; bx = patterne droite
    and al,bl               ; patterne resultante
    mov bx,arg4
    cmp bx,0
    jnz _mwlhtordu
    not al
    and es:[di],al
    jmp _mwlhFIN

_mwlhtordu:
    or es:[di],al
    jmp _mwlhFIN

_mwlhsuite:
    mov cx,dx               ; coord x2
    sub cx,bx               ; cx = x2(byte) - x1(byte)
    dec cx
    mov bx,arg4             ; couleur
    cmp bx,0
    jnz  _mwlhNORMAL
    not al                  ; inverser

    and es:[di],al          ; ecrire patterne gauche
    inc di
    mov al,0

    cld
    rep stosb               ; ecrire 0
    pop ax
    not al
    and es:[di],al          ; ecrire patterne droite
    jmp _mwlhFIN

_mwlhNORMAL:
    or es:[di],al          ; ecrire patterne gauche
    inc di
    mov al,0ffh

    cld
    rep stosb              ; ecrire 0xff
    pop ax
    or es:[di],al          ; ecrire patterne droite

_mwlhFIN:
    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_mwlh endp

;*
;*  MWLV( x, y1, y2, color) - ecrire une ligne verticale
;*  int x, y1, y2, color;
;*
;*   x  : arg1   x,y1,y2 en coordonnees points
;*   y1 : arg2
;*   y2 : arg3
;*   color : arg4
;*
_mwlv PROC FAR
    push bp
    mov bp,sp
    push es
    push di
    push si

    mov ax,MONOCHROME_BASE
    mov es,ax
    mov ax,arg2             ; y1
    dec ax
    mov arg2,ax             ; y1 -1

    ; determiner la patterne a ecrire : @(masqV+(x div 8))
    mov dx,arg1             ; coord x
    mov si,dx               ; si = x en points
    shr dx,1                ; dx = x div 8
    shr dx,1
    shr dx,1
    and si,07h              ; si = x mod 8
    mov al,[masqV+si]

    mov cx,arg3             ; cx = y2
    sub cx,arg2             ; cx = y2 - y1
; inc cx

    mov bx,arg4             ; color
    cmp bl,1
    jz _mwlvNORMAL          ; si color = 1
    not al                  ; color = 0 : inverser le masque

_mwlvINVERSE:               ; BOUCLE d'ECRITURE de la ligne
    push ax                 ; pattern dans la pile
    mov ax,arg2             ; ax = y1
    add ax,cx               ; ax = y1 + cx   . cx est decrement‚ a chaque pas
;-------
    push ax                 ; sauvegarder le numero de ligne a traiter
    and ax,03h              ; determiner sur quel plan on va ecrire
    cmp ax,0
    jnz _mwlvIp1
    mov di,0
    jmp _mwlvIsuite

_mwlvIp1:
    cmp ax,1
    jnz _mwlvIp2
    mov di,2000h
    jmp _mwlvIsuite

_mwlvIp2:
    cmp ax,2
    jnz _mwlvIp3
    mov di,4000h
    jmp _mwlvIsuite

_mwlvIp3:
    mov di,6000h

_mwlvIsuite:
    pop ax
    shr ax,1
    shr ax,1

    mov bl,octet_ligne
    mul bl
    add di,ax
;    mov dx,arg1 ; coord x
    add di,dx
;    mov al,arg3
    pop ax
    and es:[di],al


;++++++++++
;    shr ax,1
;    jc _mwlv_imp2
;    mov di,0
;    jmp _mwlv42

;_mwlv_imp2:
;    mov di,2000h

;_mwlv42:
;    mov bl,80
;    mul bl
;    add di,ax               ; di = offset ligne en byte dans le plan d'ecriture
;    add di,dx               ; rajouter l'offset colonne en byte
;    pop ax                  ; recuperer la pattern
;    and es:[di],al
    loop _mwlvINVERSE        ; boucle d'incrementation de y
    jmp _mwlvFIN             ; aller en FIN

_mwlvNORMAL:
    push ax
    mov ax,arg2
    add ax,cx
;-------
    push ax                 ; sauvegarder le numero de ligne a traiter
    and ax,03h              ; determiner sur quel plan on va ecrire
    cmp ax,0
    jnz _mwlvNp1
    mov di,0
    jmp _mwlvNsuite

_mwlvNp1:
    cmp ax,1
    jnz _mwlvNp2
    mov di,2000h
    jmp _mwlvNsuite

_mwlvNp2:
    cmp ax,2
    jnz _mwlvNp3
    mov di,4000h
    jmp _mwlvNsuite

_mwlvNp3:
    mov di,6000h

_mwlvNsuite:
    pop ax
    shr ax,1
    shr ax,1

    mov bl,octet_ligne
    mul bl
    add di,ax
;    mov dx,arg1 ; coord x
    add di,dx
;    mov al,arg3
    pop ax
    or es:[di],al




;+++++++++
;    shr ax,1
;    jc _mwlv_imp
;    mov di,0
;    jmp _mwlv4

;_mwlv_imp:
;    mov di,2000h

;_mwlv4:
;    mov bl,80
;    mul bl
;    add di,ax
;    add di,dx
;    pop ax

;    or es:[di],al
    loop _mwlvNORMAL

_mwlvFIN:
    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_mwlv endp

;*
;*  MRBYTE(x, y)  - lire byte sur l'ecran
;*  int x, y;         0 <= x <= 90   et   0 <= y <= 347
;*
_mrbyte PROC FAR
    push bp
    mov bp,sp
    push es
    push di

    mov ax,MONOCHROME_BASE
    mov es,ax
    mov ax,arg2 ; coord y
    push ax
    and ax,03h
    cmp ax,0
    jnz _mrbyte_plan1
    mov di,0
    jmp _mrbyte4

_mrbyte_plan1:
    cmp ax,1
    jnz _mrbyte_plan2
    mov di,2000h
    jmp _mrbyte4

_mrbyte_plan2:
    cmp ax,2
    jnz _mrbyte_plan3
    mov di,4000h
    jmp _mwbyte4

_mrbyte_plan3:
    mov di,6000h

_mrbyte4:
    pop ax
    shr ax,1
    shr ax,1

    mov bl,octet_ligne
    mul bl
    add si,ax
    mov dx,arg1 ; coord x
    add si,dx
    lodsb         ; octet lu
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_mrbyte endp

;*
;*  INVERTC(x1,y1,x2,y2)   - inverse video d'une fenetre
;*  int x1, y1, x2, y2;
;*      0 <= x <= 79
;*      0 <= y <= 199
;*
_invertc PROC FAR
    push bp
    mov bp,sp
    push es
    push di
    push si

    mov ax,COLOR_BASE
    mov es,ax
    mov ax,arg2 ; coord y1
    dec ax
    mov arg2,ax

    mov dx,arg1 ; coord x1

    mov cx,arg4 ; coord y2
    sub cx,arg2  ; coord y1
; inc cx

_invertc12:
    push cx
    mov ax,arg2
    add ax,cx
    shr ax,1
    jc _invertc_imp
    mov di,0
    jmp _invertc4

_invertc_imp:
    mov di,2000h

_invertc4:
    mov bl,80
    mul bl
    add di,ax
    add di,dx

    mov cx,arg3 ; coord x2
    sub cx,arg1  ; coord x1
    inc cx

_invertc2:
    not BYTE PTR es:[di]
    inc di
    loop _invertc2
    pop cx
    loopne _invertc12

    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_invertc endp

;*
;*  CWPIXEL( x , y , color ) - affichage d'un pixel en mode CGA
;*                             0 < x < 639     0 < y < 199
;*       le calcul est le suivant :
;*       l'offset ds la page graphique est donne par
;*                 2000h * ( y mod 2 ) + ( 80 * ( y div 2 )) + ( x div 8 )
;*       et la position du bit dans l'octet est donnee par
;*                 7 - ( x mod 8 )
;*
_cwpixel PROC FAR
     push bp
     mov bp,sp
     push es
     push di
     push si

     mov ax,COLOR_BASE
     mov es,ax
     mov ax,arg2 ; coord y

     ; determiner sur quel plan memoire on doit ecrire le pixel
     push ax
     and ax,01h
     cmp ax,0
     jnz _cpoint_plan1
     mov di,0
     jmp _cpoint4

_cpoint_plan1:
     mov di,2000h

_cpoint4:
     pop ax
     shr ax,1
     mov bl,octet_ligne
     mul bl
     add di,ax
     mov dx,arg1 ; coord x
     mov si,dx
     shr dx,1
     shr dx,1
     shr dx,1

     add di,dx

     and si,07h
     mov al,[masqV+si]
     mov bx,arg3   ; recupere la couleur du point
     cmp bl,0
     jz _cpoint6
     or es:[di],al
     jmp _cpoint8

_cpoint6:
     not al
     and es:[di],al

_cpoint8:
    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_cwpixel endp

;*
;*  CWBYTE( x , y , byte ) - affichage d'un octet en mode CGA
;*                           0 < x < 79     0 < y < 199
;*  int x,y;
;*  char byte;
;*     le calcul est le suivant :
;*     l'offset ds la page graphique est donne par
;*                 2000h * ( y mod 4 ) + ( 80 * ( y div 2 )) + ( x div 8 )
;*
_cwbyte PROC FAR ;
    push bp
    mov bp,sp
    push es
    push di

    mov ax,COLOR_BASE
    mov es,ax
    mov ax,arg2 ; coord y
    push ax
    and ax,01h
    cmp ax,0
    jnz _cwbyte_plan1
    mov di,0
    jmp _cwbyte4

_cwbyte_plan1:
    mov di,2000h

_cwbyte4:
    pop ax
    shr ax,1

    mov bl,octet_ligne
    mul bl
    add di,ax
    mov dx,arg1 ; coord x
    add di,dx
    mov al,arg3
    mov es:[di],al

    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_cwbyte endp


;*
;*  CWLH( x1, x2, y, pattern)   - ecrire une ligne horizontale
;*  int x1, x2, y, color;
;*   x1 : arg1
;*   x2 : arg2
;*   y  : arg3
;*   pattern : arg4
;*
_cwlh PROC FAR
    push bp
    mov bp,sp
    push es
    push di
    push si

    mov ax,COLOR_BASE
    mov es,ax
    mov ax,arg3 ; coord y
    shr al,1
    jc _cwlh_impair
    mov di,0
    jmp _cwlh4

_cwlh_impair:
    mov di,2000h

_cwlh4:
    mov bl,80
    mul bl
    add di,ax
    add di,arg1 ; coord x1
    mov cx,arg2 ; coord x2
    sub cx,arg1
    inc cx
    mov al,arg4

_cwlh12:
    cld
    rep stosb

    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_cwlh endp

;*
;*  CWLV( x, y1, y2, color) - tracer ligne verticale
;*  int x, y1, y2, color;
;*
;*   x  : arg1
;*   y1 : arg2
;*   y2 : arg3
;*   color : arg4
;*
_cwlv PROC FAR
    push bp
    mov bp,sp
    push es
    push di
    push si

    mov ax,COLOR_BASE
    mov es,ax
    mov ax,arg2
    dec ax
    mov arg2,ax

    mov dx,arg1 ; coord x
    mov si,dx
    shr dx,1
    shr dx,1
    shr dx,1
    and si,07h
    mov al,[masqV+si]

    mov cx,arg3
    sub cx,arg2
; inc cx

    mov bx,arg4 ; color
    cmp bl,1
    jz _cwlv12
    not al

_cwlv14:
    push ax
    mov ax,arg2
    add ax,cx
    shr ax,1
    jc _cwlv_imp2
    mov di,0
    jmp _cwlv42

_cwlv_imp2:
    mov di,2000h

_cwlv42:
    mov bl,80
    mul bl
    add di,ax
    add di,dx
    pop ax
    and es:[di],al
    loop _cwlv14 ; boucle d'incrementation de y
    jmp _cwlv8

_cwlv12:
    push ax
    mov ax,arg2
    add ax,cx
    shr ax,1
    jc _cwlv_imp
    mov di,0
    jmp _cwlv4

_cwlv_imp:
    mov di,2000h

_cwlv4:
    mov bl,80
    mul bl
    add di,ax
    add di,dx
    pop ax

    or es:[di],al
    loop _cwlv12

_cwlv8:
    pop si
    pop di
    pop es
    mov sp,bp
    pop bp
    ret
_cwlv endp

;*
;*  CRBYTE(x, y) - lire un byte
;*  int x, y
;*
_crbyte PROC FAR ; read byte on screen
    push bp
    mov bp,sp
    push ds
    push si

    mov ax,COLOR_BASE
    mov ds,ax
    mov ax,arg2 ; coord y
    shr al,1
    jc _crbyte_impair
    mov si,0
    jmp _crbyte4

_crbyte_impair:
    mov si,2000h

_crbyte4:
    mov bl,80
    mul bl
    add si,ax
    mov dx,arg1 ; coord x
    add si,dx
    lodsb

    pop si
    pop ds
    mov sp,bp
    pop bp
    ret
_crbyte endp

_TEXT ends
     end
