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

EXTRN   _task0        :FAR
EXTRN   __call_it     : FAR
EXTRN   _msdosv_int15 : DWORD
EXTRN   __checkINT15  : FAR
EXTRN   __itDiv0      : FAR
EXTRN   __itIllegal   : FAR

;* gestion des E/S disque *
PUBLIC  __getlol        ; get List of List
PUBLIC  __int13         ; READ / WRITE operations
PUBLIC  __int15         ; gestion preemption durant l'entree sortie
PUBLIC  __Dreset        ; reset controleur floppy
PUBLIC  __getCurrDrive  ; donne le drive courant
PUBLIC  _get_bpb        ; recuperer le bios parameter block
;PUBLIC  _getdskstatus  ; retourne status du controleur
PUBLIC  __dskchg        ; AT : indique si disquette changee
PUBLIC  __dskrdy        ;      indique si drive ready
;PUBLIC  _GetDiskTyp    ; Disk type

;* lancement de la tache initiale *
PUBLIC __starttask0

;* souris *
;EXTRN  _ib:DWORD
;EXTRN  _it_mouse:FAR
;PUBLIC _prolog_it_mouse

;* divers *
PUBLIC __getconf
PUBLIC _outp
PUBLIC _inp
PUBLIC __setKbdLed
PUBLIC _fastcpy
PUBLIC _fastset
PUBLIC __int_itDiv0
PUBLIC __int_itIllegal

;* gestion memoire *
PUBLIC __allocmem
PUBLIC __disallocmem
PUBLIC __getfreemem

;* gestion du temps *
PUBLIC  __getsystime
PUBLIC  __getsysdate


_DATA SEGMENT
_sseg  dw 0
_soff  dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*  GETLOL() - recuperer l'adresse de la liste des listes
;*             retourne l'@ dans ES:BX -> DX:AX si OK , sinon -1
;*
__getlol PROC FAR
    push bp
    push bx
    push cx
    push ds
    push es
    push si
    push di

    mov ah,52h
    int 21h

    jnc  _getlol_ok
    mov ax,-1
    mov dx,-1
    jmp _getlol_end

_getlol_ok:
    mov dx,es
    mov ax,bx

_getlol_end:
    pop di
    pop si
    pop es
    pop ds
    pop cx
    pop bx
    pop bp
    ret
__getlol ENDP


;*
;*  INT15 - filtre l'interruption 15h pour capturer les sous fonctions
;*          90h (device wait)  et 91h (device post)
;*
__int15 PROC FAR
;    sauvegarder les parametres pass‚s a l'INT 15h
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
     push ss
     push ax
     mov ax,DGROUP
     mov ds,ax
     call far ptr __checkINT15
     add sp,4
     pop bp
     pop es
     pop di
     pop si
     pop dx
     pop cx
     pop bx
     or ax,ax
     jz BIOSint15    ; appeler l'ancienne IT si 0
     cmp ax,2        ; 2 = erreur
     jnz MYINT15
     push bx
     mov bx, sp
     or word ptr ss:[bx+10],1     ; positionner CARRY FLAG
     pop bx

MYINT15:
     pop ax
     pop ds
     xor ah,ah
     iret

BIOSint15:
     pop ax
     pop ds
     int 61h  ; perform BIOS INT15
     xor ah,ah ;;
     iret
__int15 ENDP


;*
;*  GETFREEMEM - donne la taille de la memoire
;*               disponible en KILO OCTET
;*
__getfreemem PROC FAR
    int 12h
    ret
__getfreemem ENDP


;*
;*  ALLOCMEM(npara) - allouer n paragraphes
;*                    retourne l'@ du bloc si OK , sinon -1
;*
__allocmem PROC FAR
    push bp
    mov bp,sp
    push bx
    push cx
    push ds
    push es
    push si
    push di

    mov bx,arg1             ; nb de paragraphes
    mov ah,48h

    int 21h
    jnc  _alloc_ok
    mov ax,-1
    mov dx,bx               ; nb de paragraphes disponibles
    jmp _alloc_end

_alloc_ok:
    mov dx,ax
    xor ax,ax

_alloc_end:
    pop di
    pop si
    pop es
    pop ds
    pop cx
    pop bx
    pop bp
    ret
__allocmem ENDP

;*
;*  DISALLOCMEM(addbloc) - libere un bloc memoire
;*
__disallocmem PROC FAR
    push bp
    mov bp,sp
    push es

    mov es,arg2                  ; SEGMENT bloc
    mov ah,49h

    int 21h
    jnc  _disalloc_ok
    jmp _disalloc_end

_disalloc_ok:
    mov ax,0

_disalloc_end:
    pop es
    pop bp
    ret
__disallocmem ENDP

;*
;*  GETCONF - donne la configuration materielle
;*
__getconf PROC FAR
    int 11h
    ret
__getconf ENDP


;*
;*  INP(port) - lire octet depuis "port"
;*
_inp PROC FAR
     push bp
     mov bp,sp
     push dx
     mov dx,arg1
     in  al,dx
     xor ah,ah
     pop dx
     pop bp
     ret
_inp ENDP

;*
;*  OUTP(port,val) - ecrire val dans "port"
;*
_outp PROC FAR
     push bp
     mov bp,sp
     push dx
     push ax
     mov dx,arg1
     mov ax,arg2
     out dx,al
     pop ax
     pop dx
     pop bp
     ret
_outp ENDP

;*
;*  STARTTASK0(stk_addr) - lancer la tache initiale
;*  int *stk_addr;
;*
__starttask0 PROC FAR
     cli
     push bp
     mov bp,sp

     mov ax,ss       ; svgarde anciennes valeurs
     mov _sseg,ax    ;
     mov _soff,bp    ;


     mov ax,[bp+6]   ; offset de stk_addr
     mov dx,[bp+8]   ; segment de stk_addr

     mov ss,dx       ; change l'adresse courante de pile
     mov sp,ax
     push ax         ; reserve une place pour MAGIC
     call FAR PTR _task0
     pop  ax
     mov ax,DGROUP
     mov ds,ax
     mov ax,_sseg     ;
     mov bp,_soff     ;
     mov sp,bp
     mov ss,ax
     mov ah, 0dh      ; flush DOS buffers
     int 21h
     pop bp
     ret
__starttask0 ENDP

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
;*  INT_ITDIV0 -
;*
__int_itDiv0 PROC FAR
     push ds
     push ax
     push bx
     push cx
     push dx
     push si
     push di
     push es
     push bp
     call far ptr __itDiv0
     pop bp
     pop es
     pop di
     pop si
     pop dx
     pop cx
     pop bx
     pop ax
     pop ds
     iret
__int_itDiv0 ENDP

;*
;*  INT_ITILL -
;*
__int_itIllegal PROC FAR
     push ds
     push ax
     push bx
     push cx
     push dx
     push si
     push di
     push es
     push bp
     call far ptr __itIllegal
     pop bp
     pop es
     pop di
     pop si
     pop dx
     pop cx
     pop bx
     pop ax
     pop ds
     iret
__int_itIllegal ENDP



;*
;*   DRESET - reset disk controleur
;*
__Dreset PROC FAR
     push di
     push si
     mov ah,0
     mov dx,arg1   ; floppy drive
     int 13h
     pop si
     pop di
     ret
__Dreset ENDP

;*
;*   GETCURRDRIVE() - donne le drive courant
;*
__getCurrDrive PROC FAR
     push di
     push si

     mov ah,19h
     int 21h
     xor ah,ah

     pop si
     pop di
     ret
__getCurrDrive ENDP

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

;*
;*   SETKBDLED()
;*
__setKbdLed  PROC FAR
      push ax
      mov ah,1
      sti
      int 16h
      pop ax
      ret
__setKbdLed ENDP


;*
;*  GETSYSTIME ( @ time array) - place heure / minute / seconde
;*                               ds le time array
;*                bp+6 , bp+8
;*
__getsystime PROC FAR
     push bp
     mov bp,sp

     mov ah,2ch                 ; appel fonction DOS
     int 21h

     les bx,[bp+6]              ; es = seg array  bx = offset array
     xor ax,ax
     mov es:[bx],ax
     mov es:[bx+2],ax
     mov es:[bx+4],ax
     mov es:[bx],ch             ; heures
     mov es:[bx+2],cl           ; minutes
     mov es:[bx+4],dh           ; secondes

     pop bp
     ret
__getsystime ENDP

;*
;*   GETSYSDATE( @ date array) - place mois / jour / annee
;*                                ds le date array
;*                 bp+6 , bp+8
;*
__getsysdate PROC FAR
     push bp
     mov bp,sp

     mov ah,2ah                 ; appel fonction DOS
     int 21h

     les bx,[bp+6]              ; es = seg array  bx = offset array
     xor ax,ax
     mov es:[bx],ax
     mov es:[bx+2],ax
     mov es:[bx+4],ax

; a l'americaine

     mov es:[bx],dh             ; mois
     mov es:[bx+2],dl           ; jour
     mov es:[bx+4],cx           ; annee

;     mov es:[bx],cx             ; annee
;     mov es:[bx+2],dh           ; mois
;     mov es:[bx+4],dl           ; jour

     pop bp
     ret
__getsysdate ENDP

;*
;*  FASTCPY(destination, source, count) - copie rapide
;*  char *destination;
;*  char *source;
;*  int count;
;*
_fastcpy PROC FAR
     push bp
     mov bp,sp
     push ds
     push di
     push si
   ;  pushf
   ;  cli
     les di,arg1         ; recupere l'adresse destination
     lds si,arg3         ; recupere l'adresse source
     mov cx,arg5         ; recupere le compteur
     or cx,cx
     jz _copy2
     rep movsb

_copy2:
   ;  popf
     pop si
     pop di
     pop ds
     pop bp
     ret
_fastcpy ENDP

;*
;*  FASTSET(destination, val, count) - initialisation rapide
;*  char *destination;
;*  char val;
;*  int count;
;*
_fastset PROC FAR
     push bp
     mov bp,sp
     push ds
     push di
     push si
;     pushf
;     cli
     les di,arg1         ; recupere l'adresse destination
     mov ax,arg3         ; recupere l'adresse source
     mov cx,arg4         ; recupere le compteur
     or cx,cx
     jz _set2
     rep stosb

_set2:
;     popf
     pop si
     pop di
     pop ds
     pop bp
     ret
_fastset ENDP

;*
;*  GETDSKSTATUS - retourne le status de la derniere operation effectuee
;*
;_getdskstatus PROC FAR
;     push di
;     push si
;     push es
;     push ds
;     mov ah,1
;
;     int 13h
;     pop ds
;     pop es
;     pop si
;     pop di
;     ret
;_getdskstatus ENDP

;*
;*  DSKCHG  : pout AT seulement . indique si une disquette a ete remplacee
;*
__dskchg PROC FAR
     push bp
     mov bp,sp
     push di
     push si
     mov ah,16h
     mov dx,arg1          ; drive … tester
     int 13h
     mov al,ah
     xor ah,ah            ; ah = no du drive
     pop si
     pop di
     pop bp
     ret
__dskchg ENDP

;*
;*  DSKRDY  :
;*
__dskrdy PROC FAR
     push bp
     mov bp,sp
     push di
     push si
     mov ah,10h
     mov dx,arg1          ; drive … tester
     int 13h
     mov al,ah
     xor ah,ah
     pop si
     pop di
     pop bp
     ret
__dskrdy ENDP

;*
;*   GETDISKTYP(driveID,&(long)nsec) - disque type
;*
;_GetDiskTyp PROC FAR
;     push bp
;     mov bp,sp
;     push di
;     push si
;
;     mov dx,arg1
;     mov ah,15h
;     int 13h
;
;     les bx,arg2
;     mov es:[bx],dx             ; placer poids faibles du nb de secteurs
;     mov es:[bx+2],cx           ; placer poids forts
;
;     pop si
;     pop di
;     pop bp
;     ret
;_GetDiskTyp ENDP

;*
;*  MOUSE_INTERFACE (@ descripteur interface block)
;*
;_getinfo PROC FAR
;     push es
;     push ax
;     push bx
;     mov ax,bx
;     mov bx,seg _ib
;     mov es,bx
;     mov bx,offset _ib
;     mov es:[bx],ax       ;etat boutons
;     mov es:[bx+2],cx     ; Xmou
;     mov es:[bx+4],dx     ; Ymou
;     pop bx
;     pop ax
;     pop es
;     ret
;_getinfo ENDP

;_prolog_it_mouse PROC FAR
;     push ds
;     push ax
;     mov ax, DGROUP
;     mov ds,ax
;     pop ax
;     call FAR PTR _getinfo  ; recopier l'information
;     call FAR PTR _it_mouse
;     pop ds
;     ret
;_prolog_it_mouse ENDP

_TEXT ENDS
END
