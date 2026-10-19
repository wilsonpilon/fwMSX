;***** Amazonia ***************************************************************
; Amazonia versao ROM 64kb, usando os 16Kb baixos para armazenar dados como
; tela de abertura, tela de credito e os 16Kb altos para musica
;******************************************************************************
.rom
.size 48
BOOT            equ 0D400h                      ;Endereco da rotina de nossa BOOT
CART            equ 4000h                       ;Endereco do CARTUCHO
RECOM           equ 69D6h                       ;comando RECOMECA (ver tambem 6A1D)
REPLAYER        equ 4000h                       ;Endereco onde o REPLAYER vai ser executado
BOOTJOGO        equ 69EEh                       ;Inicio do Jogo adaptado
.include "source/msxsys.asm"

;***** 16Kb mais baixos da ROM ************************************************
        org 0000h
AMAZON0:
.incbin "64k/AMAZONIA.DA0"                      ;Tela no formato BSAVE,R em #9200
AMAZONR:
.include "source/pt3-rom.asm"                   ;Replayer PT3 em #4000

;***** Programa principal e de inicializacao **********************************
        org CART                                ;Pagina #1
        db 041h,042h
        dw initmain                             ;Ponto de Execucao da ROM
        ds 12                                   ;Completa cabecalho

initmain:
        di                                      ;Desabilita interrupcoes
        im 1                                    ;Tipo de interrupcao
        ld sp,RDPRIM                            ;Leitura de um slot primario

        call search_slotset                     ;Rotina de procura do SLOT
        call init_port                          ;Procura os PORTOS do VDP
        
        call DISSCR
        call loadreplayer
        call ENASCR

        ld a,2
        call set_screen
        call loadscreen                         ;Carrega a tela do Amazonia

        call INIT                               ;Inicializa a Musica
        call TUNES                              ;Toca a musica
STOPTUN:
        call GICINI                             ;Para musica
        call transferencia                      ;Transfere os blocos do jogo
        ld a,0                                  ;Inicia o modo SCREEN 0
        call set_screen                         ;Modo 0
        
        call creditos
       
        ld a,0
        ld [CSRSW],a
        ld h,12
        ld l,5
        call POSIT
        ld a,1
        ld [CAPST],a
        jp BOOT                                 ;Boot do jogo

;***** Transferencia dos dados da ROM para a RAM ******************************
transferencia:
        call search_slotram                     ;Busca RAM
        call bloco1                             ;Primeiro Bloco 8K em #6000
        call bloco2                             ;Segundo Bloco 20Kb em #8000
        call bloco3                             ;Terceiro Bloco 1K em #E000
        call blocoW                             ;Bloco de rotinas de inicializacao
        ret

;***** Bloco 1 [AMAZONIA.001] de 8kb ******************************************
bloco1:
        ld hl,AMAZON1                           ;Inicio do Amazonia 
        ld de,#6000                             ;Endereco destino RAM [pagina 1, programa AMAZON1 comeca em #6000]
        ld b,#20                                ;laco externo 32 * 256 = 8kb
        ld c,0
        call rom2ram
        ret

;***** Bloco 2 [AMAZONIA.002] de 21kb *****************************************
bloco2:
        ld hl,AMAZON2                           ;Inicio do Amazonia 
        ld de,#8000                             ;Endereco destino RAM [pagina 2, programa AMAZON2 comeca em #8000]
        ld b,#54                                ;laco externo 84 * 256 = 21kb
        ld c,0
        call rom2ram
        ret

;***** Bloco 3 [AMAZONIAdSK] de 256b *****************************************
bloco3:
        ld hl,AMAZONDSK                         ;Rotinas de acesso a disco
        ld de,#E000                             ;Pagina 3 comecando em #E000
        ld b,1                                  ;256 bytes
        ld c,0
        call rom2ram
        ret

;***** Bloco W Rotinas de Preparacao [em BOOT] ********************************
blocoW:
        ld b,8                                  ;laco externo tamanho da rotina
        ld c,0
        ld hl,booti                             ;Programa de ativacao em BOOT
        ld de,BOOT                              ;Pagina 3
        call rom2ram                            ;Transfere dados
        ret

;***** Transfere dados da ROM para a RAM usando a BIOS ************************
rom2ram:
.lop1:
        push bc                                 ;Salvo BC [booti]
        ld b,#FF                                ;laco interno
.lop2:
        push bc                                 ;Salvo laco
        push hl                                 ;Salvo apontador do codigo [booti]
        push de                                 ;Salvo endereco de destino [Pagina 3]

        ld a,[hl]                               ;Leio byte do boot [booti]
        ld e,a                                  ;Valor salvo em DE
        ld d,0                                  ;
        ld a,[slotram]                          ;Monta registro de Slot-ID

        pop hl                                  ;Recupera endereco de destino [CART]
        call WRSLT                              ;Escreve o byte na RAM
        inc hl                                  ;Incrementa proximo destino
        push hl                                 ;Salva proximo destino
        pop de                                  ;Reposiciona destino em DE
        pop hl                                  ;Recupera endereco do boot [booti]
        inc hl                                  ;Proximo endereco de origem em HL

        pop bc
        djnz .lop2                              ;Fim do laco interno
        pop bc
        djnz .lop1                              ;Fim do laco externo
        ret

;***** Carrega a tela *********************************************************
; Esta rotina usa a pagina baixa dos 48k
; Prepara a mesma, envia dados para a VRAM e coloca novamente a 
; BIOS na pagina 0
loadscreen:
        di                                      ;Desabilita as interrupcoes [Importante]
        call setrompage0                        ;Colocar a parte baixa da ROM em 0000h-3FFFh
        ld hl,AMAZON0                           ;Dados da tela
        ld de,0                                 ;Inicio da VRAM
        ld bc,4000h                             ;16Kb a transferir
        call fillvram                           ;Manda os 16Kb par a VRAM
        call recbios                            ;Retorna a BIOS
        ret
;***** Carrega o replayer *****************************************************
; Esta rotina usa a pagina baixa dos 48k
; Prepara a mesma, envia dados para a VRAM e coloca novamente a 
; BIOS na pagina 0 e copia os dados da VRAM para a RAM em 4000
loadreplayer:
        di                                      ;Desabilita as interrupcoes [Importante]
        call DISSCR                             ;Desabilita a SCREEN
        call setrompage0                        ;Colocar a parte baixa da ROM em 0000h-3FFFh
        ld hl,AMAZONR                           ;Dados da tela
        ld de,0                                 ;Inicio da VRAM
        ld bc,2048                              ;2Kb a transferir
        call fillvram                           ;Manda os 16Kb par a VRAM
        call recbios                            ;Retorna a BIOS
        ld hl,0                                 ;Inicio temporario VRAM
        ld de,0C000h                            ;Area da RAM temporaria
        ld bc,2048                              ;tamaho
        call LDIRMV
        call CLS
        ret

;***** Mostra os creditos *****************************************************
creditos:
        ld hl,AMAZON1                           ;Transfere alfabeto do jogo
        ld de,800h + 22*8                       ;para VRAM SCREEN 0
        ld bc,0A8h*8                            ;parcial
        call LDIRVM
        ld hl,COPYR                             ;(C)
        ld de,800h + 251*8                      ;para VRAM SCREEN 0
        ld bc,8                                 ;
        call LDIRVM       
        
        ld de,CRD0
        call pstring
        ld de,CRD1
        call pstring
        ld de,CRD2
        call pstring
        ld de,CRD3
        call pstring
        ld de,CRD4
        call pstring
        ld de,CRD5
        call pstring
        ld de,CRD6
        call pstring
        ld de,CRD7
        call pstring
        ld de,CRD8
        call pstring
        ld de,CRD9
        call pstring
        ret

INIT:
	di                                      ;Disable interrupts
	ld hl,MUSIC-100                         ;hl <- initial address of module - 100
	call PT3_INIT                           ;Inits PT3 player
	ei                                      ;Enable interrupts
        ret

TUNES:
	halt                                    ;Synchro
	di                                      ;Disable interrupts
	; --- Place this instruction on interrupt or after HALT instruction to synchronize music ---
	call PT3_ROUT                           ;Write values on PSG registers
	; --- To speed up VDP writes you can place this instruction after all of them, but before next INT ---
	call PT3_PLAY                           ;Calculates PSG values for next frame
	ei                                      ;Enable interrupts

        ld a,0
        call GTTRIG
        cp 0FFh
        jp z,STOPTUN
        jp TUNES                                ;Close main loop

.include "source/slots.asm"
.include "source/video.asm"

AMAZON1:
.incbin  "64k/AMAZONIA.DA1"                     ;Primeiro bloco do Jogo (#6000, 8k)
AMAZON2:
.incbin  "64k/AMAZONIA.DA2"                     ;Segundo bloco do jogo (#8000, 21k)
AMAZONDSK:
.incbin  "64k/AMAZONIA.DA3"                     ;Bloco de salvamento (#E000, 256b)
ARQMUSICA:
.incbin  "64k/AMAZONIA.DA4"                     ;Musica do Jogo (#0000, xk)
MUSIC	equ ARQMUSICA + 100                     ;Remove o header (100 bytes)

;***** Rotina para ser executada em #D2000, inicia jogo ***********************
booti:
.PHASE 0D400h
        di

        ld a,[slotram]
        ld h,#40
        call ENASLT

        ld a,[slotram]
        ld h,#80
        call ENASLT

        ld a,[slotram]
        ld h,#C0
        call ENASLT

        call 04000h

        ld hl,#F9E0                             ;Carregador BAS
        ld a,#FF
        ld [hl],a

        in a,[#A8]
        and 240
        rrca
        rrca
        ld b,a
        in a,[#a8]
        or b
        ld hl,#F9E1
        ld [hl],a
       
        ei

        ld hl,RECOM                             ;Implementa um HOOK no SELECT
        ld a,0C3h
        ld [hl],a
        inc hl
        ld a,JOGO%256
        ld [hl],a
        inc hl
        ld a,JOGO/256
        ld [hl],a

        jp BOOTJOGO

JOGO:
        di
        call SETRP                              ;Retorna a BIOS
        jp 0

SETRP:
        ld a,[EXPTBL]
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
.DEPHASE
bootf:

COPYR:  .db 0FFh,07h,77h,7Fh,7Fh,77h,07h,0FFh
CRD0:   .db 1,6,41h,6Dh,61h,7Ah,8Eh,6Eh,69h,61h,20h,70h,6Fh,72h,20h,52h,65h,6Eh,61h,74h,6Fh,20h,44h,65h,67h,69h,6Fh,76h,61h,6Eh,69h,00h
CRD1:   .db 5,11,3Ch,0FBh,3Eh,20h,32h,30h,32h,30h,20h,54h,49h,4Ch,54h,20h,6Fh,6Eh,6Ch,69h,6Eh,65h,00h
CRD2:   .db 7,12,65h,20h,42h,69h,74h,6Eh,61h,6Dh,69h,63h,20h,53h,6Fh,66h,74h,77h,61h,72h,65h,00h
CRD3:   .db 11,5,50h,72h,6Fh,67h,72h,61h,6Dh,61h,64h,6Fh,20h,70h,6Fh,72h,00h
CRD4:   .db 12,5,43h,6Ch,0A0h,75h,64h,69h,6Fh,20h,43h,6Fh,73h,74h,61h,20h,65h,20h,52h,65h,6Eh,61h,74h,6Fh,20h,44h,65h,67h,69h,6Fh,76h,61h,6Eh,69h,00h
CRD5:   .db 14,5,4Ch,6Fh,61h,64h,69h,6Eh,67h,20h,53h,63h,72h,65h,65h,6Eh,20h,70h,6Fh,72h,20h,41h,6Eh,64h,79h,20h,47h,72h,65h,65h,6Eh,00h
CRD6:   .db 16,5,4Dh,0A3h,73h,69h,63h,61h,20h,70h,6Fh,72h,20h,50h,65h,64h,72h,6Fh,20h,50h,69h,6Dh,65h,6Eh,74h,61h,00h
CRD7:   .db 18,5,43h,0A2h,64h,69h,67h,6Fh,20h,41h,64h,69h,63h,69h,6Fh,6Eh,61h,6Ch,20h,70h,6Fh,72h,00h
CRD8:   .db 19,5,44h,61h,6Eh,69h,65h,6Ch,20h,52h,61h,76h,61h,7Ah,69h,20h,65h,20h,57h,69h,6Ch,73h,6Fh,6Eh,20h,50h,69h,6Ch,6Fh,6Eh,00h
CRD9:   .db 23,5,50h,72h,65h,73h,73h,69h,6Fh,6Eh,65h,20h,3Ch,53h,45h,4Ch,45h,43h,54h,3Eh,20h,70h,61h,72h,61h,20h,69h,6Eh,69h,63h,69h,61h,72h,00h

;**** Dados do Programa / Variaveis *******************************************
; As variaveis ficam a partier de #D500 na pagina #3
vars            equ 0DE00h                      ;Area de Variaveis
slotvar         equ vars + 1                    ;Slot
slotram         equ slotvar + 1                 ;RAM

vdp_reg98       equ slotram + 1                 ;VDP
vdp_reg99       equ vdp_reg98 + 1
vdp_reg9A       equ vdp_reg99 + 1
vdp_reg9B       equ vdp_reg9A + 1

vdp_reg98r      equ vdp_reg9B + 1
vdp_reg99r      equ vdp_reg98r + 1
vdp_reg9Ar      equ vdp_reg99r + 1
vdp_reg9Br      equ vdp_reg9Ar + 1

dbg             equ vdp_reg9Br + 1              ;Area de Depuracao

        .org 0DE10h
.include	"source/pt3-ram.asm"            ;Variaveis do Replayer em DE10
