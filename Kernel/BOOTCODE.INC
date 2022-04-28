;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   Bootcode.inc (c) Ville Turjanmaa
;;   License: GPL. See file copying for details.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;    16 bit functions
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
print:

    push  si
    mov   si,leftpr
    call  printplain
    pop   si
   
printplain:

    pusha
    mov   dx,0x1000
    mov   es,dx
    cmp   byte [es:display_atboot],2
    je    printplain_exit
    mov   ds,dx
    cld
    lodsb
  prpl1:
    mov   ah,0xe
    xor   bh,bh
    int   0x10
    lodsb
    cmp   al,0
    jne   prpl1
  printplain_exit:
    popa

    ret
   
setbase1000:

    push  ax
    mov   ax,0x1000
    mov   es,ax
    mov   ds,ax
    pop   ax

    ret
   
getkey:

    push  ecx
    push  edx
    add   ebx,0x0101
    xor   eax,eax
   
  gk1:
    in    al,0x60
    mov   cl,al
  gk0:
    in    al,0x60
    cmp   al,cl
    je    gk0
    cmp   ax,11
    jg    gk0
    mov   cl,al
   
    add   al,47
    mov   [ds:keyinbs],al
    mov   si,keyinbs
    call  printplain
   
  gk12:
    in    al,0x60
    cmp   al,cl
    je    gk12
    cmp   ax,240
    jne   gk13
    mov   al,cl
    jmp   gk14
  gk13:
    add   cl,128
    cmp   al,cl
    jne   gk1
    sub   al,128
  gk14:
   
    movzx edx,bl
    cmp   eax,edx
    jb    gk1
    movzx edx,bh
    cmp   eax,edx
    jg    gk1
    test  ebx,0x010000
    jnz   gk3
    mov   cx,0x1000
    mov   dx,cx
    add   eax,47
    mov   cx,ax
    cmp   cx,58
    jb    gk_nozero
    sub   cx,10
  gk_nozero:
    mov   [ds:keyin],cl
    mov   si,keyin
    call  printplain
  gk3:
    sub   eax,48
    pop   edx
    pop   ecx

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;     16 bit start
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
start_of_code:

; Reset 16 bit selectors, registers and stack

    mov  ax,0x1000
    mov  es,ax
    mov  ds,ax

    mov  ax,0x2000
    mov  ss,ax
    mov  sp,0xffff

    xor  ax,ax
    xor  bx,bx
    xor  cx,cx
    xor  dx,dx
    xor  si,si
    xor  di,di
    xor  bp,bp

; Draw welcome screen

    call setbase1000

    cmp  byte [es:display_modechg],2
    je   no_mode_atboot
   
    mov  ax,0x0003
    mov  bx,0x0000
    mov  dx,0x0000
    int  0x10

  no_mode_atboot:

    call setbase1000
    mov  ax,0xb800
    mov  es,ax
    mov  di,0
    mov  si,d80x25
    mov  cx,80*25
    mov  ah,1*16+15
  dfl1:
    cld
    lodsb
    stosw
    loop dfl1
  
    call setbase1000
   
; Display Menuet wesion
   
    mov   si,linef2
    call  printplain
    mov   si,version
    call  print
  
; 386+ ?
   
    pushf
    pop   ax
    mov   dx,ax
    xor   ax,0x4000
    push  ax
    popf
    pushf
    pop   ax
    and   ax,0x4000
    and   dx,0x4000
    cmp   ax,dx
    jnz   cpufine
    mov   si,not386
    call  print
    jmp   $
  cpufine:

; Reset 32 bit selectors, registers and stack

    mov   ax,0x1000
    mov   es,ax
    mov   ds,ax

    mov   ax,0x2000
    mov   ss,ax
    mov   esp,0xfff0

    xor   eax,eax
    xor   ebx,ebx
    xor   ecx,ecx
    xor   edx,edx
    xor   esi,esi
    xor   edi,edi
    xor   ebp,ebp

; Flush keyboard controller
   
    mov   ecx,10000
  fl1:
    in    al,0x64
    loop  fl1
    test  al,1
    jz    fl2
    in    al,0x60
    jmp   fl1
  fl2:
   
; Display Vesa version
   
    mov   ax,0x0
    mov   es,ax
    mov   ax,0x4f00
    mov   di,0xa000
    int   0x10
    cmp   ax,0x004f
    je    vesaok2
    mov   dx,0x1000
    mov   es,dx
    mov   si,novesa
    call  print
    mov   ax,16
    jmp   novesafound
  vesaok2:
    mov   ax,[es:di+4]
    mov   dx,ax
    add   ax,48*256+48
    push  word 0x1000
    pop   es
    mov   [es:vervesa+19],ah
    mov   [es:vervesa+21],al
    mov   si,vervesa
    call  print
  novesafound:
    call  setbase1000
   
; User selects graphics mode
   
    movzx eax,byte [es:preboot_graph]
    cmp   eax,0
    jne   pre_graph
    mov   si,gr_mode
    call  printplain
  gml0:
    mov   ebx,0x0A01
    call  getkey
  pre_graph:
    cmp   eax,1
    jl    sgml1
    cmp   eax,8
    jg    sgml1
    mov   si,ax
    sub   si,1
    shl   si,4
    add   si,gr_table
    mov   bx,[es:si+0]
    mov   cx,[es:si+4]
    mov   dx,[es:si+8]
    jmp   gml10
  sgml1:
    cmp   al,9
    jnz   gml00
    mov   bx,0x13
    mov   cx,640
    mov   dx,480
    push  word 0x0
    pop   es
    mov   [es:0x9000],byte 32
    push  word 0x1000
    pop   es
    jmp   gml10
  gml00:
    cmp   al,0xa
    jnz   gml02
    mov   bx,0x12
    mov   cx,640
    mov   dx,480
    push  word 0x0
    pop   es
    mov   [es:0x9000],byte 32
    push  word 0x1000
    pop   es
    jmp   gml10
  gml02:
    jmp   gml0
  gr_table:
    dd    0x112+0100000000000000b ,  640 ,  480 , 0
    dd    0x115+0100000000000000b ,  800 ,  600 , 0
    dd    0x118+0100000000000000b , 1024 ,  768 , 0
    dd    0x11B+0100000000000000b , 1280 , 1024 , 0
    dd    0x112 ,  640 , 480 , 0
    dd    0x115 ,  800 , 600 , 0
    dd    0x118 , 1024 , 768 , 0
    dd    0x11B , 1280 ,1024 , 0
  gml10:
    push  word 0x0000
    pop   es
    mov   [es:0x9008],bx
    mov   [es:0x900A],cx
    mov   [es:0x900C],dx
    push  word 0x1000
    pop   es
    mov   ax,32
    cmp   bx,0x13
    je    nov
    cmp   bx,0x12
    je    nov

; Default graphics or probe ?

    ; bx - mode : cx - x size : dx - y size

    movzx eax,byte [es:preboot_gprobe]
    cmp   eax,0
    jne   noprobe

    test  bx,0100000000000000b
    jz    noprobe
    mov   si,probetext
    call  printplain
    push  bx
    mov   ebx,0x0201
    call  getkey
    pop   bx
    cmp   eax,1
    je    noprobe

    push  cx dx

    mov   bx,0x100

  newprobe:

    inc   bx
    cmp   bx,0x17f
    jne   probemore

    mov   si,prnotfnd
    call  printplain

    jmp   $

  probemore:

    mov   ax,0x4f01
    mov   cx,bx
    and   cx,0xfff
    push  word 0x0000
    pop   es
    mov   di,0xa000
    int   0x10

    mov   eax,[es:di]       ; Lfb ?
    test  eax,10000000b
    jz    newprobe

    mov   eax,[es:di+0x12]  ; X size ?
    cmp   ax,word [esp+2]
    jne   newprobe

    mov   eax,[es:di+0x14]  ; Y size ?
    cmp   ax,dx
    jne   newprobe

    movzx eax,byte [es:di+0x19]
    cmp   eax,24
    jb    newprobe

    push  word 0x0000       ; Save probed mode
    pop   es
    add   bx,0100000000000000b
    mov   [es:0x9008],bx
    push  word 0x1000
    pop   es

    push  bx

    mov   si,prid
    call  printplain

    pop   bx dx cx

  noprobe:
   

; Find Vesa 2.0 Lfb and Bpp
   
    mov   ax,0x4f01
    mov   cx,bx
    and   cx,0xfff
    push  word 0x0000
    pop   es
    mov   di,0xa000
    int   0x10
    ; LFB
    mov   ecx,[es:di+0x28]
    mov   [es:0x9018],ecx
    ; BPP
    movzx ax,byte [es:di+0x19]
    mov   [es:0x9000],ax
    ; ---- vbe voodoo
    BytesPerScanLine equ 0x10
    push  ax
    mov   ax, [es:di+BytesPerScanLine]
    mov   [es:0x9001],ax
    pop   ax
    ; -----
  nov:
    cmp   ax,24
    jnz   nbpp24
    mov   si,bt24
    jmp   bppl
  nbpp24:
    cmp   ax,32
    jnz   nbpp32
    mov   si,bt32
    jmp   bppl
  nbpp32:
    mov   si,btns
    call  print
    jmp   $
  bppl:
    call  printplain
   
; Find Vesa 1.2 bank switch address
   
    mov   ax,0x4f0A
    mov   bx,0x0
    int   0x10
    xor   eax,eax
    xor   ebx,ebx
    mov   ax,es
    shl   eax,4
    mov   bx,di
    add   eax,ebx
    xor   ebx,ebx
    mov   bx,[es:di]
    add   eax,ebx
    push  word 0x0
    pop   es
    mov   [es:0x9014],eax
    push  word 0x1000
    pop   es
   
; MTRR graphics acceleration
   
    movzx eax,byte [es:preboot_mtrr]
    cmp   eax,0
    jne   pre_mtrr
    mov   si,gr_acc
    call  printplain
    mov   ebx,0x0201
    call  getkey
  pre_mtrr:
    push  word 0x0000
    pop   es
    mov   [es:0x901C],al
    push  word 0x1000
    pop   es
    mov   si,linef
    call  printplain
   
; Get mouse port
   
    movzx eax,byte [es:preboot_mouse]
    cmp   eax,0
    jne   pre_mouse
    mov   si,askmouse
    call  print
    mov   ebx,0x0301
    call  getkey
  pre_mouse:
    push  word 0x0000
    pop   es
    mov   [es:0x9010],al
    push  word 0x1000
    pop   es
    mov   si,linef
    call  printplain

; Memory size
   
    movzx eax,byte [es:preboot_memory]
    cmp   eax,0
    jne   pre_mem
    mov   si,mem_model
    call  printplain
    mov   ebx,0x0701
    call  getkey
    push  word 0x0000
    pop   es
    mov   [es:0x9030],al
    push  word 0x1000
    pop   es
    mov   si,linef
    call  printplain
  pre_mem:
   
; Direct write to LFB, paging disabled
   
    movzx eax,byte [es:preboot_lfb] 
    mov   eax,1         ; Paging disabled
    cmp   eax,0
    jne   pre_lfb
    mov   si,gr_direct
    call  printplain
    mov   ebx,0x0201
    call  getkey
  pre_lfb:
    push  word 0x0000
    pop   es
    mov   [es:0x901E],al
    mov   ax,0x1000
    mov   es,ax
   
; Boot device
   
    movzx eax,byte [es:preboot_device]
    cmp   eax,0
    jne   pre_device
    mov   si,bdev
    call  printplain
    mov   ebx,0x0301
    call  getkey
  pre_device:
    dec   al
    mov   [es:boot_dev],al
    mov   si,linef
    call  printplain

; Read diskette to memory
   
    cmp   [boot_dev],byte 0
    jne   no_sys_on_floppy
    mov   si,diskload
    call  print
    mov   ax,0x0000         ; Reset drive
    mov   dx,0x0000
    int   0x13
    mov   cx,0x0001         ; Startcyl,startsector
    mov   dx,0x0000         ; Starthead,drive
    push  word 80*2         ; Read, number of sectors
  reads:
    pusha
    xor   si,si
  newread:
    push  word 0x0
    pop   es
    mov   bx,0xa000         ; es:bx -> data area
    mov   ax,0x0200+18      ; Read, number of sectors to read
    int   0x13
    cmp   ah,0
    jz    goodread
    add   si,1
    cmp   si,10
    jnz   newread
    mov   si,badsect
    call  printplain
    jmp   $
  goodread:
    ; move -> 1mb
    mov   si,movedesc
    push  word 0x1000
    pop   es
    mov   cx,256*18
    mov   ah,0x87
    int   0x15

    cmp   ah,0               ; Was the move successfull ?
    je    goodmove
    mov   dx,0x3f2           ; Floppy motor off
    mov   al,0
    out   dx,al
    mov   si,memmovefailed
    call  print
    jmp   $
  goodmove:

    mov   eax,[es:movedesc+0x18+2]
    add   eax,512*18
    mov   [es:movedesc+0x18+2],eax
    popa
    inc   dh
    cmp   dh,2
    jnz   bb2
    mov   dh,0
    inc   ch
    pusha                     ; Display prosentage
    push  word 0x1000
    pop   es
    xor   eax,eax  ; 5
    mov   al,ch
    shr   eax,2
    and   eax,1
    mov   ebx,5
    mul   bx
    add   al,48
    mov   [es:pros+1],al
    xor   eax,eax  ; 10
    mov   al,ch
    shr   eax,3
    add   al,48
    mov   [es:pros],al
    mov   si,pros
    call  printplain
    popa
  bb2:
    pop   ax
    dec   ax
    push  ax
    cmp   ax,0
    jnz   rs
    jmp   readdone
  rs:
    jmp   reads
  movedesc:
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
    db    0xff,0xff,0x0,0xa0,0x00,0x93,0x0,0x0
    db    0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
    db    0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
  readdone:
    pop   ax
    mov   si,backspace
    call  printplain
    call  printplain
    mov   si,okt
    call  printplain
  no_sys_on_floppy:
    mov   dx,0x3f2            ; Floppy motor off
    mov   al,0
    out   dx,al
   
; Pageing table

    push  word 0x0000
    pop   es
    mov   ecx,[es:0x9018]
    push  ecx

    map_mem equ 63            ; Amount of memory to map

    mov   bx,0x6000
    mov   es,bx                   ; [es:di] = 6000:0
    xor   edi,edi
    mov   ecx,256*map_mem         ; Map (mapmem) M
    mov   eax,7
    cld
  pt2:
    cmp   ecx,256*(map_mem-8)     ; 8 M map to LFB
    jnz   pt3
    pop   eax
    add   eax,7
  pt3:
    cmp   ecx,256*(map_mem-12)    ; 12 M back to linear = physical
    jnz   pt4
    mov   eax,12*0x100000 + 7
  pt4:
    stosd
    add   eax,4096
    loop  pt2

; 4 kb paging 
   
    mov   bx , 0x7000
    mov   es , bx                 ; [es:di] = 7000:0
    xor   edi, edi
    mov   ecx, 64 / 4
    mov   eax, 0x60007            ; For 0 M
    cld
  pd4k:
    stosd
    add   eax, 0x01000
    loop  pd4k
    mov   eax, 0x70000 +8+16      ; Page directory and enable caches
    mov   cr3, eax
   
; Set graphics
   
    mov   dx,0x0000
    mov   es,dx
    mov   bx,[es:0x9008]
    mov   ax,bx                ; Vga & 320x200
    cmp   ax,0x13
    je    setgr
    cmp   ax,0x12
    je    setgr
    mov   ax,0x4f02            ; Vesa
  setgr:
    int   0x10
    cmp   ah,0
    jz    gmok
    mov   si,fatalsel
    call  print
    jmp   $
  gmok:
    mov   dx,0x1000
    mov   es,dx
   
; Set mode 0x12 graphics registers
   
    cmp   bx,0x12
    jne   gmok2
   
    mov   al,0x05
    mov   dx,0x03ce
    out   dx,al      ; Select GDC mode register
    mov   al,0x02
    mov   dx,0x03cf
    out   dx,al      ; Set write mode 2
   
    mov   al,0x02
    mov   dx,0x03c4
    out   dx,al      ; Select VGA sequencer map mask register
    mov   al,0x0f
    mov   dx,0x03c5
    out   dx,al      ; Set mask for all planes 0-3
   
    mov   al,0x08
    mov   dx,0x03ce
    out   dx,al      ; Select GDC bit mask register
                     ; For writes to 0x03cf
  gmok2:
    mov   dx,0x1000
    mov   es,dx


; Eof - Return to kernel
   
   
