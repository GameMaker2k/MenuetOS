;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   M32 Menu {bottom}
;
;   Copyright 2015 Ville Turjanmaa
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

use32

    org  0x0

    db	 'MENUET01'		; Header id
    dd	 0x01			; Version
    dd	 START			; Start of code
    dd	 I_END			; Size of image
    dd	 0x100000		; Memory for app
    dd	 0x7fff0		; Esp
    dd	 0x0,0x0		; I_Param,I_Icon

START:				; Start of execution

    mov  eax,58
    mov  ebx,loadfile
    int  0x40

    call get_dimensions

    call read_applications

    call draw_window

still:

    mov  eax,23 		; Wait here for event
    mov  ebx,100
    int  0x40

    cmp  eax,1			; Redraw request
    je	 red
    cmp  eax,2			; Key in buffer
    je	 key
    cmp  eax,3			; Button in buffer
    je	 button

    call draw_time

    push dword [checksum]
    call read_applications
    pop  eax
    cmp  eax , [checksum]
    je	 still

    call draw_window
    jmp  still

red:				; Redraw
    call draw_window
    jmp  still

key:				; Key
    mov  eax,2			; Just read it and ignore
    int  0x40

    jmp  still

button: 			; Button
    mov  eax,17 		; Get id
    int  0x40

    shr  eax , 8

    cmp  eax , 100
    jne  nomenustart

    ; mdm at processes ?
    mov  ecx , 1
  newprocesscheck:
    mov  ebx , processtable
    mov  eax , 9
    int  0x40
    cmp  [processtable+50],byte 0
    jne  nomdm
    mov  eax , [processtable+10]
    and  eax , 0xffffff
    cmp  eax , 'MDM'
    je	 newl1
  nomdm:
    add  ecx , 1
    cmp  ecx , 250
    jb	 newprocesscheck
    jmp  startmenu
    ;
  newl1:

    mov  eax , 60
    mov  ebx , 2
    mov  ecx , [childpid]
    mov  edx , ipcmessage
    mov  esi , 1
    mov  edi , 1
    int  0x40
    mov  [childpid],dword 0
    jmp  still

  startmenu:
    mov  eax , 58
    mov  ebx , startapp
    int  0x40
    mov  ecx , eax
    mov  ebx , processtable
    mov  eax , 9
    int  0x40
    mov  eax , [processtable+30]
    mov  [childpid],eax
    jmp  still
  nomenustart:

    jmp  still


get_dimensions:

    mov  eax , 14
    int  0x40

    add  ax , 1
    mov  [screeny],ax
    shr  eax , 16
    add  ax , 1
    mov  [screenx],ax

    ret


draw_window:

    mov  eax,12 		; Function 12: window draw
    mov  ebx,1			; Start of draw
    int  0x40

				; DRAW WINDOW ( Type 4 )
    mov  eax,0			; Function 0 : define and draw window
    mov  ebx,[screenx]
    sub  ebx,1
    mov  ecx,[screeny]
    sub  ecx,30
    shl  ecx,16
    add  ecx,29
    mov  edx,0x01000000 	; Color of work area RRGGBB,8->color gl
    mov  esi,0x01000000 	; Pointer to window label (asciiz) or z
    mov  edi,0;                 ; Pointer to menu struct or zero
    int  0x40

    mov  eax, 38
    mov  ebx, [screenx]
    mov  ecx, 0
    mov  edx, 0xc0c0c0
  newline:
    int  0x40
    sub  edx , 0x030303
    add  ecx , 1 shl 16 + 1
    cmp  cx  , 29
    jb	 newline

    mov  eax, 38
    mov  ebx, [screenx]
    mov  ecx, 0
    mov  edx, 0xf0f0f0
    int  0x40

    mov  eax,8
    mov  ebx,20 shl 16 + 89
    mov  ecx,4	shl 16 + 19
    mov  edx,100
    mov  esi,0x20d4d4d4
    int  0x40

    mov  eax,7
    mov  ebx,0x20000+140*3*13+3*48
    mov  ecx,74 shl 16 + 1
    mov  edx,29 shl 16 + 9
  newpicline:
    push eax ebx ecx edx
    int  0x40
    pop  edx ecx ebx eax
    add  edx,1
    add  ebx,140*3
    cmp  dx,22
    jb	 newpicline

    mov  ebx,21 shl 16 + 89
    mov  ecx,5	shl 16 + 19
    mov  ebp,0xffffff
    mov  edi,ebp
    call draw_rectangle
    mov  ebx,20 shl 16 + 89
    mov  ecx,4	shl 16 + 19
    mov  ebp,0x606060
    mov  edi,ebp
    call draw_rectangle

    mov  ecx, [screenx]
    sub  ecx, 80
    call draw_separator
    mov  ecx, 131
    call draw_separator

    call draw_time

    mov  [checksum],dword 0
    call display_applications

    mov  eax,12 		; Function 12: window draw
    mov  ebx,2			; End of draw
    int  0x40

    ret


draw_time:

    mov  eax , 3
    int  0x40

    mov  ebp , eax

    mov  ebx , eax
    and  eax , 0x07
    add  eax , '0'
    mov  ecx , ebx
    and  ecx , 0x70
    shr  ecx , 4
    add  ecx , '0'
    mov  [text+0],cl
    mov  [text+1],al

    shr  ebx , 8
    mov  eax , ebx
    and  eax , 0x07
    add  eax , '0'
    mov  ecx , ebx
    and  ecx , 0x70
    shr  ecx , 4
    add  ecx , '0'
    mov  [text+3],cl
    mov  [text+4],al

    mov  [text+2],byte ':'
    test ebp , 0x010000
    jz	 dtl1
    mov  [text+2],byte ' '
  dtl1:

    mov  eax,13
    mov  ebx,[screenx]
    sub  ebx,60
    shl  ebx,16
    add  ebx,5*6-2+2*5
    mov  ecx,7 shl 16 + 14
    mov  edx,0xc0c0c0
    int  0x40

    mov  ebp,0x505050
    mov  edi,0xf0f0f0
    call draw_rectangle

    mov  eax,4			; Draw info text
    mov  ebx,[screenx]
    sub  ebx,55
    shl  ebx,16
    add  ebx,11
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,-1
    int  0x40

    ret

draw_separator:

    mov  eax, 38
    mov  ebx, ecx
    shl  ebx, 16
    add  bx , cx
    mov  ecx, 27
    mov  edx, 0xe0e0e0
    int  0x40
    add  ebx, 1 shl 16 + 1
    mov  edx, 0x606060
    int  0x40

    ret

draw_rectangle:

    mov  eax,38
    movzx edx,bx
    shr  ebx,16
    add  edx,ebx
    shl  ebx,16
    add  ebx,edx
    movzx edx,cx
    shr  ecx,16
    add  edx,ecx
    shl  ecx,16
    add  ecx,edx
    push ebx
    mov  esi,ebx
    shr  esi,16
    mov  bx ,si
    mov  edx,ebp
    int  0x40
    pop  ebx
    push ebx
    mov  esi,ebx
    and  esi,0xffff
    mov  bx ,si
    shl  ebx,16
    mov  bx,si
    mov  edx,edi
    int  0x40
    pop  ebx
    push ecx
    mov  esi,ecx
    shr  esi,16
    mov  cx ,si
    mov  edx,ebp
    int  0x40
    pop  ecx
    push ecx
    mov  esi,ecx
    and  esi,0xffff
    mov  cx ,si
    shl  ecx,16
    mov  cx,si
    mov  edx,edi
    int  0x40
    pop  ecx

    ret


read_applications:

    mov  [appcount],dword 0
    mov  edi , apps
    mov  ecx , 32*256
    mov  eax , 0
    cld
    rep  stosb

    mov  [checksum],dword 0

    mov  ecx , 1

  newappread:

    mov  eax , 9
    mov  ebx , osworkarea
    int  0x40

    push ecx

    mov  esi , osworkarea+10
    mov  edi , ecx
    imul edi , 32
    add  edi , apps
    mov  ecx , 32
    cld
    rep  movsb

    mov  eax , [osworkarea]
    add  [checksum],eax

    pop  ecx

    add  ecx , 1

    cmp  ecx , 64
    jb	 newappread

    mov  [appcount],dword 64

    ret


display_applications:

    mov  [appx],dword 10
    mov  [appp],dword apps
    mov  [appl],dword 4*6+13

    mov  eax , 1
    mov  edi , apps

  newappdisplay:

    push eax edi

    cmp  [edi],dword 'OS/I'
    je	 noappdisplay
    cmp  [edi],dword 'ICON'
    je	 noappdisplay
    cmp  [edi],word 'MD'
    je	 noappdisplay
    cmp  [edi],dword 'MPAN'
    je	 noappdisplay

    mov  eax,edi
    mov  ebx,0
  newlen:
    cmp  [eax],byte 32
    jbe  lenfound
    add  ebx , 1
    add  eax , 1
    jmp  newlen
  lenfound:

    cmp  ebx , 0
    je	 noappdisplay

    imul ebx , 6
    add  ebx , 13
    mov  [appl],ebx

    add  ebx , [appx]
    add  ebx , 250
    cmp  ebx , [screenx]
    jae  appsdone

    mov  [appp],dword edi
    call draw_app

    mov  eax , [appl]
    add  eax , 6*3
    add  [appx],eax

  noappdisplay:

    pop  edi eax

    add  edi , 32
    add  eax , 1

    cmp  eax , [appcount]
    jbe  newappdisplay

    ret

  appsdone:

    pop  edi eax

    ret


draw_app:

    mov  eax,13
    mov  ebx,[appx]
    add  ebx,143
    shl  ebx,16
    add  ebx,[appl]
    mov  ecx,6 shl 16 + 14+2
    mov  edx,0xc0c0c0
    int  0x40

    mov  ebp,0x505050
    mov  edi,0xf0f0f0
    call draw_rectangle

    mov  eax,4			; Draw info text
    mov  ebx,[appx]
    add  ebx,143+8
    shl  ebx,16
    add  ebx,11
    mov  ecx,0x000000
    mov  edx,[appp]
    mov  esi,-1
    int  0x40

    ret




; DATA AREA

text:

    db	 '15:28',0

screenx: dq 0x0
screeny: dq 0x0

appx:	 dd 0
appp:	 dd 0
appl:	 dd 0

checksum: dq 0x0

menu_struct:		   ; Menu Struct

    dq	 0		   ; Version

    dq	 0x100		   ; Start value of ID to return ( ID + Line )

			   ; Returned when menu closes and
			   ; user made no selections.

    db	 0,'FILE',0	   ; ID = 0x100 + 1
    db	 1,'New',0	   ; ID = 0x100 + 2
    db	 1,'Open..',0	   ; ID = 0x100 + 3
    db	 1,'Save..',0	   ; ID = 0x100 + 4
    db	 1,'-',0	   ; ID = 0x100 + 5
    db	 1,'Quit',0	   ; ID = 0x100 + 6

    db	 0,'HELP',0	   ; ID = 0x100 + 7
    db	 1,'Contents..',0  ; ID = 0x100 + 8
    db	 1,'About..',0	   ; ID = 0x100 + 9

    db	 255		   ; End of Menu Struct

startapp:

    dd	 16
    dd	 0
    dd	 param
    dd	 0
    dd	 0x10000
    db	 '/rd/1/mdm',0

param: db 'AA.00000000.MAINMENU.',0

loadfile:

    dd	 0
    dd	 0
    dd	 100
    dd	 0x20000
    dd	 0x10000
    db	 '/rd/1/mpanel.bmp',0

appcount:   dd 0
childpid:   dq 0x0
ipcmessage: db 'T',0

processtable: times 2000 db ?

osworkarea: times 1024*16 db ?

apps:

I_END:





