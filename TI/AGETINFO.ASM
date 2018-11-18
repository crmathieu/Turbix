;****************************************************************************
;*   AGETINFO.ASM                                                           *
;*                                                                          *
;*   Donne des informations sur le processeur utilis‚ , la frequence        *
;*                                                                          *
;****************************************************************************

arg1 equ  6[bp]
arg2 equ  8[bp]
arg3 equ 10[bp]
arg4 equ 12[bp]

p_8088	equ	0
p_8086	equ	1
p_V20	equ	2
p_V30	equ	3
p_80188 equ	4
p_80186 equ	5
p_80286	equ	6
p_80386	equ	7

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

PUBLIC  _get_proc        ; recuperer le type de processeur
PUBLIC  _get_freq	 ; recuperer la frequence

_DATA SEGMENT
_sseg  dw 0
_soff  dw 0
_DATA ENDS

_TEXT SEGMENT

;*
;*   GET_PROC()
;*
_get_proc proc far
	pushf

; tester 80286/80386

	xor 	ax,ax
	push 	ax
	popf
	pushf
	pop 	ax
	and	ax,0F000h
	cmp	ax,0F000h
	je	pas_386

; tester 286 ou 386

	mov	dl,p_80286
	mov 	ax,07000h
	push	ax
	popf
	pushf
	pop 	ax
	and 	ax,07000h
	je      pfin
	inc	dl
	jmp	pfin

; tester 80186/80188

pas_386:
	mov	dl,p_80188
	mov	al,0FFh
	mov	cl,021h
	shr	al,cl
	jne	t88_86

; tester nec V20 / V30

	mov	dl,p_V20
	sti
	push	si
	mov	si,0
	mov	cx,0FFFFh
	rep	lods byte ptr es:[si]
	pop	si
	or	cx,cx
	je	t88_86

; Arriv‚ ICI c'est un 8088 ou 8086

	mov	dl,p_8088

; tester si 80x88 ou 80x86 et V20 ou V30

t88_86:

	push	cs
	pop	es
	std
	mov	di,offset q_end
	mov	al,0FBh
	mov	cx,3
	cli
	rep	stosb
	cld
	nop
	nop
	nop
	inc	dx

q_end:
	sti

pfin:
	popf
	xor	dh,dh
	mov	ax,dx
	ret
_get_proc	endp

;*
;*   GET_FREQ()
;*
_get_freq proc far
	push	si
	call 	fstart
	mov	si,8000h
	mov     ax,03E8h
	add	ax,0032h
	mov	cx,0064h
	div	cl
	mov	cl,al
	in	al,61h
	mov	bl,al
	or	al,01
	cli
	out	61h,al

; ICI : 100 instruction MUL SI

boucle:
;*** 1 ***
	mul	si	;1
	mul	si	;2
	mul	si	;3
	mul	si	;4
	mul	si	;5
	mul	si	;6
	mul	si	;7
	mul	si	;8
	mul	si	;9
	mul	si	;10
	mul	si	;11
	mul	si	;12
	mul	si	;13
	mul	si	;14
	mul	si	;15
	mul	si	;16
	mul	si	;17
	mul	si	;18
	mul	si	;19
	mul	si	;20
;*** 2 ***
	mul	si	;1
	mul	si	;2
	mul	si	;3
	mul	si	;4
	mul	si	;5
	mul	si	;6
	mul	si	;7
	mul	si	;8
	mul	si	;9
	mul	si	;10
	mul	si	;11
	mul	si	;12
	mul	si	;13
	mul	si	;14
	mul	si	;15
	mul	si	;16
	mul	si	;17
	mul	si	;18
	mul	si	;19
	mul	si	;20
;*** 3 ***
	mul	si	;1
	mul	si	;2
	mul	si	;3
	mul	si	;4
	mul	si	;5
	mul	si	;6
	mul	si	;7
	mul	si	;8
	mul	si	;9
	mul	si	;10
	mul	si	;11
	mul	si	;12
	mul	si	;13
	mul	si	;14
	mul	si	;15
	mul	si	;16
	mul	si	;17
	mul	si	;18
	mul	si	;19
	mul	si	;20
;*** 4 ***
	mul	si	;1
	mul	si	;2
	mul	si	;3
	mul	si	;4
	mul	si	;5
	mul	si	;6
	mul	si	;7
	mul	si	;8
	mul	si	;9
	mul	si	;10
	mul	si	;11
	mul	si	;12
	mul	si	;13
	mul	si	;14
	mul	si	;15
	mul	si	;16
	mul	si	;17
	mul	si	;18
	mul	si	;19
	mul	si	;20
;*** 5 ***
	mul	si	;1
	mul	si	;2
	mul	si	;3
	mul	si	;4
	mul	si	;5
	mul	si	;6
	mul	si	;7
	mul	si	;8
	mul	si	;9
	mul	si	;10
	mul	si	;11
	mul	si	;12
	mul	si	;13
	mul	si	;14
	mul	si	;15
	mul	si	;16
	mul	si	;17
	mul	si	;18
	mul	si	;19
	mul	si	;20

	dec	cx
	je	bfin
	jmp	boucle

bfin:

	mov	al,bl
	out	61h,al
	sti
	call	fstop
	pop	si
	ret
_get_freq endp

;*
;*   FSTART() FSTOP() - sous fonctions de get_freq
;*
fstart proc near
	in	al,61h
	and	al,0FCh
	out	61h,al
	mov	al,0B4h
	out	43h,al
	mov	al,0
	out	42h,al
	jmp	suite

suite:

	out	42h,al
	ret
fstart endp

fstop proc near
	in	al,42h
	mov	ah,al
	in	al,42h
	xchg	ah,al
	neg	ax
	ret
fstop endp


_TEXT ENDS
END
