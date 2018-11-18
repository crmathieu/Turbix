;****************************************************************************
;*  SQUEL.ASM                                                               *
;*                                                                          *
;*    Squelette d'un programme Purement Assembleur                          *
;*                                                                          *
;****************************************************************************


_STACK SEGMENT PARA STACK 'STACK'
        dw 200h dup(?)
_STACK ENDS

_TEXT SEGMENT  BYTE PUBLIC 'CODE'
_TEXT ENDS

_DATA SEGMENT  WORD PUBLIC 'DATA'
_DATA ENDS

DGROUP GROUP  _DATA, _STACK
 ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

_TEXT SEGMENT

;*
;*   redonne la main au Dos
;*
_exit proc near
    mov ah,4ch
    int 21h
_exit endp

;*
;*   point d'entree du programme
;*
start:
    mov ah,14           ;
    mov al,'#'          ;    Traitement
    mov bh,0            ;    Principal
    int 10h             ;
    call _exit

_TEXT ENDS

    END start     ; indique le nom du point d'entree
