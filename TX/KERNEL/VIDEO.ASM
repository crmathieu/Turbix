;****************************************************************************
;*  VIDEO.ASM                                                               *
;*                                                                          *
;*  Gestion video du systeme , plus quelques fonctions video BIOS non       *
;*                utilisees                                                 *
;*                                                                          *
;*  Autres Modules ASSEMBLEUR :                                             *
;*                 GR.ASM , SYSTEM.ASM , IT_HARD.ASM , CXTSWP.ASM           *
;*                                                                          *
;****************************************************************************

arg1       equ  6[bp]
arg2       equ  8[bp]
arg3       equ 10[bp]
arg4       equ 12[bp]
arg5       equ 14[bp]
arg6       equ 16[bp]
arg7       equ 18[bp]

VIDRETRACE equ    730

VIDEO_TEXT SEGMENT  BYTE PUBLIC 'CODE'
VIDEO_TEXT ENDS
CONST SEGMENT  WORD PUBLIC 'CONST'
CONST ENDS
_BSS SEGMENT  WORD PUBLIC 'BSS'
_BSS ENDS
_DATA SEGMENT  WORD PUBLIC 'DATA'
_DATA ENDS
DGROUP GROUP CONST, _BSS, _DATA
 ASSUME  CS: VIDEO_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

PUBLIC  __vscopy          ; recopie buffer en memoire video page 0
PUBLIC  __clr_scr         ; clear ecran avec un caractere donne
PUBLIC  __scroll_Mscr     ; mono scrolling
PUBLIC  __scroll_Cscr     ; couleur scrolling
PUBLIC  __quick_scroll    ; scrolling en RAM
PUBLIC  __vid_wait        ; attente debut video vertical retrace
PUBLIC  __vid_on          ; allumer ecran
PUBLIC  __vid_off         ; eteindre ecran
PUBLIC  __swap_page       ; MAJ page courante
PUBLIC  __qscrollWin
PUBLIC  _get_mode        ; retourne le mode video courant
PUBLIC  _get_color       ; indique si EGA ou CGA

;PUBLIC  _set_curs_size   ; maj taille curseur
;PUBLIC  _set_curs_pos    ; maj position curseur
;PUBLIC  _read_curs_pos   ; lit la position courante du curseur
;PUBLIC  _set_page        ; maj page courante
;PUBLIC  _set_mode        ; modifier le mode video
;PUBLIC  _scrollwin_up    ; scroll window up
;PUBLIC  _scrollwin_down  ; scroll window down
;PUBLIC  _read_car_att    ; read caractere & attribut
;PUBLIC  _write_car_att   ; write caractere(s) & attribut
;PUBLIC  _write_car       ; write caractere(s)
;PUBLIC  _write_tty       ; write as tty
;PUBLIC  _get_page        ; retourne la page courante
;PUBLIC  _get_chrome      ; indique si mono ou couleur


EXTRN   _adapter:WORD    ; video mode

_DATA SEGMENT
_color dw 0
_DATA ENDS

VIDEO_TEXT SEGMENT


;*
;*  VID_WAIT - attente retour vertical du spot ( carte couleur )
;*            si bit ON : retrace
;*
__vid_wait PROC FAR
    push ax
    push dx
    mov dx,03dah

vidw1 :
    in al,dx
    and al,08h
    jnz vidw1                ; attendre que le retrace bit soit OFF

vidw2 :
    in al,dx
    and al,08h
    jz vidw2                 ; attendre que le retrace bit soit ON
    pop dx
    pop ax
    ret                    ; OK on peut commencer a ecrire
__vid_wait ENDP

;*
;*  VID_OFF -  desactiver l'ecran
;*
__vid_off PROC FAR
    push ax
    push dx
    mov dx,03d8h
    mov al,21h
    out dx,al
    pop dx
    pop ax
    ret                    ; OK on peut commencer a ecrire
__vid_off ENDP

;*
;*  VID_ON -  activer l'ecran
;*
__vid_on PROC FAR
    push ax
    push dx
    mov dx,03d8h
    mov al,29h
    out dx,al
    pop dx
    pop ax
    ret                    ; OK on peut commencer a ecrire
__vid_on ENDP

;*
;*   VSCOPY(buffer, videobase, offset, words , attribut)
;*   char *buffer;            /* pointeur sur le caractere par pair */
;*   int videobase;           /* seg adresse debut RAM screen       */
;*   int offset;              /* offset video ram                   */
;*   int words;               /* nombre de mots a copier            */
;*   copier words mots du buffer video de sortie en RAM screen
;*
__vscopy PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push bx
    push cx
    push ds


    mov bx,_adapter
    mov si,6[bp]                  ; offset buffer
    mov di,12[bp]                 ; offset dans RAM screen
    mov cx,14[bp]                 ; nb de caracteres a copier
;    and cx,007fh                  ; maximum 128 caracteres (pour RETRACE,
                                   ; mais source de BUG)

    mov ds,8[bp]                  ; seg buffer    -> source ( si )
    mov es,10[bp]                 ; seg videobase -> dest   ( di )
    cld

    and bx,01
    jz  vidM                      ; monochrome

;    call FAR PTR __vid_wait

vidM:
    lodsw                         ; ecrire les mots BYTE+ATTR
    stosw                         ; le placer ds ax ( LODSW ) et ecrire
    loop vidM                     ; ax en RAM screen ( STOSW )

    pop ds
    pop cx
    pop bx
    pop di
    pop si
    pop bp

    ret
__vscopy ENDP

;*
;*   CLR_SCR(videobase, offset, count ,attribut) - effacer tout ou partie
;*   int videobase   arg1
;*   int offset      arg2
;*   int count       arg3
;*   int attribut    arg4
;*
__clr_scr PROC FAR
    push bp
    mov bp,sp
    push di
    push ax
    push cx
    push dx


    mov es,arg1
    mov di,arg2
    mov dx,arg3
    cld
    mov ax,arg4              ; tenir compte de l'attribut
    mov ah,al
    xor al,al

    mov bx,_adapter
    and bx,01
    jz  _clr3                ; MONO ou EGA
;    call __vid_on
_clr1:
    cmp dx,VIDRETRACE           ; doit ecrire plus de mots que retrace le peut?
    jg  _clr2                ; si oui

    mov cx,dx
;    call FAR PTR __vid_wait
    rep stosw
    jmp _clr4

_clr2:
    mov cx,VIDRETRACE
;    call FAR PTR __vid_wait
    rep stosw                ; transferer
    sub dx,VIDRETRACE
    jmp _clr1                ; attendre prochain retrace

_clr3 :
    mov cx,dx
    rep stosw

_clr4 :


    pop dx
    pop cx
    pop ax
    pop di
    pop bp
    ret
__clr_scr ENDP

;*
;*  SCROLL_MSCR (nbline,attribut) - scrolling MONO/EGA
;*  int nline;        /* nombre de lignes a scroller
;*  int attribut;     /* video attribut de la nouvelle ligne
;*
__scroll_Mscr PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push ds
    push ax
    push cx


    mov ax,0B000h
    mov bx,_adapter
    and bx,2
    jz  _scrollMONO
    mov ax,0B800h

_scrollMONO:
    mov ds,ax
    mov es,ax
    mov di,0
    mov si,di

    mov ax,arg1              ; ax = nb de lignes a scroller
    or ax,ax
    js  _scrolldwn           ; si < 0

    mov cx,80 * 2            ;
    mul cx                   ; ax = nb de bytes a scroller
    add si,ax                ; si = @ debut scrolling

    mov cx, 25 * 80          ; nb total de words dans un ecran
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer
    cld

    rep movsw                ; transferer
    mov ax,arg1
    mov cx,80
    mul cx
    mov cx,ax                ; cx = nb de words a mettre a blanc
    jmp _scrollblank

;  SCROLLING INVERSE
_scrolldwn:
    neg ax
    add di,3998              ; se positionner en fin de plan memoire
    mov si,di

    mov cx,80 * 2
    mul cx
    sub si,ax

    mov cx, 25 * 80          ; nb total de words dans un ecran
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer

    std                      ; progresser en decrementant

    rep movsw                ; transferer
    mov ax,arg1
    neg ax
    mov cx,80
    mul cx
    cld
    mov di,0

;  RAJOUTER LA OU LES NOUVELLES LIGNES
_scrollblank:
    mov ax,arg2
    mov ah,al
    xor al,al                ; rajouter  nrlines
    rep stosw


    pop cx
    pop ax
    pop ds
    pop di
    pop si
    pop bp
    ret
__scroll_Mscr ENDP

;*
;*  QSCROLLWIN(xg,yg,xd,yd,nbline,att,vs_base) INACHEVE : RESTE A FAIRE SCROLL INVERSE
;*   argument 1   2  3  4    5    6    7
;*
__qscrollWin PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push ds

    mov ax,arg7
    mov ds,ax
    mov es,ax

    ; calcul des offsets SI et DI
    mov ax,arg2
    mov cx,80 * 2
    mul cx
    mov si,ax
    mov ax,arg1              ; rajouter les caracteres a gauche de la fenetre
    shl ax,1
    add si,ax                ; si = offset du point  (xg,yg)
    mov di,si

    mov ax,arg5              ; ax = nb de lignes a scroller
;    or ax,ax
;    js  _qscrollWdwn           ; si < 0

    mov cx,80*2
    mul cx                   ; ax = nb de bytes a scroller

    add si,ax                ; si = @ debut scrolling


    mov cx,arg4
    inc cx                   ; effet de bord
    sub cx,arg2              ; cx = nb de lignes dans la fenetre
    sub cx,arg5              ; cx = nb de lignes a copier de SI -> DI

    mov dx,arg3
    inc dx                   ; effet de bord
    sub dx,arg1              ; ax = nb de words dans une ligne
    cld

BB:
    push cx
    mov cx,dx

    rep movsw                ; transferer
    mov bx,80                ; repositionner SI et DI a la prochaine ligne
    sub bx,dx
    shl bx,1
    add si,bx
    add di,bx
    pop cx
    loop BB

    mov cx,dx
    mov ax,arg6
    mov ah,al
    xor al,al                ; rajouter  nrlines
    rep stosw

    pop ds
    pop di
    pop si
    pop bp
    ret
__qscrollWin ENDP

;*
;*  QUICK_SCROLL(vs_base, nbline,attribut) - scrolling en memoire
;*  int vs_base;      /* adresse debut RAM vs */
;*  int nline;        /* nombre de lignes a scroller
;*  int attribut;     /* video attribut de la nouvelle ligne
;*
__quick_scroll PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push ds
    mov ds,arg1
    mov es,arg1
    mov di,0          ;arg2
    mov si,di

    mov ax,arg2     ;arg3              ; ax = nb de lignes a scroller
    or ax,ax
    js  _qscrolldwn           ; si < 0

    mov cx,80 * 2            ;
    mul cx                   ; ax = nb de bytes a scroller
    add si,ax                ; si = @ debut scrolling

    mov cx, 25 * 80          ; nb total de words dans un ecran
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer
    cld

    rep movsw                ; transferer
    mov ax,arg2     ;arg3
    mov cx,80
    mul cx
    mov cx,ax                ; cx = nb de words a mettre a blanc
    jmp _qscrollblank

;  SCROLLING INVERSE
_qscrolldwn:
    neg ax
    add di,3998              ; se positionner en fin de plan memoire
    mov si,di

    mov cx,80 * 2
    mul cx
    sub si,ax

    mov cx, 25 * 80          ; nb total de words dans un ecran
    shr ax,1                 ; ax = nb de words a scroller
    sub cx,ax                ; cx = nb de words a transferer

    std                      ; progresser en decrementant

    rep movsw                ; transferer
    mov ax,arg2   ;arg3
    neg ax
    mov cx,80
    mul cx
    cld
    mov di,0      ;arg2

;  RAJOUTER LA OU LES NOUVELLES LIGNES
_qscrollblank:
    mov ax,arg3      ;arg4
    mov ah,al
    xor al,al                ; rajouter  nrlines
    rep stosw
    pop ds
    pop di
    pop si
    pop bp
    ret
__quick_scroll ENDP


;*
;*  SCROLL_CSCR (nline,attribut) - scrolling couleur
;*  int nrline;       /* nombre de lignes a scroller
;*  int attribut;     /* video attribut de la nouvelle ligne
;*
__scroll_Cscr PROC FAR
    push bp
    mov bp,sp
    push si
    push di
    push ds


    mov ax,0B800h
    mov ds,ax
    mov es,ax
    mov di,0
    mov si,0

    mov ax,arg1               ; ax = nb de lignes a scroller
    or  ax,ax
    js  _scrolldwnC           ; si < 0

    mov cx,80 * 2             ;
    mul cx                    ; ax = nb de bytes a scroller
    add si,ax                 ; si = @ debut scrolling

    mov cx, 25 * 80           ; nb total de words dans un ecran
    shr ax,1                  ; ax = nb de words a scroller
    sub cx,ax                 ; cx = nb de words a transferer
    cld
;    call __vid_off
    rep movsw                 ; transferer

    mov ax,arg1
    mov cx,80
    mul cx
    mov cx,ax                 ; cx = nb de words a mettre a blanc
    jmp _scrollblankC

;  SCROLLING INVERSE
_scrolldwnC:
    neg ax
    add di,3998               ; se positionner en fin de plan memoire
    mov si,di

    mov cx,80 * 2
    mul cx
    sub si,ax

    mov cx, 25 * 80           ; nb total de words dans un ecran
    shr ax,1                  ; ax = nb de words a scroller
    sub cx,ax                 ; cx = nb de words a transferer

    std                       ; progresser en decrementant
;    call __vid_off
    rep movsw                 ; transferer
    mov ax,arg1
    neg ax
    mov cx,80
    mul cx
    cld
    mov di,0

;  RAJOUTER LA OU LES NOUVELLES LIGNES
_scrollblankC:
    mov ax,arg2
    mov ah,al
    xor al,al                 ; rajouter  nrlines
    rep stosw
;    call __vid_on
    mov cx,01fffh
attente:
    loop attente

scrollCfin:



    pop ds
    pop di
    pop si
    pop bp
    ret
__scroll_Cscr ENDP

;*
;*  GET_COLOR() - indique si mode CGA ou EGA (IT non repertoriee ds le BIOS)
;*
_get_color PROC FAR
    mov ah,12h
    mov cx,0
    mov bl,10h
    int 10h
    mov ax,cx
    ret
_get_color ENDP

;*
;* GET_MODE - retourne le mode video courant
;*
_get_mode PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push si
     push di
     pushf

     mov ah,15

     int 10h                    ; appel routine BIOS
     xor ah,ah
     popf
     pop di
     pop si
     pop es
     pop ds
     pop bp
     ret
_get_mode ENDP

;*
;*  SWAP_PAGE(SEG newpage,SEG oldpage,save OLD) - swap la page active
;*            avec la page demandee. Si save OLD est positionn‚, ne
;*            pas sauvegarder la page active car mode window initialis‚

__swap_page  PROC FAR
     push bp
     mov bp,sp
     push ds
     push es
     push cx
     push si
     push di
     pushf

; l'ancienne page est toujours en page 0 - les autres pages RAM SCREEN
; sont inutilisees - la nouvelle page est copiee depuis son emplacement
; en memoire vive

     cld
     mov  ax,0B800h
     mov  bx,_adapter
     and  bx,03
     jnz _swapOLD
     mov ax,0B000h

_swapOLD:      ; sauver OLD page si mode window non operant
     mov ds,ax                  ; ds = source segment
     mov ax,arg3                ; ax = flag
     cmp ax,0
     jnz _swapNEW               ; mode window operant, charger directement
                                ; la nouvelle page active
     mov ax,arg2
     mov es,ax                  ; es = dest segment
     mov di,0
     mov si,0                   ; si = @ page 0
     mov cx,2000

; eteindre l'ecran
;     call __vid_off
     rep movsw


; placer la nouvelle page en B000 ou B800

; inverser les segments

_swapNEW:
     mov ax,ds
     mov es,ax
     mov ax,arg1
     mov ds,ax

     mov di,0
     mov si,0
     mov cx,2000
     rep movsw

; rallumer l'ecran
;     call __vid_on
     popf
     pop di
     pop si
     pop cx
     pop es
     pop ds
     pop bp

     ret
__swap_page ENDP


;*
;*  GET_CHROME() - retourne 1 pour ecran couleur, 0 pour monochrome
;*
;_get_chrome PROC FAR
;    int 11h
;    and al,30h
;    cmp al,30h
;    je getmono
;    mov ax,1
;    mov _color,ax
;    ret
;
;getmono:
;    xor ax,ax
;    mov _color,ax
;    ret
;_get_chrome ENDP


;*
;* GET_PAGE - retourne le numero de la page active
;*
;_get_page PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ah,15
;
;     int 10h                    ; appel routine BIOS
;     xor ah,ah
;     mov al,bh                  ; numero de page courante
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_get_page ENDP

;*
;*  SET_MODE - mettre a jour le mode video
;*
;_set_mode PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                   ; al = mode :
;                                   ;    00 -> 40x25 texte  16 gris
;                                   ;    01 -> 40x25 texte  16/8 co
;                                   ;    02 -> 80x25 texte  16 gris
;                                   ;    03 -> 80x25 texte  16/8 co
;                                   ;    04 -> 320x200 gph  4 co
;                                   ;    05 -> 320x200 gph  4 gris
;                                   ;    06 -> 640x200 gph  b/w
;                                   ;    07 -> 80x25 texte  b/w
;                                   ;    08 -> 160x200 gph  16 co
;                                   ;    09 -> 320x200 gph  16 co
;                                   ;    0A -> 640x200 gph  4  co
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_set_mode ENDP

;*
;*  SET_CURS_SIZE -  modifie la taille du curseur
;*
;_set_curs_size PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ah,1                  ; set cursor position
;     xor al,al
;
;     mov cx,arg1               ; ch = starting scan line of cursor
;                               ; cl = ending scan line of cursor
;
;     int 10h                    ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_set_curs_size ENDP

;*
;*  SET_CURS_POS - place le curseur en x,y
;*
;_set_curs_pos PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ah,2                  ; set cursor position
;     xor al,al
;
;     mov bh,arg1               ; page number
;
;     mov dx,arg2               ; dh = row ; dl = column
;
;     int 10h                    ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_set_curs_pos ENDP

;*
;*  READ_CURS_POS - lit la position courante du curseur
;*
;_read_curs_pos PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ah,3                   ; code fonction
;     xor al,al
;
;     mov bx,arg1                ; numero de page
;
;     int 10h                    ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     mov ax,dx                  ; ah = y  al = x
;     ret
;_read_curs_pos ENDP


;*
;*  SET_PAGE - maj  page courante (A SUPPRIMER  APRES VERIFICATION)
;*
;_set_page  PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push cx
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                ; al = numero de page ( 0 - 3 / 0 - 7 )
;                                ; ah = numero de page de travail
;
; l'adresse de la page a charger est placee ds SI
; DI contient toujours 0 ( @ de la page 0 )
; on doit sauvegarder la page 0 en page no 4 avant de la remplacer par la
; nouvelle page
     ; sauvegarde de la page 0 en page de travail
;
;     cld
;     push ax
;     push ax
;     mov ax,0B000h
;     mov ds,ax
;     mov es,ax
;
;     mov cx,4000                ; taille d'une page en bytes
;     pop ax
;     mov al,ah                  ; al = numero page de travail
;     xor ah,ah
;     mul cx
;     mov di,ax                  ; di = @ page de travail
;     mov si,0                   ; si = @ page 0
;     mov cx,2000
;     rep movsw
;
;
;     ; placer la nouvelle page en B000
;     mov di,0
;     mov cx,4000
;     pop ax
;     xor ah,ah
;     mul cx
;     mov si,ax
;     mov cx,2000
;     rep movsw
;
;_set_exit:
;     popf
;     pop di
;     pop si
;     pop bx
;     pop cx
;     pop es
;     pop ds
;     pop bp
;
;     ret
;_set_page ENDP

;_swap_page2  PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push cx
;     push si
;     push di
;     pushf
;
; l'ancienne page est toujours en page 0 - les autres pages RAM SCREEN
; sont inutilisees - la nouvelle page est copier depuis son emplacement
; en memoire vive dans la table VStab
;
;     cld
;     and _color,01
;     jz  _swapM2
;     mov ax,0B800h
;     jmp _swapGO2
;
;_swapM2:
;     mov ax,0B000h
;
;_swapGO2:
;     mov ds,ax                  ; ds = source segment
;     mov ax,arg3
;     mov es,ax                  ; es = dest segment
;     mov cx,4000                ; taille d'une page en bytes
;     mov ax,arg2                ; numero actuelle page
;     mul cx
;     mov di,ax                  ; di = @ page en memoire
;     mov si,0                   ; si = @ page 0
;     mov cx,2000
;
;; eteindre l'ecran
;     call __vid_off
;     rep movsw
;
;
;; placer la nouvelle page en B000 ou B800
;
;; inverser les segments
;     mov ax,ds
;     mov es,ax
;     mov ax,arg3
;     mov ds,ax
;
;     mov di,0
;     mov cx,4000
;
;     mov ax,arg1                ; numero nelle page
;     mul cx
;     mov si,ax                  ; si = offset nelle page dans la table
;     mov cx,2000
;     rep movsw
;
;; rallumer l'ecran
;     call __vid_on
;     popf
;     pop di
;     pop si
;     pop cx
;     pop es
;     pop ds
;     pop bp
;
;     ret
;_swap_page2 ENDP

;*
;*  SCROLLWIN_UP - scrolling up window
;*
;_scrollwin_up PROC FAR
;
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                  ; al = nb lines to scroll up
;
;     mov ah,6                     ; code fonction
;
;     mov bx,arg2
;     mov bh,bl                    ; bh = valeur du filler de ligne
;
;     mov dx,arg3                  ; dh = lower row ; dl = right column
;
;     mov cx,arg4                  ; ch = upper row ; cl = left column
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;
;     ret
;_scrollwin_up ENDP

;*
;*  SCROLLWIN_DOWN - scrolling down window
;*
;_scrollwin_down PROC FAR
;
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                  ; al = nb lines to scroll down
;
;     mov ah,7                     ; code fonction
;
;     mov bx,arg2
;     mov bh,bl                    ; bh = valeur du filler de ligne
;
;     mov dx,arg3                  ; dh = lower row ; dl = right column
;
;     mov cx,arg4                  ; ch = upper row ; cl = left column
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;
;     ret
;_scrollwin_down ENDP

;*
;*  READ_CAR_ATT - read caractere & attribut
;*
;_read_car_att PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov bx,arg1                  ; bh = page number
;
;     mov ah,8                     ; code fonction
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret                          ; al = caractere ; ah = attribut
;_read_car_att ENDP

;*
;*   WRITE_CAR_ATT - write caractere(s) & attribut
;*
;_write_car_att PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                  ; al = caractere
;
;     mov ah,9                     ; code fonction
;
;     mov bx,arg2                  ; bh = page number ; bl = attribut
;
;     mov cx,arg3                  ; cx = number of caracteres
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_write_car_att ENDP

;*
;*  WRITE_CAR - write caractere(s)
;*
;_write_car PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                   ; al = caractere
;
;     mov ah,10                     ; code fonction
;
;     mov bx,arg2                  ; bh = page number ; bl = color in graph mode
;
;     mov cx,arg3                  ; cx = number of caracteres
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_write_car ENDP

;*
;*  WRITE_TTY - write caractere(s) as tty
;*
;_write_tty PROC FAR
;     push bp
;     mov bp,sp
;     push ds
;     push es
;     push si
;     push di
;     pushf
;
;     mov ax,arg1                   ; al = caractere
;
;     mov ah,14                     ; code fonction
;
;     mov bx,arg2                   ; bh = page number ; bl = color in graph mode
;
;     int 10h                      ; appel routine BIOS
;
;     popf
;     pop di
;     pop si
;     pop es
;     pop ds
;     pop bp
;     ret
;_write_tty ENDP

VIDEO_TEXT ENDS
END
