;***** Rotinas de manipulacao de SLOTS ****************************************
;*****  Busca SLOT da rom para rodar na pagina 0 [0000-3FFF]
search_slotset:
        call search_slot
        ld [slotvar],a
        jp ENASLT

;***** Busca SLOT da rom para rodar na pagina 1 [4000-7FFF] *******************
search_slot:
        call 0138h
        rrca
        rrca
        and 3
        ld c,a
        ld b,0
        ld hl,0FCC1h
        add hl,bc
        ld a,[hl]
        and 080h
        jr z,search_slot0
        or c
        ld c,a
        inc hl
        inc hl
        inc hl
        inc hl
        ld a,[hl]
        and 0Ch
search_slot0:
        or c
        ld h,080h
        ret

;***** Procura SLOT da RAM e armazena *****************************************
search_slotram:
        call RSLREG                             ;Le o registro de slot primario
        rlca                                    ;Separa o dois bits da pagina #3
        rlca
        and 3
        ld c,a                                  ;Salva
        ld b,0
        ld hl,EXPTBL                            ;Slot 0 expandido? Sim #80, Nao #0 [BIOS-ROM]
        add hl,bc                               ;OR nos dois registros
        ld a,[hl]                               ;Confere se expandido
        and 080h
        jr z,search_slotram0                    ;Nao? Encerra armazenando em SLOTRAM
        or c                                    ;Trata expandido
        ld c,a
        inc hl                                  ;Confere SLTTBL
        inc hl
        inc hl
        inc hl
        ld a,[hl]                               ;Le registro secundario
        rlca
        rlca
        rlca
        rlca
        and 0Ch
search_slotram0:
        or c
        ld [slotram],a
        ret
            
;***** Coloca a ROM na Pagina 0 ***********************************************
setrompage0:
        ld a,[slotvar]        
        jr setslotpage0    

;***** Seta a BIOS  novamente *************************************************
recbios:
        ld a,[EXPTBL]
                
;***** Coloca o SLOT na pagina 0 do Z80 ***************************************
; A: Formato FxxxSSPP
setslotpage0:
        di
        ld b,a                                  ;B = Parametro de selecao do no formato FxxxSSPP
                
        in a,[0A8h]
        and 011111100b
        ld d,a                                  ; D = Valor do slot primario
                
        ld a,b            
                
        and 03    
        or d
        ld d,a                                  ; D = Valor do slot primario
            
        out (0A8h),a                        

        ld a,b                                  ;Verifica se expandido
        bit 7,a
        ;jr z,recbiosprimary                    ;Nao expandido
        ret z                                   ;Nao expandido
                
        and 03h                                
        rrca
        rrca
        and 011000000b
        ld c,a                    
        ld a,d
        and 00111111b
        or c
        ld c,a                                  ;Valor do Slot primario na pagina 3

        ld a,b
        and 00001100b
        rrca
        rrca    
        and 03h
        ld b,a                                  ;B = Slot expandido na pagina 3
        ld a,c
        out (0A8h),a                            ;Slot: Main Slot, xx, xx, Main slot
        ld a,[0FFFFh]
        cpl
        and 011111100b
        or b
        ld [0FFFFh],a                           ;Slot Expandido Selecionado

        ld c,a
                                                ;Slot Final. Ram, rom c, rom c, Main
        ld a,d                                  ;A = Valor final
        out (0A8h),a

        and 3                                   ;Seta valor em STLTBL 
        ld de,SLTTBL    
        add a,e
        ld e,a
        jr nc,.nocarry
        inc d
.nocarry:
        ld a,c
        ld [de],a

        ret


;***** Copia 256 bytes da ROM para a RAM **************************************
copmem:
endereco:
        dw 0000h

        ld hl,[endereco]
        ld de,0C000h
        ld bc,100h
        push de
        push hl
        push bc
        ldir
        pop bc
        pop de
        pop hl
        di
        ld a,0AAh
        out (0A8h),a
        ldir
        ld a,0A0h
        out (0A8h),a
        ei
        ret