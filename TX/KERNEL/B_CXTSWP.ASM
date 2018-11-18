;****************************************************************************
;*  CXTSWP.ASM                                                              *
;*                                                                          *
;*  Gestion de la commutation des piles                                     *
;*                                                                          *
;*  Autres modules ASSEMBLEUR :                                             *
;*                 VIDEO.ASM , SYSTEM.ASM , GR.ASM , IT_HARD.ASM            *
;*                                                                          *
;****************************************************************************

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

PUBLIC  __swapstk
PUBLIC  _mkdSetjmp
PUBLIC  _mkdLongjmp
PUBLIC  __UtoK
PUBLIC  __KtoU

EXTRN  __disable : FAR
EXTRN  __restore : FAR

_TEXT SEGMENT


;*
;*  SWAPSTK( @ descripteur ancienne pile , valeur nouvelle pile  )
;*                 bp+6 , bp+8                bp+10 , bp+12
;*        mise a jour de la pile active
;*
__swapstk PROC FAR
     push bp
     mov bp,sp

     push si
     push di
     pushf
     les bx,[bp+6]              ; es = seg desc  bx = offset desc
     mov es:[bx],sp             ; svgarder l'offset pile ancienne tache
     mov es:[bx+2],ss           ;    "     le seg    "      "       "

; Nouvelle pile

     mov ax,[bp+10]             ; bx = offset  pile nelle tache
     mov dx,[bp+12]             ; ss = segment   "    "     "
     mov sp,ax
     mov ss,dx
;     mov ax,DGROUP    ; a supprimer : remplacer ca par (push ds/pop ds)
;     mov ds,ax        ;

     popf

     pop di
     pop si
     pop bp
     ret
__swapstk ENDP




;*
;*  SETJMP( @ structure de sauvegarde du contexte de la pile)
;*
_mkdSetjmp PROC FAR
     push bp
     mov bp,sp
     mov cx,es

     les bx,[bp+6]              ; es = seg structure  bx = offset
     pushf
     pop ax                     ; recuperer flag
     mov es:[bx],ax
     mov es:[bx+2],sp
     mov es:[bx+4],ss
     mov es:[bx+6],bp
     mov es:[bx+8],si
     mov es:[bx+10],di
     mov es:[bx+12],ds
     mov es:[bx+14],cx   ; es
     mov ax,0            ; valeur retour de setjmp

     ; placer l'adresse retour de SETJUMP
     ; (IP) dans la structure

     mov dx,[bp+2]
     mov es:[bx+16],dx   ; ip
     mov dx,[bp+4]
     mov es:[bx+18],dx   ; cs
     mov es,cx
     pop bp
     ret
_mkdSetjmp ENDP





;*
;*  LONGJMP( @ structure , valeur retour a la sortie de SETJMP)
;*
_mkdLongjmp PROC FAR
     push bp
     mov bp,sp

     les bx,[bp+6]              ; es = seg structure  bx = offset
     push es:[bx]
     popf                       ; recuperation flag
     mov ax,[bp+10]              ; recuperer valeur retour
     mov sp,es:[bx+2]
     mov ss,es:[bx+4]
     mov bp,es:[bx+6]
     mov si,es:[bx+8]
     mov di,es:[bx+10]
     mov ds,es:[bx+12]

     ; placer le contenu de structure.IP at .CS sur la pile
     ; et le transferer dans IP et CS

     push es:[bx+18]    ; recuperer CS
     push es:[bx+16]    ; recuperer IP
     mov cx,es:[bx+14]
     mov es,cx
     pop dx
     mov [bp+2],dx
     pop dx
     mov [bp+4],dx

     add sp,2           ; skip bp (empile dans SETJMP)
     ret
_mkdLongjmp ENDP




;*************** Description de la pile au moment de    ******************
;
;                     l'appel de KERNELMODE
;
;
;               USER                                         KERNEL
;  +     _____________________
;       |       argn          |
;       |_____________________|
;       |         :           |
;       |_____________________|
;       |       arg1          |
;       |_____________________|                       _____________________
;       |                     |                      |                     |
;       |__  @ retour dans  __|   CALL  primitive    |__  @ retour dans  __|
;       |   tache appelante   |              =======>|    primitive        |
;       |_____________________|             |        |_____________________|
;       |        bp           |             |        |        bp           |
;       |_____________________|             |        |_____________________|
;       |                     |             |        |                     |
;       |   Variables locales |             |_______________________
;       |   de la primitive   |                                     |
;   +18 |_____________________|<----- valeur a sauvegarder    _     |
;       |                     |       dans USP                 |    |
;   +16 |__  @ retour dans  __|   CALL kernelMode              |    |
;       |     primitive       |                                |____|
;   +14 |_____________________|                                |
;       |        bp           |<----- valeur a placer          |
;   +12 |_____________________|       sur la pile systeme     _|
;       |  valeur KSP         |
;   +10 |_____________________|
;       |                     |
;    +8 |__   @ sauvegarde  __|
;       |        USP          |
;    +6 |_____________________|
;       |                     |
;       |__  @ retour dans  __|   CALL  UtoK
;       |     kernelmode      |
;       |_____________________|
;       |        bp           |
;  -    |_____________________| <- sp , bp



;*
;*  UtoK( @ USP , valeur KSP)
;*
__UtoK PROC FAR
     push bp
     mov bp,sp
     mov cx,es

     les bx,[bp+6]              ; es = seg  bx = offset
     mov dx,bp
     add dx,18
     mov es:[bx],dx             ; svgarder dans USP le contexte de pile
                                ; qui suit les variables locales

     ; se placer sur la pile systeme
     mov sp,[bp+10]             ; recuperer valeur KSP

     ; On travail maintenant sur la pile systeme
     push [bp+16]               ; placer @ retour  dans primitive
     push [bp+14]               ; sur pile systeme + pointeur parametres
     push [bp+12]               ; de la primitive invoquant kernelMode()
     mov es,cx
     pop bp
     ret                        ; on est a la base de la pile KERNEL
__UtoK ENDP



;*************** Description de la pile au moment de    ******************
;
;                     l'appel de USERMODE
;
;
;               KERNEL                                        USER
;   +    _____________________                __
;       |                     |                 |    |_____________________|
;       |__   @ retour dans __|  CALL userMode  |_   |  variables locales  |
;       |     primitive       |                 | |  |  de la primitive    |
;   +10 |_____________________|                _| |  |_____________________|
;       |        bp           |<-- valeur BP      |  |                     |
;    +8 |_____________________|    pointant sur   |_ |__   @ retour dans __|
;       |    valeur  USP      |    les parametres    |     la primitive    |
;    +6 |_____________________|    ( a restituer )   |_____________________|
;       |                     |                      |         bp          |
;       |__  @ retour dans  __|   CALL  KtoU         |_____________________|
;       |     userMode        |
;       |_____________________|
;       |        bp           |
;  -    |_____________________| <- sp , bp


;*
;*  KtoU( valeur USP )
;*
__KtoU PROC FAR
     push bp
     mov bp,sp

     ; se remettre sur la pile utilisateur
     mov sp,[bp+6]

     ; placer l'@ retour dans la primitive sur la pile USER
     push [bp+12]
     push [bp+10]
     push [bp+8]      ; bp

     pop  bp
     ret
__KtoU ENDP


_TEXT ENDS
END
