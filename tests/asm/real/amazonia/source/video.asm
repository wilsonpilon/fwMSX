;***** Rotinas de manipulacao da VRAM *****************************************
;***** Inicializa a VRAM ******************************************************
;  HL : Ponteiro
; A = 1 Escreve
; A = 0 Le
initvram:
        or a
        jp z,initvramrd
initvramwr:     
        di                
        ld a,[vdp_reg99]
        ld c,a                            
        out [c],l
        set 6,h
        nop
        out [c],h
        ret
                
initvramrd:
        di
        ld a,[vdp_reg99]
        ld c,a                            
        out [c],l
        res 6,h
        nop
        out [c],h
        ret


;***** Transfere dados para a VRAM ********************************************
; HL : Origem
; DE : Destino na VRAM
; BC : Quantidade de dados
; A: Pagina
fillvram:
        ex de,hl
        push de
        push bc
        call initvramwr    
        pop bc
        pop hl
fillvram_10:
        ld a,c                
        or a
        ld d,a
                
        ld a,[vdp_reg98]            
        ld c,a
        ld a,b

        jp z,fillvram_11
        ld b,d
fillvram_10l:
        outi                
        jp nz,fillvram_10l
        or a
        ret z
fillvram_11:
    
fillvram_12:
        ld b,0
fillvram_12l:
        outi
        jp nz,fillvram_12l
        dec a
        jp nz,fillvram_12
        ret

;***** Inicializa os PORTOS da VRAM *******************************************
init_port:
        ld hl,07h
        ld a,[0FCC1h]
        call RDSLT
        ld [vdp_reg98],a
        inc a
        ld [vdp_reg99],a
        inc a
        ld [vdp_reg9A],a
        inc a
        ld [vdp_reg9B],a
                
        ld hl,06h
        ld a,[0FCC1h]
        call RDSLT

        ld [vdp_reg98r],a
        inc a
        ld [vdp_reg99r],a
        inc a
        ld [vdp_reg9Ar],a
        inc a
        ld [vdp_reg9Br],a
        ret

set_screen:
        push af
        ld a,1                                  ;Preto
        ld [BDRCLR],a                           ;Cor da Borda
        ld [BAKCLR],a                           ;Cor de Fundo
        ld a,15                                 ;Branco
        ld [FORCLR],a                           ;Cor de Frente
        ld a,40
        ld [LINLEN],a
        ld [LINL40],a
        pop af
        call CHGMOD                             ;Troca de modo
        ret

pstring:
        ld a,[de]
        ld l,a
        inc de
        ld a,[de]
        ld h,a
        call POSIT
        inc de
        ld h,d
        ld l,e
.lop:
        ld a,[hl]
        cp 0
        jr z,.saida
        call CHPUT
        inc hl
        jr .lop
.saida:
        ret