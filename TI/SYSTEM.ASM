;****************************************************************************
;*   SYSTEM.ASM                                                             *
;*                                                                          *
;*   Gestion des E/S disque , du temps , de la date , de l'initialisation   *
;*           de la memoire , du lancement de la tache initiale              *
;*                                                                          *
;*                                                                          *
;*   Autres modules ASSEMBLEUR :                                            *
;*                  VIDEO.ASM , GR.ASM , IT_HARD.ASM , CXTSWP.ASM           *
;*                                                                          *
;****************************************************************************

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

PUBLIC  _get_bpb        ; recuperer le bios parameter block

_DATA SEGMENT
_sseg  dw 0
_soff  dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*   GET_BPB(dev, &Bios_pb)
;*
_get_bpb  PROC FAR
      push bp
      mov bp,sp
      push ds
      push di
      push si
      push bx
      push cx
      push dx

      mov ah,44h
      mov al,0Dh
      mov bl,byte ptr arg1
      mov ch,8
      mov cl,60h
      mov dx,arg2
      mov ds,arg3
      int 21h
      jc _get_bpb_err
      xor ax,ax
      jmp _get_bpb_fin

_get_bpb_err:
      mov ax,1

_get_bpb_fin:
      pop dx
      pop cx
      pop bx
      pop si
      pop di
      pop ds
      pop bp
      ret
_get_bpb ENDP

_TEXT ENDS
END
