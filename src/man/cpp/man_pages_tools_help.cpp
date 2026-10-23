// fwMSX -- MAN: NestorBASIC, SEE Tracker e MSXBAS2ROM. ARQUIVO GERADO por tools/paleobasic/import_tools_help.py a partir de third-party/paleobasic -- nao editar a mao.
//
// NestorBASIC: Nestor Soriano (Konamiman). SEE: Sound Effect Editor 3.10, Fuzzy Logic 1991/95. MSXBAS2ROM: Amaury Carvalho (GPL).
// Ver doc/man-spec.md, secao "Origem e direitos".
#include "man_internal.h"

namespace man {

std::vector<Topic> ToolsHelpTopics() {
    std::vector<Topic> v;
    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {
        v.push_back(Topic{name, aliases, category, usage, summary, page, false});
    };

    add("nestorbasic", "nestor-basic,nb", "NestorBASIC: introducao", "USR(n)", "NestorBASIC: biblioteca de 87 funcoes (RAM mapeada, disco, VRAM, som) para o MSX-BASIC",
        R"MAN(NESTORBASIC (Nestor Soriano / Konamiman)

NestorBASIC (by Nestor Soriano / Konami Man, 2004) e um conjunto de rotinas em codigo de maquina que da acesso, direto do MSX-BASIC, a memoria mapeada (ate 4Mb), VRAM inteira, disco/setores fisicos, compressao grafica, execucao de programas BASIC guardados em RAM, tocador Moonblaster, efeitos sonoros PSG e mais - tudo TurboBASIC-compativel.

CARREGAMENTO
Uma unica vez por programa: bload "nbasic.bin",r. Depois disso:
- p(0) = numero de segmentos de RAM disponiveis (sempre >=5), ou codigo de erro (0 a 4) se a instalacao falhou
- O primeiro usr() (ou usr0()) fica reservado pro NestorBASIC - usr1 a usr9 continuam livres

IMPORTANTE: o carregamento NUNCA pode ser feito dentro de func/ret (GOSUB/RETURN) - o bload...,r mexe na pilha do BASIC, entao um RETURN depois quebra. Use GOTO/rotulo (ver o template gerado por Arquivo -> Novo Nestor Basic..., que ja faz isso certo).

MODELO DE CHAMADA
Toda funcao usa usr(numero), parametros/retornos no array inteiro p() (defint p:dim p(15) no minimo) e, quando a funcao mexe com nomes de arquivo ou texto, o array de strings f$() (dim f$(1) se precisar de duas strings ao mesmo tempo, dim f$(0) senao). O valor devolvido por usr() e sempre o codigo de erro (0 = sem erro). Cada topico de funcao aqui mostra o nome do wrapper Dignified .NB_* correspondente (gerado em Arquivo -> Novo Nestor Basic...) e um exemplo pronto de como chama-lo - use-os ao inves de montar p()/usr() na mao sempre que possivel.

SEGMENTOS LOGICOS
A RAM mapeada e organizada em segmentos logicos de 16K (endereco &H0000 a &H3FFF dentro de cada um), numerados de forma uniforme independente de slot/segmento fisico real:
- 0 = o proprio NestorBASIC
- 1 = TurboBASIC (so sobrescreva se nao for usar o compilador)
- 2 = RAM principal do BASIC, pagina 2 (&H8000-&HBFFF)
- 3 = RAM principal do BASIC, pagina 3 (&HC000-&HFFFF)
- 4 = buffer interno (usado pelas funcoes marcadas (S4) - nao guarde dados la enquanto usa-las)
- 5 a p(0)-1 = livres pro programa (o 5 vira o tocador Moonblaster se .NB_LoadPlayer for chamado)
- 255 = alias especial pra RAM principal do BASIC (&H8000-&HFFFF direto, sem converter endereco)
- Acima de p(0)-1: segmentos de VRAM (ver .NB_VReadByte e funcoes de VRAM) - so nas funcoes que aceitam

SOBRE ESTA AJUDA
Navegue pela arvore a esquerda (grupos = secoes do manual original) ou digite na caixa de busca pra filtrar por nome, numero de funcao ou grupo.

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-0", "nb-desinstalar-o-nestorbasic-s4,nb_uninstall", "NestorBASIC: Funcoes gerais", "USR(0)", "Desinstalar o NestorBASIC (S4): Desinstala o NestorBASIC: libera o usr() e, no DOS 2,...",
        R"MAN(FUNCAO 0 do NestorBASIC  (grupo: Funcoes gerais)

WRAPPER (Basic Dignified do paleobasic): .NB_Uninstall
CHAMADA
    erro = .NB_Uninstall(liberarRam)

Desinstala o NestorBASIC: libera o usr() e, no DOS 2, desaloca todos os segmentos. Tambem para qualquer interrupcao do usuario, efeito PSG ou musica em execucao. Sempre desinstale antes de voltar ao DOS - senao os segmentos alocados ficam presos ate reiniciar o computador.

ENTRADA
- p(0) = 0 para NAO liberar a area de RAM principal do BASIC reservada pela rotina de salto (~500 bytes) - um CLEAR feito depois da instalacao continua valido; <>0 pra liberar (faz um CLEAR automatico, FRE(0) volta ao valor de antes de instalar - as variaveis sao reinicializadas)

Nunca retorna erro.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-1", "nb-informacoes-gerais-e-sobre-um-segmento,nb_getinfo", "NestorBASIC: Funcoes gerais", "USR(1)", "Informacoes gerais e sobre um segmento: Devolve informacoes gerais sobre o NestorBASIC...",
        R"MAN(FUNCAO 1 do NestorBASIC  (grupo: Funcoes gerais)

WRAPPER (Basic Dignified do paleobasic): .NB_GetInfo
CHAMADA
    numSegmentos, erro = .NB_GetInfo(segmento)

Devolve informacoes gerais sobre o NestorBASIC instalado e sobre um segmento especifico.

ENTRADA
- p(0) = segmento logico a investigar

SAIDA
- p(0) = numero de segmentos de RAM disponiveis
- p(1) = versao principal do NestorBASIC
- p(2) = versao secundaria (BCD, mostrar em hexadecimal)
- p(3) = versao principal do MSX-DOS
- p(4) = versao secundaria do MSX-DOS (BCD, mostrar em hexadecimal)
- p(5) = tamanho ocupado na RAM principal pela rotina de salto do NestorBASIC
- p(6) = tamanho da VRAM em K (64 ou 128)
- p(7) = endereco inicial da area livre no segmento 0 (maximo &H3DA8; ha um bug documentado no manual original - o valor volta pertencendo a pagina 1, ex. &H7C60, e precisa ser ajustado pro intervalo &H0000-&H4000 antes de usar)
- p(8) = numero da ultima funcao chamada (esta funcao 1 nao conta)
- p(9) = numero de arquivos abertos no momento
- p(10) = numero maximo de arquivos simultaneos (so vale sob DOS 1)
- p(11) = slot do segmento pedido em p(0) (255 se nao existir ou for VRAM)
- p(12) = segmento fisico do segmento pedido em p(0)
- f$(0) = caminho completo do arquivo NBASIC.BIN

ERROS
-1 = o segmento pedido em p(0) nao existe, e VRAM ou e o 255 (mesmo assim, p(0) a p(10) continuam validos).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-2", "nb-ler-um-byte-de-um-segmento,nb_readbyte", "NestorBASIC: Acesso a segmentos (RAM)", "USR(2)", "Ler um byte de um segmento: - p(0) = segmento",
        R"MAN(FUNCAO 2 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_ReadByte
CHAMADA
    byte, erro = .NB_ReadByte(segmento, endereco)

ENTRADA
- p(0) = segmento
- p(1) = endereco

SAIDA
- p(2) = byte lido

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-3", "nb-ler-um-byte-de-um-segmento-com-autoincremento,nb_readbyteinc", "NestorBASIC: Acesso a segmentos (RAM)", "USR(3)", "Ler um byte de um segmento (com autoincremento): Igual a funcao 2, mas devolve o endereco...",
        R"MAN(FUNCAO 3 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_ReadByteInc
CHAMADA
    byte, novoEndereco, erro = .NB_ReadByteInc(segmento, endereco)

Igual a funcao 2, mas devolve o endereco seguinte - util pra ler sequencialmente em loop.

ENTRADA
- p(0) = segmento
- p(1) = endereco

SAIDA
- p(2) = byte lido
- p(1) = p(1)+1

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-4", "nb-ler-um-inteiro-de-um-segmento,nb_readint", "NestorBASIC: Acesso a segmentos (RAM)", "USR(4)", "Ler um inteiro de um segmento: Le 2 bytes (byte baixo no endereco, byte alto no...",
        R"MAN(FUNCAO 4 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_ReadInt
CHAMADA
    valor, erro = .NB_ReadInt(segmento, endereco)

Le 2 bytes (byte baixo no endereco, byte alto no endereco+1).

ENTRADA
- p(0) = segmento
- p(1) = endereco

SAIDA
- p(2) = inteiro lido

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-5", "nb-ler-um-inteiro-de-um-segmento-com-autoincremento,nb_readintinc", "NestorBASIC: Acesso a segmentos (RAM)", "USR(5)", "Ler um inteiro de um segmento (com autoincremento): - p(0) = segmento",
        R"MAN(FUNCAO 5 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_ReadIntInc
CHAMADA
    valor, novoEndereco, erro = .NB_ReadIntInc(segmento, endereco)

ENTRADA
- p(0) = segmento
- p(1) = endereco

SAIDA
- p(2) = inteiro lido
- p(1) = p(1)+2

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-6", "nb-escrever-um-byte-num-segmento,nb_writebyte", "NestorBASIC: Acesso a segmentos (RAM)", "USR(6)", "Escrever um byte num segmento: - p(0) = segmento",
        R"MAN(FUNCAO 6 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_WriteByte
CHAMADA
    erro = .NB_WriteByte(segmento, endereco, valor)

ENTRADA
- p(0) = segmento
- p(1) = endereco
- p(2) = byte a escrever

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-7", "nb-escrever-um-byte-num-segmento-com-autoincremento,nb_writebyteinc", "NestorBASIC: Acesso a segmentos (RAM)", "USR(7)", "Escrever um byte num segmento (com autoincremento): - p(0) = segmento",
        R"MAN(FUNCAO 7 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_WriteByteInc
CHAMADA
    novoEndereco, erro = .NB_WriteByteInc(segmento, endereco, valor)

ENTRADA
- p(0) = segmento
- p(1) = endereco
- p(2) = byte a escrever

SAIDA
- p(1) = p(1)+1

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-8", "nb-escrever-um-inteiro-num-segmento,nb_writeint", "NestorBASIC: Acesso a segmentos (RAM)", "USR(8)", "Escrever um inteiro num segmento: - p(0) = segmento",
        R"MAN(FUNCAO 8 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_WriteInt
CHAMADA
    erro = .NB_WriteInt(segmento, endereco, valor)

ENTRADA
- p(0) = segmento
- p(1) = endereco
- p(2) = inteiro a escrever

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-9", "nb-escrever-um-inteiro-num-segmento-com-autoincreme,nb_writeintinc", "NestorBASIC: Acesso a segmentos (RAM)", "USR(9)", "Escrever um inteiro num segmento (com autoincremento): - p(0) = segmento",
        R"MAN(FUNCAO 9 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_WriteIntInc
CHAMADA
    novoEndereco, erro = .NB_WriteIntInc(segmento, endereco, valor)

ENTRADA
- p(0) = segmento
- p(1) = endereco
- p(2) = inteiro a escrever

SAIDA
- p(1) = p(1)+2

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-10", "nb-copiar-um-bloco-entre-segmentos,nb_copysegtoseg", "NestorBASIC: Acesso a segmentos (RAM)", "USR(10)", "Copiar um bloco entre segmentos: p(3)+p(4) precisa ficar abaixo de &H4000, senao o...",
        R"MAN(FUNCAO 10 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_CopySegToSeg
CHAMADA
    novoEndOrigem, novoEndDestino, erro = .NB_CopySegToSeg(segOrigem, endOrigem, segDestino, endDestino, tamanho, incOrigem, incDestino)

p(3)+p(4) precisa ficar abaixo de &H4000, senao o resultado e imprevisivel.

ENTRADA
- p(0) = segmento de origem
- p(1) = endereco de origem
- p(2) = segmento de destino
- p(3) = endereco de destino
- p(4) = tamanho
- p(5) = <>0 autoincrementa p(1)
- p(6) = <>0 autoincrementa p(3)

SAIDA
- p(1) = p(1)+p(4) se p(5)<>0
- p(3) = p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-11", "nb-preencher-uma-area-de-ram-com-um-byte,nb_fillram", "NestorBASIC: Acesso a segmentos (RAM)", "USR(11)", "Preencher uma area de RAM com um byte: p(1)+p(3) precisa ficar abaixo de &H4000.",
        R"MAN(FUNCAO 11 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_FillRam
CHAMADA
    erro = .NB_FillRam(segmento, endereco, byte, tamanho)

p(1)+p(3) precisa ficar abaixo de &H4000.

ENTRADA
- p(0) = segmento
- p(1) = endereco inicial
- p(2) = byte
- p(3) = tamanho da area

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-12", "nb-preencher-uma-area-de-ram-com-um-byte-com-autoin,nb_fillraminc", "NestorBASIC: Acesso a segmentos (RAM)", "USR(12)", "Preencher uma area de RAM com um byte (com autoincremento): - p(0) = segmento",
        R"MAN(FUNCAO 12 do NestorBASIC  (grupo: Acesso a segmentos (RAM))

WRAPPER (Basic Dignified do paleobasic): .NB_FillRamInc
CHAMADA
    novoEndereco, erro = .NB_FillRamInc(segmento, endereco, byte, tamanho)

ENTRADA
- p(0) = segmento
- p(1) = endereco inicial
- p(2) = byte
- p(3) = tamanho da area

SAIDA
- p(1) = p(1)+p(3)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-13", "nb-ler-um-byte-da-vram,nb_vreadbyte", "NestorBASIC: Acesso a VRAM", "USR(13)", "Ler um byte da VRAM: - p(0) = bloco",
        R"MAN(FUNCAO 13 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VReadByte
CHAMADA
    byte, erro = .NB_VReadByte(bloco, endereco)

ENTRADA
- p(0) = bloco
- p(1) = endereco

SAIDA
- p(2) = byte lido

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-14", "nb-ler-um-byte-da-vram-com-autoincremento,nb_vreadbyteinc", "NestorBASIC: Acesso a VRAM", "USR(14)", "Ler um byte da VRAM (com autoincremento): - p(0) = bloco",
        R"MAN(FUNCAO 14 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VReadByteInc
CHAMADA
    byte, novoBloco, novoEndereco, erro = .NB_VReadByteInc(bloco, endereco)

ENTRADA
- p(0) = bloco
- p(1) = endereco

SAIDA
- p(2) = byte lido
- p(0):p(1) = p(0):p(1)+1 (pode virar o bloco)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-15", "nb-ler-um-inteiro-da-vram,nb_vreadint", "NestorBASIC: Acesso a VRAM", "USR(15)", "Ler um inteiro da VRAM: - p(0) = bloco",
        R"MAN(FUNCAO 15 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VReadInt
CHAMADA
    valor, erro = .NB_VReadInt(bloco, endereco)

ENTRADA
- p(0) = bloco
- p(1) = endereco

SAIDA
- p(2) = inteiro lido

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-16", "nb-ler-um-inteiro-da-vram-com-autoincremento,nb_vreadintinc", "NestorBASIC: Acesso a VRAM", "USR(16)", "Ler um inteiro da VRAM (com autoincremento): - p(0) = bloco",
        R"MAN(FUNCAO 16 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VReadIntInc
CHAMADA
    valor, novoBloco, novoEndereco, erro = .NB_VReadIntInc(bloco, endereco)

ENTRADA
- p(0) = bloco
- p(1) = endereco

SAIDA
- p(2) = inteiro lido
- p(0):p(1) = p(0):p(1)+2

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-17", "nb-escrever-um-byte-na-vram,nb_vwritebyte", "NestorBASIC: Acesso a VRAM", "USR(17)", "Escrever um byte na VRAM: - p(0) = bloco",
        R"MAN(FUNCAO 17 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VWriteByte
CHAMADA
    erro = .NB_VWriteByte(bloco, endereco, valor)

ENTRADA
- p(0) = bloco
- p(1) = endereco
- p(2) = byte a escrever

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-18", "nb-escrever-um-byte-na-vram-com-autoincremento,nb_vwritebyteinc", "NestorBASIC: Acesso a VRAM", "USR(18)", "Escrever um byte na VRAM (com autoincremento): - p(0) = bloco",
        R"MAN(FUNCAO 18 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VWriteByteInc
CHAMADA
    novoBloco, novoEndereco, erro = .NB_VWriteByteInc(bloco, endereco, valor)

ENTRADA
- p(0) = bloco
- p(1) = endereco
- p(2) = byte a escrever

SAIDA
- p(0):p(1) = p(0):p(1)+1

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-19", "nb-escrever-um-inteiro-na-vram,nb_vwriteint", "NestorBASIC: Acesso a VRAM", "USR(19)", "Escrever um inteiro na VRAM: - p(0) = bloco",
        R"MAN(FUNCAO 19 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VWriteInt
CHAMADA
    erro = .NB_VWriteInt(bloco, endereco, valor)

ENTRADA
- p(0) = bloco
- p(1) = endereco
- p(2) = inteiro a escrever

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-20", "nb-escrever-um-inteiro-na-vram-com-autoincremento,nb_vwriteintinc", "NestorBASIC: Acesso a VRAM", "USR(20)", "Escrever um inteiro na VRAM (com autoincremento): - p(0) = bloco",
        R"MAN(FUNCAO 20 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_VWriteIntInc
CHAMADA
    novoBloco, novoEndereco, erro = .NB_VWriteIntInc(bloco, endereco, valor)

ENTRADA
- p(0) = bloco
- p(1) = endereco
- p(2) = inteiro a escrever

SAIDA
- p(0):p(1) = p(0):p(1)+2

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-21", "nb-copiar-um-bloco-da-vram-pra-ram,nb_copyvramtoram", "NestorBASIC: Acesso a VRAM", "USR(21)", "Copiar um bloco da VRAM pra RAM: p(3)+p(4) precisa ficar abaixo de &H4000.",
        R"MAN(FUNCAO 21 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_CopyVramToRam
CHAMADA
    novoBlocoOrigem, novoEndOrigem, novoEndDestino, erro = .NB_CopyVramToRam(blocoOrigem, endOrigem, segDestino, endDestino, tamanho, incOrigem, incDestino)

p(3)+p(4) precisa ficar abaixo de &H4000.

ENTRADA
- p(0) = bloco de origem (VRAM)
- p(1) = endereco de origem (VRAM)
- p(2) = segmento de destino
- p(3) = endereco de destino
- p(4) = tamanho
- p(5) = <>0 autoincrementa origem
- p(6) = <>0 autoincrementa destino

SAIDA
- p(1) = p(1)+p(4) se p(5)<>0
- p(2):p(3) = p(2):p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-22", "nb-copiar-um-bloco-da-ram-pra-vram,nb_copyramtovram", "NestorBASIC: Acesso a VRAM", "USR(22)", "Copiar um bloco da RAM pra VRAM: - p(0) = segmento de origem",
        R"MAN(FUNCAO 22 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_CopyRamToVram
CHAMADA
    novoEndOrigem, novoBlocoDestino, novoEndDestino, erro = .NB_CopyRamToVram(segOrigem, endOrigem, blocoDestino, endDestino, tamanho, incOrigem, incDestino)

ENTRADA
- p(0) = segmento de origem
- p(1) = endereco de origem
- p(2) = bloco de destino (VRAM)
- p(3) = endereco de destino (VRAM)
- p(4) = tamanho
- p(5) = <>0 autoincrementa origem
- p(6) = <>0 autoincrementa destino

SAIDA
- p(1) = p(1)+p(4) se p(5)<>0
- p(2):p(3) = p(2):p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-23", "nb-copiar-um-bloco-entre-duas-areas-da-vram,nb_copyvramtovram", "NestorBASIC: Acesso a VRAM", "USR(23)", "Copiar um bloco entre duas areas da VRAM: Tamanho maximo &H4000 bytes por chamada - pra...",
        R"MAN(FUNCAO 23 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_CopyVramToVram
CHAMADA
    novoBlocoOrigem, novoEndOrigem, novoBlocoDestino, novoEndDestino, erro = .NB_CopyVramToVram(blocoOrigem, endOrigem, blocoDestino, endDestino, tamanho, incOrigem, incDestino)

Tamanho maximo &H4000 bytes por chamada - pra blocos maiores (ate 64K), repita a chamada incrementando os enderecos a cada volta.

ENTRADA
- p(0) = bloco de origem
- p(1) = endereco de origem
- p(2) = bloco de destino
- p(3) = endereco de destino
- p(4) = tamanho (maximo &H4000)
- p(5) = <>0 autoincrementa origem
- p(6) = <>0 autoincrementa destino

SAIDA
- p(0):p(1) = p(0):p(1)+p(4) se p(5)<>0
- p(2):p(3) = p(2):p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-24", "nb-preencher-uma-area-de-vram-com-um-byte,nb_fillvram", "NestorBASIC: Acesso a VRAM", "USR(24)", "Preencher uma area de VRAM com um byte: Maximo 16K por chamada.",
        R"MAN(FUNCAO 24 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_FillVram
CHAMADA
    erro = .NB_FillVram(bloco, endereco, byte, tamanho)

Maximo 16K por chamada.

ENTRADA
- p(0) = bloco
- p(1) = endereco inicial
- p(2) = byte
- p(3) = tamanho da area (maximo 16K)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-25", "nb-preencher-uma-area-de-vram-com-um-byte-com-autoi,nb_fillvraminc", "NestorBASIC: Acesso a VRAM", "USR(25)", "Preencher uma area de VRAM com um byte (com autoincremento): Maximo 16K por chamada.",
        R"MAN(FUNCAO 25 do NestorBASIC  (grupo: Acesso a VRAM)

WRAPPER (Basic Dignified do paleobasic): .NB_FillVramInc
CHAMADA
    novoBloco, novoEndereco, erro = .NB_FillVramInc(bloco, endereco, byte, tamanho)

Maximo 16K por chamada.

ENTRADA
- p(0) = bloco
- p(1) = endereco inicial
- p(2) = byte
- p(3) = tamanho da area (maximo 16K)

SAIDA
- p(0):p(1) = p(0):p(1)+p(3)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-26", "nb-procurar-arquivos-s4,nb_findfile", "NestorBASIC: Acesso a disco", "USR(26)", "Procurar arquivos (S4): buscarProximo=0 procura o primeiro arquivo que bate com a mascara...",
        R"MAN(FUNCAO 26 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_FindFile
CHAMADA
    nomeArquivo$, erro = .NB_FindFile(mascara$, buscarProximo, atributosBusca)

buscarProximo=0 procura o primeiro arquivo que bate com a mascara (vazia = *.*); deixe 1 pras buscas seguintes (a propria funcao mantem isso em p(0)).

ENTRADA
- f$(1) = mascara de busca (pode ter unidade e, no DOS 2, caminho)
- p(0) = 0 primeira busca / 1 proxima busca
- p(1) = atributos de busca: 2*H+4*S+8*V+16*D (incluir Ocultos/Sistema/so Volume/Diretorios)

SAIDA
- f$(0) = nome do arquivo encontrado
- p(0) = 1
- p(1) = atributos do arquivo (sempre 0 no DOS 1): R+2*H+4*S+8*V+16*D+32*A
- p(2) a p(6) = hora, minuto, dia, mes, ano da ultima modificacao
- p(7) = primeiro cluster (2 a 4095)
- p(8) = unidade logica (0=A:,...,7=H:)
- p(9)/p(10) = tamanho do arquivo, parte baixa/alta (p(9)+65536*p(10))
- p(11) = contador de resultados (0 se nao achou na primeira busca; incrementa a cada busca seguinte)

Erro quando nao ha mais arquivos batendo com a mascara (erro de arquivo nao encontrado).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-27", "nb-renomear-um-arquivo-s4,nb_renamefile", "NestorBASIC: Acesso a disco", "USR(27)", "Renomear um arquivo (S4): No DOS 1 aceita curingas e renomeia varios de uma vez; no DOS 2...",
        R"MAN(FUNCAO 27 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_RenameFile
CHAMADA
    erro = .NB_RenameFile(nome$, novoNome$)

No DOS 1 aceita curingas e renomeia varios de uma vez; no DOS 2 so um arquivo por chamada.

ENTRADA
- f$(0) = nome atual
- f$(1) = nome novo (so o nome, sem unidade/caminho)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-28", "nb-apagar-um-arquivo-s4,nb_deletefile", "NestorBASIC: Acesso a disco", "USR(28)", "Apagar um arquivo (S4): No DOS 1 aceita curingas (apaga varios de uma vez); no DOS 2 so...",
        R"MAN(FUNCAO 28 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_DeleteFile
CHAMADA
    erro = .NB_DeleteFile(nome$)

No DOS 1 aceita curingas (apaga varios de uma vez); no DOS 2 so um arquivo, e da erro se estiver aberto.

ENTRADA
- f$(0) = nome do arquivo (pode ter unidade e, no DOS 2, caminho)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-29", "nb-mover-um-arquivo-dos-2,nb_movefile", "NestorBASIC: Acesso a disco", "USR(29)", "Mover um arquivo (DOS 2): So existe no DOS 2 (no DOS 1 sempre da erro 1). Move uma pasta...",
        R"MAN(FUNCAO 29 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_MoveFile
CHAMADA
    erro = .NB_MoveFile(nome$, novoLocal$)

So existe no DOS 2 (no DOS 1 sempre da erro 1). Move uma pasta inteira move tambem seu conteudo.

ENTRADA
- f$(0) = nome/caminho atual
- f$(1) = novo local (so o caminho, sem nome de arquivo nem letra de unidade - so pode mover dentro da mesma unidade)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-30", "nb-criar-um-arquivo-ou-diretorio,nb_createfile", "NestorBASIC: Acesso a disco", "USR(30)", "Criar um arquivo ou diretorio: O arquivo criado fica com 0 bytes e fechado (abra depois...",
        R"MAN(FUNCAO 30 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_CreateFile
CHAMADA
    erro = .NB_CreateFile(nome$, atributos)

O arquivo criado fica com 0 bytes e fechado (abra depois com .NB_OpenFile). No DOS 1 so cria arquivos e p(0) e ignorado; no DOS 2 sempre cria com o atributo Archive alem dos pedidos.

ENTRADA
- f$(0) = nome do arquivo/subdiretorio (pode ter unidade e caminho)
- p(0) = atributos de criacao (ignorado no DOS 1): R+2*H+4*S (arquivo) ou 2*H+16 (diretorio)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-31", "nb-abrir-um-arquivo,nb_openfile", "NestorBASIC: Acesso a disco", "USR(31)", "Abrir um arquivo: - f$(0) = nome do arquivo (pode ter unidade e, no DOS 2, caminho)",
        R"MAN(FUNCAO 31 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_OpenFile
CHAMADA
    numeroArquivo, erro = .NB_OpenFile(nome$)

ENTRADA
- f$(0) = nome do arquivo (pode ter unidade e, no DOS 2, caminho)

SAIDA
- p(0) = numero atribuido ao arquivo (use nas demais funcoes de arquivo)

No DOS 1, erro 3 = ja tem arquivos demais abertos (o maximo sai de .NB_GetInfo).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-32", "nb-fechar-um-arquivo,nb_closefile", "NestorBASIC: Acesso a disco", "USR(32)", "Fechar um arquivo: Sempre feche um arquivo que nao vai mais usar - senao dados escritos...",
        R"MAN(FUNCAO 32 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_CloseFile
CHAMADA
    erro = .NB_CloseFile(numeroArquivo)

Sempre feche um arquivo que nao vai mais usar - senao dados escritos podem ficar presos nos buffers internos do DOS, e a entrada de diretorio pode nao ser atualizada.

ENTRADA
- p(0) = numero do arquivo

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-33", "nb-ler-de-um-arquivo-s4,nb_readfile", "NestorBASIC: Acesso a disco", "USR(33)", "Ler de um arquivo (S4): Le a partir da posicao atual do ponteiro do arquivo (avanca...",
        R"MAN(FUNCAO 33 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_ReadFile
CHAMADA
    bytesLidos, novoEnderecoDestino, erro = .NB_ReadFile(numeroArquivo, segDestino, enderecoDestino, tamanho, incrementar)

Le a partir da posicao atual do ponteiro do arquivo (avanca sozinho depois). Pra ler ate o fim (ou o arquivo inteiro, se for menor que 16K), peca 16K (tamanho=&H4000) e ignore erro 1/199 (so significam fim de arquivo).

ENTRADA
- p(0) = numero do arquivo
- p(2) = segmento de destino
- p(3) = endereco de destino
- p(4) = numero de bytes a ler
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = bytes realmente lidos
- p(3) = p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-34", "nb-ler-de-um-arquivo-pra-vram-s4,nb_readfiletovram", "NestorBASIC: Acesso a disco", "USR(34)", "Ler de um arquivo pra VRAM (S4): Maximo &H4000 bytes por chamada.",
        R"MAN(FUNCAO 34 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_ReadFileToVram
CHAMADA
    bytesLidos, novoBlocoDestino, novoEnderecoDestino, erro = .NB_ReadFileToVram(numeroArquivo, blocoDestino, enderecoDestino, tamanho, incrementar)

Maximo &H4000 bytes por chamada.

ENTRADA
- p(0) = numero do arquivo
- p(2) = bloco de destino (VRAM)
- p(3) = endereco de destino
- p(4) = numero de bytes a ler (maximo &H4000)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = bytes realmente lidos
- p(2):p(3) = p(2):p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-35", "nb-ler-setores-do-disco-s4,nb_readsectors", "NestorBASIC: Acesso a disco", "USR(35)", "Ler setores do disco (S4): Leitura bruta, direto por setor fisico (nao passa por...",
        R"MAN(FUNCAO 35 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_ReadSectors
CHAMADA
    bytesLidos, novoEnderecoDestino, erro = .NB_ReadSectors(unidade, primeiroSetor, segDestino, enderecoDestino, numSetores, incrementar)

Leitura bruta, direto por setor fisico (nao passa por arquivo). Sem leitura parcial: erro = p(7) volta 0; sem erro = p(7) = p(4)*512. Maximo 32 setores (16K) por chamada.

ENTRADA
- p(0) = unidade (0=A:,...,7=H:)
- p(1) = primeiro setor
- p(2) = segmento de destino
- p(3) = endereco de destino
- p(4) = numero de setores (maximo 32)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = p(4)*512 (0 se deu erro)
- p(3) = p(3)+p(4)*512 se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-36", "nb-ler-setores-do-disco-pra-vram-s4,nb_readsectorstovram", "NestorBASIC: Acesso a disco", "USR(36)", "Ler setores do disco pra VRAM (S4): Maximo 32 setores (16K) por chamada.",
        R"MAN(FUNCAO 36 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_ReadSectorsToVram
CHAMADA
    bytesLidos, novoBlocoDestino, novoEnderecoDestino, erro = .NB_ReadSectorsToVram(unidade, primeiroSetor, blocoDestino, enderecoDestino, numSetores, incrementar)

Maximo 32 setores (16K) por chamada.

ENTRADA
- p(0) = unidade
- p(1) = primeiro setor
- p(2) = bloco de destino (VRAM)
- p(3) = endereco de destino
- p(4) = numero de setores (maximo 32)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = p(4)*512 (0 se deu erro)
- p(2):p(3) = p(2):p(3)+p(4)*512 se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-37", "nb-escrever-num-arquivo-s4,nb_writefile", "NestorBASIC: Acesso a disco", "USR(37)", "Escrever num arquivo (S4): Escreve a partir da posicao atual do ponteiro (avanca sozinho...",
        R"MAN(FUNCAO 37 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_WriteFile
CHAMADA
    bytesEscritos, novoEnderecoOrigem, erro = .NB_WriteFile(numeroArquivo, segOrigem, enderecoOrigem, tamanho, incrementar)

Escreve a partir da posicao atual do ponteiro (avanca sozinho depois).

ENTRADA
- p(0) = numero do arquivo
- p(2) = segmento de origem
- p(3) = endereco de origem
- p(4) = numero de bytes a escrever
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = bytes realmente escritos
- p(3) = p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-38", "nb-escrever-num-arquivo-a-partir-da-vram-s4,nb_writefilefromvram", "NestorBASIC: Acesso a disco", "USR(38)", "Escrever num arquivo a partir da VRAM (S4): Maximo &H4000 bytes por chamada.",
        R"MAN(FUNCAO 38 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_WriteFileFromVram
CHAMADA
    bytesEscritos, novoBlocoOrigem, novoEnderecoOrigem, erro = .NB_WriteFileFromVram(numeroArquivo, blocoOrigem, enderecoOrigem, tamanho, incrementar)

Maximo &H4000 bytes por chamada.

ENTRADA
- p(0) = numero do arquivo
- p(2) = bloco de origem (VRAM)
- p(3) = endereco de origem
- p(4) = numero de bytes (maximo &H4000)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = bytes realmente escritos
- p(2):p(3) = p(2):p(3)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-39", "nb-escrever-setores-do-disco-s4,nb_writesectors", "NestorBASIC: Acesso a disco", "USR(39)", "Escrever setores do disco (S4): Sem escrita parcial. Maximo 32 setores (16K) por chamada.",
        R"MAN(FUNCAO 39 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_WriteSectors
CHAMADA
    bytesEscritos, novoEnderecoOrigem, erro = .NB_WriteSectors(unidade, primeiroSetor, segOrigem, enderecoOrigem, numSetores, incrementar)

Sem escrita parcial. Maximo 32 setores (16K) por chamada.

ENTRADA
- p(0) = unidade
- p(1) = primeiro setor
- p(2) = segmento de origem
- p(3) = endereco de origem
- p(4) = numero de setores (maximo 32)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = p(4)*512 (0 se deu erro)
- p(3) = p(3)+p(4)*512 se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-40", "nb-escrever-setores-do-disco-a-partir-da-vram-s4,nb_writesectorsfromvram", "NestorBASIC: Acesso a disco", "USR(40)", "Escrever setores do disco a partir da VRAM (S4): Maximo 32 setores (16K) por chamada.",
        R"MAN(FUNCAO 40 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_WriteSectorsFromVram
CHAMADA
    bytesEscritos, novoBlocoOrigem, novoEnderecoOrigem, erro = .NB_WriteSectorsFromVram(unidade, primeiroSetor, blocoOrigem, enderecoOrigem, numSetores, incrementar)

Maximo 32 setores (16K) por chamada.

ENTRADA
- p(0) = unidade
- p(1) = primeiro setor
- p(2) = bloco de origem (VRAM)
- p(3) = endereco de origem
- p(4) = numero de setores (maximo 32)
- p(6) = <>0 avanca p(3)

SAIDA
- p(7) = p(4)*512 (0 se deu erro)
- p(2):p(3) = p(2):p(3)+p(4)*512 se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-41", "nb-preencher-um-arquivo-com-um-byte-s4,nb_fillfile", "NestorBASIC: Acesso a disco", "USR(41)", "Preencher um arquivo com um byte (S4): Maximo &H4000 por chamada.",
        R"MAN(FUNCAO 41 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_FillFile
CHAMADA
    bytesEscritos, erro = .NB_FillFile(numeroArquivo, byte, tamanho)

Maximo &H4000 por chamada.

ENTRADA
- p(0) = numero do arquivo
- p(1) = byte
- p(4) = tamanho (maximo &H4000)

SAIDA
- p(7) = bytes realmente escritos

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-42", "nb-mover-o-ponteiro-de-um-arquivo,nb_seekfile", "NestorBASIC: Acesso a disco", "USR(42)", "Mover o ponteiro de um arquivo: Pra saber a posicao atual sem mover: metodo=1,...",
        R"MAN(FUNCAO 42 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_SeekFile
CHAMADA
    novaPosBaixa, novaPosAlta, erro = .NB_SeekFile(numeroArquivo, metodo, offsetBaixo, offsetAlto)

Pra saber a posicao atual sem mover: metodo=1, offsetBaixo=0, offsetAlto=0. Pro tamanho do arquivo: metodo=2, offsetBaixo=0, offsetAlto=0.

ENTRADA
- p(0) = numero do arquivo
- p(1) = metodo: 0=a partir do inicio, 1=a partir da posicao atual, 2=a partir do fim
- p(2)/p(3) = deslocamento com sinal, parte baixa/alta (16 bits cada)

SAIDA
- p(4)/p(5) = nova posicao do ponteiro, parte baixa/alta

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-43", "nb-obter-a-unidade-padrao-e-as-disponiveis,nb_getdrives", "NestorBASIC: Acesso a disco", "USR(43)", "Obter a unidade padrao e as disponiveis: - p(0) = unidade padrao (0=A:,...,7=H:)",
        R"MAN(FUNCAO 43 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_GetDrives
CHAMADA
    unidadePadrao, vetorUnidades, erro = .NB_GetDrives()

SAIDA
- p(0) = unidade padrao (0=A:,...,7=H:)
- p(1) = vetor de bits das unidades disponiveis (bit 0 = A:, bit mais alto = H:)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-44", "nb-definir-a-unidade-padrao,nb_setdrive", "NestorBASIC: Acesso a disco", "USR(44)", "Definir a unidade padrao: - p(0) = unidade a definir (0=A:,...,7=H:)",
        R"MAN(FUNCAO 44 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_SetDrive
CHAMADA
    erro = .NB_SetDrive(unidade)

ENTRADA
- p(0) = unidade a definir (0=A:,...,7=H:)

Erro 62 = unidade invalida (nao existe ou maior que 7).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-45", "nb-obter-informacoes-de-espaco-em-disco,nb_getdiskspace", "NestorBASIC: Acesso a disco", "USR(45)", "Obter informacoes de espaco em disco: Espaco livre em bytes = p(1)*p(3)*512; em K =...",
        R"MAN(FUNCAO 45 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_GetDiskSpace
CHAMADA
    setoresPorCluster, totalClusters, clustersLivres, erro = .NB_GetDiskSpace(unidade)

Espaco livre em bytes = p(1)*p(3)*512; em K = p(1)*p(3)/2 (e o mesmo trocando p(3) por p(2) pro total).

ENTRADA
- p(0) = unidade (0=padrao,1=A:,...,8=H: - reparar que aqui e diferente!)

SAIDA
- p(1) = setores por cluster
- p(2) = total de clusters (maximo 4096)
- p(3) = clusters livres

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-46", "nb-obter-o-diretorio-padrao-dos-2,nb_getdir", "NestorBASIC: Acesso a disco", "USR(46)", "Obter o diretorio padrao (DOS 2): So existe no DOS 2 (no DOS 1 sempre da erro 1). String...",
        R"MAN(FUNCAO 46 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_GetDir
CHAMADA
    diretorio$, erro = .NB_GetDir(unidade)

So existe no DOS 2 (no DOS 1 sempre da erro 1). String devolvida nao tem letra de unidade nem barra no inicio/fim; raiz = string vazia.

ENTRADA
- p(0) = unidade (0=padrao,1=A:,...,8=H: - reparar que aqui e diferente!)

SAIDA
- f$(0) = diretorio padrao

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-47", "nb-definir-o-diretorio-padrao-dos-2,nb_setdir", "NestorBASIC: Acesso a disco", "USR(47)", "Definir o diretorio padrao (DOS 2): So existe no DOS 2. Nao muda a unidade padrao (use...",
        R"MAN(FUNCAO 47 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_SetDir
CHAMADA
    erro = .NB_SetDir(caminho$)

So existe no DOS 2. Nao muda a unidade padrao (use .NB_SetDrive pra isso).

ENTRADA
- f$(0) = unidade (opcional) + diretorio

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-48", "nb-obter-o-tamanho-do-ram-disk-dos-2,nb_getramdisksize", "NestorBASIC: Acesso a disco", "USR(48)", "Obter o tamanho do RAM disk (DOS 2): So existe no DOS 2. Tamanho em K = resultado*16; 0 =...",
        R"MAN(FUNCAO 48 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_GetRamDiskSize
CHAMADA
    tamanho16k, erro = .NB_GetRamDiskSize()

So existe no DOS 2. Tamanho em K = resultado*16; 0 = RAM disk nao existe.

SAIDA
- p(0) = tamanho em segmentos de 16K

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-49", "nb-criar-o-ram-disk-dos-2,nb_createramdisk", "NestorBASIC: Acesso a disco", "USR(49)", "Criar o RAM disk (DOS 2): So existe no DOS 2. Por padrao o NestorBASIC ja pegou todos os...",
        R"MAN(FUNCAO 49 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_CreateRamDisk
CHAMADA
    tamanhoCriado, erro = .NB_CreateRamDisk(tamanho16k)

So existe no DOS 2. Por padrao o NestorBASIC ja pegou todos os segmentos livres pra si ao instalar - use .NB_SetSegmentCount pra liberar segmentos antes de chamar esta funcao. Se nao houver o tamanho pedido mas der pra criar um menor, cria e nao da erro (so retorna o tamanho real criado); se nao houver segmento nenhum, da erro.

ENTRADA
- p(0) = tamanho pedido em segmentos de 16K

SAIDA
- p(0) = tamanho realmente criado

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-50", "nb-obter-os-atributos-de-um-arquivo-dos-2,nb_getattrbyhandle / .nb_getattrbyname", "NestorBASIC: Acesso a disco", "USR(50)", "Obter os atributos de um arquivo (DOS 2): So existe no DOS 2. No manual original e uma...",
        R"MAN(FUNCAO 50 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_GetAttrByHandle / .NB_GetAttrByName
CHAMADA
    atributos, erro = .NB_GetAttrByHandle(numeroArquivo)
    atributos, erro = .NB_GetAttrByName(nome$)

So existe no DOS 2. No manual original e uma unica funcao (p(0)=255 + f$(0) = nome pra consultar por nome, ao inves de numero de arquivo ja aberto) - aqui virou dois wrappers separados pra nao precisar lembrar do valor magico 255.

SAIDA
- p(1) = atributos: R+2*H+4*S+8*V+16*D+32*A

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-51", "nb-definir-os-atributos-de-um-arquivo-dos-2,nb_setattrbyhandle / .nb_setattrbyname", "NestorBASIC: Acesso a disco", "USR(51)", "Definir os atributos de um arquivo (DOS 2): So existe no DOS 2. So da pra mudar...",
        R"MAN(FUNCAO 51 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_SetAttrByHandle / .NB_SetAttrByName
CHAMADA
    atributosFinal, erro = .NB_SetAttrByHandle(numeroArquivo, atributos)
    atributosFinal, erro = .NB_SetAttrByName(nome$, atributos)

So existe no DOS 2. So da pra mudar Sistema/Oculto/Somente leitura/Archive num arquivo (Oculto num diretorio); tentar mudar qualquer outro atributo da erro. Nao da pra mudar pelo nome um arquivo que ja esta aberto - use a versao por numero nesse caso.

SAIDA
- p(1) = atributos realmente definidos

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-52", "nb-interpretar-um-caminho-dos-2,nb_parsepath", "NestorBASIC: Acesso a disco", "USR(52)", "Interpretar um caminho (DOS 2): So existe no DOS 2. So trata a string (nao acessa o...",
        R"MAN(FUNCAO 52 do NestorBASIC  (grupo: Acesso a disco)

WRAPPER (Basic Dignified do paleobasic): .NB_ParsePath
CHAMADA
    ultimoItem$, unidadeLogica, erro = .NB_ParsePath(caminho$, ehVolume)

So existe no DOS 2. So trata a string (nao acessa o disco, nao muda unidade/diretorio padrao).

ENTRADA
- f$(0) = caminho a interpretar
- p(10) = <>0 se o caminho se refere a um rotulo de volume

SAIDA
- f$(1) = ultimo item do caminho
- p(8) = unidade logica (1=A:,...,8=H:)
- p(9) = posicao do ultimo item na string (0 se nao ha ultimo item)
- p(0) a p(7) = varios booleanos -1/0 (contem caracteres alem da unidade, tem diretorio, tem unidade, tem nome de arquivo, tem extensao, ultimo item ambiguo, ultimo item e ponto/ponto-ponto, ultimo item e ponto-ponto - todos 0 se p(10)<>0)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-53", "nb-comprimir-dados-graficos,nb_compressgraphics", "NestorBASIC: Compressao e descompressao grafica", "USR(53)", "Comprimir dados graficos: Comprime da VRAM pra um ou mais segmentos consecutivos de RAM -...",
        R"MAN(FUNCAO 53 do NestorBASIC  (grupo: Compressao e descompressao grafica)

WRAPPER (Basic Dignified do paleobasic): .NB_CompressGraphics
CHAMADA
    tamanhoComprimido, segmentosUsados, novoBlocoOrigem, novoEndOrigem, novoSegDestino, novoEndDestino, erro = .NB_CompressGraphics(blocoOrigem, endOrigem, segDestino, endDestino, tamanho, incOrigem, incDestino)

Comprime da VRAM pra um ou mais segmentos consecutivos de RAM - se encher um segmento, continua sozinha no proximo (por isso segmento/endereco de destino podem voltar diferentes do que foi passado). Nao suporta segmentos VRAM nem o 255 como destino.

ENTRADA
- p(0) = bloco de origem (VRAM)
- p(1) = endereco de origem
- p(2) = primeiro segmento de destino
- p(3) = endereco de destino
- p(4) = tamanho dos dados a comprimir
- p(5) = <>0 avanca origem
- p(6) = <>0 avanca destino

SAIDA
- p(7) = tamanho comprimido em RAM
- p(8) = numero de segmentos de RAM usados
- p(0):p(1) = p(0):p(1)+p(4) se p(5)<>0
- p(2):p(3) = endereco seguinte ao ultimo usado, se p(6)<>0

ERROS
5 = os segmentos acabaram antes de terminar de comprimir todos os dados pedidos.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-54", "nb-descomprimir-dados-graficos,nb_decompressgraphics", "NestorBASIC: Compressao e descompressao grafica", "USR(54)", "Descomprimir dados graficos: Descomprime de um ou mais segmentos consecutivos de RAM pra...",
        R"MAN(FUNCAO 54 do NestorBASIC  (grupo: Compressao e descompressao grafica)

WRAPPER (Basic Dignified do paleobasic): .NB_DecompressGraphics
CHAMADA
    tamanhoDescomprimido, segmentosUsados, novoBlocoDestino, novoEndDestino, novoSegOrigem, novoEndOrigem, erro = .NB_DecompressGraphics(blocoDestino, endDestino, segOrigem, endOrigem, incDestino, incOrigem)

Descomprime de um ou mais segmentos consecutivos de RAM pra VRAM (mesma logica de continuar sozinha no proximo segmento). Nao suporta segmentos VRAM nem o 255 como origem.

ENTRADA
- p(0) = bloco de destino (VRAM)
- p(1) = endereco de destino
- p(2) = primeiro segmento de origem
- p(3) = endereco de origem
- p(5) = <>0 avanca destino
- p(6) = <>0 avanca origem

SAIDA
- p(7) = tamanho descomprimido em VRAM
- p(8) = numero de segmentos de RAM usados
- p(0):p(1) = p(0):p(1)+p(7) se p(5)<>0
- p(2):p(3) = endereco seguinte ao ultimo usado, se p(6)<>0

ERROS
6 = dado invalido encontrado, ou os segmentos acabaram sem achar a marca de fim.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-55", "nb-executar-um-programa-basic-guardado-em-ram-s4,nb_runbasicprogram", "NestorBASIC: Execucao de programas BASIC guardados em RAM", "USR(55)", "Executar um programa BASIC guardado em RAM (S4): Se der certo, o programa atual NUNCA...",
        R"MAN(FUNCAO 55 do NestorBASIC  (grupo: Execucao de programas BASIC guardados em RAM)

WRAPPER (Basic Dignified do paleobasic): .NB_RunBasicProgram
CHAMADA
    erro = .NB_RunBasicProgram(segmento, endereco)

Se der certo, o programa atual NUNCA retorna daqui - o novo programa roda da linha 1, com as variaveis numericas do programa atual preservadas (strings tambem, desde que estejam na area de strings do BASIC - force isso com c$=c$+"" antes de chamar, se precisar). So retorna em caso de erro.

ENTRADA
- p(0) = segmento
- p(1) = endereco inicial

ERROS
-1 = segmento nao existe (ou e o 255).
-2 = RAM principal do BASIC insuficiente pro novo programa + variaveis existentes.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-56", "nb-ativar-um-programa-basic-guardado-em-ram-s4,nb_activatebasicprogram", "NestorBASIC: Execucao de programas BASIC guardados em RAM", "USR(56)", "Ativar um programa BASIC guardado em RAM (S4): Igual a funcao 55, mas ao inves de rodar...",
        R"MAN(FUNCAO 56 do NestorBASIC  (grupo: Execucao de programas BASIC guardados em RAM)

WRAPPER (Basic Dignified do paleobasic): .NB_ActivateBasicProgram
CHAMADA
    erro = .NB_ActivateBasicProgram(segmento, endereco)

Igual a funcao 55, mas ao inves de rodar da linha 1, larga no modo direto (programa carregado e pronto, mas parado). Se der certo tambem nunca retorna daqui; mesmos erros de 55.

ENTRADA
- p(0) = segmento
- p(1) = endereco inicial

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-57", "nb-salvar-o-programa-basic-ativo-com-cabecalho-espe,nb_savebasicprogram", "NestorBASIC: Execucao de programas BASIC guardados em RAM", "USR(57)", "Salvar o programa BASIC ativo com cabecalho especial (S4): Salva com o cabecalho que as...",
        R"MAN(FUNCAO 57 do NestorBASIC  (grupo: Execucao de programas BASIC guardados em RAM)

WRAPPER (Basic Dignified do paleobasic): .NB_SaveBasicProgram
CHAMADA
    arquivoAberto, erro = .NB_SaveBasicProgram(byteCabecalho, nome$)

Salva com o cabecalho que as funcoes 55/56 esperam pra depois carregar (com .NB_ReadFile, por exemplo) num segmento e ativar/executar.

ENTRADA
- p(0) = byte salvo na primeira posicao do cabecalho (o NestorBASIC nao usa, livre pro programa)
- f$(0) = caminho + nome do arquivo

SAIDA
- p(1) = -1 se nao deu erro; senao, o numero do arquivo (ainda aberto) onde ocorreu o erro de disco - feche com .NB_CloseFile antes de tentar de novo

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-58", "nb-executar-codigo-de-maquina-bios-sub-bios-ram-are,nb_execcode", "NestorBASIC: Funcoes diversas", "USR(58)", "Executar codigo de maquina (BIOS/SUB-BIOS/RAM/area de sistema): subBios=0 executa direto...",
        R"MAN(FUNCAO 58 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_ExecCode
CHAMADA
    af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, a, cy, z, erro = .NB_ExecCode(subBios, endereco, regAf, regBc, regDe, regHl, regIx, regIy, regAf2, regBc2, regDe2, regHl2)

subBios=0 executa direto (BIOS, RAM principal via BLOAD, ou area de sistema como o hook &HFFCA); subBios<>0 chama via EXTROM (so enderecos ate &H3FFF). Devolve o registrador A e as flags Cy/Z ja separadas do par AF de saida (mais convenientes que decompor na mao).

ENTRADA
- p(0) = 0 direto / <>0 via SUB-BIOS
- p(1) = endereco da rotina
- p(2) a p(11) = registradores de entrada: AF,BC,DE,HL,IX,IY,AF',BC',DE',HL'

SAIDA
- p(2) a p(11) = registradores de saida (mesma ordem)
- p(12) = registrador A
- p(13) = flag Cy (-1 se ligada)
- p(14) = flag Z (-1 se ligada)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-59", "nb-executar-codigo-de-maquina-guardado-num-segmento,nb_execcodeinsegment", "NestorBASIC: Funcoes diversas", "USR(59)", "Executar codigo de maquina guardado num segmento de RAM: A rotina precisa estar montada...",
        R"MAN(FUNCAO 59 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_ExecCodeInSegment
CHAMADA
    af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, a, cy, z, erro = .NB_ExecCodeInSegment(segmento, endereco, regAf, regBc, regDe, regHl, regIx, regIy, regAf2, regBc2, regDe2, regHl2)

A rotina precisa estar montada no intervalo &H8000-&HBFFF (o segmento e trocado pra pagina 2 antes de chamar). Mesmos registradores da funcao 58. Nao suporta segmentos VRAM nem o 255.

ENTRADA
- p(0) = segmento
- p(1) = endereco da rotina
- p(2) a p(11) = registradores de entrada

SAIDA
- p(2) a p(14) = registradores de saida + A + Cy + Z (igual a funcao 58)

ERROS
-1 = segmento invalido.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-60", "nb-imprimir-uma-string-em-modo-grafico,nb_printgraphic", "NestorBASIC: Funcoes diversas", "USR(60)", "Imprimir uma string em modo grafico: So funciona em SCREEN 5 a 11 - equivalente a...",
        R"MAN(FUNCAO 60 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_PrintGraphic
CHAMADA
    erro = .NB_PrintGraphic(texto$)

So funciona em SCREEN 5 a 11 - equivalente a PRINT#1,texto$ com OPEN"GRP:"AS#1 ja feito, mas compativel com TurboBASIC. Fora dessas SCREENs nao faz nada, sem erro. Maximo 80 caracteres; nao pode ter o caractere 0 (chr$(0)).

ENTRADA
- f$(0) = string a imprimir

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-61", "nb-guardar-uma-string-num-segmento,nb_storestring", "NestorBASIC: Funcoes diversas", "USR(61)", "Guardar uma string num segmento: Termina com um byte 0. Maximo 80 caracteres; nao pode...",
        R"MAN(FUNCAO 61 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_StoreString
CHAMADA
    novoEndereco, erro = .NB_StoreString(texto$, segmento, endereco, incrementar)

Termina com um byte 0. Maximo 80 caracteres; nao pode ter o caractere 0.

ENTRADA
- f$(0) = string
- p(0) = segmento
- p(1) = endereco
- p(2) = <>0 avanca p(1)

SAIDA
- p(1) = p(1)+len(f$(0))+1 se p(2)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-62", "nb-ler-uma-string-guardada-num-segmento,nb_restorestring", "NestorBASIC: Funcoes diversas", "USR(62)", "Ler uma string guardada num segmento: Le ate achar um byte 0, ou 80 caracteres. Fora do...",
        R"MAN(FUNCAO 62 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_RestoreString
CHAMADA
    texto$, novoEndereco, erro = .NB_RestoreString(segmento, endereco, incrementar)

Le ate achar um byte 0, ou 80 caracteres. Fora do TurboBASIC, um caractere 255 na string vira 34 (aspas) por causa de como o BASIC classico atribui strings.

ENTRADA
- p(0) = segmento
- p(1) = endereco
- p(2) = <>0 avanca p(1)

SAIDA
- f$(1) = string lida
- p(1) = p(1)+len(f$(1))+1 se p(2)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-63", "nb-inicializar-o-modo-piscante-da-screen-0,nb_initblink", "NestorBASIC: Funcoes diversas", "USR(63)", "Inicializar o modo piscante da SCREEN 0: Habilita cores e tempos, e limpa a area de VRAM...",
        R"MAN(FUNCAO 63 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_InitBlink
CHAMADA
    erro = .NB_InitBlink(corFrente, corFundo, tempoLigado, tempoDesligado)

Habilita cores e tempos, e limpa a area de VRAM do blink. tempoLigado/tempoDesligado (0 a 15) ambos 0 so limpa a area, sem mudar cores/tempos. So funciona em SCREEN 0 com pelo menos 41 colunas (senao erro -1, sem fazer nada).

ENTRADA
- p(0) = cor de frente do texto piscante
- p(1) = cor de fundo
- p(2) = tempo ligado (0-15)
- p(3) = tempo desligado (0-15)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-64", "nb-criar-ou-apagar-um-bloco-piscante,nb_setblinkblock", "NestorBASIC: Funcoes diversas", "USR(64)", "Criar ou apagar um bloco piscante: Chame .NB_InitBlink antes. So funciona em SCREEN 0 com...",
        R"MAN(FUNCAO 64 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_SetBlinkBlock
CHAMADA
    novaColuna, novaLinha, erro = .NB_SetBlinkBlock(criar, coluna, linha, largura, altura, incColuna, incLinha)

Chame .NB_InitBlink antes. So funciona em SCREEN 0 com pelo menos 41 colunas (nao faz nada, sem erro, fora disso; mas da erro -1 se coluna/linha passarem de 79/27).

ENTRADA
- p(0) = 0 apaga / <>0 cria
- p(1) = coluna inicial (0-79)
- p(2) = linha inicial (0-27)
- p(3) = largura em colunas
- p(4) = altura em linhas
- p(5) = <>0 avanca p(1)
- p(6) = <>0 avanca p(2)

SAIDA
- p(1) = p(1)+p(3) se p(5)<>0
- p(2) = p(2)+p(4) se p(6)<>0

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-65", "nb-obter-informacoes-sobre-interrupcoes,nb_getinterruptinfo", "NestorBASIC: Funcoes diversas", "USR(65)", "Obter informacoes sobre interrupcoes: - p(0) = -1 se algum processo de interrupcao esta...",
        R"MAN(FUNCAO 65 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_GetInterruptInfo
CHAMADA
    algumAtivo, interrupcaoAtiva, sfxTocando, musicaTocando, erro = .NB_GetInterruptInfo()

SAIDA
- p(0) = -1 se algum processo de interrupcao esta ativo
- p(1) = -1 se a interrupcao do usuario esta ativa
- p(2)/p(3) = segmento/endereco da interrupcao do usuario (validos mesmo parada)
- p(4) = -1 se um efeito PSG esta tocando
- p(5) = -1 se uma musica esta tocando

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-66", "nb-definir-ou-parar-a-interrupcao-do-usuario,nb_setuserinterrupt", "NestorBASIC: Funcoes diversas", "USR(66)", "Definir ou parar a interrupcao do usuario: A rotina roda a 50/60Hz e precisa estar...",
        R"MAN(FUNCAO 66 do NestorBASIC  (grupo: Funcoes diversas)

WRAPPER (Basic Dignified do paleobasic): .NB_SetUserInterrupt
CHAMADA
    erro = .NB_SetUserInterrupt(acao, segmento, endereco)

A rotina roda a 50/60Hz e precisa estar montada em &H8000-&HBFFF do segmento indicado.

ENTRADA
- p(0) = 0 para / 1 define e ativa / -1 inverte o estado atual
- p(1) = segmento (ignorado se p(0)<>1)
- p(2) = endereco (ignorado se p(0)<>1)

ERROS
7 = valor invalido em p(0).
-1 = segmento invalido/VRAM/255.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-67", "nb-obter-informacoes-sobre-os-efeitos-psg,nb_getsfxinfo", "NestorBASIC: Efeitos sonoros PSG", "USR(67)", "Obter informacoes sobre os efeitos PSG: So valido depois de .NB_InitSfxSet. Nunca retorna...",
        R"MAN(FUNCAO 67 do NestorBASIC  (grupo: Efeitos sonoros PSG)

WRAPPER (Basic Dignified do paleobasic): .NB_GetSfxInfo
CHAMADA
    tocando, numeroEfeito, volumeMaximoAtual, erro = .NB_GetSfxInfo(volumeMaximo)

So valido depois de .NB_InitSfxSet. Nunca retorna erro.

ENTRADA
- p(0) = novo volume maximo (-1 = nao muda; >15 vira 15)

SAIDA
- p(1) = -1 se algum efeito esta tocando
- p(2)/p(3) = numero/prioridade do efeito tocando (ou do ultimo tocado)
- p(4)/p(5) = segmento/endereco do conjunto de efeitos
- p(6) = numero do maior efeito definido
- p(7) = volume maximo

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-68", "nb-inicializar-um-conjunto-de-efeitos-psg,nb_initsfxset", "NestorBASIC: Efeitos sonoros PSG", "USR(68)", "Inicializar um conjunto de efeitos PSG: - p(0) = segmento",
        R"MAN(FUNCAO 68 do NestorBASIC  (grupo: Efeitos sonoros PSG)

WRAPPER (Basic Dignified do paleobasic): .NB_InitSfxSet
CHAMADA
    erro = .NB_InitSfxSet(segmento, endereco)

ENTRADA
- p(0) = segmento
- p(1) = endereco

ERROS
8 = formato invalido (nao criado pelo editor SEE v3.xx).
-1 = segmento invalido/VRAM/255.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-69", "nb-tocar-um-efeito-sonoro-psg,nb_playsfx", "NestorBASIC: Efeitos sonoros PSG", "USR(69)", "Tocar um efeito sonoro PSG: Se ja tiver outro efeito tocando: prioridade igual, ou o novo...",
        R"MAN(FUNCAO 69 do NestorBASIC  (grupo: Efeitos sonoros PSG)

WRAPPER (Basic Dignified do paleobasic): .NB_PlaySfx
CHAMADA
    erro = .NB_PlaySfx(efeito, prioridade)

Se ja tiver outro efeito tocando: prioridade igual, ou o novo com prioridade alta, o antigo para e o novo toca; o antigo com prioridade alta e o novo baixa, o antigo continua (erro 11).

ENTRADA
- p(0) = numero do efeito
- p(1) = prioridade (0=baixa, <>0=alta)

ERROS
9 = efeito nao definido (trilha OFF no editor).
10 = efeito nao existe (numero maior que o maximo).
11 = outro efeito de prioridade maior ja esta tocando.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-70", "nb-parar-o-efeito-sonoro-psg,nb_stopsfx", "NestorBASIC: Efeitos sonoros PSG", "USR(70)", "Parar o efeito sonoro PSG: Para o efeito em execucao e silencia o PSG. Nunca retorna erro.",
        R"MAN(FUNCAO 70 do NestorBASIC  (grupo: Efeitos sonoros PSG)

WRAPPER (Basic Dignified do paleobasic): .NB_StopSfx
CHAMADA
    erro = .NB_StopSfx()

Para o efeito em execucao e silencia o PSG. Nunca retorna erro.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-71", "nb-carregar-inicializar-ou-desinstalar-o-tocador-s4,nb_loadplayer", "NestorBASIC: Tocador Moonblaster", "USR(71)", "Carregar/inicializar ou desinstalar o tocador (S4): acao 3 (autodetectar) carrega o Wave...",
        R"MAN(FUNCAO 71 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_LoadPlayer
CHAMADA
    tocadorCarregado, arquivoAberto, erro = .NB_LoadPlayer(acao)

acao 3 (autodetectar) carrega o Wave se achar Moonsound, senao o 1.4. Num Turbo-R, se trocar de processador (Z80/R800) depois de carregado, chame de novo pra carregar a versao certa.

ENTRADA
- p(0) = 0 Moonblaster 1.4 / 1 Moonblaster Wave 1.05 / 3 autodetectar / -1 desinstalar

SAIDA
- p(0) = tocador carregado (0, 1 ou -1)
- p(1) = -1 se nao deu erro; senao, numero do arquivo NBASIC.BIN (ainda aberto) onde ocorreu o erro de disco - feche com .NB_CloseFile antes de tentar de novo

ERROS
-1 = segmento 5 nao existe ou nao e do mapper primario.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-72", "nb-obter-informacoes-sobre-a-musica-tocando,nb_getmusicinfo", "NestorBASIC: Tocador Moonblaster", "USR(72)", "Obter informacoes sobre a musica tocando: p(12)=0 se nenhum tocador foi carregado (os...",
        R"MAN(FUNCAO 72 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_GetMusicInfo
CHAMADA
    tocando, pausada, nomeMusica$, erro = .NB_GetMusicInfo()

p(12)=0 se nenhum tocador foi carregado (os demais resultados, exceto deteccao de chips de som, ficam invalidos nesse caso). f$(0)/f$(1) so valem se p(0)<>0. Nunca retorna erro.

SAIDA
- p(0) = -1 se uma musica esta tocando ou pausada
- p(1)/p(2) = -1 se e Moonblaster 1.4 / Moonblaster Wave
- p(4) = -1 se pausada
- p(5)/p(6) = segmento/endereco da musica tocando (ou da ultima tocada)
- p(7)/p(8) = posicao/passo atual (0-15)
- p(9)/p(10)/p(11) = -1 se detectou MSX-MUSIC/MSX-AUDIO/OPL4
- p(12) = -1 se ha um tocador inicializado
- p(13) = tocador instalado (0=Moonblaster 1.4, 1=Wave - so vale se p(12)=-1)
- f$(0) = nome da musica (sempre 40 caracteres na 1.4, 50 na Wave)
- f$(1) = samplekit/wavekit carregado quando a musica foi salva (maiusculas, sem extensao; NONE se nao tinha nenhum)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-73", "nb-habilitar-desabilitar-os-chips-de-som,nb_setsoundchips", "NestorBASIC: Tocador Moonblaster", "USR(73)", "Habilitar/desabilitar os chips de som: So tem efeito na proxima musica tocada ou apos...",
        R"MAN(FUNCAO 73 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_SetSoundChips
CHAMADA
    statusMsxMusic, statusMsxAudio, statusOpl4, erro = .NB_SetSoundChips(acao, msxMusica, msxAudio, opl4)

So tem efeito na proxima musica tocada ou apos parar/reiniciar a atual.

ENTRADA
- p(0) = 0 so consultar / 1 aplicar p(1)-p(3) / 2 habilitar todos os encontrados
- p(1)/p(2)/p(3) = MSX-MUSIC/MSX-AUDIO/OPL4: 0=nao mexe, 1=desabilita, 2=habilita, -1=inverte

SAIDA
- p(4)/p(5)/p(6) = MSX-MUSIC/MSX-AUDIO/OPL4: 0=nao encontrado, 1=encontrado mas desabilitado, 2=habilitado

ERROS
7 = parametro invalido.
12 = tocador nao instalado.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-74", "nb-comecar-a-tocar-uma-musica-moonblaster,nb_playmusic", "NestorBASIC: Tocador Moonblaster", "USR(74)", "Comecar a tocar uma musica Moonblaster: Moonblaster Wave pode ocupar ate 3 segmentos...",
        R"MAN(FUNCAO 74 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_PlayMusic
CHAMADA
    erro = .NB_PlayMusic(segmento, endereco)

Moonblaster Wave pode ocupar ate 3 segmentos consecutivos.

ENTRADA
- p(0) = segmento
- p(1) = endereco inicial

ERROS
-1 = segmento invalido/VRAM/255.
12 = tocador nao inicializado.
13 = musica salva em modo EDIT (nao da pra tocar), ou sem musica Moonblaster Wave valida no endereco.
14 = ja tem outra musica tocando.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-75", "nb-parar-a-musica,nb_stopmusic", "NestorBASIC: Tocador Moonblaster", "USR(75)", "Parar a musica: Para a musica tocando e silencia os chips de som. Nunca retorna erro (nao...",
        R"MAN(FUNCAO 75 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_StopMusic
CHAMADA
    erro = .NB_StopMusic()

Para a musica tocando e silencia os chips de som. Nunca retorna erro (nao faz nada se nao tiver musica tocando/tocador nao inicializado).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-76", "nb-pausar-continuar-a-musica,nb_pausemusic", "NestorBASIC: Tocador Moonblaster", "USR(76)", "Pausar/continuar a musica: - p(0) = 0 pausa / 1 continua / -1 inverte o estado atual",
        R"MAN(FUNCAO 76 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_PauseMusic
CHAMADA
    erro = .NB_PauseMusic(acao)

ENTRADA
- p(0) = 0 pausa / 1 continua / -1 inverte o estado atual

ERROS
7 = parametro invalido (nao faz nada, sem erro, se nao tiver musica tocando/pausada ou tocador nao inicializado).

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-77", "nb-fade-out-da-musica,nb_fademusic", "NestorBASIC: Tocador Moonblaster", "USR(77)", "Fade-out da musica: Delay = ciclos de 1/50 ou 1/60s entre passos do fade - quanto menor,...",
        R"MAN(FUNCAO 77 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_FadeMusic
CHAMADA
    fazendoFade, delayAtual, erro = .NB_FadeMusic(delay)

Delay = ciclos de 1/50 ou 1/60s entre passos do fade - quanto menor, mais rapido. Ao terminar (volume chega a zero), a musica para sozinha. Nunca retorna erro.

ENTRADA
- p(0) = 0 so consultar / -1 pausa o fade / 1..254 inicia/continua com esse delay

SAIDA
- p(1) = -1 se esta fazendo fade
- p(2) = delay atual (-1 se pausado)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-78", "nb-carregar-um-samplekit-music-module-s4,nb_loadsamplekit", "NestorBASIC: Tocador Moonblaster", "USR(78)", "Carregar um samplekit Music Module (S4): Formato Moonblaster: 56 bytes de cabecalho + 32K...",
        R"MAN(FUNCAO 78 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_LoadSamplekit
CHAMADA
    bytesLidos, erro = .NB_LoadSamplekit(numeroArquivo)

Formato Moonblaster: 56 bytes de cabecalho + 32K de amostras. Le de um arquivo ja aberto, na posicao atual do ponteiro. Se o retorno vier maior que 16K, so os primeiros 16K foram pra RAM de amostras; 32824 nao cabe em inteiro e volta como -32712 (limite do BASIC). Nao faz nada (sem erro) se o tocador nao foi inicializado ou nao ha Music Module.

ENTRADA
- p(0) = numero do arquivo (ja aberto)

SAIDA
- p(7) = numero de bytes lidos

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-79", "nb-carregar-um-wavekit-moonsound-s4,nb_loadwavekit", "NestorBASIC: Tocador Moonblaster", "USR(79)", "Carregar um wavekit Moonsound (S4): Precisa estar salvo em modo USER (senao erro 15). Le...",
        R"MAN(FUNCAO 79 do NestorBASIC  (grupo: Tocador Moonblaster)

WRAPPER (Basic Dignified do paleobasic): .NB_LoadWavekit
CHAMADA
    bytesAlto, bytesBaixo, erro = .NB_LoadWavekit(numeroArquivo)

Precisa estar salvo em modo USER (senao erro 15). Le de um arquivo ja aberto, na posicao atual do ponteiro. Nao faz nada (sem erro) se o tocador Wave nao foi inicializado ou nao ha Moonsound.

ENTRADA
- p(0) = numero do arquivo (ja aberto)

SAIDA
- p(6)/p(7) = numero de bytes lidos (32 bits: p(6)*65536+p(7))

ERROS
15 = sem wavekit valido nessa posicao do arquivo, ou nao esta em modo USER.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-80", "nb-obter-definir-o-numero-de-segmentos-alocados,nb_setsegmentcount", "NestorBASIC: Controle de uso de segmentos", "USR(80)", "Obter/definir o numero de segmentos alocados: Se pedir menos de 6, o NestorBASIC usa 5...",
        R"MAN(FUNCAO 80 do NestorBASIC  (grupo: Controle de uso de segmentos)

WRAPPER (Basic Dignified do paleobasic): .NB_SetSegmentCount
CHAMADA
    segmentosAlocados, segmentosMaximo, erro = .NB_SetSegmentCount(numSegmentos)

Se pedir menos de 6, o NestorBASIC usa 5 mesmo assim (ou 6 se o tocador Moonblaster estiver carregado); se pedir mais que o disponivel, aloca o maximo possivel (ate 247). Funciona no DOS 1 e no DOS 2, mas so faz sentido de verdade no DOS 2 (no DOS 1 so mexe em variaveis internas, sem liberar/reservar nada de fato). Nunca retorna erro.

ENTRADA
- p(0) = numero de segmentos pedido (0 = so consultar, sem mudar nada)

SAIDA
- p(0) = numero de segmentos alocados apos a chamada
- p(1) = numero maximo de segmentos que podem ser alocados

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-81", "nb-informacoes-sobre-nestorman-internestor-suite,nb_getnestormaninfo", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(81)", "Informacoes sobre NestorMan/InterNestor Suite: Nunca retorna erro.",
        R"MAN(FUNCAO 81 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_GetNestorManInfo
CHAMADA
    status, segmentoNestorManSeg4, erro = .NB_GetNestorManInfo()

Nunca retorna erro.

SAIDA
- p(0) = 0 NestorMan nao instalado / 1 NestorMan instalado / 3 NestorMan e InterNestor Suite instalados
- p(1) a p(4) = segmento NestorMan de cada modulo do InterNestor Suite 1 a 4 (so validos se p(0)=3)
- p(5) = segmento NestorMan do segmento 4 do NestorBASIC (usado como buffer intermediario em transferencias - so valido se p(0)=1 ou 3)

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-82", "nb-executar-uma-funcao-do-nestorman,nb_callnestorman", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(82)", "Executar uma funcao do NestorMan: Chamada indireta via hook estendido da BIOS -...",
        R"MAN(FUNCAO 82 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_CallNestorMan
CHAMADA
    af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, a, cy, z, erro = .NB_CallNestorMan(funcao, regAf, regBc, regDe, regHl, regIx, regIy, regAf2, regBc2, regDe2, regHl2)

Chamada indireta via hook estendido da BIOS - equivalente a .NB_ExecCode com endereco &HFFCA. Mesmos registradores de entrada/saida de .NB_ExecCode.

ENTRADA
- p(0) = numero da funcao NestorMan
- p(2) a p(11) = registradores de entrada (AF,BC,DE,HL,IX,IY,AF',BC',DE',HL')

SAIDA
- p(2) a p(14) = registradores de saida + A + Cy + Z

ERROS
Atencao: o manual original se contradiz aqui - a secao de erros gerais diz que as funcoes 80, 81 e 82 nunca retornam erro, mas a descricao desta funcao especifica diz -1 se o NestorMan nao estiver instalado. Teste na pratica se depender disso. De qualquer forma, nao verifica se a funcao pedida realmente existe no NestorMan.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-83", "nb-copiar-dados-do-nestorman-pro-nestorbasic,nb_copyfromnestorman", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(83)", "Copiar dados do NestorMan pro NestorBASIC: segmento de destino nao pode ser o 1...",
        R"MAN(FUNCAO 83 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_CopyFromNestorMan
CHAMADA
    novoEndOrigem, novoEndDestino, erro = .NB_CopyFromNestorMan(segOrigemNestorMan, endOrigem, segDestino, endDestino, tamanho, incOrigem, incDestino)

segmento de destino nao pode ser o 1 (TurboBASIC) - copie primeiro pra outro segmento (ex.: o 4) se precisar.

ENTRADA
- p(0) = segmento de origem (NestorMan)
- p(1) = endereco de origem
- p(2) = segmento de destino (NestorBASIC)
- p(3) = endereco de destino
- p(4) = tamanho
- p(5) = <>0 avanca origem
- p(6) = <>0 avanca destino

SAIDA
- p(1) = p(1)+p(4) se p(5)<>0
- p(3) = p(3)+p(4) se p(6)<>0

ERROS
-1 = segmento invalido.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-84", "nb-copiar-dados-do-nestorbasic-pro-nestorman,nb_copytonestorman", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(84)", "Copiar dados do NestorBASIC pro NestorMan: segmento de origem nao pode ser o 1...",
        R"MAN(FUNCAO 84 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_CopyToNestorMan
CHAMADA
    novoEndOrigem, novoEndDestino, erro = .NB_CopyToNestorMan(segOrigem, endOrigem, segDestinoNestorMan, endDestino, tamanho, incOrigem, incDestino)

segmento de origem nao pode ser o 1 (TurboBASIC).

ENTRADA
- p(0) = segmento de origem (NestorBASIC)
- p(1) = endereco de origem
- p(2) = segmento de destino (NestorMan)
- p(3) = endereco de destino
- p(4) = tamanho
- p(5) = <>0 avanca origem
- p(6) = <>0 avanca destino

SAIDA
- p(1) = p(1)+p(4) se p(5)<>0
- p(3) = p(3)+p(4) se p(6)<>0

ERROS
-1 = segmento invalido.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-85", "nb-executar-uma-rotina-do-internestor-suite,nb_callinternestorsuite", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(85)", "Executar uma rotina do InterNestor Suite: Pra ler/escrever as constantes/variaveis de...",
        R"MAN(FUNCAO 85 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_CallInterNestorSuite
CHAMADA
    af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, a, cy, z, erro = .NB_CallInterNestorSuite(modulo, endereco, regAf, regBc, regDe, regHl, regIx, regIy, regAf2, regBc2, regDe2, regHl2)

Pra ler/escrever as constantes/variaveis de configuracao dos modulos, use .NB_GetNestorManInfo (segmento NestorMan de cada modulo) e depois .NB_CopyFromNestorMan/.NB_CopyToNestorMan. Mesmos registradores de .NB_ExecCode.

ENTRADA
- p(0) = numero do modulo (1 a 4)
- p(1) = endereco da rotina
- p(2) a p(11) = registradores de entrada

SAIDA
- p(2) a p(14) = registradores de saida + A + Cy + Z

ERROS
-1 = InterNestor Suite nao instalado, ou numero de modulo invalido.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("nb-86", "nb-executar-uma-rotina-do-internestor-lite,nb_callinternestorlite", "NestorBASIC: NestorMan e InterNestor Suite/Lite", "USR(86)", "Executar uma rotina do InterNestor Lite: So existe uma instancia (sem selecao de modulo)....",
        R"MAN(FUNCAO 86 do NestorBASIC  (grupo: NestorMan e InterNestor Suite/Lite)

WRAPPER (Basic Dignified do paleobasic): .NB_CallInterNestorLite
CHAMADA
    af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, a, cy, z, erro = .NB_CallInterNestorLite(endereco, regAf, regBc, regDe, regHl, regIx, regIy, regAf2, regBc2, regDe2, regHl2)

So existe uma instancia (sem selecao de modulo). Pra checar se esta instalado sem chamar essa rotina: p(0)=0:p(1)=&Hffca:p(2)=0:p(4)=&H2203:e=usr(58):if p(12)=0 then (nao instalado) - equivalente a usar .NB_ExecCode direto e olhar o registrador A devolvido. Mesmos registradores de .NB_ExecCode.

ENTRADA
- p(1) = endereco da rotina (ver o manual do InterNestor Lite pra lista)
- p(2) a p(11) = registradores de entrada

SAIDA
- p(2) a p(14) = registradores de saida + A + Cy + Z

ERROS
-1 = InterNestor Lite nao instalado.

VER TAMBEM
  nestorbasic

FONTE
  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.)MAN");

    add("seetracker", "see,see-tracker", "SEE Tracker: Introducao", "", "O que e o SEE: SEE (Sound Effect Editor) e um editor de efeitos sonoros (SFX curtos -...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Introducao

SEE (Sound Effect Editor) e um editor de efeitos sonoros (SFX curtos - tiro, explosao, moeda, etc.), NAO um tracker de musica, pro chip PSG do MSX (AY-3-8910/YM2149). Feito por Fuzzy Logic (R. v/d Meulen e A. v/d Wal, Holanda), shareware - pode ser copiado livremente, mas uso comercial exige pagamento aos autores (ver endereco no fim do manual original).

A versao presente neste repositorio (see/) e a v3.10a (25/01/94), a mais recente/completa. Existe tambem uma v3.00 mais antiga (see/SEE3HELP.DOC/.TED), mantida aqui so como historico - superada pela v3.10 em tudo (corrige um bug real do driver de replay, funciona sob DOS2, etc., ver Mudancas entre versoes no proximo topico).

O programa gera arquivos .SEE (varios efeitos por arquivo) ou .SFX (um efeito isolado), tocados na hora via um pequeno driver Z80 (see/SEE3PLAY.ASC) que fica residente e e chamado pelo BASIC (ou por codigo de maquina) sempre que um efeito precisa tocar - o mesmo tipo de integracao BLOAD + DEFUSR/USR() que o NestorBASIC desta IDE ja usa (ver Ajuda -> Nestor Basic...).

Por que estudar isso: o objetivo declarado e construir, nesta IDE, um tracker de SFX nativo compativel com o formato .SEE, para SFX gerados aqui poderem tocar tanto no editor original quanto (o caminho principal) via NestorBASIC. Esta tela de Ajuda e o resultado de uma sessao de estudo - ainda NAO existe nenhum editor/gerador .SEE nesta IDE.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-mudancas-entre-versoes-v2-v3-00-v3-10", "", "SEE Tracker: Introducao", "", "Mudancas entre versoes (v2 -> v3.00 -> v3.10): Da v2.xx pra v3.xx (reescrita total):",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Introducao

Da v2.xx pra v3.xx (reescrita total):
- Edicao mais rapida, varios patterns na tela ao mesmo tempo (13 visiveis).
- Comandos de bloco (copiar/substituir um intervalo de patterns de uma vez).
- Novo formato de arquivo, mais compacto.

Da v3.00 pra v3.10 (a versao presente neste repositorio):
- Funciona 100% sob DOS2 (rodando de dentro do BASIC).
- Boot de qualquer drive/(sub)diretorio.
- Checagem correta do arquivo ao abrir um .SEE.
- Bug real corrigido no driver de replay: com MAXVOL (a variavel SEEVOL do driver - ver grupo Motor de replay) ajustado abaixo de 15, alguns *slides* de volume ficavam incorretos. O proprio manual avisa: use o SEE3PLAY.asc da v3.10, nao um driver antigo, para SFX corretos.
- Um bit de afinacao (Tuning) dos canais de rustle que nao estava sendo limpo ao exibi-los na tela do editor.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-arquivos-deste-projeto-pasta-see", "", "SEE Tracker: Introducao", "", "Arquivos deste projeto (pasta see/): Inventario do que esta em see/ e pra que serve - use...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Introducao

Inventario do que esta em see/ e pra que serve - use como referencia de qual arquivo e autoritativo (v3.10) e qual e so historico (v3.00):

Programa v3.10 (o que importa)
- SEEV3_10.BIN + SEEV3_10.LIB - o editor SEE em si (SEEV3_10.BAS e o bootstrap BASIC: BLOAD do .bin, depois de mais 2 overlays - loadfuz.bin/softmenu.bin - que NAO estao presentes nesta pasta).
- SEE3PLAY.ASC - fonte Z80 do driver de replay (o mais importante pra compatibilidade - ver grupo Motor de replay).
- SEE3HELP.TXT - manual da v3.10 (base desta tela de Ajuda).
- SEEBASIC.BIN - um driver de replay ja montado (usado pelo exemplo SEE.BAS).
- SEE3.PRF - arquivo de preferencias (9 bytes, salvo pelo BASIC ao redor do editor, nao pelo editor em si - ver Preferencias e Disk mode).

Exemplos reais de .SEE (usados pra conferir o cabecalho byte a byte neste estudo)
- SEEDRUMS.SEE - exemplo oficial (kit de bateria via PSG) que acompanha a v3.10.
- FIREBIRD.SEE, PLICS.SEE, QUARTH.SEE - coletados de jogos/projetos reais.

v3.00, historico (superado)
- SEE3.COM/SEE3BOOT.BIN/SEE3.LIB - o editor v3.00 (DOS1/BASIC).
- SEE3HELP.DOC - manual da v3.00 (diverge da v3.10 em varios pontos - ver topico anterior).
- SEE3HELP.TED - o mesmo manual, no formato binario de um editor de texto/processador de texto MSX (referencia a ADRES.MRG/STANDARD.CHR) - so a origem de onde .DOC/.TXT foram exportados, sem conteudo extra.
- SEE.LDR/SEE.BAT - menu de boot em BASIC (ler manual / rodar o programa / voltar ao menu do disco de origem).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-tela-principal", "", "SEE Tracker: Usando o editor original", "", "Tela principal: No topo: menu principal, navegavel pelos cursores. No meio: ate 13...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

No topo: menu principal, navegavel pelos cursores. No meio: ate 13 patterns visiveis de uma vez (rolando quando voce sobe/desce); o pattern atual (o 7o da lista, no centro) fica destacado com uma barra. Embaixo: a linha de status, que mostra toda acao feita.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-menu-principal-e-teclas-de-funcao", "", "SEE Tracker: Usando o editor original", "", "Menu principal e teclas de funcao: - NEW [F4] - limpa todos os patterns e dados de SFX.",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

- NEW [F4] - limpa todos os patterns e dados de SFX.
- Preferences [F10] - tela de preferencias (ver topico proprio).
- Diskop [F5] - tela de disco (salvar/carregar .SEE/.SFX).
- Quit [ESC] - sai do SEE3, volta pro BASIC.
- Play SFX [F1] / Play SFX visual [F6] - toca o SFX atual (a segunda opcao tambem mostra os patterns tocando). Stop noise/Play status [STOP] corta o som.
- Sound FX nr [F2] - numero do SFX selecionado (0-255).
- Start pattern [F3] - pattern inicial deste SFX (RETURN reseta pra OFF).
- Set next FX pat [F9] - fecha o SFX atual e ja prepara o pattern inicial do PROXIMO numero de SFX (incrementa o Sound FX nr sozinho).
- Quant [F7] - passo do cursor Y apos editar um canal (Y = Y + Quant).
- Poly [F8] - quando ligado, o canal de edicao avanca (X = X + 1) sozinho apos editar.
- Edit [SELECT] - liga/desliga o modo de edicao.
- Block - mostra o intervalo de bloco marcado (RETURN remove).
- Max Volume - volume maximo/mestre do efeito (ver formula exata em Slides de afinacao e volume, grupo Motor de replay).
- Pattern filter/Print - presentes no menu mas nao implementados (o proprio manual diz ``).

Outras teclas da tela principal: [+]/[-] muda o numero do SFX; [SELECT] alterna 50/60Hz.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-edicao-de-pattern-os-11-canais", "", "SEE Tracker: Usando o editor original", "", "Edicao de pattern: os 11 canais: Cada pattern (linha da grade de edicao) tem 11 canais...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

Cada pattern (linha da grade de edicao) tem 11 canais lado a lado:
- event (1) - comandos de controle do pattern (ver proximo topico).
- snd1/2/3 (2-4) - frequencia PSG de cada um dos 3 canais de som. Sem dado = canal desligado automaticamente.
- rus1/2/3 (5-7) - canal de rustle (ruido) usado por cada canal de som. So existe um registrador de ruido real no PSG (compartilhado pelos 3 canais), mas cada canal escolhe usar (ou nao) esse ruido independentemente.
- vol1/2/3 (8-10) - volume de cada canal de som (0-15).
- wave (11) - padrao do envelope de volume PSG (so aparece quando algum canal de volume usa a Wave do PSG).
- time (12) - periodo do envelope (regs. 11/12 do PSG, 12 bits, 000-FFF).

Para digitar dados, use 0-9/A-F (hexadecimal); Backspace apaga/desliga o canal atual.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-canal-event-comandos-de-controle", "", "SEE Tracker: Usando o editor original", "", "Canal Event: comandos de controle: Digite a primeira letra do comando pra editar (H de...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

Digite a primeira letra do comando pra editar (H de HALT, F de FOR, E de END, etc.); Backspace limpa.

- -- (vazio) - nao faz nada extra, so toca os dados PSG deste pattern.
- HALT (x) - espera x interrupcoes (1/50 ou 1/60s cada) antes de tocar os dados PSG DESTE pattern (ou seja, o dado do proprio pattern do HALT so soa depois da espera).
- FOR (x) - marca o inicio de um loop, repetido x vezes. Ate 4 loops podem estar ativos ao mesmo tempo (aninhados).
- NEXT - fecha o loop mais recente (volta pro FOR se ainda faltar repeticao).
- START - marca um ponto de retomada (diferente do FOR: sem contador, e um so, nao aninha).
- RERUN - volta pro ultimo START - sempre, sem contador (na pratica cria um loop infinito ate o som ser cortado de fora, tipo a parte 'sustain' de um efeito).
- TMP (x) - muda o tempo de reproducao (medido em interrupcoes por passo).
- END - fim do efeito.

EXEMPLO DO MANUAL (FOR/NEXT)
000  FOR 7   xxx yyy zzz
001   -      xxx yyy zzz
002  NEXT     -   -   -
003  END      -   -   -
Os patterns 000+001 repetem 7 vezes. Detalhe importante, so visivel lendo o driver de replay (nao esta no manual): FOR/START disparam so na PRIMEIRA vez que o ponteiro chega neles - nas voltas seguintes do loop, o driver so reaplica os DADOS PSG daquele pattern (sem reprocessar o evento FOR/START de novo). Detalhe tecnico completo em Como FOR/NEXT e START/RERUN realmente funcionam (grupo Motor de replay).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-efeitos-de-canal-slides-d-u-e-envelope-wave", "", "SEE Tracker: Usando o editor original", "", "Efeitos de canal: slides D:/U: e envelope (Wave): Em qualquer canal de frequencia, rustle...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

Em qualquer canal de frequencia, rustle ou volume, SHIFT+letra liga um efeito de slide:
- D:xxx - Down slide (o valor do registrador diminui).
- U:xxx - Up slide (o valor do registrador aumenta).
(xxx = taxa do slide; menos digitos nos canais de rustle/volume que no de frequencia.)

Pegadinha do proprio manual: nos canais de FREQUENCIA, D (down) faz o valor do registrador diminuir, mas isso faz o som ficar mais AGUDO (e vice-versa) - o registrador PSG e um divisor de periodo, nao uma frequencia direta.

Nos canais de volume existe ainda Wave (SHIFT+W) - ativa o envelope de volume padrao do PSG (as 15 formas de hardware) em vez de slide por software; quando usado, defina o padrao e o tempo do envelope nos canais wave/time (ver Registradores do PSG no grupo Motor de replay pra saber exatamente o que isso desliga).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-bloco-edicao-em-lote", "", "SEE Tracker: Usando o editor original", "", "Bloco (edicao em lote): So existe um bloco por vez, definido por um pattern inicial/final...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

So existe um bloco por vez, definido por um pattern inicial/final (mostrado no menu principal como Block: xxx-yyy); os patterns dentro dele ficam destacados com uma cor.

Comandos (tecla CODE + letra):
- CODE+S - marca o pattern inicial do bloco.
- CODE+E - marca o pattern final.
- CODE+G - vai pro primeiro pattern do bloco.
- CODE+C - copia o bloco pra posicao do cursor.
- CODE+M/RETURN - substitui (replace) na posicao do cursor.
- CODE+Backspace - apaga os patterns do bloco.
- CODE+Espaco - remove a marcacao do bloco (sem apagar patterns).

(A tecla [+]/espaco no menu principal tambem remove o bloco com RETURN.)

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-preferencias-e-disk-mode", "", "SEE Tracker: Usando o editor original", "", "Preferencias e Disk mode: Preferencias [F10] - ajustes de tela, salvos em SEE3.prf no...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Usando o editor original

Preferencias [F10] - ajustes de tela, salvos em SEE3.prf no drive atual (o mesmo escolhido no Diskop). Se esse arquivo existir na hora de dar boot no SEE, ele e carregado automaticamente.

Disk mode [F5] - salvar/carregar SFX, com um formatador de disco embutido. Dois tipos de arquivo:
- .SEE - todos os SFX do projeto, num arquivo so (ver Formato de arquivo .SEE).
- .SFX - um unico SFX isolado.

Ao carregar um .SFX, o pattern inicial do SFX atual precisa estar definido antes (senao da erro). Ao salvar um .SFX, o SEE exige um pattern inicial e um evento de fim (END ou RERUN) corretos, checando isso antes de gravar. O SEE tambem verifica a identificacao do arquivo ao abrir um .SEE/.SFX, recusando arquivos que nao sejam da v3.xx (ver Cabecalho do arquivo, onde o CHECK REAL do driver de replay - so 4 bytes - e mais frouxo que o do editor).

O SEE lembra o ultimo diretorio visto de cada tipo de arquivo (.SEE/.SFX) separadamente. Nao ha cruncher (compressao) implementado nesta versao, apesar do menu mencionar a opcao.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-teclas-da-tela-principal", "", "SEE Tracker: Referencia de teclas", "", "Teclas da tela principal: - F1 - toca o SFX atual.",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Referencia de teclas

- F1 - toca o SFX atual.
- F6 - idem, mostrando os patterns tocando.
- F2 - escolhe o SFX.
- F3 - define o pattern inicial.
- F4 - New (limpa tudo).
- F5 - Disk mode.
- F7 - define o Quant.
- F8 - liga/desliga Poly.
- F9 - fecha o SFX atual e prepara o pattern inicial do proximo.
- F10 - Preferencias.
- SELECT - alterna 50/60Hz.
- STOP - corta o SFX tocando.
- [+]/[-] - aumenta/diminui o numero do SFX.
- ESC - sai do SEE.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-teclas-do-menu-principal-e-da-edicao-de-pattern", "", "SEE Tracker: Referencia de teclas", "", "Teclas do menu principal e da edicao de pattern: Menu principal: setas movem o cursor;...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Referencia de teclas

Menu principal: setas movem o cursor; Espaco confirma; Return idem (com efeito especial em alguns campos); Home vai pro topo; Graphic/Trig B entram na edicao de pattern.

Edicao de pattern: setas movem; Home vai pro pattern 0; Return vai pro canal 0 (event); ESC/Graphic/Trig B voltam pro menu; Backspace limpa o canal atual; DEL apaga o pattern; INS insere um pattern; 0-9/A-F digitam dado no canal; [Q] define o Quant; [P] vai pra um pattern especifico.

SHIFT +: Home vai pro primeiro pattern do SFX atual; Backspace limpa o pattern inteiro; DEL/INS como o normal mas movendo os patterns seguintes pra baixo/cima; [U]/[D] afinacao (tuning) rapida; [W] liga Volume Wave (so nos canais de volume).

CTRL +: [E]/[S]/[R]/[V]/[W] vao direto pro canal Event/Sound/Rustle/Volume/Wave; [Q]/[P] como acima; Espaco define o pattern inicial do SFX atual; Return apaga o SFX atual (OFF).

CODE + (comandos de bloco): ver topico Bloco (edicao em lote).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-teclas-do-disk-mode", "", "SEE Tracker: Referencia de teclas", "", "Teclas do Disk mode: Setas movem o cursor; Espaco confirma; Return idem; Home vai pro...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Referencia de teclas

Setas movem o cursor; Espaco confirma; Return idem; Home vai pro topo da tela; ESC/F5 saem (voltam ao menu principal); [+]/[-] mudam o numero do SFX.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-visao-geral-do-arquivo", "", "SEE Tracker: Formato de arquivo .SEE", "", "Visao geral do arquivo: Um .SEE tem pelo menos 3 partes, nesta ordem:",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Formato de arquivo .SEE

Um .SEE tem pelo menos 3 partes, nesta ordem:
1. Cabecalho (16 bytes) - identificacao + 4 contadores (ver proximo topico).
2. Tabela de posicoes (512 bytes, a partir do offset $0010) - o pattern inicial de cada um dos 256 SFX possiveis (ver Tabela de posicoes de SFX).
3. Dados de pattern (a partir do offset $0210 no esquema de enderecamento do driver) - um registro de 15 bytes por pattern (ver Formato de um pattern (15 bytes)).

Confirmado nos 4 arquivos de exemplo desta pasta (FIREBIRD/PLICS/QUARTH/SEEDRUMS.SEE, inspecionados byte a byte): todos comecam com os 4 bytes ASCII SEE3, mas o RESTO da identificacao de 8 bytes varia - SEE3org+$10 em 3 deles, SEE3EDIT no QUARTH.SEE (provavelmente salvo por um build ligeiramente diferente do editor). Isso bate exatamente com o que o driver de replay realmente verifica (so os 4 primeiros bytes - ver Cabecalho do arquivo).

Achado novo (2026-08-06): nos 4 arquivos, tamanho do arquivo - HIPTA (fim dos dados de pattern) da exatamente 1056 bytes sobrando no final, sempre o mesmo valor independente do tamanho do arquivo - hipotese forte de uma 4a area de tamanho fixo (talvez uma tabela de nomes de SFX, usada pela tela Set next FX pat?) nao documentada no Apendice B do manual nem mencionada no driver de replay (o que bateria: o driver so precisa ler ate o fim dos patterns, nunca olha o que vem depois). Nao investigado a fundo ainda - ver Rumo a um tracker compativel.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-cabecalho-do-arquivo-16-bytes-resolvido-por-anal", "", "SEE Tracker: Formato de arquivo .SEE", "", "Cabecalho do arquivo (16 bytes) - RESOLVIDO por analise cruzada: Offsets a partir do...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Formato de arquivo .SEE

Offsets a partir do inicio do arquivo:
- $00-$07 - Identificacao (8 bytes). O manual diz que deveria ser literalmente SEEv3.xx; nos 4 arquivos de exemplo desta pasta o texto real e outro (SEE3org+$10 ou SEE3EDIT). O que realmente importa pro replay: o driver SEE3PLAY.ASC so compara os 4 primeiros bytes contra o texto SEE3 - o resto do campo e ignorado pelo player (pode ser usado pelo EDITOR pra guardar uma sub-versao, mas nao afeta se o som toca).
- $08-$09 - Highest used pattern. Confirmado (2026-08-06): le sempre $03FF (1023) nos 4 arquivos de exemplo, apesar de tamanhos bem diferentes - nao e uma contagem por arquivo, e uma constante de capacidade do formato: bate exatamente com %PATTS EQU &H0210 ;max 1024 patts do proprio driver (patterns enderecaveis vao de 0 a 1023 = $3FF).
- $0A-$0B - Highest used pattern+1 offset_address. Formula confirmada nos 4 arquivos, divisao exata (resto zero) em todos: (numero de patterns realmente usados no arquivo) * 15 + 528 (528 = 16 de cabecalho + 512 da tabela de posicoes). Usado pelo driver como guarda de seguranca em tempo de execucao: SEEADR + este_valor = endereco (em MEMORIA, nao no arquivo) do byte logo depois do ultimo dado de pattern valido; se o ponteiro de leitura passar disso, o driver corta o som sozinho (protecao contra dado .SEE corrompido/incompleto).
- $0C-$0D - Highest used SFX. Deu valores pequenos e plausiveis nos 3 arquivos SEE3org (32/9/13), usado pra validar o numero de SFX pedido em SETSFX (erro se for maior que este valor). Anomalia isolada: no QUARTH.SEE (unico com ID SEE3EDIT, provavelmente de um build ligeiramente diferente) esse campo leu 48394 - um numero absurdo pra um indice de SFX que so vai de 0 a 255. Fica como duvida especifica desse arquivo/variante, sem afetar a formula confirmada para o formato SEE3org (o que interessa pra escrever nosso proprio gerador).
- $0E-$0F - O manual chama esse campo de xx (sem explicar). O driver, apesar de rotular o campo de origem como Unused num comentario, copia ele pra uma variavel de trabalho chamada literalmente _FLELN (File length) - e nunca le essa copia de novo em lugar nenhum da logica de replay. Nos 3 arquivos SEE3org leu sempre 0 (nao bate com o tamanho real do arquivo) - ou seja, na pratica nao guarda o tamanho do arquivo (ou so e preenchido em circunstancias que nao apareceram nestes 4 exemplos). Continua irrelevante pro player de qualquer forma, ja que nunca e lido de volta.

Nota tecnica sobre o proprio driver: a rotina SEE_IN (SEE3PLAY.ASC) faz LD B,4 no loop que confere a identificacao (compara so 4 bytes), e o LDIR que copia estes 4 campos pra memoria de trabalho comeca logo em seguida, sem reposicionar o ponteiro - lido ao pe da letra, isso copiaria a partir do byte 4 do arquivo, nao do 8. Testado contra os 4 arquivos reais: essa leitura literal da numeros sem sentido (nenhuma divisao inteira, nenhuma consistencia entre arquivos), enquanto ler a partir do byte 8 (como os proprios comentarios %HISPT EQU &H08 do driver dizem) bate perfeitamente em todos os 4. Suspeita forte: erro de transcricao no .ASC (LD B,4 deveria ser LD B,8 - o template SEE_ID comparado logo abaixo tem exatamente 8 bytes declarados, "SEE3???" + $10, entao um loop de 4 s usaria metade do template declarado). Pra fins de formato de arquivo, os offsets $08/$0A/$0C/$0E (como o manual e os EQU descrevem) sao os corretos.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-tabela-de-posicoes-de-sfx", "", "SEE Tracker: Formato de arquivo .SEE", "", "Tabela de posicoes de SFX: A partir do offset $0010: 512 bytes = 256 entradas de 2 bytes...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Formato de arquivo .SEE

A partir do offset $0010: 512 bytes = 256 entradas de 2 bytes (little-endian), uma por numero de SFX possivel (0-255). Cada entrada guarda o numero do pattern inicial daquele SFX.

Um SFX nao definido usa o byte alto $FF como sentinela (o driver testa exatamente isso em SETSFX: se o byte alto lido for $FF, devolve erro SFX nao existe). Isso deixa $FF00-$FFFF fora do alcance como pattern inicial valido - na pratica irrelevante, ja que o driver reserva no maximo 1024 patterns ($000-$3FF).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-formato-de-um-pattern-15-bytes", "", "SEE Tracker: Formato de arquivo .SEE", "", "Formato de um pattern (15 bytes): Cada pattern e um registro de 15 bytes fixos - este...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Formato de arquivo .SEE

Cada pattern e um registro de 15 bytes fixos - este layout foi conferido cruzando o Apendice B do manual com o loop SETPSG do driver (SEE3PLAY.ASC), byte a byte, e os dois batem perfeitamente (ao contrario do cabecalho, aqui nao ha divergencia):

- $00 - Event. Bits 6-4 = comando (AND $70); bits 3-0 = valor. Ver Bits exatos do byte de evento no grupo Motor de replay - o driver so testa 3 bits, nao o byte inteiro como o manual da a entender.
- $01-$02 - Frequencia canal 1 (12 bits, little-endian: $01=byte baixo, $02=byte alto). No byte alto: bit 7 = Tuning Up, bit 6 = Tuning Down, bits 3-0 = nibble alto da frequencia (bits 11-8).
- $03-$04 - Frequencia canal 2 (mesmo layout de $01-$02).
- $05-$06 - Frequencia canal 3 (idem).
- $07 - Rustle: bit 7 = Tuning Up, bit 6 = Tuning Down, bits 4-0 (so 5 bits, nao 6 como o manual sugere - ver Registradores do PSG) = valor de rustle (0-31).
- $08 - Controle de canais (mixer): bits 0-2 = liga (0) canais de frequencia 1/2/3; bits 3-5 = liga (0) rustle nos canais 1/2/3 (convencao real do PSG: 0 = ligado).
- $09 - Volume canal 1: bit 7 = Tuning Up, bit 6 = Tuning Down, bit 4 = usa envelope de hardware do PSG (Wave ligado), bits 3-0 = volume (0-15).
- $0A - Volume canal 2 (mesmo layout de $09).
- $0B - Volume canal 3 (idem).
- $0C-$0D - Periodo do envelope PSG (regs. 11/12, 12 bits, little-endian) - vai direto pro PSG, sem nenhum processamento de tuning/escala.
- $0E - Forma do envelope PSG (reg. 13) - idem, direto pro PSG.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-api-do-driver-tabela-de-vetores", "", "SEE Tracker: Motor de replay (SEE3PLAY.ASC)", "", "API do driver (tabela de vetores): O driver monta em $C000 (fixo no fonte original) com...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Motor de replay (SEE3PLAY.ASC)

O driver monta em $C000 (fixo no fonte original) com uma tabela de saltos no comeco - o mesmo estilo de API que o NestorBASIC usa (varios pontos de entrada fixos, chamados via DEFUSR/USR() do BASIC):
- +0 SEE_IN - inicializa o driver: confere a identificacao do arquivo (SEE3, so os 4 primeiros bytes), copia o cabecalho pra memoria de trabalho e, se SEETID=0, pendura a rotina principal no hook H_TIMI ($FD9F, interrupcao de VBlank da ROM) - ou seja, por padrao o driver toca sozinho, uma vez por interrupcao, sem precisar de nenhum CALL manual por frame.
- +3 SEE_EX - desliga o driver, silencia o PSG e restaura o hook de interrupcao original.
- +6 SETSFX - inicia um SFX novo (recebe numero do SFX + prioridade); recusa se ja houver um SFX de prioridade maior tocando.
- +9 CUTSFX - corta o SFX atual na hora.
- +12 SEEINT (SEEINT: JP MAIN.A) - ponto de entrada alternativo pra quem quiser chamar o driver a partir da PROPRIA rotina de interrupcao (em vez de deixar o driver usar H_TIMI sozinho) - controlado pela variavel SEETID.

Variaveis de estado (memoria do driver, nao do arquivo .SEE): SEEADR (endereco base de onde o .SEE foi carregado), SEEMAP (pagina de memoria/mapper, trocada e restaurada a cada chamada via IN/OUT ($FE)), SEETID (0=usa H_TIMI sozinho, <>0=temporizacao externa), SEESTA (bits de status: instalado / SFX tocando / ocupado trocando de SFX), SFXPRI (prioridade do SFX atual) e SEEVOL (volume maximo/mestre, 0-15 - ver formula exata abaixo).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-bits-exatos-do-byte-de-evento", "", "SEE Tracker: Motor de replay (SEE3PLAY.ASC)", "", "Bits exatos do byte de evento: O manual descreve o byte de evento como se o nibble alto...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Motor de replay (SEE3PLAY.ASC)

O manual descreve o byte de evento como se o nibble alto inteiro definisse o comando. Lendo o driver (!EVENT:), o que realmente acontece e mais especifico:
AND $70 isola so os bits 6-4 (o bit 7 e ignorado pelo despacho, apesar do manual falar em 'bit 7 high' pra RERUN/END - isso e so uma convencao do EDITOR ao escrever o byte, nao algo que o player verifica).

Valores testados (apos a mascara $70) e o que disparam:
- $00 - nada (so toca os dados PSG do pattern).
- $10 - HALT, valor = bits 3-0 (0-15).
- $20 - FOR, valor = contagem de repeticoes (0-15).
- $30 - NEXT.
- $40 - START.
- $50 - RERUN.
- $60 - TEMPO, valor = bits 3-0.
- qualquer outro resultado da mascara (ou seja, so sobra $70) - cai no EVEND (fim de efeito), o else implicito da cadeia de comparacoes.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-como-for-next-e-start-rerun-realmente-funcionam", "", "SEE Tracker: Motor de replay (SEE3PLAY.ASC)", "", "Como FOR/NEXT e START/RERUN realmente funcionam: Este e o detalhe menos obvio do formato...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Motor de replay (SEE3PLAY.ASC)

Este e o detalhe menos obvio do formato - so aparece lendo o driver com atencao, o manual so mostra o resultado (Pattern 000+001 will be repeated 7 times), nao o mecanismo.

FOR guarda, num dos 4 slots de loop (LOOPNR circula 0-3), o CONTADOR (nibble do evento) e o endereco do proprio pattern onde estava o FOR - nao o pattern seguinte. Isso acontece so na primeira vez que o ponteiro de pattern chega naturalmente nesse endereco.

NEXT decrementa o contador do slot de loop mais recente. Se ainda for maior que zero, ele faz o ponteiro de pattern pular direto pro endereco guardado pelo FOR - mas so pra ler os dados PSG daquele pattern outra vez (o byte de evento do FOR e simplesmente pulado, nao reprocessado - por isso FOR nunca dispara duas vezes pro mesmo slot). No frame SEGUINTE, o ponteiro ja avancou normalmente pro pattern logo depois do FOR (repetindo a sequencia inteira do bloco, um pattern por frame, ate o contador zerar). Quando o contador chega a zero, o slot de loop e liberado e o NEXT deixa o ponteiro simplesmente continuar pro pattern seguinte (normalmente o fim do bloco de repeticao).

START/RERUN funcionam igual, mas mais simples: um so slot global (CLPADR, sem pilha/contador), sem limite de repeticoes - todo RERUN volta pro ultimo START marcado, pra sempre, ate o efeito ser cortado de fora (CUTSFX)/sobrescrito por outro SFX. Isso e o jeito do formato representar a parte 'sustain' (que toca indefinidamente) de um efeito.

Implicacao pratica pra um tracker compativel: ao gerar/interpretar dados .SEE, o pattern que contem FOR/START sempre faz parte da sequencia audivel (seus dados PSG tocam), exatamente como qualquer outro pattern - FOR/START so adicionam o efeito colateral de marcar onde voltar.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-registradores-do-psg-e-o-mapeamento-exato", "", "SEE Tracker: Motor de replay (SEE3PLAY.ASC)", "", "Registradores do PSG e o mapeamento exato: SETPSG percorre os 15 bytes do pattern...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Motor de replay (SEE3PLAY.ASC)

SETPSG percorre os 15 bytes do pattern (pulando o byte de evento) e escreve, nesta ordem, nos 14 registradores do PSG (OUT ($A0)=numero do registrador, OUT ($A1)=dado):
- Regs. 0-5 - frequencia dos 3 canais (12 bits cada, mascarada pra 4 bits no byte alto antes de escrever).
- Reg. 6 - rustle, mascarado com AND $1F (5 bits, 0-31) antes de escrever - o valor real do hardware, mais estreito que os b5-b0 (6 bits) que o manual descreve.
- Reg. 7 (mixer) - bits 0-5 copiados do byte $08 do pattern (AND $3F), com o bit 7 sempre forcado a 1 antes de escrever (detalhe de implementacao deste driver especifico, nao necessariamente um bit de hardware significativo).
- Regs. 8-10 - volume dos 3 canais. Bit 4 (Wave no editor) e o proprio bit M do PSG real (liga o gerador de envelope de hardware em vez de volume fixo) - quando esse bit esta ligado, o driver escreve o byte cru, sem aplicar slide de tuning nem a escala de Max Volume (ver proximo topico): o envelope de hardware manda sozinho.
- Regs. 11-12 - periodo do envelope (copiado cru do pattern, $0C-$0D).
- Reg. 13 - forma do envelope (copiado cru do pattern, $0E).

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-slides-de-afinacao-e-volume-tuning-formulas-exat", "", "SEE Tracker: Motor de replay (SEE3PLAY.ASC)", "", "Slides de afinacao e volume (tuning) - formulas exatas: O driver guarda, numa tabela...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Motor de replay (SEE3PLAY.ASC)

O driver guarda, numa tabela interna (PSGREG), o ultimo valor realmente escrito em cada registrador do PSG. Um slide (D:/U: no editor, bits 6/7 do campo) soma ou subtrai a TAXA (valor do campo no pattern) a esse ultimo valor - ou seja, o slide e sempre relativo ao frame ANTERIOR, nao ao valor bruto do pattern atual.

- Frequencia/rustle, slide pra cima (TUN_UP/TUNWUP) - soma sem nenhum limite (pode estourar o registrador se a taxa for grande demais/o slide for longo demais).
- Frequencia/rustle, slide pra baixo (TUN_DW/TUNWDW) - subtrai sem limite tambem.
- Volume, slide pra baixo (VOL_DW) - diferente: usa uma rotina propria que trava em zero (nao deixa estourar por baixo).
- Volume, slide pra cima usa a mesma TUN_UP generica (sem travar em 15) - uma assimetria real do driver, nao um erro de leitura deste estudo.

Max Volume (SEEVOL): depois do slide (se houver), o volume de 0-15 e escalado assim antes de ir pro PSG:
volume_final = SEEVOL - (15 - volume_bruto), travado em 0 se o resultado for negativo.
Ou seja, com volume_bruto=15 (max), a saida e exatamente SEEVOL; com volume_bruto=0, a saida so deixa de ser 0 se SEEVOL>15 (impossivel, ja que e um valor de 4 bits) - na pratica SEEVOL funciona como um TETO, nao como um multiplicador.

Detalhe fino: o valor guardado de volta na tabela PSGREG (pra alimentar o PROXIMO slide) e o volume antes dessa escala por SEEVOL - ou seja, mudar o Max Volume no meio de um efeito nao contamina a matematica dos slides seguintes, so o volume realmente audivel.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("see-chamando-o-driver-a-partir-do-basic-defusr", "", "SEE Tracker: Integracao com MSX-BASIC", "", "Chamando o driver a partir do BASIC (DEFUSR): O exemplo see/SEE.BAS (BASIC tokenizado)...",
        R"MAN(SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  Integracao com MSX-BASIC

O exemplo see/SEE.BAS (BASIC tokenizado) mostra o padrao de uso pretendido pelos autores - o mesmo estilo BLOAD + DEFUSR/USR() que o NestorBASIC desta IDE ja usa (ver Ajuda -> Nestor Basic...):
1. BLOAD "seebasic.bin",R carrega o driver montado na memoria.
2. Mapeia os pontos de entrada da tabela de vetores (ver API do driver) pra nomes de funcao via DEFUSR=endereco:A=USR(...): ENABLESEE/DISABLESEE (=SEE_IN/SEE_EX), LDSEE (carrega/confere o arquivo .SEE que ja esta em SEEADR), STARTFX/CUTFX (=SETSFX/CUTSFX).
3. Variaveis auxiliares: FXMAP (mapper), FXADR (endereco base do .SEE carregado), FXNUM (numero do SFX a tocar) e SEEFILE (nome do arquivo, gravado nos primeiros bytes reservados pro driver conferir).
4. Fluxo tipico: carregar o driver -> LDSEE -> ENABLESEE (pendura o driver no H_TIMI) -> escrever FXNUM e chamar STARTFX toda vez que quiser tocar um efeito -> DISABLESEE ao sair.

Aviso do proprio exemplo: cuidado pra nao recarregar (BLOAD) o driver por cima dele mesmo enquanto o SEE ainda esta ativo na interrupcao - trocar o codigo de baixo do hook de H_TIMI no meio de uma interrupcao trava a maquina.

VER TAMBEM
  seetracker

FONTE
  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.)MAN");

    add("msxbas2rom", "msx-turbo-basic,msxbas2rom-home", "MSXBAS2ROM: Primeiros passos", "", "Visao geral do projeto: > Compile your MSX BASIC programs into ROMs with ease for use in...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Primeiros passos: Visao geral do projeto

MSXBAS2ROM  MSX BASIC TO ROM COMPILER

> Compile your MSX BASIC programs into ROMs with ease  for use in emulators or real MSX hardware.
> Fast, flexible, and open source.

PROJECT HIGHLIGHTS

- Latest release: see the project Releases page;
- Platforms: Windows (32/64-bit), Linux, macOS;
- Compilation modes:
  - Binary compiled  the program will run faster but there's some compatibility limitations;
  - P-code  listable, highly compatible, but the program will run slower.
- Integration: Works with emulators like WebMSX, OpenMSX, and real hardware via SofaRUN, ODO, etc;
- Open Source: GPL-licensed, contributions welcome.

QUICK REFERENCE

| Section                                                                  | Description |
|--------------------------------------------------------------------------|-------------|
| Installation | OS-specific setup for Windows, Linux, and macOS |
| Getting Started | Install, write your first program, compile, and run |
| Usage         | Command line options, compilation modes |
| Reference Guide | Detailed reference guide for MSXBAS2ROM |
| Examples | Ready-to-run code samples and complete games  |
| Games published | Games made with MSXBAS2ROM |
| Contributing         | How to help the project grow |
| Branding & Credits | Ways to acknowledge MSXBAS2ROM in your project |

HOW IT WORKS

-  How MSXBAS2ROM compiles an MSX BASIC program  *Overview of the compilation process* (Brazilian Portuguese);
-  Creating a game from scratch with MSXBAS2ROM  Part 1  *Step-by-step game development* (Brazilian Portuguese).

More in the Demonstrations page.

GAMES MADE WITH MSXBAS2ROM

Visit the Games page to explore:

- MSX Adventure  Atari 2600 remake game;
- K-Jo Chases the Cheese  maze game;
- The 4 masters of melody Ex  memory game;
- PickinX  arcade adaptation.

WHY USE MSXBAS2ROM?

- Cross-platform: build ROMs from any supported OS;
- 4 compilation modes: flexibility between speed and compatibility;
- ROM-ready output: run in seconds on emulator or cartridge loader;
- Developer-friendly: open source, active maintenance.

PROJECT STATUS

> MSXBAS2ROM is an active, evolving tool  expect updates, new features, and community input.

SUPPORT THE PROJECT

If you enjoy MSXBAS2ROM, consider helping sustain its development:

- ITCH.IO (games developed with MSXBAS2ROM!!!);
- PATREON;
- PAYPAL;
- CATARSE.ME;
- Brazilian PIX: contact me by email for more information.

Follow me on LinkedIn.

LICENSE

Released under the GPL License.

> *MSXBAS2ROM  Bringing MSX BASIC into the ROM era.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-install", "", "MSXBAS2ROM: Primeiros passos", "", "Instalacao: > Get MSXBAS2ROM up and running on your systemwhether through package...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Primeiros passos: Instalacao

INSTALLATION GUIDE  MSXBAS2ROM

> Get MSXBAS2ROM up and running on your systemwhether through package managers or manual downloads.

AVAILABLE INSTALLATION METHODS

1. Linux via PPA (recommended)

For Ubuntu and derivatives, install directly from the official PPA:

sudo add-apt-repository ppa:amaurycarvalho/msxbas2rom
sudo apt-get update
sudo apt-get install msxbas2rom

This ensures MSXBAS2ROM will receive updates automatically.

2. Linux via .DEB or .RPM packages

For most Linux distributions you can install it using Debian or RPM packages downloaded from Releases Page.

1. Debian package:

sudo dpkg -i msxbas2rom.deb

2. RPM package:

sudo rpm -ivh msxbas2rom.rpm

3. Windows, Linux & macOS via Pre-built Releases

Pre-built binaries are available on the GitHub Releases Page.

1. Navigate to the "Assets" section of the latest release;
2. Download the appropriate msxbas2rom executable for your platform;
3. (Optional) Extract the archive if needed;
4. Place the executable in your systems PATHfor example:
   - Linux/macOS: /usr/bin/
   - Windows: %USERPROFILE%\msxbas2rom\ (then add to PATH)

VERIFICATION

After installation, confirm everything is working:

msxbas2rom -v

For parameters help:

msxbas2rom -h

You can also view all available command-line documentation via:

msxbas2rom --doc

These commands verify that the tool is properly installed.

MACOS / SOURCE BUILD INSTRUCTIONS

If no pre-built binaries are available for your systemor you prefer building from sourcefollow these steps:

1. Clone the repository and ensure development dependencies are installed;
2. On macOS, remove or wrap out deprecated includes like malloc.h to avoid build errors;
3. Run the following:

make all

4. Upon success, find the binary under the bin/Release/ directory.

SUMMARY TABLE

| Method                     | Platforms          | Notes                                       |
|----------------------------|-------------------|---------------------------------------------|
| PPA (apt)                  | Ubuntu/Linux      | Auto-updates, simplest for Linux users      |
| DEB or RPM                 | Linux      | Easy way for Linux users      |
| Pre-built Release          | Windows/Linux/macOS     | Manual download, flexible placement         |
| Build from Source (macOS)  | macOS (+ others)  | Recommended if no binaries are available    |

WHATS NEXT?

Proceed to the Getting Started guide to write, compile, and run your first MSX BASIC program using MSXBAS2ROM.

> *MSXBAS2ROM  Bridging MSX BASIC and ROM development with ease.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-gettingstarted", "", "MSXBAS2ROM: Primeiros passos", "", "Primeiros passos: > This guide will walk you through installing MSXBAS2ROM, creating a...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Primeiros passos: Primeiros passos

GETTING STARTED WITH MSXBAS2ROM

> This guide will walk you through installing MSXBAS2ROM, creating a simple MSX BASIC program, compiling it into a ROM, and running it on an emulator or real MSX hardware.

1. INSTALL MSXBAS2ROM

Before starting, make sure you have MSXBAS2ROM installed on your system.

- Windows and Linux installation steps are detailed here: Installation Guide

Once installed, you can verify the tool is available by running:

msxbas2rom -v

If the command displays a version number, youre ready to continue.

2. CREATE YOUR FIRST MSX BASIC PROGRAM

Open your favorite text editor and create a plain text file named hello.bas with the following content:

10 PRINT "HELLO WORLD!"
20 END

Save the file in a working folder.

3. COMPILE THE PROGRAM

MSXBAS2ROM supports 4 compilation modes:

1. Plain ROM compiled mode *(default)*  generates a binary compiled Z80 code plain ROM for better performance (32K or 48K);
   - ExecROM is not compatible with 48K Plain ROM mode, so use SofaRUN or ODO loaders.
2. ASCII8 MegaROM compiled mode  generates a binary compiled Z80 code MegaROM in ASCII8 format (+128K, with a 2048K size limit);
3. Konami SCC MegaROM compiled mode  generates a binary compiled Z80 code MegaROM in Konami SCC format (+128K, with a 2048K size limit);
4. P-code mode (DEPRECATED)  generates a p-coded MSX-BASIC ROM: program remains editable and listable in MSX BASIC interpreter but it will run slower (32K or 48K).

Plain ROM compiled mode (default)
msxbas2rom hello.bas

ASCII8 MegaROM compiled mode
msxbas2rom -x hello.bas

Konami SCC MegaROM compiled mode
msxbas2rom -x --scc hello.bas

P-code mode (deprecated)
msxbas2rom -p hello.bas

After running the command, you will get a file named hello.rom in the same folder.

4. RUN YOUR ROM

Using an Emulator

You can quickly run your ROM in popular emulators:

- WebMSX  just drag and drop the ROM file into emulator screen;
- OpenMSX:
openmsx hello.rom

On Real Hardware

Copy the ROM file to an SD card or storage device supported by your MSX setup and use a loaders such as SofaRUN or ODO.

5. TIPS & NOTES

- File names: Keep your file names MSX-compatible (8.3 format) to ensure proper loading on all systems;
- Testing: Run your ROM on both emulator and real hardware for compatibility;
- Compilation mode choice:
  - Use Plain ROM compiled mode for small or medium projects;
  - Use ASCII8 MegaROM compiled mode for larger projects;
  - Use Konami SCC MegaROM compiled mode for projects that need native SCC support.

See Compiler Architecture for more information.

NEXT STEPS

- Learn more commands and options in the Usage Guide;
- Read the Reference Guide for more detailed information;
- Explore the Examples page for code examples;
- Check out Games published to see whats possible.

> *MSXBAS2ROM  Bringing MSX BASIC into the ROM era.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-usage", "", "MSXBAS2ROM: Primeiros passos", "", "Uso e opcoes de linha de comando: > Learn how to compile MSX BASIC programs into ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Primeiros passos: Uso e opcoes de linha de comando

USAGE GUIDE  MSXBAS2ROM

> Learn how to compile MSX BASIC programs into ROM files.

BASIC SYNTAX

The simplest way to use MSXBAS2ROM is via the command line:

msxbas2rom program.bas

This will convert program.bas into program.rom using default settings.

When compiling, ensure that:

- Source file is in plain text MSX BASIC format (.BAS);
- Assets (images, sprites, sounds) are in accessible paths if referenced;
- Output path is writable.

SYNTAX AND OPTIONS

msxbas2rom [options]

- General options:

| Option          | Description                                                                 |
|-----------------|----------------------------------------------------------------------------|
| -h or --help | help |
| -q or --quiet | quiet (no verbose) |
| -d or --debug | debug mode (show details) |
| -D or --doc | display quick reference guide |
| -H or --history | display app history |
| -v or --version | display app version |

- Compile options (optional):

| Option          | Description                                                                 |
|-----------------|----------------------------------------------------------------------------|
| -c | plain ROM compile mode (DEFAULT) |
| -a or --auto | auto mode (fallback to ASCII8 when plain ROM overflows) |
| -x or -8 or --ascii8 | ASCII8 MegaROM mapper |
| -6 or --ascii16 | ASCII16 MegaROM mapper (limited support) |
| -7 or --ascii16x | ASCII16-X MegaROM mapper (limited support) |
| -4 or --konami | Konami MegaROM mapper |
| -k or --scc | Konami SCC MegaROM MegaROM mapper |

- Path options (optional):

| Option          | Description                                                                 |
|-----------------|----------------------------------------------------------------------------|
| -i | input path (default=source file path) |
| -o | output path (default=source file path) |

- Special options (optional):

| Option          | Description                                                                 |
|-----------------|----------------------------------------------------------------------------|
| -s | generate symbols for Z80 debugging (default format: .noi) |
| --noi or --noice | generate symbols in .noi format (OpenMSX) |
| --cdb | generate symbols in .cdb format (sdcc) |
| --symbol | generate symbols in .symbol format (pasmo) |
| --omds | generate symbols in .omds format (openMSX deprecated) |
| --lin | write the MSX-BASIC line numbers in the binary code |
| --vscode | initialize a VSCode MSX-BASIC project in the current path |

- P-code options (DEPRECATED):

| Option          | Description                                                                 |
|-----------------|----------------------------------------------------------------------------|
| -p | tokenized p-code mode |
| -t | turbo mode (or use CALL TURBO instructions) |
| --nsr | no strip remark lines (tokenized/turbo mode) |

Output:

``

You can combine multiple options in a single command.

EXAMPLES

1. Show help

msxbas2rom -h

2. Show version

msxbas2rom -v

3. Show documentation

msxbas2rom --doc

4. Compile with Default Settings

msxbas2rom program.bas

It's equivalent to code below:

msxbas2rom -c program.bas

Also, you can send the ROM output to a target folder:

msxbas2rom -o /target_folder/subfolder program.bas

5. Compile to ASCII8 MegaROM

msxbas2rom -x program.bas

6. Compile to Konami SCC MegaROM

msxbas2rom -x --scc program.bas

7. Compile and generate symbols for OpenMSX debugger

Command lines below will generate the following symbols files:

- .symbol: pasmo assembler symbol format;
- .omds: xml format for use with the old OpenMSX standalone debugger or import it in another tools;
- .noi: NoICE format for use with the new OpenMSX integrated debugger.

msxbas2rom -s program.bas
msxbas2rom -x -s program.bas
msxbas2rom -x -s --scc program.bas

Learn more about debugging your MSX-BASIC program here.

NEXT STEPS

- Read the Reference Guide for more detailed information;
- Explore the Examples page for code examples;
- Check out Games published to see whats possible.

> *MSXBAS2ROM  From BASIC to ROM in a single command.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-documentation", "", "MSXBAS2ROM: Guia de referencia", "", "Visao geral da referencia: Welcome to the MSXBAS2ROM Reference Guide. This section...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Visao geral da referencia

DOCUMENTATION OVERVIEW

Welcome to the MSXBAS2ROM Reference Guide. This section gathers all the essential documentation to help you understand, explore, and master the compilers features.

Use the links below to dive into specific topics:

DOCUMENTATION SECTIONS

- Coding Limitations and Differences
  Learn how to compile BASIC programs into ROMs, supported statements, data types, and hardware requirements.

- Resource Directives
  Understand how to include external files, handle assets, and manage project resources.

- MSX Tile Forge Support
  Design screen maps with MTF and use it in your program.

- nMSXTiles Support
  Design screens with nMSXTiles and use it in your program.

- Tiny Sprite Support
  Design sprites easily with Tiny Sprite tool.

- Arkos Tracker Music Support
  Integrate AT music with commands like CMD PLYLOAD, CMD PLYPLAY, and customize playback.

- Extended Commands
  Discover compiler-specific commands (SCREEN LOAD, CMD RESTORE, etc.) that expand the original MSX BASIC capabilities.

- Extended Functions
  Explore powerful functions such as HEAP(), MSX(), collision helpers, and more.

- VSCode integration
  Brings a modern development workflow to your MSX BASIC project.

- Debugging with OpenMSX Debugger
  Debugging your MSX BASIC program compiled with MSXBAS2ROM.

- Compiler Architecture
  A deeper look at the compilers internal flow and design principles.

- Getting Help & References
  How to use built-in docs, find references, and get support.

NOTES
- This documentation is constantly evolving;
- If you spot mistakes or have suggestions, feel free to contribute.

> *MSXBAS2ROM  powering your creativity, one line of BASIC at a time.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-compiling-code", "", "MSXBAS2ROM: Guia de referencia", "", "Limitacoes e diferencas ao compilar: This section explains how to compile MSX BASIC...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Limitacoes e diferencas ao compilar

COMPILING CODE

This section explains how to compile MSX BASIC programs into ROMs using MSXBAS2ROM.

BASIC COMPILATION
msxbas2rom [options]

- Generates `` as output.
- Default mode is compile (-c) generating a 32k or 48k ROM file.

EXAMPLES
msxbas2rom mygame.bas
msxbas2rom -c mydemo.bas

For advanced options, see the Usage Guide.

STATEMENTS SUPPORTED:

     DEFDBL, DEFINT, DEFSNG, DEFSTR, DIM,
     REM, END, CLS, BEEP, PRINT, INPUT, LINE INPUT,
     GOTO, GOSUB, RETURN, SOUND, PLAY,
     RANDOMIZE, LET, FOR/NEXT, DATA, READ, RESTORE,
     IF/THEN/ELSE, IF/GOTO/ELSE, IF/GOSUB/ELSE,
     LOCATE, COLOR, SCREEN, WIDTH, PSET, PRESET,
     LINE, CIRCLE, PAINT, DRAW, OUT, POKE, VPOKE
     SPRITE$, PUT SPRITE, ON...GOSUB, ON...GOTO,
     SWAP, WAIT, ON INTERVAL GOSUB, INTERVAL ON/OFF,
     ON KEY GOSUB, KEY ON/OFF, KEY() ON/OFF,
     ON STRIG GOSUB, STRIG() ON/OFF,
     ON SPRITE GOSUB, SPRITE ON/OFF,
     SET SCROLL, SET PAGE, PUT TILE,
     SET TILE COLOR, SET TILE PATTERN,
     DEF USR, PRINT USING

STATEMENTS SUPPORTED PARTIALLY ONLY:

     STOP, STOP ON/OFF, ON STOP GOSUB,
     COPY, OPEN, CLOSE, MAXFILES, BLOAD,
     SET VIDEO, COPY SCREEN,
     CLEAR

FUNCTIONS SUPPORTED:

     INT, FIX, RND, SIN, COS, TAN, ATN, EXP,
     LOG, SQR, VAL, TIME, SGN, ABS, ASC, INKEY,
     LEN, CSNG, CDBL, CINT, CHR$, INKEY$, INPUT$,
     SPACE$, SPC, TAB, STRING$, STR$, LEFT$, RIGHT$, MID$,
     INSTR, OCT$, HEX$, BIN$, PEEK, VPEEK, INP, POS, LPOS,
     CSRLIN, STRIG, STICK, PDL, PAD, BASE, VARPTR,
     PLAY, VDP, POINT, COLLISION, FRE, HEAP, TILE,
     RESOURCE, USR, PSG, MSX, NTSC, TURBO, MAKER,
     USING$, PLYSTATUS

     TIME and MID$ assignments
     VDP assignment

DATA TYPES, OPERATORS AND OPERATIONS SUPPORTED:

     Integer, Float, String, Fixed Bidimensional Arrays
     Math expressions and basic operators (+-*/^)
     OR, AND, XOR, EQV, IMP, NOT, MOD, SHL, SHR
     &h0000, &o0000, &b00000000

Internal Data Types

| BASIC Type | Example | Internal Representation | Range | Notes |
|-------------|----------|--------------------------|-------|-------|
| Integer (%) | A% = 42 | 16-bit signed integer | -32768 to +32767 | Fastest type; ideal for counters, indexes, logic control |
| Single (!) | A! = 3.1415 | 24-bit IEEE-like real | about 4.5 digits | Used for precise math operations; slower |
| Double (#) | A# = 3.1415 |  |  | Same as Single; Double precision is not available. |
| String ($) | A$ = "HELLO" | 1 byte length + 255 bytes data | 0-255 characters | Sequences of ASCII characters (pascal style); flexible but heavier |
| Array | DIM A(10) or DIM B$(5) | Linear contiguous memory structures | RAM available space | Supports both numeric and string arrays; 2 dimension only |

 Notes:

- Integers are 16-bit signed values stored in little-endian order (LSB + MSB). This type is optimized for speed and memory efficiency, being the default choice for most arithmetic and control structures;
- Single and Double are floating numbers in a special format 3-byte value. It's accuracy is about 4.5 digits. Double precision is not available. Slower than integers due to floating-point math library calls. Prefer integer arithmetic for loops and counters when possible;
- Strings store sequences of ASCII characters (pascal style). Use short, fixed-length strings when possible. Avoid heavy concatenations inside loops;
- Arrays are linear contiguous memory structures, with each element represented by the base type (integer, float, or string).

LIMITATIONS AND DIFFERENCES:

Some tradeoffs and differences from MSX BASIC's behavior were necessary to improve the fast execution of compiled code. One of these was the imposition of a strong type system.

See other important points below.

     - Minimum hardware requirements for compiled programs:
       * MSX1 machine (with ROM BIOS and ROM BASIC);
       * 16K of RAM for running as Cartridge without resources;
       * 32K of RAM for running as Cartridge with resources;
       * 64K of RAM for running with ExecROM (for 32K ROMs) or ODO
         (for 48K ROMs) via disk driver or Caslink3/CASDuino/MSX2CAS
         via cassete;
       * 64K Memory Mapper and/or MegaRAM for running with SofaRun;
       * MegaROM compiled mode needs a Memory Mapper and/or MegaRAM with
         at least the same size as the final ROM for running it in memory
         with ExecROM (note: not required for running as ASCII8 or Konami SCC
         Cartridge in emulators);
     - Partial file support (OPEN "GRP:", PRINT #...);
     - No dynamic arrays (REDIM);
     - No variant data type (only string, integer and single);
     - All variables has an unique data type (dont change in runtime);
     - Variables are not initialized at startup (do it yourself);
     - Default data type is single (use DEFINT A-Z to modify it to integer);
     - Singles cannot be used with boolean operations;
     - Printing numerical decimal values above 9999 will result in
       scientific notation;
     - Printing will not wrap the full numeric data on the next line,
       instead it will just wrap the characters that don't fit.
     - CIRCLE tracing and aspect parameters not supported;
     - STOP works like END;
     - CLEAR works without parameters;
     - LOCATE/PRINT works in graphical mode (screen 2+) without use of
       OPEN 'GRP:'. When in screen 2 tiled mode, it works similar as text
       mode coords (faster output);
     - COPY works only with graphical parameters (no files support) just
       like: COPY (x0,y0)-(x1,y1) TO (x2,y2);
       COPY ... TO  / COPY  TO ... is accepted
       instead of COPY ... (try HEAP() as );
       When in screen 2 tiled mode, it works with text mode coords;
     - BLOAD works only to load standard screen files (.SCn) and TinySprite
       compatible files (.SPR). You need to inform a file name as literal (no
       via variables) because it will be loaded into ROM (it will activate
       MegaROM mode). Ex: BLOAD "file.sc5", S;
     - DATA statements allocate space in resources page and will be compiled to
       a 48K ROM when Plain ROM mode;
     - PRINT USING support only numerical formatting symbols (#+-.*$,^0) and
       works the same way as USING$() function. For exponential display,
       include more # in the format string until the correct value is
       displayed. Use 0 char symbol to fill the value with left zeros;
     - SET/GET DATE and SET/GET TIME adopts a customized syntax;
     - F1..F10 function keys return ASCII codes 246 to 255 respectively;
     - Spaces on your line code, REMs and blank lines not affect compiled code
       size, but if your include it on your program it can help the compilation
       process (not need for strip spaces from your code anymore);
     - Compiled code on Plain ROM mode is limited to a maximum of 16K. There's no
       specific limitation on MegaROM mode;
     - Resources total size on Plain ROM mode are limited to 16K (for FILE directive), and
       when compiled generates 48K ROMs thats can run as cartridge or in RAM
       with SofaRUN or ODO loaders (ExecROM do not support this ROM size). On MegaROM mode,
       where the ROM size limit is 2048K, the theorical limitation is around to 3200 resources
       (v0.3.3.2 and above).

STATEMENTS NOT SUPPORTED:

     AUTO, BLOAD, BSAVE, CALL, CLOAD,
     CONT, CSAVE, DEF FN, DELETE, EOF, ERASE,
     ERL, ERR, ERROR, FILES, FN, FPOS, FIELD,
     GET, GET DATE, GET TIME, IPL, LIST, LLIST,
     LOAD, LOC, LSET, LPRINT, MERGE, RSET,
     MOTOR, NEW, PUT, PUT KANJI, RENUM, RUN, SAVE,
     SET ADJUST, SET BEEP, SET DATE, SET PASSWORD,
     SET PROMPT, SET SCREEN, SET TIME,
     SET TITLE, TROFF, TRON,
     RESUME, ON ERROR GOTO

 See also Compiler Architecture, Resource Directives, Extended Commands and Extended Functions.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-resource-directives", "", "MSXBAS2ROM: Guia de referencia", "", "Diretivas de recursos (assets): MSXBAS2ROM allows embedding and managing external...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Diretivas de recursos (assets)

RESOURCE DIRECTIVES

MSXBAS2ROM allows embedding and managing external resources in your project.

WHAT ARE RESOURCES?
Resources are assets bundled into the ROM:
- Data files (*.txt, *.csv);
- Screen files (*.SC2, *.SC5, *.SCn...);
- Screen palettes, tilesets and maps (*.SC4Pal, *.SC4Tiles, *.SC4Map);
- Sprites (*.spr);
- Packed tilesets (*.chr.plet5, *.clr.plet5, *.scr.plet5);
- Music and sound effects (*.akm, *.akx);
- Binary data (*.bin).

USAGE

Syntax:
TEXT
FILE
INCLUDE

Example:
FILE "SCREEN.SC1"  ' item 0 on resource list
FILE "TEXT.TXT"    ' item 1 on resource list
10 SCREEN 1
20 SCREEN LOAD 0   ' display a screen from resource 0
30 CMD RESTORE 1   ' load text data from resource 1
40 READ MSG$       ' read the first text line from resource 1
50 PRINT MSG$      ' print the line

NOTES
- Resources are stored in ROM and accessible at runtime;
- Resources total size on Plain ROM mode are limited to 16K (for FILE directive), and when compiled generates 48K ROMs thats can run as cartridge or in RAM with SofaRUN or ODO loaders (ExecROM do not support this ROM size);
- On MegaROM mode, where the ROM size limit is 2048K, the theoretical limitation is around to 3273 resources (v0.3.3.2 and above);
- Check Extended Functions for functions that interact with resources.

Files types
MSX BASIC program sources
- All .BAS files are in plain text format, so they can be opened by any text editor.
Screen resources
- .SCn are MSX BASIC "BSAVE,S" native binary format;
  - Screen files can be created and exported to .SCn using nMSXTiles;
  - Also, image files can be converted and exported to .SCn using MSX Screen Converter.
- .SC4Pal files are palette data created with MSX Tile Forge;
- .SC4Tiles files are tileset data created with MSX Tile Forge;
- .SC4Map files are screen maps created with MSX Tile Forge.
Sprite resources
- .SPR files are in plain text format and can be opened by any text editor and edited with Tiny Sprite as well.
Sound resources
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3.

RECOMMENDED RESOURCES STACK

- MSXBAS2ROM;
- Arkos Tracker 3;
- MSX Tile Forge;
- MSX Tiny Sprite.

Additional tools recommended:

- MSX Screen Converter;
- nMSXTiles for Linux;
- nMSXTiles for Windows and macOS.

LEARN MORE:

- MSX Tile Forge Support
  Design screen maps with MTF and use it in your program.

- Tiny Sprite Support
  Design sprites easily with Tiny Sprite tool.

- Arkos Tracker Music Support
  Integrate AT music with commands like CMD PLYLOAD, CMD PLYPLAY, and customize playback.

- Compiler Architecture

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-extended-commands", "", "MSXBAS2ROM: Guia de referencia", "", "Comandos estendidos: MSXBAS2ROM extends MSX BASIC with new commands.",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Comandos estendidos

EXTENDED COMMANDS

MSXBAS2ROM extends MSX BASIC with new commands.

WHY USE EXTENDED COMMANDS?
- Performance optimizations;
- Features unavailable in classic BASIC;
- Better integration with modern tools.

EXAMPLES

General Extended Commands

10 CMD TURBO 1
20 IF TURBO() THEN PRINT "TURBO MODE IS ON" : END
30 PRINT "CANNOT START TURBO MODE (IS IT A TURBO R OR PANASONIC MACHINE?)"

Memory Extended Commands

10 SCREEN 1
20 BA% = BASE(5)                           ' screen 1 name table vram address
30 M$ = "HELLO WORLD"
40 S% = LEN(S$)
50 MA% = VARPTR(M$) + 1                    ' message address in RAM
60 CMD RAMTORAM MA%, HEAP(), S%            ' copy message to next free memory in RAM
70 CMD RAMTOVRAM HEAP(), BA%, S%           ' copy message to screen
80 END

Screen Extended Commands

FILE "SCREEN.SC2"   ' resource 0
FILE "SCREEN.SC5"   ' resource 1
10 SCREEN 2
20 SCREEN OFF
30 SCREEN LOAD 0
40 SCREEN ON
50 A$ = INPUT$(1)
60 SCREEN 5
70 SCREEN OFF
80 SCREEN LOAD 1
90 SCREEN ON
100 A$ = INPUT$(1)

Screen Tiled Mode and Fonts Extended Commands

10 SCREEN 2
20 SET FONT 1
30 PRINT "HELLO WORLD"
40 A$ = INPUT$(1)
50 END

Sprite Extended Commands

FILE "SPRITE.SPR"
10 SCREEN 2
20 SPRITE LOAD 0
30 PUT SPRITE 0,(100,100),15,0
40 IF INKEY = 0 THEN 40
50 END

Data Commands

Reading plain text files

FILE "TEXT.TXT"
10 CMD RESTORE 0
20 READ L$ : PRINT L$
30 READ L$ : PRINT L$
40 RESTORE 0   ' reposition to first line of file
50 READ L$ : PRINT L$

Reading CSV files

FILE "DATA.CSV"
10 CMD RESTORE 0
20 READ S$
30 IF S$="*" THEN END
40 PRINT S$
50 GOTO 20

Reading binary files as integers

FILE "data1.bin"
FILE "data2.bin"
FILE "data3.bin"
10 CMD RESTORE 0  ' 1st file resource
15 IRESTORE 0     ' reposition to file start (byte %0)
20 IREAD A%, B%
30 PRINT A%, B%
40 CMD RESTORE 1  ' 2nd file resource
45 IRESTORE 2     ' reposition to byte #2
50 IREAD A%, B%
60 PRINT A%, B%
70 CMD RESTORE 2  ' 3rd file resource
75 IRESTORE 4     ' reposition to byte #4
80 IREAD A%, B%
90 PRINT A%, B%

COMPLETE LIST

General Extended Commands

  Turn off keyboard clicks alternative

    CMD KEYCLKOFF

  Clear keyboard buffer and reset joysticks ports

    CMD CLRKEY

  Activate cpu turbo mode (R800 or 5.37mhz)

    CMD TURBO <0=off | 1=on>

    Note: works only with TurboR or MSX2 Panasonic machines.

  Exec a binary uncompressed resource as an assembly code

    CMD RUNASM

    - There is a limit of 256k for the Assembly code;
    - All jumping instructions must be relative (JRs instead of JPs);
    - No CALLs to addresses inside the program are allowed.

  Exec a text resource as a plain text basic code (must start with ':')

    CMD RUNBAS

  Play a text resource with Basic

    CMD PLAY [, ]

    note: channel C will be disabled to use with SOUND command

  Mute the PSG (for PLAY use)

    CMD MUTE

  Draw a text resource with Basic

    CMD DRAW

  SET/GET DATE/TIME customized syntax

     SET DATE iYear%, iMonth%, iDay%
     SET TIME iHour%, iMinute%, iSecond%
     GET DATE iYear%, iMonth%, iDay% [, iWeekDay% [, iDateFormat%]]
     GET TIME iHour%, iMinute%, iSecond%

     Code Date Format
     0    YY/MM/DD
     1    MM/DD/YY
     2    DD/MM/YY

     Week day: 0=Sunday

Memory Extended Commands

  Write RAM to VRAM address

    CMD RAMTOVRAM , ,

  Write VRAM to RAM address

    CMD VRAMTORAM , ,

  Write RAM to RAM address

    CMD RAMTORAM , ,

  Write RESOURCE to RAM address

    CMD RSCTORAM ,  [, ]

Screen Extended Commands

  Inhibits the screen display

    SCREEN OFF

    Also: CMD DISSCR

  Display the screen

    SCREEN ON

    Also: CMD ENASCR

  Enable/disable a page alternating effect on screen mode 5 and above via VDP R#1 (Cadari Bit) and R#13. Pass 0 to stop the effect.

    CMD PAGE ,  [, ]

  Load and display a screen resource

    SCREEN LOAD

Screen Tiled Mode and Fonts Extended Commands

  Fill screen 2 with spaces to use with SETFNT/PRINT

    CMD CLRSCR

  Load internal FONTs to VRAM font pattern table (screen mode >= 1)

    SET FONT [, ]
      0 = BIOS default font
      1 = Compile Zanac Style
      2 = Konami Gradius Style 1
      3 = Konami Gradius Style 2

    Also: CMD SETFNT [, ]

  Update tiled font color with current fore/background COLOR

    CMD UPDFNTCLR

  Set clipping on and off (screen modes 5 through 8, and same as '#C of xbasic)

    CMD CLIP <0=off | 1=on>

  Put a tile character into screen position (for tiled mode, screens 0-2)

    PUT TILE , (,)

  Set/Get tile color (for tiled mode, screens 1, 2 and 4)

    SET TILE COLOR , ,  [, ]

    SET TILE COLOR , (,...,) [, (,...,) [, ]]

    SET TILE COLOR , <4 integers color buffer array source> [, ]

    GET TILE COLOR , <4 integers color buffer array dest> [, ]

  Set/Get tile pattern (for tiled mode, screens 0, 1, 2 and 4)

    SET TILE PATTERN , (,...,) [, ]

    SET TILE PATTERN , <4 integers pattern buffer array source> [, ]

    GET TILE PATTERN , <4 integers pattern buffer array dest> [, ]

  Flip a tile pattern

    SET TILE FLIP ,  [, ]

  Rotate a tile pattern

    SET TILE ROTATE ,  [, ]

  Enable/Disable tiled mode (screens 2 and 4, for use with PRINT)

    SET TILE

  Copy screen to array (only for modes 1, 2 and 4)

    SCREEN COPY TO  [SCROLL ]

    *direction = same as STRIG

  Copy array to screen (only for modes 1, 2, and 4)

    SCREEN PASTE FROM

  Do a screen scroll (only for modes 1, 2 and 4)

    SCREEN SCROLL

    *direction = same as STRIG

Sprite Extended Commands
> See a more detailed example for SET/GET SPRITE here and here.

  Load a TinySprite (msx.jannone.org) compatible resource (.SPR)

    SPRITE LOAD

    - Export your sprites as a Backup and save it as .SPR text file;
    - Use with sprite parameter size 2 or 3 on SCREEN statement;
    - You can use, alternatively: BLOAD "file.spr",S

  Set/Get sprite color

    SET SPRITE COLOR , <8 integers color buffer array>
    GET SPRITE COLOR , <8 integers color buffer array>
      Example:
        10 DIM CB%(7)
        20 GET SPRITE COLOR 0, CB%
        30 SET SPRITE COLOR 1, CB%

  Set/Get sprite pattern

    SET SPRITE PATTERN , <16 integers pattern buffer array>
    GET SPRITE PATTERN , <16 integers pattern buffer array>
      Example:
        10 DIM PB%(3,3)
        20 GET SPRITE PATTERN 0, PB%
        30 SET SPRITE PATTERN 1, PB%

  Flip a sprite pattern

    SET SPRITE FLIP ,

  Rotate a sprite pattern

    SET SPRITE ROTATE ,

Data Commands

  Set READ statement to get lines from a text resource file (*.TXT or*.CSV) informed on FILE directive

    CMD RESTORE

    - Use normal RESTORE  and READ statements to read any specific line from the resource text;
    --- Line number zero is the first line on the resource text;
    --- Each line on TXT files will be treated like an unique string;
    --- Each line on CSV files will be treated like a DATA content;
    - Use IRESTORE  and IREAD to read binary resources.

Additional Extended Commands

  Write a compressed resource (pletter) to VRAM address

    CMD WRTVRAM ,

  Write a compressed .ALF resource (pletter) to VRAM font pattern table

    CMD WRTFNT

  Write a compressed resource (pletter) to VRAM tile pattern table

    CMD WRTCHR

  Write a compressed resource (pletter) to VRAM tile color table

    CMD WRTCLR

  Write a compressed resource (pletter) to VRAM screen table

    CMD WRTSCR

  Write a TinySprite (msx.jannone.org) compatible resource (.SPR) to VRAM sprite pattern and color tables

    CMD WRTSPR

    - Export your sprites as a Backup and save it as .SPR text file;
    - Use with sprite parameter size 2 or 3 on SCREEN statement;
    - You can use, alternativally: BLOAD "file.spr",S

  Write a compressed resource (pletter) to VRAM sprite pattern table

    CMD WRTSPRPAT

  Write a compressed resource (pletter) to VRAM sprite color table

    CMD WRTSPRCLR

  Write a compressed resource (pletter) to VRAM sprite attribute table

    CMD WRTSPRATR

NOTES

     - LOCATE/PRINT works in graphical mode (screen 2+) without use of
       OPEN 'GRP:'. When in screen 2 tiled mode, it works similar as text
       mode coords (faster output);
     - CMD WRTSCR/CMD WRTCHR activate the tiled mode in screen 2, forcing text
       mode coords;
     - PUT TILE , (x,y) - for tiled mode used with screens 0-2;
     - RANDOMIZE statement (without parameters) works like a RND(-TIME)
       function;
     - COPY works only with graphical parameters (no files support) just
       like: COPY (x0,y0)-(x1,y1) TO (x2,y2);
       COPY ... TO  / COPY  TO ... is accepted
       instead of COPY ... (try HEAP() as );
       When in screen 2 tiled mode, it works with text mode coords;
     - BLOAD works only to load standard screen files (.SCn) and TinySprite
       compatible files (.SPR). You need to inform a file name as literal (no
       via variables) because it will be loaded into ROM (it will activate
       MegaROM mode). Ex: BLOAD "file.sc5", S;
     - IDATA (integer data) and IREAD (integer read) works similar as DATA/READ
       but use less ROM memory and its faster. Use IRESTORE  to
       change the pointer position on binary files resources;
     - DATA statements allocate space in resources page and will be compiled to
       a 48kb ROM;
     - PRINT USING support only numerical formatting symbols (#+-.*$,^0) and
       works the same way as USING$() function. For exponential display,
       include more # in the format string until the correct value is
       displayed. Use 0 char symbol to fill the value with left zeros;
     - F1..F10 function keys return ASCII codes 246 to 255 respectively;
     - Resources total size are limited to 16kb (for FILE directive), and
       when compiled generates 48kb ROMs thats can run as cartridge or in RAM
       with SofaRUN or ODO loaders (ExecROM do not support this ROM size);

See also: Extended Functions and Arkos Tracker Support Music.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-extended-functions", "", "MSXBAS2ROM: Guia de referencia", "", "Funcoes estendidas: Functions provide powerful ways to retrieve information or extend MSX...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Funcoes estendidas

EXTENDED FUNCTIONS

Functions provide powerful ways to retrieve information or extend MSX BASIC behavior.

WHY USE EXTENDED FUNCTIONS?
- Simplify complex routines;
- Provide compiler-optimized alternatives.

EXAMPLES
- HEAP()  return the RAM free area start address;
- MSX()  return the machine MSX version;
- COLLISION(s1,s2)  sprite collision helper.

COMPLETE LIST

**General Functions**

    - FRE() return free RAM size only (doesnt accept parameters);
    - HEAP() return first free RAM address;
    - TILE(x,y) return character from position (screens mode 0-2, text coords);
    - MSX() return current machine version (0: MSX1, 1: MSX2,
      2: MSX2+, 3: MSXturboR);
    - NTSC() return true to a NTSC (or PAL-M) machine and false to a PAL one;
    - TURBO() return true to cpu turbo mode (R800 or 5.37mhz) or false
      to standard mode (Z80/3.57mhz). Use with CMD TURBO...
    - VDP() without parameters return VDP version (0: TMS9918A, 1: V9938,
      2: V9958, x: VDP ID);
    - MAKER() return the manufacturer ID [1:ASCII/Microsoft, 2:Canon, 3:Casio,
      4:Fujitsu, 5:General, 6:Hitachi, 7:Kyocera, 8:Matsushita (Panasonic),
      9:Mitsubishi, 10:NEC, 11:Nippon Gakki (Yamaha), 12:JVC, 13:Philips,
      14:Pioneer, 15:Sanyo, 16:Sharp, 17:SONY, 18:Spectravideo, 19:Toshiba,
      20:Mitsumi, 21:Telematica, 22:Gradiente, 23:Sharp Brazil,
      24:GoldStar(LG), 25:Daewoo, 26:Samsung,
      212:1chipMSX/Zemmix Neo(KdL firmware)];
      Works only in some MSX2 machines;

**Data Functions**

    - INKEY() is an alternative to INKEY$, but returning an integer instead;
    - IPEEK()/IPOKE is similar to PEEK()/POKE, but applied for integer data;
    - USING$(format$, number) works just like PRINT USING statement;

**Music Functions**

    AKM PLAYER STATUS (bit 7 = end of song reached, bit 0 = loop status)

        = PLYSTATUS()

    Read PSG register

       n = PSG(  )

**Sprite Collision Detection Functions**

    - COLLISION() return if any sprite collided with another sprite, else
      return -1;
    - COLLISION() return if a sprite  collided with another sprite,
      else return -1;
    - COLLISION(,) return n2 if sprite n1 collided with n2, else
      return -1;

    <-1=no collision|collided sprite number> = COLLISION( <-1=any sprite | sprite1> [,  ] )

Examples

       Beep if any sprite collided with each other:
          SN# = COLLISION(-1)
          IF SN# >= 0 THEN BEEP
       Beep if any sprite collided with sprite 2:
          SN# = COLLISION(2)
          IF SN# >= 0 THEN BEEP
       Beep if sprite 4 collided with sprite 5:
          SN# = COLLISION(5)
          IF SN# = 4 THEN BEEP
       Beep if sprite 5 collided with sprite 4 (direct test):
          SN# = COLLISION( 5, 4 )
          IF SN# >= 0 THEN BEEP
       Beep if sprite 0 collided with sprite 1 (direct test):
          SN# = COLLISION( 0, 1 )
          IF SN# >= 0 THEN BEEP

Notes

- Sprites with same X and Y position are considered the same object, thus there's no collision in this case.

**Resource Functions**

    Get resource data address

        = RESOURCE()
       - RESOURCE(number) return resource start address (use with COPY);

    Get resource data size

       = RESOURCESIZE()

**Deprecated Functions (for tokenized mode)**

    Resource number

        = USR0()

    Resource size

        = USR1()

    PLAY() function alternative

       <0=false> = USR2(0)

    INKEY$ function alternative (for tokenized mode)

        = USR2(1)

    INPUT$(1) function alternative (for tokenized mode)

        = USR2(2)

    Sprite collision

       <-1=no collision|collided sprite number> = USR3( <-1=any sprite | sprite1 | &h1122> )

    Arkos Tracker Status

        = USR2(3)

See also Arkos Tracker Music Support.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-music-support", "", "MSXBAS2ROM: Guia de referencia", "", "Musica (Arkos Tracker): MSXBAS2ROM integrates Arkos Tracker music directly into compiled...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Musica (Arkos Tracker)

ARKOS TRACKER MUSIC SUPPORT

MSXBAS2ROM integrates Arkos Tracker music directly into compiled ROMs.

SOME SUPPORTED COMMANDS
- CMD PLYLOAD ,
- CMD PLYSONG
- CMD PLYPLAY
- CMD PLYMUTE

EXAMPLE
FILE "music.akm"           ' resource 0
FILE "sound_effects.akx"   ' resource 1
10 CMD PLYLOAD 0, 1
10 CMD PLYSONG 0
20 CMD PLYPLAY
30 A$ = INPUT$(1)
40 CMD PLYMUTE
50 END

More examples

More downloadable code examples can be found here.

COMPLETE LIST

  Load an AKM (songs) and AKX (effects) uncompressed files resources in memory setting up the first song to play

    CMD PLYLOAD  [, ]

  Initialize a song from the previous AKM resource loaded

    CMD PLYSONG []

  Play/continue a song in memory

    CMD PLYPLAY

  Mute/pause a song in memory

    CMD PLYMUTE

  Play a sound effect from the previous AKX resource loaded

    CMD PLYSOUND  [,  [, ] ]

  Set the song loop status

    CMD PLYLOOP <0=off | 1=on>

  Restart a song in memory (after stopped by loop turned off)

    CMD PLYREPLAY

NOTES

- Arkos Tracker player use the minimalist binary file (AKM and AKX), with songs composed to play at 50hz, exported at 0100 address, and must be included as the first resource in the list (AKM as 1st and AKX as 2nd);
- PT3 player support is now deprecated, dont use it. Also, it cannot be used in concurrence with Arkos Tracker 2 player too.

>  Learn more in Extended Commands.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-mtf-support", "", "MSXBAS2ROM: Guia de referencia", "", "Mapas de tela (MSX Tile Forge): MSXBAS2ROM integrates MSX Tile Forge screen maps directly...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Mapas de tela (MSX Tile Forge)

MSX TILE FORGE SUPPORT

MSXBAS2ROM integrates MSX Tile Forge screen maps directly into compiled ROMs.

COMMAND SYNTAX

MTF - Map Transfer

Loads palette/tileset resources into VRAM or copies map data to the screen.

Syntax

  MTF
  MTF ,[,]

Operations

  0 (Palette/Tileset) - Load resource into VRAM

      Loads a palette or tileset resource into VRAM.

      MTF [, 0, 0, 0, 0, 0, 0, 0, ]

      page            Destination screen page (default=0, for screen 4 only).

  0 (Map) - Full map copy using relative screen coordinates (default)

      Copies a full screen from the map using screen-based coordinates.

      MTF
      MTF , 0, , [, 0, 0, 0, 0, ]

      horizontal_col  Horizontal screen position in the map (default=0).
      vertical_row    Vertical screen position in the map (default=0).
      page            Destination screen page (default=0, for screen 4 only).

  1 (Map) - Full map copy using absolute tile coordinates

      Copies a full screen from the map starting at an absolute map position.

      MTF , 1, , [, 0, 0, 0, 0, ]

      map_x           Absolute X coordinate in the map (default=0).
      map_y           Absolute Y coordinate in the map (default=0).
      page            Destination screen page (default=0, for screen 4 only).

  2 (Map) - Partial map copy (window)

      Copies a rectangular area from the map to a screen position.

      MTF ,2,
          ,,
          ,,
          ,
          [,]

      map_x           Source X coordinate in the map.
      map_y           Source Y coordinate in the map.
      width           Window width in tiles.
      height          Window height in tiles.
      screen_x        Destination X coordinate on screen.
      screen_y        Destination Y coordinate on screen.
      page            Destination screen page (default=0, for screen 4 only).

Notes

  - For palette and tileset resources, MTF loads data into VRAM.
  - For map resources, MTF copies map data to the screen.
  - Operation 0 is the default for map resources.
  - Relative coordinates (operation 0) are screen-based and ideal for
    fixed-screen games.
  - Absolute coordinates (operations 1 and 2) are tile-based and ideal
    for scrolling maps and partial screen updates.

EXAMPLES

Loading a palette

FILE "mtf.SC4Pal"           ' 0

10 SCREEN 2
20 CMD MTF 0                ' load a palette from resource 0

Loading a tileset

FILE "mtf.SC4Tiles"         ' 0

10 SCREEN 2
21 CMD MTF 0                ' load a tileset from resource 0

Full map copy using relative coordinates (default)

Use operation=0 (horizontal x vertical full screens relative coords) parameter when your program implement fixed screens.

CMD MTF , 0, ,

The screens relative coords into the map works like this:

Code example:

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2 (.SC4Super it's also included automatically)

10 SCREEN 2

20 CMD MTF 0                ' load palette from resource 0
21 CMD MTF 1                ' load tileset from resource 1

30 CMD MTF 2                ' load from resource 2 the first screen from the map {0,0} (operation=0)
31 A$ = INPUT$(1)

40 CMD MTF 2, 0, 1          ' load the second screen from the map {1,0} (operation=0)
41 A$ = INPUT$(1)

50 CMD MTF 2, 0, 2          ' load screen {2,0} from the map (operation=0)
51 A$ = INPUT$(1)

60 CMD MTF 2, 0, 0, 1       ' load screen {0,1} (operation=0)
61 A$ = INPUT$(1)

70 CMD MTF 2, 0, 2, 1       ' load screen {2,1} (operation=0)
71 A$ = INPUT$(1)

80 CMD MTF 2, 0, 1, 2       ' load screen {1,2} from the map
81 A$ = INPUT$(1)

Full map copy using absolute coordinates

Use operation=1 (absolute coords) parameter when your program needs to implement screen scrolls.

CMD MTF , 1, ,

The screens absolute coords into the map works like this:

Code example:

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2 (.SC4Super it's also included automatically)

10 SCREEN 2, 2, 0
20 CMD MTF 0                ' load palettes
30 CMD MTF 1                ' load tileset

40 X% = 0 : Y% = 0

50 CMD MTF 2, 1, X%, Y%     ' load from resource 2 the screen started at x,y from the map (operation=1)

60 K% = STICK(0) OR STICK(1)
61 IF INKEY = 27 THEN END
62 IF K% = 0 THEN 60

70 IF K% = 1 OR K% = 2 OR K% = 8 THEN Y% = Y% - 1
71 IF K% = 4 OR K% = 5 OR K% = 6 THEN Y% = Y% + 1
72 IF K% = 6 OR K% = 7 OR K% = 8 THEN X% = X% - 1
73 IF K% = 2 OR K% = 3 OR K% = 4 THEN X% = X% + 1
74 GOTO 50

Copying a partial map window

Use operation=2 (partial map copy) when your program needs to update only a portion of the screen, such as scrolling areas, status panels, minimaps, or animated map regions.

CMD MTF , 2, , , , , ,

The command copies a rectangular window from the map starting at absolute coordinates {map_x,map_y} and transfers it to the screen position {screen_x,screen_y}.

In the example below, a 10 x 6 tile window is copied from map position {20,12} and displayed on the screen at position {5,8}.

Code example:

FILE "mtf.SC4Pal"           ' 0
FILE "mtf.SC4Tiles"         ' 1
FILE "mtf.SC4Map"           ' 2

10 SCREEN 2

20 CMD MTF 0                ' load palette from resource 0
21 CMD MTF 1                ' load tileset from resource 1

30 CMD MTF 2                ' load first screen from the map
31 A$ = INPUT$(1)

40 ' Copy a 10x6 tile window from map position {20,12}
41 ' to screen position {5,8}
42 CMD MTF 2, 2, 20, 12, 10, 6, 5, 8
43 A$ = INPUT$(1)

50 ' Copy an 8x4 tile window from map position {40,20}
51 ' to screen position {0,0}
52 CMD MTF 2, 2, 40, 20, 8, 4, 0, 0
53 A$ = INPUT$(1)

60 ' Copy a 16x10 tile window from map position {0,32}
61 ' to screen position {8,4}
62 CMD MTF 2, 2, 0, 32, 16, 10, 8, 4
63 A$ = INPUT$(1)

Unlike operations 0 and 1, which redraw an entire screen, operation 2 transfers only the specified rectangular area, making it useful for partial screen updates and optimized scrolling effects.

More examples

More downloadable code examples can be found here.

NOTES

- MTF support works only with MSX BASIC screen modes 2 and 4, with full screen size of 32 columns x 24 lines;
- If you are only using MSX1 machines in screen mode 2, you don't need to include the .SC4Pal resource in your program. But don't worry, if you include it anyway and call CMD MTF, it will be ignored when running on MSX1 machines;
- You only need to include .SC4Map file into your program as a resource, because .SC4Super will be included automatically with it;
- The compiler will convert the .SC4Map+.SC4Super files into an internal format to maximize loading speed at runtime. For this reason, the map resource will spend more ROM space than its source original files sizes combined;
- The theoretical maximum map size is around 510 full horizontal screens x 227 full vertical screens (totaling ~115 thousand screens), but MegaROM format (2048K max size) limits this to around 2600 full screens;
- Reserve the 32 to 127 tiles numbers range in your tileset bank to the ASCII table characters if you want to use PRINT statement. Create your own font set at this range or use SET FONT statement;
- Pass text mode coords (0 to 31 range for x and 0 to 23 range for y) when using the LOCATE statement with PRINT;
- Its almost certain that you will need to compile your program as a MegaROM format, because of the map sizes envolved.

>  Learn more in Documentation Overview.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-nmt-support", "", "MSXBAS2ROM: Guia de referencia", "", "Mapas de tela (nMSXTiles): MSXBAS2ROM integrates nMSXTiles screens directly into compiled...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Mapas de tela (nMSXTiles)

NMSXTILE SUPPORT

MSXBAS2ROM integrates nMSXTiles screens directly into compiled ROMs.

COMMAND SYNTAX

Screen Commands

  Inhibits the screen display

    SCREEN OFF

  Display the screen

    SCREEN ON

  Load and display a screen resource

    SCREEN LOAD

Screen Tiled Mode and Fonts Commands

  Enable/Disable tiled mode (screens 2 and 4, for enable tiled PRINT)

    SET TILE

  Load internal FONTs to VRAM font pattern table (screen mode >= 1)

    SET FONT [, ]
      0 = BIOS default font
      1 = Compile Zanac Style
      2 = Konami Gradius Style 1
      3 = Konami Gradius Style 2

  Put a tile character into screen position (for tiled mode, screens 0-2)

    PUT TILE , (,)

See more commands here.

EXAMPLES

Loading Screens

FILE "SCREEN.SC2"   ' resource 0 - for screen 2
FILE "SCREEN.SC5"   ' resource 1 - for screen 5
10 SCREEN 2
20 SCREEN OFF
30 SCREEN LOAD 0
40 SCREEN ON
50 A$ = INPUT$(1)
60 SCREEN 5
70 SCREEN OFF
80 SCREEN LOAD 1
90 SCREEN ON
100 A$ = INPUT$(1)

Enabling Screen Tiled Mode and Loading Built-in Fonts

10 SCREEN 2
20 SET TILE ON
30 SET FONT 1
40 LOCATE 8, 10
50 PRINT "HELLO WORLD"
60 A$ = INPUT$(1)
70 END

More examples

More downloadable code examples can be found here.

IMPORTING AND EXPORTING SCREENS

Use Project > New to create a new screen file and Screen > Import Screen SC2 menu options to load an existing one.

After, use Screen > Export Screen SC2 to save your changes.

Also, image files can be converted and exported to .SCn format using MSX Screen Converter.

USING IT IN YOUR CODE

Screen files can be loaded into your ROM using FILE directive. They are identified by the compiler via its file extension (*.SC2, *.SC5, *.SCn...) and the compiler will pack it automatically using pletter compression format.

Use SCREEN LOAD statement to show the screen resource into the machine screen at runtime.

NOTES

- nMSXTiles support works only with MSX BASIC screen modes 2 and 4, with full screen size of 32 columns x 24 lines;
- Reserve the 32 to 127 tiles numbers range in your tileset bank to the ASCII table characters if you want to use PRINT statement in tiled mode. Create your own font set at this range or use SET FONT statement;
- Pass text mode coords (0 to 31 range for x and 0 to 23 range for y) when using the LOCATE statement with PRINT in tiled mode;
- Its almost certain that you will need to compile your program as a MegaROM format, because of the screen sizes envolved.

>  Learn more in Documentation Overview.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-ts-support", "", "MSXBAS2ROM: Guia de referencia", "", "Sprites (Tiny Sprite): MSXBAS2ROM integrates Tiny Sprite sprites sets directly into...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Sprites (Tiny Sprite)

TINY SPRITE SUPPORT

MSXBAS2ROM integrates Tiny Sprite sprites sets directly into compiled ROMs.

COMMAND SYNTAX

SPRITE LOAD

EXAMPLES

MSX1 sprite set exported by Tiny Sprite

!type
msx1
#Slot 0
................
................
.....FFFF.......
....FFFFFFF.....
...FF.....FFF...
...F.2..2...F...
...F........F...
...FF.......F...
....F.1..1..F...
....FF.111..F...
.....FF.....F...
......FFFFFFF...
................
................
................
................
#Slot 1
................
................
......FFFF......
....FFFFFFFF....
...FF......FF...
...FF.4..4.FF...
...FF......FF...
...FF......FF...
....F.1..1.F....
....F.1111.F....
....FF....FF....
.....FFFFFF.....
................
................
................
................

MSX2 sprite set exported by Tiny Sprite

!type
msx2
#Slot 0
................
................
.....FFFF.......
....FFFFFFF.....
...FF.....FFF...
...F.2..2...F...
...F........F...
...FF.......F...
....F.1..1..F...
....FF.111..F...
.....FF.....F...
......FFFFFFF...
................
................
................
................
#Slot 1
................
................
......FFFF......
....FFFFFFFF....
...FF......FF...
...FF.4..4.FF...
...FF......FF...
...FF......FF...
....F.1..1.F....
....F.1111.F....
....FF....FF....
.....FFFFFF.....
................
................
................
................

Loading a sprite set

FILE "sprite1.spr"   ' resource 0: msx1 sprite set
FILE "sprite2.spr"   ' resource 1: msx2 sprite set

10 SCREEN 2, 2, 0    ' screen mode 2 (msx1)
20 SPRITE LOAD 0     ' load resource 0 (msx1 sprite set)
30 GOSUB 100         ' show sprite on screen

50 SCREEN 7, 2, 0    ' screen mode 7 (msx2)
60 SPRITE LOAD 1     ' load resource 1 (msx2 sprite set)
70 GOSUB 100         ' show sprite on screen

90 SCREEN 0
91 END

100 COLOR 15,4,0
101 CLS
102 PUT SPRITE 0, (100,100)
103 PUT SPRITE 1, (100,100)
104 PUT SPRITE 2, (100,100)
105 A$ = INPUT$(1)
106 RETURN

Sprite related extended commands and functions

More sprite related extended commands can be found here and special functions here.

More examples

More downloadable code examples can be found here.

NOTES

- .SPR files are in plain text format and can be opened by any text editor or edited by Tiny Sprite;
- Use *Export Sprites* > *Export As Backup* button at Tiny Sprite tool to generate the sprite set in plain text format. Copy and paste it at your favorite plain text editor and save it using .SPR extension;
- Use *Load Backup* button at Tiny Sprite tool to load an existing sprite set;
- The main difference between msx1 and msx2 sprite sets are in the header after *!type* clause. So, its easy to modify it using a text editor.

>  Learn more in Documentation Overview.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-vscode-integration", "", "MSXBAS2ROM: Guia de referencia", "", "Integracao com VSCode: This document describes how to integrate MSXBAS2ROM development...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Integracao com VSCode

VSCODE INTEGRATION GUIDE

This document describes how to integrate MSXBAS2ROM development with Visual Studio Code.

There are three different approaches available. Choose the one that best fits your workflow.

1. USING THE `--VSCODE` COMPILER PARAMETER (RECOMMENDED)

When running MSXBAS2ROM with the --vscode parameter, a .vscode/ folder is automatically generated in the current directory. This folder contains a ready-to-use configuration for building, running, and debugging your project using the openMSX emulator.

Example

msxbas2rom --vscode

_Note: don't forget to check and configure the correct openMSX emulator installation path in the generated .vscode/tasks.json file._

Available Features in VSCode

- Compile: Ctrl+Shift+B;
- Run in emulator: Ctrl+Shift+T;
- Debug in emulator: F5.

Debugging Behavior

When debugging is started, the VSCode integration automatically maps each MSX-BASIC line to corresponding Z80 breakpoints inside the emulator. This allows you to:

- Step through execution line-by-line (Z80 assembly);
- Inspect program flow in detail;
- Debug your compiled code directly within openMSX.

2. MANUAL SETUP

If you prefer full control over your environment, you can configure VSCode manually.

Detailed instructions are available here:
 Manual Setup Guide

3. USING A VSCODE EXTENSION

For a more advanced debugging experience, you can use a dedicated VSCode extension.

An experimental extension is available and can be tested here.

 See also: Debugging with OpenMSX and Reference Guide.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-msxbas2rom-vscode-manual-setup", "", "MSXBAS2ROM: Guia de referencia", "", "VSCode - configuracao manual: This document describes how to manually integrate...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: VSCode - configuracao manual

VSCODE MANUAL SETUP

This document describes how to manually integrate MSXBAS2ROM development with Visual Studio Code using:

-   Makefile for using with make tool;
-   VSCode tasks.json and launch.json;
-   openMSX debug automation using debug.tcl script;
-   .gitignore for using with git repositories.

This setup allows a streamlined workflow for compiling, running, and debugging MSX BASIC programs converted to ROM using MSXBAS2ROM.

OVERVIEW

The development workflow becomes:

    VSCode Build (Ctrl+Shift+B)

    Makefile executes MSXBAS2ROM

    ROM and NOI debug symbols generated

    VSCode Test task launches openMSX (Ctrl+Shift+T)

    debug.tcl loads symbols and configures debugging

Artifacts produced during compilation:

| File | Purpose |
|--------|--------------------------|
| .rom | Compiled MSX ROM |
| .noi | Symbol/debug information |
| .bas | Original BASIC source |

The .noi file allows the debugger in openMSX to map addresses to labels and symbols.

PROJECT STRUCTURE

Recommended project layout:

    project/

     program.bas
     program.rom
     program.noi

     Makefile
     debug.tcl
     .gitignore
     .vscode/
         launch.json
         tasks.json

MAKEFILE

The Makefile controls the build pipeline and emulator execution.

Targets included:

-   all  compile BASIC into ROM
-   run  launch emulator
-   test  launch emulator with debugging
-   clean  remove generated files

EXAMPLE MAKEFILE

# MSXBAS2ROM Project Makefile

BAS := $(firstword $(wildcard *.bas))
BASE := $(basename $(BAS))

COMPILER := msxbas2rom
EMULATOR := openmsx

ROM := $(firstword $(wildcard $(BASE)*.rom))
NOI := $(BASE).noi

.PHONY: all clean run test

all:
    $(COMPILER) $(BAS) -a --noi

run: all
    @ROM_FILE=$$(ls $(BASE)*.rom | head -n 1); \
    echo "Running $$ROM_FILE"; \
    exec $(EMULATOR) -cart $$ROM_FILE

test: all
    @ROM_FILE=$$(ls $(BASE)*.rom | head -n 1); \
    echo "Debugging $$ROM_FILE"; \
    exec $(EMULATOR) -cart $$ROM_FILE -script debug.tcl

clean:
    rm -f *.rom *.noi *.omds *.symbol *.cdb

EXPLANATION

Build

    make all

Runs:

    msxbas2rom program.bas -a --noi

Which generates:

    program.rom
    program.noi

Test

    make test

Launches the emulator:

    openmsx -cart program.rom -script debug.tcl

GIT REPOSITORIES

Example .gitignore for using with git repositories.

# binaries and symbols
.rom
.noi
.omds
.symbol
.cdb

VSCODE INTEGRATION

VSCode uses tasks.json and launch.json to run build and test commands.

This allows:

-   build via Ctrl+Shift+B with problem highlighting
-   run via F5 with quick emulator launch
-   test via Ctrl+Shift+T with quick emulator launch for debugging

.VSCODE/TASKS.JSON

{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build",
      "type": "shell",
      "command": "make all",
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": {
        "owner": "msxbas2rom",
        "fileLocation": ["relative", "${workspaceFolder}"],
        "pattern": {
          "regexp": "^(.+\\.bas):(\\d+): ERROR: (.+)$",
          "file": 1,
          "line": 2,
          "message": 3
        }
      }
    },
    {
      "label": "clean",
      "type": "shell",
      "command": "make clean",
      "problemMatcher": []
    },
    {
      "label": "test",
      "type": "shell",
      "command": "make test",
      "group": {
        "kind": "test",
        "isDefault": true
      },
      "problemMatcher": []
    }
  ]
}

.VSCODE/LAUNCH.JSON

{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Run",
      "type": "node-terminal",
      "request": "launch",
      "command": "make run; exit",
      "cwd": "${workspaceFolder}"
    }
  ]
}

PROBLEM MATCHER

The problemMatcher allows VSCode to detect compiler errors and jump
directly to the corresponding source line.

Example compiler output:

    test1.bas:5: ERROR: FOR without a NEXT

The matcher extracts:

| Field   | Value |
|---------|--------------------|
| file    | test1.bas |
| line    | 5 |
| message | FOR without a NEXT |

VSCode will highlight the error in the editor automatically.

OPENMSX DEBUG AUTOMATION

The openMSX emulator supports automation through Tcl scripts.

Using:

    openmsx -cart program.rom -script debug.tcl

The emulator will execute the script after startup.

This enables:

-   automatic loading of debug symbols
-   breakpoints
-   debugger initialization
-   execution tracing

DEBUG.TCL

The debug.tcl file configures the debugging environment.

EXAMPLE DEBUG.TCL

# debug.tcl

puts "==== MSXBAS2ROM Debug Session ===="

# find first .noi file in current directory
set noi_files [glob -nocomplain *.noi]

if {[llength $noi_files] == 0} {
    puts "No .noi file found."
    return
}

# get first file
set noi_file [lindex $noi_files 0]

# remove extension to get ROM name
set rom [file rootname $noi_file]

puts "ROM base name: $rom"

proc load_symbols {} {
    global noi_file

    puts "Loading debug symbols: $noi_file"
    debug symbols load $noi_file NoICE

    # resolve symbol
    if {[catch {debug symbols lookup -name START_PGM} result]} {
        puts "Symbol START_PGM not found."
        return
    }

    # extract address
    set entry [lindex $result 0]
    set addr [dict get $entry value]

    puts "Setting breakpoint at start of the program: $addr"
    debug breakpoint create -address $addr

    puts "Debugger ready."
}

# run after emulator startup
after 1000 load_symbols

DEVELOPMENT WORKFLOW

Typical cycle during development:

1. Compile: Ctrl+Shift+B
2. Run: F5
3. Test debugging: Ctrl+Shift+T

This integration provides:

-   fast compile/test cycle
-   automated emulator launch
-   integrated error navigation
-   debugger automation
-   reproducible builds

It brings a modern development workflow to MSX BASIC ROM development.

 See also: Debugging with OpenMSX and Reference Guide.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-debugging-with-openmsx", "", "MSXBAS2ROM: Guia de referencia", "", "Depuracao com o openMSX: Debugging your MSX BASIC program compiled with MSXBAS2ROM is...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Depuracao com o openMSX

DEBUGGING WITH OPENMSX DEBUGGER

Debugging your MSX BASIC program compiled with MSXBAS2ROM is possible using the OpenMSX integrated debugger (release 20.0+).

>  Although you cannot debug the BASIC source directly, you can debug the generated assembly binary code  allowing you to track execution, inspect variables, and set breakpoints with symbolic information.

OVERVIEW

When compiling a program with MSXBAS2ROM, the compiler can generate a .NOI file that contains symbolic information about your BASIC lines and variables.

This file allows the OpenMSX debugger to show human-readable symbols like:

- Lines (e.g., LIN_10, LIN_20)
- Variables (e.g., VAR_A$, VAR_B%)

These symbols can then be used to set breakpoints, inspect memory, and step through your compiled code as it runs on the MSX emulator.

COMPILING WITH SYMBOL SUPPORT

To generate the .NOI symbol file, compile your BASIC program using the -s parameter:

msxbas2rom -s myprogram.bas

This will create both the .ROM binary and a corresponding .NOI file with symbol definitions.

>  Reference: Usage  Compile and Generate Symbols for OpenMSX Debugger

EXAMPLE

MSX BASIC Source

10 CLS
20 A$ = "HELLO WORLD!"
30 B% = 2025
40 PRINT A$
50 PRINT "HELLO FROM YEAR "; B%
60 END

Generated `.NOI` File

def LOADER 4010H  ; jump
def VAR_CURSEGM C023H  ; variable
def MR_CALL 41C8H  ; jump
def MR_CALL_TRAP 41CBH  ; jump
def MR_CHANGE_SGM 41CEH  ; jump
def MR_GET_BYTE 41D1H  ; jump
def MR_GET_DATA 41D4H  ; jump
def MR_JUMP 41D7H  ; jump
def END_STMT 8063H  ; jump
def START_PGM 8070H  ; jump
def LIN_10 8089H  ; jump
def LIN_20 808CH  ; jump
def LIN_30 8098H  ; jump
def LIN_40 809EH  ; jump
def LIN_50 80A7H  ; jump
def LIN_60 80B6H  ; jump
def END_PGM 80B9H  ; jump
def LIT_0 80BCH  ; jump
def LIT_1 80C9H  ; jump
def VAR_A$ C038H  ; variable
def VAR_B% C138H  ; variable

Summary of Symbol Types and Debug Uses

Symbol Type | Example | Typical Use
-- | -- | --
LIN_* | LIN_20 | Line-level breakpoints
VAR_* | VAR_B% | Variable read/write watch
LIT_* | LIT_0 | Constant literal inspection
START_PGM |  | Start of the program checkpoint
END_PGM |  | End of the program checkpoint
END_STMT |  | END statement checkpoint
VAR_CURSEGM |  | Current MegaROM segment number inspection
MR_* | MR_GET_DATA | MegaROM control routines
LOADER |  | ROM initialization checkpoint

OPENING THE DEBUGGER IN OPENMSX

1. Launch OpenMSX and load your compiled ROM:
   openmsx -cart myprogram.rom

2. Open the OpenMSX integrated debugger (available since release 20.0+):
   - Menu  *Debugger*  toggle the following options: *Tool bar*, *Disassembly*, *Slots*, *CPU registers*, *CPU flags*, *Stack*, *Memory* and *Symbol Manager*;
   - On the *Symbol Manager* box, select *Load symbol file...*
   - Select the .NOI file generated by MSXBAS2ROM. Now, you can see your program symbols on the "All symbols" option.

3. Once loaded, youll see symbols like LIN_10, LIN_20, and VAR_A$ displayed in the debugger.
   The screen will look like the one below:

>  See release notes for more about the integrated debugger:
> - OpenMSX 21.0 Release Notes

DEBUGGING WORKFLOW

Setting Breakpoints

You can place breakpoints on line symbols using *Set breakpoint* menu option:

A red marker will be placed next to the line number symbol:

Also, you can set breakpoints on variable memory addresses using the *Debugger*  *Breakpoints*  *Editor* option:

This allows you to stop execution when the code reaches a certain BASIC line or when a variable is read/written.

Inspecting Memory and Variables

The Memory Inspector in OpenMSX debugger can show variable contents.

Use the .NOI symbols to locate the variable addresses (e.g., VAR_A$ or VAR_B%) and inspect them directly in the Hex Memory View.

>  See Internal Data Types for more information about how to understand variables binary data.

COMMON DEBUGGING STRATEGIES

Symbol-Based Breakpoint Strategy

Instead of using raw memory addresses, use the symbolic names generated by MSXBAS2ROM to map line flow and follow jump tracing.

For example, putting a breakpoint on LIN_30 symbol will stop execution exactly when BASIC line 30 begins  even though youre debugging compiled Z80 code. You can then step through the generated instructions to understand how your BASIC logic translates to assembly.

>  Use LIN_* symbols as logical checkpoints  theyre the closest equivalent to breakpoints per BASIC line.

Variable Watch & Memory Inspection

Since every BASIC variable has a symbol like VAR_A$ or VAR_B%, you can monitor reads and writes directly. Then, in the Memory Inspector, you can:

- View variable contents in hexadecimal or ASCII form;
- Track changes across function calls or loops;
- Identify overwrites or unintended data corruption.

>  This is especially useful for debugging string concatenations, numeric overflow, or array index errors.
>  See Internal Data Types for more information about how to understand variables binary data.

Timing & Performance Checkpoints

OpenMSXs debugger allows cycle-level stepping by tracking the machine internal time (*Settings*  *GUI*  *Status bar*  *Show*) or the screen frame counter (*Tools*  *Toys*  *frame counter*). So, you can:

- Measure the performance of specific routines;
- Compare execution time before and after optimizations;
- Identify heavy BASIC lines that produce large instruction blocks.

>  Document useful symbol patterns in your games internal wiki (e.g. where main loop starts, where player state lives), so it will be easier to track them.

 See also: Reference Guide

FOOTER

>  *Empower your MSX creativity  compile fast and debug smart with MSXBAS2ROM.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-compiler-architecture", "", "MSXBAS2ROM: Guia de referencia", "", "Arquitetura do compilador: > MSXBAS2ROM was designed to transform MSX BASIC programs into...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Arquitetura do compilador

COMPILER ARCHITECTURE

> MSXBAS2ROM was designed to transform MSX BASIC programs into optimized
Z80 machine code while maintaining strong compatibility with the MSX
BASIC environment and ecosystem.

This document describes the internal architecture of the
MSXBAS2ROM compiler and the structure of the ROM images it
produces.

The goal is to provide a clear overview of how a BASIC source file is
transformed into a bootable MSX ROM cartridge and how the generated
program is organized in memory.

COMPILATION FLOW

The compiler follows a traditional multiphase compilation pipeline
adapted for MSX BASIC semantics and the constraints of the Z80
architecture.

The main phases are:

1.  Lexical Analysis
2.  Syntactic Analysis
3.  Semantic Analysis and Code Generation
4.  Binary Building and ROM Packaging

Each phase progressively transforms the source program into a
lower-level representation until it becomes a valid MSX ROM image.

1. LEXICAL ANALYSIS

During lexical analysis, the source code is scanned and divided into
atomic elements called lexemes.

Examples of lexemes include:

-   Keywords (IF, FOR, GOTO, PRINT)
-   Identifiers (variable names)
-   Operators (+, -, *, /, AND, OR)
-   Literals (numbers and strings)
-   Delimiters (( ) , :)

The lexical analyzer converts the text source into a token stream,
which simplifies further processing by removing formatting concerns such
as whitespace or comments.

Key responsibilities:

-   Token identification
-   Numeric literal parsing
-   String literal handling
-   BASIC keyword recognition
-   Error detection for invalid tokens

The result of this phase is a sequence of tokens used by the parser.

2. SYNTACTIC ANALYSIS

The syntactic analysis phase interprets the token stream and validates
it against the grammar rules of MSX BASIC.

During this phase the compiler builds an Abstract Syntax Tree (AST)
representing the structure of the program.

Typical constructs represented in the AST include:

-   Expressions
-   Statements
-   Control flow structures
-   Function calls
-   Variable references

Example BASIC statement:

    IF A > 10 THEN PRINT "HELLO"

This becomes a tree describing:

-   A conditional expression (A > 10)
-   A conditional branch
-   A PRINT command with a string literal

The AST becomes the central intermediate representation used in the
rest of the compilation pipeline.

3. SEMANTIC ANALYSIS AND CODE GENERATION

Once the AST is built, the compiler performs semantic analysis to
ensure the program is logically correct and to prepare for code
generation.

Major tasks in this phase include:

Symbol Table Construction

The compiler builds a symbol table containing information about:

-   Variables
-   Arrays
-   Labels
-   Functions
-   Constants

This table allows the compiler to resolve identifiers and allocate
memory locations.

Type and Context Validation

Although MSX BASIC is loosely typed, the compiler must still verify:

-   Valid function usage
-   Correct parameter counts
-   Valid variable contexts
-   Control flow correctness

Z80 Code Generation

After semantic validation, the compiler generates Z80 assembly
instructions.

These instructions are emitted using an internal code builder that:

-   Translates BASIC statements to Z80 routines
-   Calls runtime support functions
-   Handles expression evaluation
-   Manages stack usage

The output of this stage is a relocatable code representation ready
to be assembled into the final ROM image.

4. BINARY BUILDING AND ROM PACKAGING

In the final stage the compiler produces a complete MSX ROM image.

This stage includes:

-   Translating generated code into binary form
-   Linking runtime modules
-   Linking resources (maps, data, assets)
-   Resolving symbol references
-   Organizing memory segments
-   Producing the final ROM file

The final result is a ROM cartridge binary that can run on:

-   Real MSX hardware
-   MSX emulators
-   Flash cartridges

DESIGN GOALS

The MSXBAS2ROM compiler was designed around several key principles.

MSX BASIC COMPATIBILITY

The compiler aims to maintain strong compatibility with MSX BASIC
syntax and semantics, enabling existing programs to be ported with
minimal changes.

CROSSPLATFORM TOOLING

The compiler is implemented as a command line tool compatible with:

-   Linux
-   Windows
-   macOS

This allows integration into modern development workflows and build
pipelines.

EXPANDABILITY

The architecture supports extensibility through:

-   New compiler directives
-   Extended BASIC commands
-   Additional runtime modules
-   External resource integration

PERFORMANCE

One of the main goals of MSXBAS2ROM is to generate fast Z80 machine
code, significantly outperforming interpreted BASIC execution.

ROM MEMORY SCHEME

Programs compiled with MSXBAS2ROM are packaged into a ROM image that
follows the MSX cartridge memory model.

The MSX memory space is divided into four pages of 16 KB each,
totaling 64 KB of addressable memory.

Different ROM formats may organize these pages differently depending on
the cartridge type.

PLAIN ROM MEMORY SCHEME

Plain ROM cartridges are typically 48 KB in size.

They occupy three ROM pages while the last page is RAM used during
execution.

OVERVIEW

    +----------------------------------------------------+
    | PAGE 0 (16K)                                       |
    |----------------------------------------------------|
    | MSX ROM BIOS                                       |
    |----------------------------------------------------|
    | USER RESOURCES                                     |
    |    MAP                                           |
    |    DATA                                          |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 1 (16K)                                       |
    |----------------------------------------------------|
    | MSX ROM BASIC                                      |
    |----------------------------------------------------|
    | COMPILER RUNTIME KERNEL                            |
    |    MSX Boot Loader                               |
    |    General Support Routines                      |
    |    Tiny Sprite Support                           |
    |    MSX Tile Forge Support                        |
    |    Arkos Tracker Support                         |
    |    Floating Point Math Pack                      |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 2 (16K)                                       |
    |----------------------------------------------------|
    | BASIC Boot Loader                                  |
    | USER CODE                                          |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 3 (16K)                                       |
    |----------------------------------------------------|
    | MSX RAM                                            |
    |    Kernel Workspace (flags)                      |
    |    User Workspace (variables)                    |
    |    Code Stack                                    |
    |    MSX BIOS Workspace and routines               |
    +----------------------------------------------------+

Description

- Page 0: Contains system BIOS and optional user resources such as tile maps or
game data;
- Page 1: Contains the runtime kernel required for executing compiled BASIC
programs;
- Page 2: Stores the compiled program code and boot loader responsible for
starting the application;
- Page 3: Represents RAM used for runtime execution including variables and stack.

MEGAROM MEMORY SCHEME (KONAMI SCC / ASCII8)

Large programs and games often require ROM sizes larger than 48 KB.

For this reason the compiler also supports MegaROM cartridges,
typically ranging from:

128 KB up to 2048 KB.

These cartridges use bank switching to dynamically map ROM segments
into the address space.

OVERVIEW

    +----------------------------------------------------+
    | PAGE 0 (16K)                                       |
    |----------------------------------------------------|
    | MSX ROM BIOS                                       |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 1 (16K)                                       |
    |----------------------------------------------------|
    | MSX ROM BASIC                                      |
    |----------------------------------------------------|
    | Segments 0 (8K) and 1 (8K)                         |
    |                                                    |
    | COMPILER RUNTIME KERNEL                            |
    |    MSX Boot Loader                               |
    |    General Support Routines                      |
    |    Tiny Sprite Support                           |
    |    MSX Tile Forge Support                        |
    |    Arkos Tracker Support                         |
    |    Floating Point Math Pack                      |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 2 (16K)                                       |
    |----------------------------------------------------|
    | Segments 2..N                                      |
    |                                                    |
    | BASIC Boot Loader                                  |
    | USER CODE                                          |
    | Additional code banks                              |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 2 (continued after code segments)             |
    |----------------------------------------------------|
    | USER RESOURCES                                     |
    |    MAP segments                                  |
    |    DATA segments                                 |
    +----------------------------------------------------+

    +----------------------------------------------------+
    | PAGE 3 (16K)                                       |
    |----------------------------------------------------|
    | MSX RAM                                            |
    |    Kernel Workspace (flags)                      |
    |    User Workspace (variables)                    |
    |    Code Stack                                    |
    |    MSX BIOS Workspace                            |
    +----------------------------------------------------+

Bank Switching

MegaROM cartridges use mapper hardware such as:

-   Konami SCC
-   ASCII8

These allow different 8 KB segments of the ROM to be dynamically
mapped into memory during program execution.

This enables:

-   Very large games
-   Streaming of assets
-   Large maps and resources
-   Music and sound data banks

ADDITIONAL RESOURCES

See compiler process flow model for more detailed information.

 See also Resource Directives, Extended Commands and Extended Functions.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-getting-help", "", "MSXBAS2ROM: Guia de referencia", "", "Como obter ajuda: Need support while working with MSXBAS2ROM? Here are your options:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Guia de referencia: Como obter ajuda

GETTING HELP & REFERENCES

Need support while working with MSXBAS2ROM? Here are your options:

BUILT-IN DOCUMENTATION
msxbas2rom --version
msxbas2rom --doc
msxbas2rom --history

ONLINE RESOURCES
- Official Wiki Home;
- Getting Started Guide;
- Usage Guide.

COMMUNITY
- Open issues on the GitHub repository;
  Use following prefixes on the issue description:
  - [Feature]: Request for a new feature;
  - [Bug]: Problem report;
  - [Improvement]: Enhancement of existing behavior;
  - [Question] or [Discussion]: Clarification or open-ended talk;
  - [Asset]: For new assets to be added to the project (ex: images, demos, games);
  - [Task]: General to-do or implementation detail.
- Contribute fixes or docs via Contributing.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-examples", "", "MSXBAS2ROM: Exemplos", "", "Exemplos prontos: > Discover how MSXBAS2ROM transforms MSX BASIC code into playable games...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Exemplos: Exemplos prontos

EXAMPLES

> Discover how MSXBAS2ROM transforms MSX BASIC code into playable games and demos. Explore source files, compile examples, and learn compilation tips.

MSX BASIC PROJECTS REPOSITORY

Browse a selection of "Work in Progress" MSX BASIC projects compiled with MSXBAS2ROM.

Game demonstration 1 (compiled as a 48kb ROM cartridge)

If you want to know how to develop a game with MSXBAS2ROM, give a try to the example below.

Download the game source code here, compile and test it.

msxbas2rom gd1.bas

You can watch a more detailed explanation here (brazilian portuguese video).

Game demonstration 2 (compiled as a MegaROM cartridge)

Download the game source code here, compile and test it.

msxbas2rom -x gd2.bas

Note: to run this MegaROM compiled game on WebMSX emulator you will need to set the ROM format as ASCII8 (or KonamiSCC) after load it on the cartridge slot.

Tesouro Perdido (compiled as a MegaROM cartridge)

Download the game source code here, compile and test it.

msxbas2rom -x tesperd.bas

Note: to run this MegaROM compiled game on WebMSX emulator you will need to set the ROM format as ASCII8 (or KonamiSCC) after load it on the cartridge slot.

Scroll on tiled screen modes 1, 2 and 4

Repeat the process in the same way as in the previous examples.

- Demo 1: all directions scroll (screen mode 1, text);
- Demo 2: horizontal scroll with sprite (screen mode 2);
- Demo 3: changing background during horizontal scroll action (screen mode 4);
- Demo 4: all directions scroll (screen mode 2).

Scroll on graphical screen modes 8 and 12

Here you must use the parameters "-c -x" to compile these ones:

- Demo 5: horizontal scroll.

HOW TO COMPILE AND RUN

See Getting Started page for more information.

WANT TO SHARE YOUR DEMO?

Submit your details via a GitHub issue (prefix: [Asset]) or pull request, and we'll add your project to this list.

- Title and Brief Description;
- Screenshot or demo video;
- ROM download link, if available.

We'll happily feature it on this page!

> *MSXBAS2ROM  turning your BASIC games into ROM adventures.*

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-game-demo-1-gd1", "", "MSXBAS2ROM: Demo: Game Demo 1", "", "gd1.bas: ' By: Amaury Carvalho, 2022",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: Game Demo 1: gd1.bas

'====================================================
' GAME DEMONSTRATION 1 (FOR MSX 1)
' By: Amaury Carvalho, 2022
'====================================================
' How to compile:
'   msxbas2rom -c gd1.bas
' Links:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://www.julien-nevo.com/arkostracker/
'   https://launchpad.net/nmsxtiles
'   http://msx.jannone.org/tinysprite/tinysprite.html
'====================================================

FILE "gd1.akm"        ' 0 - Music (exported from Arkos Tracker 2)
FILE "gd1.akx"        ' 1 - Sound Effects (exported from Arkos Tracker 2)
FILE "gd1.chr.plet5"  ' 2 - Tiles Patterns (exported from nMSXTiles)
FILE "gd1.clr.plet5"  ' 3 - Tiles Colors (exported from nMSXTiles)
FILE "gd1.spr"        ' 4 - Sprites (exported from TinySprite)
FILE "gd1_0_0.plet5"  ' 5 - Background for Screen 2 (exported from nMSXTiles)

10 SCREEN 2, 2, 0
11 COLOR 15, 0, 0

20 CMD DISSCR         ' Disable screen
21 CMD PLYLOAD 0, 1   ' Load music and sound effect resource
22 CMD WRTCHR 2       ' Load tiles patterns resource
23 CMD WRTCLR 3       ' Load tiles colors resource
24 CMD WRTSPR 4       ' Load sprite resource
25 CMD WRTSCR 5       ' Load screen resource
26 CMD SETFNT 0       ' Load system default font

30 CMD PLYSONG 0      ' Load game song theme (Bach Invention 14)
31 CMD PLYPLAY        ' Play theme

40 LOCATE 00, 00 : PRINT "GAME DEMONSTRATION 1";
41 LOCATE 22, 00 : PRINT "HITS:";
42 CMD ENASCR         ' Enable screen

'==== PLAYER START POSITION AND SCORE
50 PX% = 8
51 PY% = 24
52 PP% = 0

'==== SHOW SPIDER
60 SX% = RND(1) * 100 MOD 192 + 32
61 SY% = RND(1) * 100 MOD 128 + 32
62 PUT SPRITE 1, (SX%, SY%), 8

'==== SHOW PLAYER
70 PUT SPRITE 0, (PX%, PY%), 15, 0

'==== GET PLAYER INPUT
80 PS% = STICK(0) OR STICK(1)
81 PB% = STRIG(0) OR STRIG(1)

'==== CHECK FOR TRIGGER
90 IF PB% = 0 THEN 200
91     PUT SPRITE 0,,,2                   ' Change player's sprite pattern
92     IF COLLISION(0, 1) < 0 THEN 110    ' Sprite 0 not collided with sprite 1?

'==== PLAYER HIT THE SPIDER
100 CMD PLYSOUND 2                        ' Play sound effect from instrument 2
101 PP% = PP% + 1
102 LOCATE 28, 00 : PRINT PP%;
103 PB% = STRIG(0) OR STRIG(1)
104 IF PB% <> 0 THEN 103
105 GOTO 60

'==== PLAYER MISS THE SPIDER
110 CMD PLYSOUND 3                        ' Play sound effect from instrument 3
111 PB% = STRIG(0) OR STRIG(1)
112 IF PB% <> 0 THEN 111
113 GOTO 70

'==== CHECK FOR PLAYER MOVIMENT
200 IF PS% = 0 THEN 80
201   DX% = 0 : DY% = 0

210   IF PS% = 1 THEN DY% = -1 : GOTO 220
211   IF PS% = 2 THEN DX% = 1 : DY% = -1 : GOTO 220
212   IF PS% = 3 THEN DX% = 1 : GOTO 220
213   IF PS% = 4 THEN DX% = 1 : DY% = 1 : GOTO 220
214   IF PS% = 5 THEN DY% = 1 : GOTO 220
215   IF PS% = 6 THEN DX% = -1 : DY% = 1 : GOTO 220
216   IF PS% = 7 THEN DX% = -1 : GOTO 220
217   IF PS% = 8 THEN DX% = -1 : DY% = -1

220 DX% = DX% + PX%
221 DY% = DY% + PY%
222 IF DX% < 8 OR DX% > 232 OR DY% < 24 OR DY% > 160 THEN 80

230 PX% = DX%
231 PY% = DY%
232 GOTO 70

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-game-demo-2-gd2", "", "MSXBAS2ROM: Demo: Game Demo 2", "", "gd2.bas: ' By: Amaury Carvalho, 2022",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: Game Demo 2: gd2.bas

'====================================================
' GAME DEMONSTRATION 2 (FOR MSX 2/2+, MEGAROM)
' By: Amaury Carvalho, 2022
'====================================================
' How to compile (megarom):
'   msxbas2rom -c -x gd2.bas
' Links:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://www.julien-nevo.com/arkostracker/
'   http://msx.jannone.org/conv/
'   http://msx.jannone.org/tinysprite/tinysprite.html
'====================================================

FILE "gd2.akm"        ' 0 - Music (exported from Arkos Tracker 2)
FILE "gd2.akx"        ' 1 - Sound Effects (exported from Arkos Tracker 2)
FILE "gd2_1.spr"      ' 2 - Sprites for MSX 1 (exported from TinySprite)
FILE "gd2_2.spr"      ' 3 - Sprites for MSX 2 (exported from TinySprite)
FILE "SCREEN.SC2"     ' 4 - Background for Screen 2 (exported from MSX Screen Converter)
FILE "SCREEN.SC8"     ' 5 - Background for Screen 8 (exported from MSX Screen Converter)
FILE "SCREEN.S12"     ' 6 - Background for Screen 12 (exported from MSX Screen Converter)

'==== GAME INITIALIZATION
10 DIM AX%(5), AY%(5), AS%(5), AIX%(5), AIY%(5)

20 ON MSX() GOTO 22, 23, 23, 23
21   SN% = 2 : SR% = 4 : SS% = 2 : GOTO 30      ' 0: MSX 1 => screen 2
22   SN% = 8 : SR% = 5 : SS% = 3 : GOTO 30      ' 1: MSX 2 => screen 8
23   SN% = 12 : SR% = 6 : SS% = 3               ' else: MSX 2+ or TR... => screen 12

'==== SCREEN INITIALIZATION
30 SCREEN SN%, 3, 0   ' Screen mode, sprite magnified, keyclick off
31 CMD DISSCR         ' Disable screen
32 CMD PLYLOAD 0, 1   ' Load music and sound effect resource
33 SCREEN LOAD SR%    ' Load screen resource
34 SPRITE LOAD SS%    ' Load sprite resource
35 CMD PLYSONG 0      ' Load game song theme (Gustav Holst - The Planets - Mars)
36 CMD PLYPLAY        ' Play theme
37 CMD ENASCR         ' Enable screen

'==== PLAYER AND ASTEROIDS START POSITION
40 PX% = 120
41 PY% = 100
42 FOR I% = 0 TO 5
43    GOSUB 1020 : AX%(I%) = R% MOD 192 + 32
44    GOSUB 1020 : AY%(I%) = R% MOD 128 + 32
45    AS%(I%) = 2
46    GOSUB 1020 : R% = R% MOD 2 : IF R% = 0 THEN AIX%(I%) = 3 ELSE AIX%(I%) = -3
47    GOSUB 1020 : R% = R% MOD 2 : IF R% = 0 THEN AIY%(I%) = 3 ELSE AIY%(I%) = -3
48 NEXT

50 GOSUB 240   ' SHOW PLAYER
51 GOSUB 310   ' SHOW ASTEROIDS

'==== MAIN LOOP
60 GOSUB 1000  ' GET PLAYER INPUT
61 GOSUB 200   ' MOVE PLAYER
62 GOSUB 300   ' MOVE ASTEROIDS
63 GOSUB 400   ' MOVE BULLETS
64 GOSUB 500   ' CHECK FOR TRIGGER
65 GOTO 60

'==== MOVE PLAYER
200 IF PS% = 0 THEN RETURN
201   DX% = 0 : DY% = 0
202   ON PS% GOTO 210, 211, 212, 213, 214, 215, 216, 217

210     DY% = -2 : GOTO 220
211     DX% = 2 : DY% = -2 : GOTO 220
212     DX% = 2 : GOTO 220
213     DX% = 2 : DY% = 2 : GOTO 220
214     DY% = 2 : GOTO 220
215     DX% = -2 : DY% = 2 : GOTO 220
216     DX% = -2 : GOTO 220
217     DX% = -2 : DY% = -2

220 DX% = DX% + PX%
221 DY% = DY% + PY%
222 IF DX% < 4 OR DX% > 220 OR DY% < 4 OR DY% > 156 THEN RETURN

230 PX% = DX%
231 PY% = DY%

'==== SHOW PLAYER
240 PUT SPRITE 0, (PX%, PY%), 15, 0
241 PUT SPRITE 1, (PX%, PY%), 11, 1
242 PUT SPRITE 2, (PX%, PY%),  8, 2
243 RETURN

'==== MOVE ASTEROIDS
300 FOR I% = 0 TO 5
301   AX%(I%) = AX%(I%) + AIX%(I%) : IF AX%(I%) < 4 OR AX%(I%) > 220 THEN AIX%(I%) = -AIX%(I%)
302   AY%(I%) = AY%(I%) + AIY%(I%) : IF AY%(I%) < 4 OR AY%(I%) > 156 THEN AIY%(I%) = -AIY%(I%)
303 NEXT

'==== SHOW ASTEROIDS
310 FOR I% = 0 TO 5
311    PUT SPRITE 3+I%, (AX%(I%), AY%(I%)), 14, 3
312    IF COLLISION(3+I%) >= 0 THEN AIX%(I%) = -AIX%(I%)
313 NEXT
314 RETURN

'==== MOVE BULLETS
400 RETURN

'==== CHECK FOR TRIGGER
500 IF PB% = 0 THEN RETURN
501    GOTO 530

'==== BULLET HIT AN ASTEROID
510 CMD PLYSOUND 2,2                      ' Play sound effect from instrument 2 on channel 2
511 RETURN

'==== PLAYER HIT AN ASTEROID
520 CMD PLYSOUND 1,2                      ' Play sound effect from instrument 1 on channel 2
521 RETURN

'==== PLAYER SHOOTS
530 CMD PLYSOUND 3,2                      ' Play sound effect from instrument 3 on channel 2
531 RETURN

'==== GET PLAYER INPUT
1000 PS% = STICK(0) OR STICK(1)
1001 PB% = STRIG(0) OR STRIG(1)
1002 GOTO 1020

'==== WAIT FOR TRIGGER RELEASE
1010 PB% = STRIG(0) OR STRIG(1)
1011 IF PB% <> 0 THEN 1010
1012 RETURN

'==== GET RANDOM NUMBER
1020 R% = RND(1) * 100
1021 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-readme", "", "MSXBAS2ROM: Exemplos (demo)", "", "README.md: demo/ contains playable examples, showcase projects, and sample content...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Exemplos (demo): README.md

DEMO ASSETS

PURPOSE

demo/ contains playable examples, showcase projects, and sample content demonstrating what can be built with MSXBAS2ROM.

STRUCTURE

- Demo/game folders grouped by project or feature theme;
- Each subfolder may include .bas sources, assets, and generated outputs.

USAGE

- Use this folder for manual demonstration and exploratory validation;
- Prefer tests/integration/ and tests/unit/ for automated regression coverage.

WIKI

- Search for more examples at the project wiki.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll1-scroll1", "", "MSXBAS2ROM: Demo: scroll1", "", "scroll1.bas: ' by: Amaury Carvalho (2022)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll1: scroll1.bas

' SCROLL DEMONSTRATION 1
' by: Amaury Carvalho (2022)

10 SCREEN 1

20 LOCATE 15, 12 : PRINT "X";

30 S% = STICK(0)
40 B% = STRIG(0)

50 IF B% <> 0 THEN 20
60 IF S% = 0 THEN 30

70 SCREEN SCROLL S%
80 GOTO 30

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll2-scroll2", "", "MSXBAS2ROM: Demo: scroll2", "", "scroll2.bas: ' by: Amaury Carvalho (2022)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll2: scroll2.bas

' SCROLL DEMONSTRATION 2
' by: Amaury Carvalho (2022)

FILE "STARS.SC2"     ' screen resource
FILE "SHIP.SPR"      ' tiny sprite resource

10 SCREEN 2, 2

20 SCREEN LOAD 0     ' load 1st resource on screen
30 SPRITE LOAD 1     ' load 2nd resource on sprite bank

40 PUT SPRITE 0, (100, 100), 15

50 S% = STICK(0)

60 IF S% = 3 OR S% = 7 THEN SCREEN SCROLL S%

70 GOTO 50

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll3-scroll3", "", "MSXBAS2ROM: Demo: scroll3", "", "scroll3.bas: ' by: Amaury Carvalho (2022)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll3: scroll3.bas

' SCROLL DEMONSTRATION 3
' by: Amaury Carvalho (2022)

FILE "STARS.SC4"     ' screen resource
FILE "SHIP.SPR"      ' tiny sprite resource

10 DIM SC%(15, 23)   ' 32 cols (0-15: 2 cols for each integer), 24 rows (0-23)

20 SCREEN 4, 2

30 SCREEN LOAD 0     ' load 1st resource on screen
40 SPRITE LOAD 1     ' load 2nd resource on sprite bank

50 PUT SPRITE 0, (100, 100),,0
51 PUT SPRITE 1, (100, 100),,1

60 SCREEN COPY TO SC% SCROLL 3            ' copy to array and do a scroll on the data copied

70 FOR Y% = 0 TO 17
71   R% = ((RND(1) * 100) MOD 30) + 32
72   SC%(15, Y%) = R%                     ' change last column two bytes
74 NEXT

80 SCREEN PASTE FROM SC%                  ' copy from array to screen

90 GOTO 60

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll4-scroll4", "", "MSXBAS2ROM: Demo: scroll4", "", "scroll4.bas: ' by: Amaury Carvalho (2022)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll4: scroll4.bas

' SCROLL DEMONSTRATION 4
' by: Amaury Carvalho (2022)

FILE "SAND.SC2"

10 SCREEN 2, 2

20 COLOR 15, 0, 0

30 SCREEN LOAD 0

40 S% = STICK(0)

50 IF S% = 0 THEN 40

60 SCREEN SCROLL S%

70 GOTO 40

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll5-scr02", "", "MSXBAS2ROM: Demo: scroll5", "", "SCR02.BAS:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll5: SCR02.BAS

10 SCREEN 2
20 BLOAD"SCREEN.SC2",S
30 GOTO 30

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll5-scr07", "", "MSXBAS2ROM: Demo: scroll5", "", "SCR07.BAS:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll5: SCR07.BAS

10 COLOR 15,0,0: SCREEN 7
20 DATA 1,0,1,2,1,1,1,1,3,2,2,3,4,2,1,4,3,2,1,3,5,5,3,2
30 DATA 2,4,5,5,4,3,3,4,5,6,4,3,5,5,5,6,5,5,6,6,6,7,6,6
40 FOR C=0 TO 15:READ R,G,B:COLOR=(C,R,G,B):NEXT
50 BLOAD"SCREEN.SC7",S
60 GOTO 60

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll5-scr08", "", "MSXBAS2ROM: Demo: scroll5", "", "SCR08.BAS:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll5: SCR08.BAS

10 SCREEN 8
20 BLOAD"SCREEN.SC8",S

25 F% = 0 : T% = 1

30 K% = STICK(0)
40 IF K% = 0 THEN 30
50 IF K% = 3 THEN GOSUB 100
60 IF K% = 7 THEN GOSUB 200
70 GOTO 30

100 COPY (0, 0)-(251,212),F% TO (4,0),T%
101 COPY (252,0)-(255,212),F% TO (0,0),T%
102 GOTO 300

200 COPY (4, 0)-(255,212),F% TO (0,0),T%
201 COPY (0, 0)-(3,212),F% TO (252,0),T%

300 SET PAGE T%
301 IF T% = 1 THEN F% = 1 : T% = 0 ELSE F% = 0 : T% = 1
302 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-scroll5-scr12", "", "MSXBAS2ROM: Demo: scroll5", "", "SCR12.BAS:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: scroll5: SCR12.BAS

10 SCREEN 12
20 BLOAD"SCREEN.S12",S

25 F% = 0 : T% = 1

30 K% = STICK(0)
40 IF K% = 0 THEN 30
50 IF K% = 3 THEN GOSUB 100
60 IF K% = 7 THEN GOSUB 200
70 GOTO 30

100 COPY (0, 0)-(251,212),F% TO (4,0),T%
101 COPY (252,0)-(255,212),F% TO (0,0),T%
102 GOTO 300

200 COPY (4, 0)-(255,212),F% TO (0,0),T%
201 COPY (0, 0)-(3,212),F% TO (252,0),T%

300 SET PAGE T%
301 IF T% = 1 THEN F% = 1 : T% = 0 ELSE F% = 0 : T% = 1
302 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-demo-tesouro-perdido-tesperd", "", "MSXBAS2ROM: Demo: Tesouro Perdido", "", "tesperd.bas: ' Original de Renato Degiovani",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Demo: Tesouro Perdido: tesperd.bas

' O TESOURO PERDIDO
' Original de Renato Degiovani
' Adaptado por Amaury Carvalho, 2023
' Para compilar, digite no terminal:
'   msxbas2rom -c -x tesperd.bas

FILE "tesperd.akm"                  ' 0
FILE "tesperd.akx"                  ' 1
FILE "SCREEN1.SC2"                  ' 2
FILE "SCREEN2.SC2"                  ' 3
FILE "SCREEN3.SC2"                  ' 4
FILE "SCREEN4.SC2"                  ' 5
FILE "SCREEN5.SC2"                  ' 6
FILE "SCREEN6.SC2"                  ' 7
FILE "SCREEN7.SC2"                  ' 8
FILE "SCREEN8.SC2"                  ' 9

05 CMD PLYLOAD 0, 1                 ' Carrega musica e efeitos do Arkos Tracker

10 SCREEN 2,,0 : COLOR 1, 15
11 SCREEN LOAD 2                    ' Carrega tela de abertura
12 CMD PLYSONG 0 : CMD PLYPLAY      ' Toca tema musical de abertura
13 LOCATE 60, 130 : PRINT "O TESOURO PERDIDO"
14 LOCATE 64, 150 : PRINT "Renato Degiovani"

20 Frase$ = ""
21 Lamp = 0
22 Oleo = 0
23 Vaso = 0
24 Prta = 0
29 GOSUB 912
30 CMD PLYSONG 1 : CMD PLYPLAY      ' Toca tema musical do jogo

100 SCREEN LOAD 3                   ' Carrega fase inicial do jogo
110 LOCATE 20, 110 : PRINT "Voce esta ao lado de um"
111 LOCATE 20, 120 : PRINT "buraco no chao."
112 IF Lamp=1 GOTO 120
113 LOCATE 20, 130 : PRINT "Aqui tem um lampiao."
120 LOCATE 20, 150 : PRINT "E = entre no buraco"
123 LOCATE 20, 160 : PRINT "T = seus objetos"
124 IF Lamp=1 GOTO 130
128 LOCATE 20, 170 : PRINT "P = pegue o lampiao"
130 GOSUB 940
131 IF Frase$="P" or Frase$ = "p" GOTO 150
132 IF Frase$="E" or Frase$ = "e" GOTO 200
133 IF Frase$="T" or Frase$ = "t" GOTO 190
140 GOSUB 900
145 GOTO 100

150 Lamp=1
155 GOSUB 990
160 LOCATE 20, 110 : PRINT "Peguei o lampiao."
165 GOSUB 912
167 GOTO 100

190 GOSUB 800
192 GOTO 100

200 SCREEN LOAD 4
201 IF Lamp=1 GOTO 210
202 LOCATE 20, 110 : PRINT "Aqui esta escuro."
203 GOSUB 912
204 GOTO 100
210 LOCATE 20, 110 : PRINT "Voce esta dentro do"
212 LOCATE 20, 120 : PRINT "buraco, numa caverna."
225 LOCATE 20, 130 : PRINT "T = seus objetos"
226 LOCATE 20, 140 : PRINT "X = sair da caverna"
227 LOCATE 20, 150 : PRINT "S = ir para o sul"
228 LOCATE 20, 160 : PRINT "L = ir para leste"
229 LOCATE 20, 170 : PRINT "O = ir para oeste"
230 GOSUB 940
231 IF Frase$="X" OR Frase$="x" GOTO 100
232 IF Frase$="T" or Frase$="t" GOTO 290
233 IF Frase$="L" or frase$="l" GOTO 300
234 IF Frase$="S" or frase$="s" GOTO 400
235 IF Frase$="O" or frase$="o" GOTO 500
240 GOSUB 900
245 GOTO 200
290 GOSUB 800
292 GOTO 200

300 SCREEN LOAD 6+Oleo
310 LOCATE 20, 110 : PRINT "Voce esta numa sala da"
311 LOCATE 20, 120 : PRINT "caverna."
313 IF Oleo=1 GOTO 325
314 LOCATE 20, 130 : PRINT "Tem uma poca de oleo aqui."
316 IF Vaso=0 GOTO 325
317 LOCATE 20, 150 : PRINT "P = ponha o oleo no vaso"
325 LOCATE 20, 160 : PRINT "T = seus objetos"
326 LOCATE 20, 170 : PRINT "O = ir para oeste"
330 GOSUB 940
331 IF Frase$="T" OR Frase$="t" GOTO 390
332 IF Frase$="O" or frase$="o" GOTO 200
333 IF Frase$="P" or frase$="p" GOTO 380
340 GOSUB 900
345 GOTO 300

380 Oleo=1
381 GOSUB 990
382 LOCATE 20, 110 : PRINT "Ok, coloquei!"
384 GOSUB 912
386 GOTO 300
390 GOSUB 800
392 GOTO 300

400 SCREEN LOAD 5
410 LOCATE 20,110 : PRINT "Voce esta num corredor"
411 LOCATE 20,120 : PRINT "estreito, onde tem uma porta"
414 IF Prta=1 GOTO 428
415 LOCATE 20,130 : PRINT "A Porta esta travada e a"
416 LOCATE 20,140 : PRINT "dobradica esta enferrujada."
418 IF Vaso=0 GOTO 429
419 IF Oleo=0 GOTO 429
420 LOCATE 20,150 : PRINT "P = passe oleo na dobradica"
425 IF Prta=0 GOTO 429
428 LOCATE 20,160 : PRINT "S = ir para o sul"
429 LOCATE 20,170 : PRINT "N = ir para o norte"
430 GOSUB 940
431 IF Frase$="T" or Frase$="t" GOTO 490
432 IF Frase$="N" or Frase$="n" GOTO 200
433 IF Frase$="S" or Frase$="s" GOTO 600
434 IF Frase$="P" or Frase$="p" GOTO 480
440 GOSUB 900
445 GOTO 400
480 Prta=1
481 GOSUB 990
482 LOCATE 20, 110 : PRINT "Passei o oleo."
484 GOSUB 912
485 GOTO 400
490 GOSUB 800
495 GOTO 400

500 IF Vaso=0 THEN SCREEN LOAD 8 ELSE SCREEN LOAD 7
510 LOCATE 20,110 : PRINT "Voce esta numa sala da"
511 LOCATE 20,120 : PRINT "caverna."
513 IF Vaso=1 GOTO 525
514 LOCATE 20,130 : PRINT "Aqui tem um vaso"
517 LOCATE 20,150 : PRINT "P = pegue o vaso"
525 LOCATE 20,160 : PRINT "T = seus objetos"
526 LOCATE 20,170 : PRINT "L = ir para leste"
530 GOSUB 940
531 IF Frase$="T" OR Frase$="t" GOTO 590
532 IF Frase$="L" or Frase$="l" GOTO 200
533 IF Frase$="P" or Frase$="p" GOTO 580
540 GOSUB 900
545 GOTO 500
580 Vaso=1
581 GOSUB 990
582 LOCATE 20, 110 : PRINT "Peguei o vaso."
584 GOSUB 912
585 GOTO 500
590 GOSUB 800
595 GOTO 500

600 IF Ouro=0 THEN SCREEN LOAD 9 ELSE SCREEN LOAD 7
610 LOCATE 20, 110 : PRINT "Voce esta na sala do tesouro."
614 IF Ouro=1 GOTO 625
616 LOCATE 20, 120 : PRINT "Aqui tem uma barra de ouro."
620 LOCATE 20, 150 : PRINT "P = pegue a barra de ouro"
625 LOCATE 20, 160 : PRINT "T = seus objetos"
626 LOCATE 20, 170 : PRINT "N = ir para o norte"
630 GOSUB 940
631 IF Frase$="T" OR Frase$="t" GOTO 690
632 IF Frase$="N" or Frase$="n" GOTO 400
633 IF Frase$="P" or Frase$="p" GOTO 680
640 GOSUB 900
645 GOTO 600
680 Ouro=1
681 GOSUB 990
682 LOCATE 20, 110 : PRINT "Peguei o ouro."
684 GOSUB 912
685 GOTO 600
690 GOSUB 800
695 GOTO 600

800 GOSUB 990
810 LOCATE 20, 110 : PRINT "Voce tem os objetos:"
820 IF Lamp=1 THEN LOCATE 20, 130 : PRINT " - um lampiao"
830 IF Vaso=1 THEN LOCATE 20, 140 : PRINT " - um vaso"
840 IF Ouro=1 THEN LOCATE 20, 150 : PRINT " - uma barra de ouro"
899 GOTO 912

900 GOSUB 990
910 LOCATE 20, 110 : PRINT "Nao entendi o q eh p/fazer..."
911 CMD PLYSOUND 2                  ' Efeito de opcao errada

912 LOCATE 54, 170 : PRINT "Pressione uma tecla"
920 Frase$ = INKEY$
930 IF Frase$="" GOTO 920
931 GOTO 950

940 Frase$ = INPUT$(1)

950 CMD PLYSOUND 4                  ' Efeito de opcao selecionada
951 RETURN

990 LINE (20, 110)-(250,180),15,BF : RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-bobby-bobby", "", "MSXBAS2ROM: Jogo: Bobby", "", "bobby.bas: ' BOBBY IS STILL GOING HOME - MSXBAS2ROM DEMO (v.1.2)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Bobby: bobby.bas

'-----------------------------------------------------------------
' BOBBY IS STILL GOING HOME - MSXBAS2ROM DEMO (v.1.2)
'-----------------------------------------------------------------
' Game Info:
'   Bobby is Going Home is a platform game released for the Atari 2600 console in 1983.
'     https://en.wikipedia.org/wiki/Bobby_is_Going_Home
'     https://www.rfgeneration.com/news/2600/Banana-s-Rotten-Reviews-Bobby-Is-Going-Home-3473.php
'     https://www.retrogames.cz/play_192-Atari2600.php
'   Remake by Amaury Carvalho (2025):
'     https://github.com/amaurycarvalho/msxbasic/Bobby
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'   https://launchpad.net/nmsxtiles
'   https://github.com/pipagerardo/nMSXtiles
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom -x bobby.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/bobby.akm"                                  ' 0 - songs exported from ArkosTracker 3 (bobby.aks)
FILE "music/bobby_se.akx"                               ' 1 - sound effects exported from ArkosTracker 3 (bobby_se.aks)
FILE "levels/levels.csv"                                ' 2 - stage levels
FILE "img/splash.SC2"                                   ' 3 - splash screen (edited via nMSXTiles)
FILE "img/scene0.SC2"                                   ' 4 - scene 0 screen (edited via nMSXTiles)
FILE "img/scene1.SC2"                                   ' 5 - scene 1 screen (edited via nMSXTiles)
FILE "img/scene2.SC2"                                   ' 6 - scene 2 screen (edited via nMSXTiles)
FILE "img/scene3.SC2"                                   ' 7 - scene 3 screen (edited via nMSXTiles)
FILE "img/scene4.SC2"                                   ' 8 - scene 4 screen (edited via nMSXTiles)
FILE "img/scene5.SC2"                                   ' 9 - scene 5 screen (edited via nMSXTiles)
FILE "img/scene6.SC2"                                   ' 10 - scene 6 screen (edited via nMSXTiles)
FILE "img/scene7.SC2"                                   ' 11 - scene 7 screen (edited via nMSXTiles)
FILE "img/bobby.spr"                                    ' 12 - sprites bank (plain text, edited via Tiny Sprite)
FILE "img/flanders.SC2"                                 ' 13 - QR-Code for a Bobby-inspired song on YouTube (edited via nMSXTiles)

' Initialization
1 DEFINT A-Z
2 DIM LVA(15), BRBUF(5,14), REBUF(64,3)                 ' level data array, bridge and rolling enemies buffer
3 DIM HOS(2), SKS(4), SOT(3,1), FES(3), RET(2)          ' horizon, sky, stationary object, flying enemy and rolling enemy tiles/sprites
4 CMD PLYLOAD 0, 1                                      ' load music and effects from resources 0/1
5 SCREEN 2,2,0 : COLOR 15,0,0
6 SET TILE ON                                           ' set tile mode on (locate and print stmts works like it is in text mode)
7 IF NTSC() THEN TS = 60 : TD = 6 ELSE TS = 50 : TD = 5
8 VRSK = BASE(10) + 64                                  ' name table pointer to sky
9 VRRE = VRSK + 384 : VRBR = VRRE + 43                  ' name table pointer to rolling enemies and bridge in VRAM

' Splash screen
10 SCR = 3 : SPR = 12 : GOSUB 9050                      ' load splash screen and sprites
11 MUS = 6 : GOSUB 9030                                 ' play splash screen song
12 GOSUB 500                                            ' wait for player hit a button alternating screens
13 CMD PLYMUTE                                          ' mute song

' Game start
20 LR = 0                                               ' level row number in resource CSV
21 SC = 300                                             ' player initial score
22 LV = 5                                               ' player remainder lives
23 STI = 0                                              ' stage increment
24 HS = 1000                                            ' high score

' Level initialization
100 GOSUB 8100                                          ' load level data from resource
101 IF STN = 0 THEN LR=LR-8 : STI=STI+1 : GOTO 100      ' if end of level data, repeat last stage with a plus
102 GOSUB 400                                           ' level data initialization
103 GOSUB 8800                                          ' populate bridge and rolling enemies buffer

110 SCR = SCN + 4 : SPR = 12 : GOSUB 9050               ' load level screen and sprites
111 GOSUB 9030                                          ' play level song
112 GOSUB 8030                                          ' draw stage number on screen
113 GOSUB 8050                                          ' show player sprite
114 GOSUB 8080                                          ' show player remaining lives
115 GOSUB 320                                           ' show remaining objects on screen
116 GOSUB 600                                           ' check high score

' Gameplay loop
120 GOSUB 200                                           ' player logic
121 IF SC = 0 OR LV = 0 THEN 950                        ' if no score left or no more lives: game over
122 IF RST = 1 THEN 100                                 ' if restart is flagged: restart stage

130 GOSUB 300                                           ' remaining objects logic
131 IF SC = 0 OR LV = 0 THEN 950                        ' if no score left or no more lives: game over
132 IF RST = 1 THEN 100                                 ' if restart is flagged: restart stage

140 IF PX >= 240 THEN 900                               ' if player reached end of stage: next stage
141 IF LVA(1) = 7 THEN IF PX > 200 THEN 910             ' if player arrived at home: do player at home logic
'142 IF INKEY = 9 THEN LR = LR + 1 : GOTO 100

150 GOTO 120

' Player movement logic
200 IF TIME < PT THEN PT = TIME
201 DI = TIME - PT
202 IF DI < TD THEN RETURN                              ' time step = 10x per second
203   PT = TIME

210 GOSUB 9000                                          ' get player input
211 IF K  <> 0 GOSUB 800                                ' player press the button?
212 IF PJ <> 0 GOSUB 810                                ' is player jumping?
213 IF RST = 1 THEN RETURN                              ' if restart is flagged: return
214 ON J GOTO 220, 219, 230, 219, 240, 219, 250, 219    ' player press the stick?
219   RETURN

' --> player moves UP
220 RETURN

' --> player moves RIGHT
230 IF PX >= 240 THEN RETURN                            ' if hit right border: return
231   PX = PX + PI                                      ' moves right
232   IF PJ <> 0 THEN PS = 4 ELSE PS = (PS + 1) MOD 4   ' change player sprite
233   GOTO 260                                          ' show player and test collision

' --> player moves DOWN
240 RETURN

' --> player moves LEFT
250 IF PX <= 0 THEN RETURN                              ' if hit right border: return
251   PX = PX - PI                                      ' moves left
252   IF PJ <> 0 THEN PS=10 ELSE PS=(PS + 1) MOD 4 + 6  ' change player sprite
253   GOTO 260                                          ' show player and test collision

' Player tiles collision logic
260 GOSUB 8050                                          ' show player sprite
261 GOSUB 8060                                          ' get player tiles
262 IF PTBL >= 97 THEN IF ((PTBL - 97) MOD 4) >= 2 THEN 290     ' player hit an obstacle or rolling enemy?
263 IF PTBR >= 97 THEN IF ((PTBR - 97) MOD 4) >= 2 THEN 290     ' player hit an obstacle or rolling enemy?
264 IF PY < 111 THEN 280                                ' is player in the air?

270 IF PTBL <> 36 OR PTBR <> 36 THEN 280                ' player not fell on a hole?
271   MUS = 5 : GOSUB 9030                              ' player is drowning song
272   GOSUB 340                                         ' player loses 1 life logic
273   FOR SS = 24 TO 32 STEP 3
274     GOSUB 8070                                      ' player is drowning sprite
275     CMD PLYSOUND 6, 2                               ' player is drowning sound effect on channel 2
276     I = 1 : GOSUB 9020                              ' wait 1 second
277   NEXT
278   I = 1
279   GOTO 9020                                         ' wait 1 second

' Player sprite collision logic
280 OS = COLLISION(0,7)                                 ' check if player sprite collided with a flying enemy sprite
281 IF OS < 0 THEN RETURN                               ' if nothing collided with player, return
282   DX = PX - FX                                      ' calculate how far the sprites are from each other
283   DY = PY - FY
284   IF DX < -10 OR DX > 10 THEN RETURN                ' ignore if distance is not close enough
285   IF DY < -10 OR DY > 10 THEN RETURN
286   GOTO 290                                          ' player hit something logic

' Player hit something logic
290 GOSUB 340                                           ' player loses 1 life logic
291 GOSUB 960                                           ' show player is dead
292 IF LV = 0 THEN RETURN                               ' if no more lives, game over
293   I = 2
294   GOTO 9020                                         ' wait 2 seconds

' Remaining objects logic
300 IF TIME < OT THEN OT = TIME
301 DI = TIME - OT
302 IF DI < (TD*2) THEN RETURN                          ' time step = 5x per second
303   OT = TIME

310 IF LVA(1) < 7 THEN SC = SC - (OSF AND 1)            ' if player not at home, decrease score 2x per second
311 OSF = (OSF + 1) MOD 5                               ' change object step flag

320 GOSUB 8040                                          ' draw score on screen
321 GOSUB 8200                                          ' draw horizon sprite
322 GOSUB 8300                                          ' draw sky sprite
323 GOSUB 8400                                          ' draw bridge
324 GOSUB 8500                                          ' draw stationary obstacle
325 GOSUB 8600                                          ' draw flying enemy
326 GOSUB 8700                                          ' draw rolling enemy
327 GOSUB 260                                           ' player collision logic
328 IF LVA(1) < 7 THEN RETURN

' Home sky animation
330 BUF = HEAP()                                        ' free RAM area buffer
331 CMD VRAMTORAM VRSK, BUF, 192                        ' copy VRAM sky to RAM buffer
332 POKE BUF+192,PEEK(BUF)                              ' copy first buffer tile to buffer's end
333 CMD RAMTOVRAM BUF+1, VRSK, 192                      ' copy RAM buffer to VRAM sky

' Home special character animation
334 PUT TILE HSCT + HSCS,(24, 10)                       ' home special character tile + step
335 IF HSCS = 0 THEN HSCI = 1 : GOTO 337
336 IF HSCS = 6 THEN HSCI = -1
337 HSCS = HSCS + HSCI
338 RETURN

' Player loses 1 life logic
340 SC = SC + 100                                       ' increment score
341 IF LV > 0 THEN LV = LV - 1                          ' decrement lives
342 RST = 1                                             ' restart level
343 RETURN

' Level data initialization
400 PX = 0 : PY = 111 : PS = 0 : PJ = 0 : PI = 4        ' player x, y, sprite, jumping flag and walking step
401 PT = TIME                                           ' timer for player
402 OT = TIME : OSF = 0                                 ' timer for remaining objects and object step flag
403 HX = 0 : HY = 58                                    ' horizon object position X and Y
404 SX = 200 : SY = 20 : SXI = -8 : SYI = 4             ' sky object position X and Y, and speed
405 FX=150 : FY=90 : FXI= -8-LVA(12) : FYI= 4+LVA(12)   ' flying enemy position X and Y, and speed
406 RST = 0                                             ' restart flag and stage increment
407 HSCN = (STN - 1) MOD 5                              ' home special character related to stage number (0 - 4)
408 HSCT = 129 + HSCN*7                                 ' home special character initial tile
409 HSCS = 0                                            ' home special character tile step
410 HSCI = 1                                            ' home special character tile increment
411 RETURN

' Splash screen / youtube QR-Code
500 TT = TS*20                                          ' TT = 20 seconds
501 OT = TIME

510 GOSUB 9000                                          ' get player input
511 IF K <> 0 THEN RETURN

520 IF TIME < OT THEN OT = TIME
521 DI = TIME - OT
522 IF DI < TT THEN 510                                                 ' wait for TT seconds
523   IF SCR = 3 THEN SCR = 13 : TT = TS*10 ELSE SCR = 3 : TT = TS*20   ' alternate screens
524   GOSUB 9050                                                        ' load splash screen and sprites
525   GOTO 501                                                          ' repeat

' Check high score
600 IF SC < HS THEN RETURN
601   HS = HS + 1000                                    ' increment high score
602   GOSUB 8040                                        ' show score
603   IF LV >= 7 THEN RETURN
604     LV = LV + 1                                     ' add 1 life
605     GOSUB 8080                                      ' show player remaining lives
606     CMD PLYSOUND 4,1                                ' new life sound
607     RETURN

' Player jumping button
800 IF LVA(1) = 7 THEN RETURN                           ' ignore if player at home
801 IF PJ <> 0 THEN RETURN                              ' ignore if player is already jumping
802   CMD PLYSOUND 7,2                                  ' player jumping sound effect 7 on channel 2
803   PJ = -4 : PI = 3                                  ' player jumping and set slow walking step
804   IF PS > 5 THEN PS = 10 ELSE PS = 4                ' player is jumping sprite
805   GOTO 8050                                         ' show player sprite

' Player jumping logic
810 PY = PY + PJ
811 IF PY <= 80 THEN PJ = 4                             ' if roof, reverse jumping
812 IF PY >= 111 THEN 820                               ' if floor, end jumping
813 GOSUB 8050                                          ' show player sprite
814 GOTO 260                                            ' collision logic

' Player stops jumping logic
820 IF PS > 5 THEN PS = 6 ELSE PS = 0                   ' player sprite
821 PJ = 0 : PI = 4                                     ' stop jumping and set normal walking step
822 GOSUB 8050                                          ' show player sprite
823 GOTO 260                                            ' collision logic

' Next stage logic
900 LR = LR + 1                                         ' add level row number in resource CSV
901 SC = SC + 100
902 GOTO 100                                            ' go to next stage

' Player at home logic
910 FOR I = 1 TO 50                                     ' add 5000 points showing the score
911   SC = SC + 10
912   GOSUB 8040                                        ' show score
913   IF (I MOD 4) = 0 THEN CMD PLYSOUND 1, 2
914   GOSUB 600                                         ' check high score
915 NEXT
920 I = 4 : GOSUB 9020                                  ' wait 4 seconds
921 CMD PLYMUTE
922 LR = LR + 1                                         ' add level row number in resource CSV
923 GOTO 100

' Game over logic
950 GOSUB 8080                                          ' show player remaining lives
'951 GOSUB 8040                                          ' show score
952 I = 3 : GOSUB 9020                                  ' wait 3 seconds
953 GOSUB 9010                                          ' wait for player hit a button
954 GOTO 20                                             ' restart the game

' Player is dead logic
960 IF PS > 5 THEN PS = 11 ELSE PS = 5                  ' player is dead sprite
961 MUS = 5 : GOSUB 9030                                ' player is dead song
962 CMD PLYSOUND 2, 2                                   ' player is dead sound effect on channel 2
963 GOTO 8050                                           ' show player sprite

'-------------------------------------------
' GAME SUPPORT ROUTINES
'-------------------------------------------

' Put tile block 2x2 started by N on X,Y (vertical way)
8000 PUT TILE N,(X,Y)
8001 PUT TILE N+1,(X,Y+1)
8002 PUT TILE N+2,(X+1,Y)
8003 PUT TILE N+3,(X+1,Y+1)
8004 RETURN

' Put tile block 2x1 started by N on X,Y
8010 PUT TILE N,(X,Y)
8011 PUT TILE N+1,(X+1,Y)
8012 RETURN

' Clear tile block 2x2 on X,Y
8015 PUT TILE 32,(X,Y)
8016 PUT TILE 32,(X,Y+1)
8017 PUT TILE 32,(X+1,Y)
8018 PUT TILE 32,(X+1,Y+1)
8019 RETURN

' Show stage STN
8030 LOCATE 14,22 : PRINT USING$("###", STN+STI);
8031 RETURN

' Print score
8040 LOCATE 12, 0
8041 PRINT USING$("######", SC*10.0);
8042 RETURN

' Show player sprite
8050 SS = (PS * 2)
8051 PUT SPRITE 0,(PX,PY),1,SS
8052 PUT SPRITE 1,(PX,PY),11,SS+1
8053 RETURN

' get tiles from player sprite edges hot spots
8060 Y = (PY/8) + 2                                     ' current player position in text coords
8061 X = PX/8
8062 PTBL = TILE(X,Y)                                   ' player bottom-left tile
8063 X = (PX+12)/8
8064 PTBR = TILE(X,Y)                                   ' player bottom-right tile
8065 RETURN

' Show player (3 sprites starting in SS)
8070 PUT SPRITE 0,(PX,PY),1,SS
8071 PUT SPRITE 1,(PX,PY),11,SS+1
8072 PUT SPRITE 2,(PX,PY),4,SS+2
8073 RETURN

' Show lives
8080 X = 1 : Y = 22
8081 FOR I = 2 TO 7
8082   IF I <= LV THEN C = 58 ELSE C = 32
8083   PUT TILE C,(X, Y)
8084   X = X + 2
8085 NEXT
8086 RETURN

' Put tile block 2x2 started by N on X,Y (horizontal way)
8090 PUT TILE N,(X,Y)
8091 PUT TILE N+1,(X+1,Y)
8092 PUT TILE N+2,(X,Y+1)
8093 PUT TILE N+3,(X+1,Y+1)
8094 RETURN

' Load stage setup from levels data resource
8100 CMD RESTORE 2                                      ' levels data on resource number 2
8101 RESTORE LR                                         ' go to row number LR in levels data resource CSV
8102 FOR I = 0 TO 15
8103   READ LVA(I)
8104 NEXT
8110 STN = LVA(0)                                       ' stage number
8111 SCN = LVA(1)                                       ' scene number
8112 MUS = LVA(2)                                       ' song number
8120 HOS(1) = 33 : HOS(2) = 53                          ' horizon sprite (1=ship, 2=train)
8121 SKS(1) = 37 : SKS(2) = 41 : SKS(3)=45 : SKS(4)=49  ' sky sprite (1=condor, 2=cloud, 3=eagle, 4=sun)
8122 SOT(1,0) =  97 : SOT(1,1) =  97                    ' stationary obstacle tile (1=hydrant)
8123 SOT(2,0) = 101 : SOT(2,1) = 101                    ' stationary obstacle tile (2=flower)
8124 SOT(3,0) = 105 : SOT(3,1) = 109                    ' stationary obstacle tile (3=fountain)
8125 FES(1) = 57 : FES(2) = 59 : FES(3) = 61            ' flying enemy sprite (0=nothing, 1=bird, 2=butterfly, 3=bat)
8126 RET(1) = 113 : RET(2) = 121                        ' rolling enemy tile (0=nothing, 1=barrel, 2=chicken)
8127 RETURN

' Show horizon sprite
8200 IF LVA(3) = 0 THEN RETURN                          ' if no horizon sprite, return
8201   SS = HOS(LVA(3)) + (OSF AND 1)*2
8202   PUT SPRITE 3,(HX,HY),LVA(4),SS
8203   PUT SPRITE 4,(HX+16,HY),LVA(4),SS+1
8204   HX = HX + 4 : IF HX < 228 THEN RETURN
8205   IF HY = 193 THEN HY = 58 : HX = 0 ELSE HY = 193 : HX = 100 + (R MOD 100)
8206   RETURN

' Show sky sprite
8300 IF LVA(5) = 0 THEN RETURN                          ' if no sky sprite, return
8301   SS = SKS(LVA(5)) + (OSF AND 1)*2
8302   PUT SPRITE 5,(SX,SY),LVA(6),SS
8303   PUT SPRITE 6,(SX+16,SY),LVA(6),SS+1
8304   SX = SX + SXI
8305   SY = SY + SYI
8306   IF SX < 0 THEN SX = 0 : SXI = -SXI ELSE IF SX > 228 THEN SX = 228 : SXI = -SXI
8307   IF SY < 8 THEN SY = 8 : SYI = -SYI ELSE IF SY > 30 THEN SY = 30 : SYI = -SYI
8308   RETURN

' Show bridge
8400 IF LVA(7) = 0 THEN RETURN                          ' if no bridge, return
8401   VRBR1 = VRBR
8402   Y = BRY
8403   FOR I = 0 TO 2
8404     X = VARPTR( BRBUF(0, Y+I) )
8405     CMD RAMTOVRAM X, VRBR1, 10                     ' copy ram buffer to vram
8406     VRBR1 = VRBR1 + 32                             ' next line
8407   NEXT
8408   BRY = BRY + BRI
8409   IF BRY < 0 OR BRY > 12 THEN BRI = -BRI : BRY = BRY + BRI
8410   RETURN

' Show stationary obstacle
8500 IF LVA(8)=0 OR LVA(13) > 0 THEN RETURN             ' if no stationary obstacle or rolling enemy, return
8501   X = LVA(9) : Y = 14                              ' stationary obstacle X position
8502   N = SOT(LVA(8), OSF AND 1)                       ' stationary obstacle tile number
8503   GOTO 8090                                        ' print 2x2 block (horizontal way)

' Show flying enemy
8600 IF LVA(10) = 0 THEN RETURN                         ' if no flying enemy, return
8601   SS = FES(LVA(10)) + (OSF AND 1)
8602   PUT SPRITE 7,(FX,FY),LVA(11),SS
8603   FX = FX + FXI
8604   FY = FY + FYI
8610   IF FXI > 0 THEN 8620                             ' if moving to right, go to moving right logic
8611     IF FX > 80 OR FX < 40 THEN 8630                ' else it's moving left, so if not in first 1/3 of screen go to borders logic
8612       GOTO 8621                                    '   else go to change direction logic
8620     IF FX < 160 OR FX > 210 THEN 8630              ' moving right logic: if not in last 1/3 of screen go to border logic
8621       GOSUB 9005 : IF R < 10 THEN FXI = -FXI       '   change direction logic: 10% of chance to swap direction
8630   IF FX < 40 THEN FX = 40 : FXI = -FXI ELSE IF FX > 210 THEN FX = 210 : FXI = -FXI     ' left/right border logic
8631   IF FY < 80 THEN FY = 80 : FYI = -FYI ELSE IF FY > 110 THEN FY = 110 : FYI = -FYI     ' top/bottom border logic
8632   RETURN

' Show rolling enemy
8700 IF LVA(13) = 0 OR LVA(7) = 1 THEN RETURN           ' if no rolling enemy or there's a bridge, return
8701   Y = (OSF AND 1)*2
8702   X = VARPTR( REBUF(0, Y) ) + REX
8703   CMD RAMTOVRAM X, VRRE, 32                        ' copy ram buffer to vram
8704   X = VARPTR( REBUF(0, Y+1) ) + REX
8705   CMD RAMTOVRAM X, VRRE+32, 32
8710   REX = REX + 1 + LVA(14)                          ' rolling enemy speed
8711   IF LVA(15) < 4 THEN DX = 48 ELSE DX = 56
8712   IF REX > DX THEN REX = 0
8713   RETURN

' Populate bridge and rolling enemies buffer
' --> bridge closed
8800 BRBUF(0, 0) = &h2323 : BRBUF(1, 0) = &h2323 : BRBUF(2, 0) = &h2323 : BRBUF(3, 0) = &h2323 : BRBUF(4, 0) = &h2323
8801 BRBUF(0, 1) = &h2222 : BRBUF(1, 1) = &h2222 : BRBUF(2, 1) = &h2222 : BRBUF(3, 1) = &h2222 : BRBUF(4, 1) = &h2222
8802 BRBUF(0, 2) = &h2222 : BRBUF(1, 2) = &h2525 : BRBUF(2, 2) = &h2424 : BRBUF(3, 2) = &h2525 : BRBUF(4, 2) = &h2222
' --> bridge start to open
8803 BRBUF(0, 3) = &h2323 : BRBUF(1, 3) = &h2323 : BRBUF(2, 3) = &h2424 : BRBUF(3, 3) = &h2323 : BRBUF(4, 3) = &h2323
8804 BRBUF(0, 4) = &h2222 : BRBUF(1, 4) = &h2222 : BRBUF(2, 4) = &h2424 : BRBUF(3, 4) = &h2222 : BRBUF(4, 4) = &h2222
8805 BRBUF(0, 5) = &h2222 : BRBUF(1, 5) = &h2525 : BRBUF(2, 5) = &h2424 : BRBUF(3, 5) = &h2525 : BRBUF(4, 5) = &h2222
' --> bridge mid opened
8806 BRBUF(0, 6) = &h2323 : BRBUF(1, 6) = &h2423 : BRBUF(2, 6) = &h2424 : BRBUF(3, 6) = &h2324 : BRBUF(4, 6) = &h2323
8807 BRBUF(0, 7) = &h2222 : BRBUF(1, 7) = &h2422 : BRBUF(2, 7) = &h2424 : BRBUF(3, 7) = &h2224 : BRBUF(4, 7) = &h2222
8808 BRBUF(0, 8) = &h2522 : BRBUF(1, 8) = &h2424 : BRBUF(2, 8) = &h2424 : BRBUF(3, 8) = &h2424 : BRBUF(4, 8) = &h2225
' --> bridge almost opened
8809 BRBUF(0, 9) = &h2323 : BRBUF(1, 9) = &h2424 : BRBUF(2, 9) = &h2424 : BRBUF(3, 9) = &h2424 : BRBUF(4, 9) = &h2323
8810 BRBUF(0,10) = &h2222 : BRBUF(1,10) = &h2424 : BRBUF(2,10) = &h2424 : BRBUF(3,10) = &h2424 : BRBUF(4,10) = &h2222
8811 BRBUF(0,11) = &h2522 : BRBUF(1,11) = &h2424 : BRBUF(2,11) = &h2424 : BRBUF(3,11) = &h2424 : BRBUF(4,11) = &h2225
' --> bridge almost opened
8812 BRBUF(0,12) = &h2423 : BRBUF(1,12) = &h2424 : BRBUF(2,12) = &h2424 : BRBUF(3,12) = &h2424 : BRBUF(4,12) = &h2324
8813 BRBUF(0,13) = &h2422 : BRBUF(1,13) = &h2424 : BRBUF(2,13) = &h2424 : BRBUF(3,13) = &h2424 : BRBUF(4,13) = &h2224
8814 BRBUF(0,14) = &h2422 : BRBUF(1,14) = &h2424 : BRBUF(2,14) = &h2424 : BRBUF(3,14) = &h2424 : BRBUF(4,14) = &h2224
8815 BRY = 0                                            ' bridge Y offset
8816 BRI = 3                                            ' bridge Y increment
' --> clear rolling enemies buffer
8820 FOR Y = 0 TO 3                                     ' rolling enemies lines
8821   IF (Y AND 1) = 0 THEN C = &h2424 ELSE C = &h2323 ' empty space or floor tile
8822   FOR X = 0 TO 64                                  ' rolling enemies columns
8823     REBUF(X,Y) = C
8824   NEXT
8825 NEXT
' --> populate rolling enemies
8830 X = 16                                             ' 2nd screen position on the buffer
8831 C = RET(LVA(13))                                   ' rolling enemy tile
8832 FOR I = 1 TO LVA(15)                               ' rolling enemies quantity
8833   REBUF(X,0) = ((C+1) SHL 8) OR  C                 ' tile animation 1
8834   REBUF(X,1) = ((C+3) SHL 8) OR (C+2)
8835   REBUF(X,2) = ((C+5) SHL 8) OR (C+4)              ' tile animation 2
8836   REBUF(X,3) = ((C+7) SHL 8) OR (C+6)
8837   X = X + 4                                        ' 6 tiles of space
8838 NEXT
8840 REX = 0                                            ' rolling enemies X offset
8841 RETURN

'-------------------------------------------
' GENERAL SUPPORT ROUTINES
'-------------------------------------------

' Get player input (out: K, J)
9000 K = STRIG(0) OR STRIG(1)
9001 J = STICK(0) OR STICK(1)

' Next random number
9005 R = RND(1) * 100
9006 RETURN

' Wait for player hit a button
9010 GOSUB 9000                                         ' get player input
9011 IF K=0 THEN 9010
9012 RETURN

' Wait I seconds
9020 T1 = TIME : I = I * TS
9021 IF T1 > TIME THEN T1 = TIME
9022 DT = TIME - T1
9023 IF DT < I THEN 9021
9024 RETURN

' Play song MUS
9030 IF MUS = OLDMUS THEN RETURN                        ' ignore if same song
9031   OLDMUS = MUS
9032   CMD PLYSONG MUS
9033   CMD PLYPLAY
9034   RETURN

' Clear sprites
9040 FOR A=0 TO 31
9041   PUT SPRITE A,,0,0
9042 NEXT
9043 RETURN

' Load screen (SCR) and sprite (SPR) resources
9050 SCREEN OFF
9051 SCREEN LOAD SCR
9052 SPRITE LOAD SPR
9053 GOSUB 9040                                         ' clear sprites
9054 SCREEN ON
9055 RETURN

' Clear input buffer
9060 GOSUB 9000
9061 IF K <> 0 THEN 9060
9062 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-bobby-readme", "", "MSXBAS2ROM: Jogo: Bobby", "", "README.md: Game source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Bobby: README.md

BOBBY IS STILL GOING HOME
Game source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- Bobby is Going Home is a platform game released for the Atari 2600 console in 1983;
- This remake for MSX systems was developed by Amaury Carvalho (2025);

References

- Wikipedia;
- Game Review;
- Original Atari 2600 version emulation;
- MSXdev25 contest.

SOURCE CODE

Compiling

       msxbas2rom -x bobby.bas

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter and edited with nMSXTiles;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-donkey-donkey", "", "MSXBAS2ROM: Jogo: Donkey", "", "DONKEY.BAS: ' DONKEY.BAS - MSXBAS2ROM DEMO (v.1.2)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Donkey: DONKEY.BAS

'-----------------------------------------------------------------
' DONKEY.BAS - MSXBAS2ROM DEMO (v.1.2)
'-----------------------------------------------------------------
' Game Info:
'   DONKEY.BAS is a computer game written in 1981 and included with early versions of the PC DOS operating system
'   distributed with the original IBM PC. It is a driving game in which the player must avoid hitting donkeys.
'   The game was written by Microsoft co-founder Bill Gates and Neil Konzen.
'     https://en.wikipedia.org/wiki/DONKEY.BAS
'   Original PC version:
'     The IBM Personal Computer Donkey
'     Version 1.10 (C)Copyright IBM Corp 1981, 1982
'     Licensed Material - Program Property of IBM
'     https://github.com/philspil66/DONKEY.BAS
'   Remake to MSX by Amaury Carvalho (2025):
'     https://github.com/amaurycarvalho/msxbasic/Donkey
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom DONKEY.BAS
'-----------------------------------------------------------------

1010 KEY OFF:SCREEN 0,1,0:COLOR 15,0,0:WIDTH 40:CLS:LOCATE 19,5:PRINT "IBM"
1020 LOCATE 12,7:PRINT "Personal Computer"
1030 COLOR 10,0:LOCATE 9,10:PRINT CHR$(213)+STRING$(21,205)+CHR$(184)
1040 LOCATE 9,11:PRINT CHR$(179)+"       DONKEY        "+CHR$(179)
1050 LOCATE 9,12:PRINT CHR$(179)+STRING$(21,32)+CHR$(179)
1060 LOCATE 9,13:PRINT CHR$(179)+"    Version 1.1O     "+CHR$(179)
1070 LOCATE 9,14:PRINT CHR$(212)+STRING$(21,205)+CHR$(190)
1080 COLOR 15,0:LOCATE 4,17:PRINT "(C) Copyright IBM Corp 1981, 1982"
1090 COLOR 14,0:LOCATE 7,23:PRINT "Press space bar to continue"
1100 IF INKEY<>0 THEN GOTO 1100
1110 K = INKEY
1120 IF K = 0 THEN GOTO 1110
1130 IF K = 27 THEN GOTO 1298
1140 IF K = 32 THEN GOTO 1291
1150 GOTO 1110

1291 KEY OFF
1293 PLAY "L10CDE"
1294 GOTO 1410

1298 SCREEN 1,1,0
1299 COLOR 7,0,0:CLS:END

1410 COLOR 0
1420 DEFINT A-Y
1440 SCREEN 2,0,0:COLOR 8,1
1450 DIM Q%(500)
1460 DIM D1%(150),D2%(150),C1%(200),C2%(200)
1470 DIM DNK%(300)
1480 GOSUB 1940
1490 GOSUB 1780
1500 CLS
1510 DIM B%(300)
1520 FOR I=2 TO 300:B%(I)=-16384+192:NEXT
1530 B%(0)=2:B%(1)=193

1540 CX=110
1550 CLS
1590 LINE (0,0)-(305,199),,B
1600 LINE (6,6)-(97,195),1,BF
1610 LINE (183,6)-(305,195),1,BF
1620 LOCATE 5,3:PRINT "Donkey"
1630 LOCATE 29,3:PRINT "Driver"
1631 LOCATE 25,19:PRINT"Press Space  ";
1632 LOCATE 25,20:PRINT"Bar to switch";
1633 LOCATE 25,21:PRINT"lanes        ";
1635 LOCATE 25,23:PRINT"Press ESC    ";
1636 LOCATE 25,24:PRINT"to exit      ";
1640 FOR Y=4 TO 199 STEP 20:LINE(140,Y)-(140,Y+10):NEXT
1650 CY=105:CX=105
1660 LINE (100,0)-(100,199):LINE(180,0)-(180,199)
1670 LOCATE 6,5:PRINT SD:LOCATE 31,5:PRINT SM
1680 CY=CY-4:IF CY<60 THEN 2230
1690 COPY CAR% TO (CX,CY) ',PRESET
1700 DX=105+42*INT(RND(1)*2)
1710 FOR Y=(RND(1)*-4)*8 TO 124 STEP 6
1720 PLAY "C" 'SOUND 20000,1
1730 K=INKEY : IF K=27 THEN 1298 ELSE IF K > 0 THEN LINE (CX,CY)-(CX+28,CY+44),0,BF:CX=252-CX:COPY CAR% TO (CX,CY) : BEEP ',PRESET:SOUND 200,1
1740 IF Y>=3 THEN COPY DNK% TO (DX,Y) ',PSET
1750 IF CX=DX AND Y+25>=CY THEN 2060
1760 IF Y AND 3 THEN COPY B% TO (140,6)
1770 NEXT:LINE (DX,124)-(DX+32,149),0,BF:GOTO 1670

1780 CLS
1790 DRAW "S8C3"
1800 DRAW"BM12,1r3m+1,3d2R1ND2u1r2d4l2u1l1"
1810 DRAW"d7R1nd2u2r3d6l3u2l1d3m-1,1l3"
1820 DRAW"m-1,-1u3l1d2l3u6r3d2nd2r1u7l1d1l2"
1830 DRAW"u4r2d1nd2R1U2"
1840 DRAW"M+1,-3"
1850 DRAW"BD10D2R3U2M-1,-1L1M-1,1"
1860 DRAW"BD3D1R1U1L1BR2R1D1L1U1"
1870 DRAW"BD2BL2D1R1U1L1BR2R1D1L1U1"
1880 DRAW"BD2BL2D1R1U1L1BR2R1D1L1U1"
1890 LINE(0,0)-(40,60),,B
1900 PAINT (1,1)
1910 DIM CAR%(900)
1920 COPY (1,1)-(29,45) TO CAR%
1930 RETURN

1940 CLS
1950 DRAW"S08"
1960 DRAW "BM14,18"
1970 DRAW"M+2,-4R8M+1,-1U1M+1,+1M+2,-1"
1980 DRAW"M-1,1M+1,3M-1,1M-1,-2M-1,2"
1990 DRAW"D3L1U3M-1,1D2L1U2L3D2L1U2M-1,-1"
2000 DRAW"D3L1U5M-2,3U1"
2010 PAINT (21,14),3
2020 PRESET (37,10):PRESET (40,10)
2030 PRESET (37,11):PRESET (40,11)
2040 COPY (13,0)-(45,25) TO DNK%
2050 RETURN

2060 SD=SD+1:LOCATE 6,14:PRINT "BOOM!"
2070 COPY (DX,Y)-(DX+16,Y+25) TO D1%
2080 D1X=DX:D1Y=Y:D2X=DX+17
2090 COPY (DX+17,Y)-(DX+31,Y+25) TO D2%
2100 COPY (CX,CY)-(CX+14,CY+44) TO C1%
2110 COPY (CX+15,CY)-(CX+28,CY+44) TO C2%
2120 C1X=CX:C1Y=CY:C2X=CX+15
2130 FOR P=6 TO 0 STEP -1:Z=1/(2^P):Z1=1-Z
2140 COPY C1% TO (C1X,C1Y) : COPY C2% TO (C2X,C1Y)
2150 COPY D1% TO (D1X,D1Y) : COPY D2% TO (D2X,D1Y)
2160 C1X=CX*Z1:D1Y=Y*Z1:C2X=C2X+(291-C2X)*Z
2170 D1X=DX*Z1:C1Y=C1Y+(155-C1Y)*Z:D2X=D2X+(294-D2X)*Z
2180 COPY C1% TO (C1X,C1Y) : COPY C2% TO (C2X,C1Y)
2190 COPY D1% TO (D1X,D1Y) : COPY D2% TO (D2X,D1Y)
2200 BEEP : NEXT 'SOUND 37+RND(1)*200,4:NEXT
2210 FOR Y=1 TO 2000:NEXT

2220 CLS:GOTO 1540
2230 SM=SM+1:LOCATE 25,7:PRINT "Donkey loses!"
2240 FOR Y=1 TO 1000:NEXT
2250 CLS:GOTO 1540

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-donkey-readme", "", "MSXBAS2ROM: Jogo: Donkey", "", "README.md: Donkey, often known by its file name DONKEY.BAS, is a computer game written in...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Donkey: README.md

DONKEY.BAS

WHAT IS DONKEY.BAS?
Donkey, often known by its file name DONKEY.BAS, is a computer game written in 1981 and included with early versions of the PC DOS operating system distributed with the original IBM PC. It is a driving game in which the player must avoid hitting donkeys. The game was written by Microsoft co-founder Bill Gates and Neil Konzen.

HISTORY
When IBM was developing the PC in the late 1970s and early 1980s it contracted Microsoft to develop an operating system and a version of the BASIC programming language to release with the new computer. The operating system was released as PC DOS when included with IBM PCs and MS-DOS when sold separately by Microsoft. Both included versions of Microsoft BASIC.

DONKEY.BAS was written by Bill Gates and Neil Konzen to demonstrate the IBM PC and the BASIC programming language's capability to produce interactive programs with color graphics and sound. The game continues to generate interest in part because of the involvement of Gates at a time when Microsoft was relatively small and only six years old.

The first version of DONKEY.BAS was released in 1981, followed by version 1.10 in 1982. The operating systems with which the game was first distributed still work on modern computers with compatible BIOS and 5.25-inch floppy drives; however, IBM BASICA which ran the program under PC DOS 1.x requires ROM-based IBM Cassette BASIC, which modern computers do not have.

GAMEPLAY
DONKEY.BAS is an extremely simple driving game in which the player controls a car but cannot steer, accelerate or brake, only changing lanes to avoid a series of donkeys on the road. There is no goal other than to avoid donkeys.

The center of the screen shows a vertical scrolling road with two lanes; the areas either side of the road are used for scores and instructions. The player's car is driving up the road and every few seconds a donkey will appear at random on one side of the road at the top of the screen. As the donkey moves down the screen the player can press the space bar to switch between lanes to avoid the donkey. If the car hits the donkey, both car and donkey explode, and parts of the graphics are scattered to the four corners of the screen to the sound of a short monophonic tune played through the PC speaker, with the word "BOOM!" displayed on the left side of the screen. If the player avoids the donkey, it will scroll off the bottom of the screen, with the words "Donkey loses!" displayed on the right side of the screen, and after a few seconds another will appear. There is never more than one donkey on the screen at any one time.

The game keeps score between the player and the donkeys. If the car hits a donkey, the donkey gets a point and the player is returned to the start of the road. As the car avoids donkeys it moves slowly up the screen, giving the player less time to react when donkeys appear. If the car avoids enough donkeys the player receives a point and the car is moved back to the bottom of the road. The game displays the number of points earned by the player and donkey but does not end or change when a particular score is reached.

Apart from pressing the space bar, the only control available to the player is to press the Esc key and quit the game.

MSX VERSION

This MSX version was adapted by Amaury Carvalho (2025) and compiled with MSXBAS2ROM MSX BASIC compiler.

- Try it on WebMSX;
- Source code on Github.

REFERENCES

- Wikipedia
- Repository with original MS-BASIC source code for PC

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-dragon-treasure-drgtsr", "", "MSXBAS2ROM: Jogo: Dragon Treasure", "", "drgtsr.bas: ' Game Info:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Dragon Treasure: drgtsr.bas

'-----------------------------------------------------------------
' DRAGON TREASURE - MSXBAS2ROM DEMO
'-----------------------------------------------------------------
' Game Info:
'   Dragonfire is a 1982 game written by Bob Smith and published by Imagic for Atari 2600
'   Remake to MSX1 created by Amaury Carvalho (2023):
'     https://github.com/amaurycarvalho/msxbasic/Dragon%20Treasure
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom drgtsr.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/drgtsr.akm"                 ' 0 - game songs - exported from Arkos Tracker 3
FILE "music/drgtsr.akx"                 ' 1 - sound effects - exported from Arkos Tracker 3
FILE "img/SCR0.SC2"                     ' 2 - opening screen
FILE "img/SCR1.SC2"                     ' 3 - outdoors screen
FILE "img/SCR2.SC2"                     ' 4 - indoors screen
FILE "img/SCR3.SC2"                     ' 5 - inbetween castles screen
FILE "img/SPRITES1.SPR"                 ' 6 - sprites 1
FILE "img/SPRITES2.SPR"                 ' 7 - sprites 2

' GAME START
10 SCREEN 2, 2, 0
11 COLOR 15, 0, 0
12 CMD PLYLOAD 0, 1
13 SET TILE ON                          ' uses tiled mode instead of graphical mode

' OPENING SCREEN
20 SC% = 2 : MU% = 0 : SP% = 6
21 GOSUB 9000                           ' show screen
22 GOSUB 8000                           ' initialize fire effect
23 GOSUB 8020                           ' do fire effect
24 GOSUB 9100 : IF B% = 0 THEN 23       ' wait player press a key
25 GOSUB 9100 : IF B% <> 0 THEN 25
26 LV% = 0                              ' level number
27 E1% = 0                              ' special item 1 (sword) on/off

' OUTDOORS CASTLE SCREEN
30 SC% = 3 : MU% = 1 : SP% = 6 : JP% = 0
31 GOSUB 9000                           ' show screen
32 PX% = 210 : PY% = 110 : PS% = 0 : T1% = TIME
33 GOSUB 9200                           ' show player

' OUTDOORS CASTLE LOOP
40 GOSUB 9100               ' PLAYER INPUT
41 IF K% = 27 THEN 90
42 GOSUB 9300               ' DO PLAYER MOVES (OUTSIDE CASTLE)
43 GOSUB 9350               ' RIVER ANIMATION
44 GOSUB 9600               ' JUMPING LOGIC
45 IF PX% > 20 THEN 40

' INDOORS CASTLE SCREEN
50 SC% = 4 : MU% = 2 : SP% = 7
51 GOSUB 9000
52 PX% = 230 : PY% = 124 : PS% = 6 : T1% = TIME
53 GOSUB 9200               ' SHOW PLAYER
54 GOSUB 9500               ' SHOW ITEMS
55 I% = 1 : GOSUB 9120      ' WAIT I% SECONDS
56 SET TILE COLOR 143, 8    ' CLOSE DOOR
57 CMD PLYSOUND 1, 2        ' CLOSE DOOR SOUND

' INDOORS CASTLE LOOP
60 GOSUB 9100               ' PLAYER INPUT
61 IF K% = 27 THEN 90
62 GOSUB 9400               ' DO PLAYER MOVES (INSIDE CASTLE)
63 GOSUB 9440               ' CHECK INSIDE CASTLE COLLISIONS
64 IF E2% = 1 THEN IF PX% < 8 AND PY% < 24 THEN 70   ' EXIT DOOR LOGIC
65 GOTO 60

' PUT SPRITE 1, (100,146), 2, 20  ' DRAGON
' PUT SPRITE 2, (116,146), 2, 21
' PUT SPRITE 3, (116,130), 8, 31  ' FIREBALL UP

' INBETWEEN CASTLES SCREEN
70 LV% = LV% + 1
71 SC% = 5 : MU% = 1 : SP% = 6 : JP% = 0
72 GOSUB 9000
73 PX% = 210 : PY% = 110 : PS% = 0 : T1% = TIME
74 GOSUB 9200               ' SHOW PLAYER

' PUT SPRITE 1, (26, 110), 8, 18   ' FIREBALL RIGHT

' INBETWEEN CASTLES LOOP
80 GOSUB 9100               ' PLAYER INPUT
81 IF K% = 27 THEN 90
82 GOSUB 9300               ' DO PLAYER MOVES (OUTSIDE CASTLE)
83 IF PX% <= 20 THEN 50
84 GOSUB 9350               ' RIVER ANIMATION
85 GOSUB 9600               ' JUMPING LOGIC
86 GOTO 80

' RESTART GAME
90 GOTO 20

' FIRE EFFECT
8000 VR1% = HEAP() : VR2% = BASE(10) + 672      ' initialize: 21 lines * 32 chars = 672
8001 S% = VR1%
8010 FOR I% = 0 TO 95
8011   IF I% < 64 THEN C% = 231 : GOTO 8014
8012   GOSUB 9110
8013   C% = 232 + (R% MOD 8)
8014   POKE S%, C%
8015   S% = S% + 1
8016 NEXT
8017 RETURN

8020 S% = VR1%                                  ' propagate: 3 lines * 32 chars = 96
8030 FOR I% = 0 TO 63
8031   Y% = (I% MOD 32)
8032   IF Y% > 0 AND Y% < 31 THEN GOSUB 9110 : X% = 31 + (R% MOD 3) ELSE X% = 32
8033   C% = PEEK(S% + X%)
8034   IF C% <> 231 THEN C% = C% + 8
8040   POKE S%, C%
8041   S% = S% + 1
8042 NEXT
8050 FOR I% = 64 TO 95
8051   C% = PEEK(S%) + 1
8052   IF C% > 239 THEN GOSUB 9110 : C% = 232 + (R% MOD 8)
8053   POKE S%, C%
8054   S% = S% + 1
8055 NEXT
8060 CMD RAMTOVRAM VR1%, VR2%, 96
8061 RETURN

' LOAD SCREEN, MUSIC AND SPRITES
9000 SCREEN OFF                 ' disable screen
9001 SCREEN LOAD SC%            ' load screen resource SC
9002 SPRITE LOAD SP%            ' load sprite resource SP
9003 IF SC% <> 2 GOSUB 9050
9004 CMD PLYSONG MU%            ' set song MU
9005 CMD PLYPLAY                ' play song
9006 SCREEN ON                  ' enable screen
9007 RETURN

' SET LEVEL FOOTER
9050 SET FONT 2
9051 LOCATE 0, 22
9052 PRINT "LEVEL    LIFES   SHIELDS    GOLD";

' SHOW SCORE
9060 LOCATE 0, 23 : PRINT USING$("00", LV%);
9061 RETURN

' PLAYER INPUTS
9100 K% = INKEY
9101 J% = STICK(0) OR STICK(1)
9102 B% = STRIG(0) OR STRIG(1)

' RANDOM NUMBER
9110 R% = RND(1) * 1000
9111 RETURN

' WAIT I% SECONDS
9120 T1% = TIME : I% = I% * 60
9121 IF T1% > TIME THEN T1% = TIME
9122 DT% = TIME - T1%
9123 IF DT% < I% THEN 9121
9124 RETURN

' SHOW PLAYER
9200 PUT SPRITE 0, (PX%,PY%), 15, PS%
9201 RETURN

' DO PLAYER MOVES (OUTSIDE CASTLE)
9300 IF J% = 0 AND B% = 0 THEN RETURN
9301 IF T1% > TIME THEN T1% = TIME
9302 DT% = TIME - T1%
9303 IF DT% < 2 THEN RETURN
9304 T1% = TIME

9310 IF JP% = 0 THEN GOSUB 9330

9320 DX% = 0 : DY% = 0
9321 IF J% = 3 THEN DX% = 2 : IF JP% = 0 THEN IF PS% = 4 THEN PS% = 5 ELSE PS% = 4
9322 IF J% = 7 THEN DX% = -2 : IF JP% = 0 THEN IF PS% = 0 THEN PS% = 1 ELSE PS% = 0
9323 DX% = DX% + PX% : DY% = DY% + PY%
9324 IF DX% > 210 THEN RETURN
9325 PX% = DX% : PY% = DY%
9326 GOTO 9200

9330 IF J% <> 5 THEN 9340
9331 IF PS% < 4 THEN PS% = 2 ELSE PS% = 6    ' activate bowing
9332 GOTO 9200

9340 IF B% = 0 THEN RETURN
9341 JP% = 12                                ' activate jumping
9342 IF PS% < 4 THEN PS% = 3 ELSE PS% = 7
9343 RETURN

' RIVER ANIMATION
9350 IF T3% > TIME THEN T3% = TIME
9351 DT% = TIME - T3%
9352 IF DT% < 30 THEN RETURN
9353 T3% = TIME
9360 COPY (5, 19)-(14,19) TO TMP$
9361 COPY (17,19)-(26,19) TO (5,19)
9362 COPY TMP$ TO (17,19)
9363 SET TILE COLOR 171, STCO%
9364 IF STCO% = 15 THEN STCO% = 13 ELSE STCO% = 15
9365 SET TILE COLOR 176, STCO%
9366 RETURN

' DO PLAYER MOVES (INSIDE CASTLE)
9400 IF J% = 0 THEN RETURN
9401 IF T1% > TIME THEN T1% = TIME
9402 DT% = TIME - T1%
9403 IF DT% < 2 THEN RETURN
9404 T1% = TIME
9410 DX% = 0 : DY% = 0
9411 IF J% = 8 OR J% = 1 OR J% = 2 THEN DY% = -2
9412 IF J% >= 4 AND J% <= 6 THEN DY% = 2
9413 IF J% >= 2 AND J% <= 4 THEN DX% = 2 : IF PS% = 2 THEN PS% = 3 ELSE PS% = 2
9414 IF J% >= 6 AND J% <= 8 THEN DX% = -2 : IF PS% = 6 THEN PS% = 7 ELSE PS% = 6
9415 IF J% = 1 THEN IF PS% = 0 THEN PS% = 1 ELSE PS% = 0
9416 IF J% = 5 THEN IF PS% = 4 THEN PS% = 5 ELSE PS% = 4
9420 DX% = DX% + PX% : DY% = DY% + PY%
9421 IF DX% < 6 OR DX% > 230 OR DY% > 124 OR DY% < 8 THEN RETURN
9422 PX% = DX% : PY% = DY%
9423 GOTO 9200

' CHECK INSIDE CASTLE COLLISIONS
9440 IF T2% > TIME THEN T2% = TIME
9441 DT% = TIME - T2%
9442 IF DT% < 15 THEN RETURN
9443 T2% = TIME
9444 I% = COLLISION(0)
9445 IF I% < 0 THEN RETURN
9446 IF I% = 4 THEN 9530    ' GOT KEY
9447 RETURN

' SHOW ITEMS
9500 E2% = 0                ' SPECIAL ITEM 2 (KEY) ON/OFF
9501 IE% = LV% MOD 3 + 24   ' SPECIAL ITEM ON SCREEN (0=SHIELD, 1=HEART, 2=CROWN)
9502 IF IE% = 24 THEN IC% = 14 ELSE IF IE% = 25 THEN IC% = 8 ELSE IC% = 10
9503 IQ% = LV% * 3 + 1      ' ITEMS QUANTITY ON SCREEN (LEVEL 0 = NOTHING)
9504 IF IQ% > 9 THEN IQ% = 9
9510 FOR I% = 0 TO IQ%
9511   GOSUB 9110 : X% = R% MOD 180 + 20
9512   GOSUB 9110 : Y% = R% MOD 110 + 20
9513   GOSUB 9110 : C% = R% MOD 5 + 10
9514   GOSUB 9110 : S% = R% MOD 4 + 27
9515   IF I% = 0 THEN C% = 15  : S% = 23
9516   IF I% = 1 THEN C% = IC% : S% = IE%
9517   GOSUB 9590  ' SHOW ITEM
9520 NEXT
9521 RETURN

' KEY COLLISION AND OPEN EXIT DOOR
9530 E2% = 1
9531 SET TILE COLOR 142, 1
9532 CMD PLYSOUND 1, 2      ' OPEN DOOR SOUND
9533 I% = 0 : Y% = 184 : C% = 0
9534 GOTO 9590

9590 PUT SPRITE I%+4,(X%,Y%),C%,S%
9591 RETURN

' JUMPING LOGIC
9600 IF T4% > TIME THEN T4% = TIME
9601 DT% = TIME - T4%
9602 IF DT% < 3 THEN RETURN
9603 T4% = TIME

9610 IF JP% = 0 THEN RETURN
9611 JP% = JP% - 1
9612 IF JP% > 5 THEN PY% = PY% - 4 ELSE PY% = PY% + 4

9620 IF JP% > 0 THEN 9200
9621 IF PS% < 4 THEN PS% = 0 ELSE PS% = 4
9622 GOTO 9200             ' show player

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-dragon-treasure-music-extra-dragon-scream", "", "MSXBAS2ROM: Jogo: Dragon Treasure", "", "music\\extra\\dragon_scream.bas: 1 REM https://marmsx.msxall.com/projetos/dsg/index.php",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Dragon Treasure: music\extra\dragon_scream.bas

1 REM https://marmsx.msxall.com/projetos/dsg/index.php

5 SOUND 7,184
6 SOUND 8, 15
7 SOUND 9, 15
8 SOUND 10, 15
10 C% = 0
20 FOR I = 100 TO 800 STEP 100
30     F = 1789772 / (16 * I)
40     L% = F AND 255
50     H% = F MOD 255
60     SOUND C%*2, L%
70     SOUND C%*2+1, H%
80     T = TIME
90     IF (TIME - T) < 5 THEN 90
100    C% = C% + 1
110    IF C% > 2 THEN C% = 0
120 NEXT
121     T = TIME
122     IF (TIME - T) < 60 THEN 122
130 FOR I = 0 TO 5
140    SOUND I,0
150 NEXT

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-dragon-treasure-readme", "", "MSXBAS2ROM: Jogo: Dragon Treasure", "", "README.md: Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Dragon Treasure: README.md

DRAGON TREASURE
Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- Dragonfire) (a.k.a. Dragon Treasure) is a 1982 game written by Bob Smith and published by Imagic for Atari 2600;
- Remake to MSX1 created by Amaury Carvalho (2023).

SOURCE CODE

Compiling

       msxbas2rom drgtsr.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-flubber-flubber", "", "MSXBAS2ROM: Jogo: Flubber", "", "flubber.bas: ' FLUBBER IN THE UPSIDE DOWN WORLD (v1.2)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Flubber: flubber.bas

'-----------------------------------------------------------------
' FLUBBER IN THE UPSIDE DOWN WORLD (v1.2)
'-----------------------------------------------------------------
' Game Info:
'   Flubber is a side-scrolling bouncing ball game where the goal is to get through an obstacle course and
'   reach the finish flag in less than a minute.
'   Developed by Amaury Carvalho (2021-2023)
'   All songs are variations of Perez Prado's Mambos
'     https://github.com/amaurycarvalho/msxbasic/Flubber
'     https://www.msxdev.org/2023/06/07/msxdev23-08-flubber-in-the-upside-down-world/
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'   https://launchpad.net/nmsxtiles
'   https://github.com/pipagerardo/nMSXtiles
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom flubber.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/flubber.akm"                ' 0
FILE "music/effects.akx"                ' 1
FILE "img/splash.sc2"                   ' 2
FILE "img/playfield.sc2"                ' 3
FILE "img/finale.sc2"                   ' 4
FILE "img/flubber.spr"                  ' 5
FILE "levels/levels.txt"                ' 6

'==== GAME INITIALIZE
10 DIM PX%(1), PY%(1), PS%(1), PJ%(1), PM%(1), PC%(2)
11 DIM BX%(15), SBUF$(2), LBUF$(7)                        ' screen and level buffers
12 IF NTSC() THEN TM% = 60 : TH% = 30 : TS% = 6 ELSE TM% = 50 : TH% = 25 : TS% = 5
13 CBUF$ = "" : CBP% = VARPTR(CBUF$)                      ' cloud buffer and pointer
14 SBP% = VARPTR(SBUF$(0)) : LBP% = VARPTR(LBUF$(0))      ' screen and level buffer pointers
15 SVP% = BASE(10) + 32                                   ' screen vram pointer
16 HS# = 0                                                ' hi-score

20 SCREEN 2, 2, 0
22 SET TILE ON
23 CMD RESTORE 6
24 CMD PLYLOAD 0, 1

'==== GAME START
30 CMD DISSCR
31 COLOR 15, 0, 0
32 SCREEN LOAD 2
33 GOSUB 5200               ' set game font

40 GOSUB 2000               ' clear sprites
41 LOCATE 02,14 : PRINT "        I N   T H E";
42 LOCATE 02,16 : PRINT "     UPSIDE DOWN WORLD";
43 LOCATE 02,19 : PRINT "  BY AMAURY CARVALHO (2023)";
44 LOCATE 02,23 : PRINT "  SPACE OR TRIGGER TO START";
45 CMD PLYSONG 0            ' opening song
46 CMD PLYPLAY
47 CMD ENASCR

50 PC%(0) = 12 : PC%(1) = 0 : PC%(2) = 15 : W% = 0
51 GOSUB 3400               ' splash animation
52 GOSUB 1000 : IF B% = 0 THEN 51
53 GOSUB 2200               ' wait player to release space or button
54 CMD PLYMUTE

60 PP# = 0 : PN% = 0        ' player points
61 PL% = 3                  ' player lives            <=========== game parameter
62 LV% = 1                  ' level number            <=========== game parameter
63 MT% = 0                  ' mute flag (0=sound, 1=mute)
64 SG% = 1                  ' playing song
65 CF% = 0                  ' cloud flag

'==== LEVEL START
70 PX%(0) = 20 : PY%(0) = 71 : PS%(0) = 0 : PJ%(0) = 0
71 PX%(1) = PX%(0) : PY%(1) = PY%(0) + 32 : PS%(1) = 12 : PJ%(1) = 0
72 PM%(0) = 2 : PM%(1) = PM%(0)
73 PT% = TIME : PMT% = PT%
74 PI% = 2 : PMI% = PI%
75 T0% = TIME : T1% = TIME : T2% = TIME : T3% = TIME
76 LT% = 60                 ' level time              <=========== game parameter
77 LSS% = 0                 ' level scroll step
78 LST% = 0                 ' level status (0=playing, 1=finish flag, 2=knife)

80 GOSUB 4000               ' show playfield

90 GOSUB 2100               ' show sprites
91 GOSUB 2320               ' play song

'==== MAIN LOOP
100 GOSUB 1000       ' PLAYER INPUT
101 IF K% = 27 OR C% <> 0 THEN 3000   ' PAUSE
102 GOSUB 3100       ' MOVE PLAYER
103 GOSUB 3200       ' GRAVITY AND JUMPING
104 GOSUB 3300       ' MOVE CLOUDS
105 GOSUB 3700       ' TIME UPDATE

110 IF LT% = 0 OR PL% = 0 THEN 3800          ' TIME IS UP OR LIFE'S END
111 ON LST% GOTO 3900, 3830                  ' finish flag was reached or a knife was hitted
112 IF K% = 77 OR K% = 109 GOSUB 2300        ' M = MUTE
'113 IF K% = 9 THEN 3900                      ' TAB = NEXT LEVEL

120 GOTO 100

'==== PLAYER INPUT
1000 A% = STICK(0) OR STICK(1)
1001 B% = STRIG(0) OR STRIG(1)
1002 C% = STRIG(3)
1003 K% = INKEY

'==== RANDOM NUMBER
1100 R% = RND(1) * 100
1101 RETURN

'==== CLEAR SPRITES
2000 FOR I% = 0 TO 15
2001   PUT SPRITE I%,,0
2002 NEXT
2003 RETURN

'==== SHOW SPRITES
2100 PUT SPRITE 0,(PX%(0), PY%(0)), 1, PS%(0)
2101 PUT SPRITE 1,(PX%(0), PY%(0)), 2, PS%(0)+1
2102 PUT SPRITE 2,(PX%(1), PY%(1)), 1, PS%(1)
2103 PUT SPRITE 3,(PX%(1), PY%(1)), 12, PS%(1)+1
2104 RETURN

'==== WAIT WHILE A SPACE/TRIGGER
2200 GOSUB 1000
2201 IF B% <> 0 OR C% <> 0 OR K% <> 0 THEN 2200
2202 RETURN

'==== MUTE
2300 IF MT% = 1 THEN 2310
2301   MT% = 1
2302   SG% = 4       ' mute song
2303   GOTO 2320

2310 MT% = 0
2311 SG% = 1         ' gameplay song

2320 CMD PLYSONG SG%
2321 CMD PLYPLAY
2322 RETURN

'==== WAIT n CYCLES (n = NC%)
2400 T2% = TIME
2410 IF TIME < T2% THEN T2% = TIME
2411 DI% = TIME - T2%
2412 IF DI% < NC% THEN 2410
2413 RETURN

'==== PAUSE
3000 GOSUB 2200                                                   ' WAIT WHILE A SPACE/TRIGGER
3001 CMD PLYSONG 4 : CMD PLYPLAY : CMD PLYSOUND 4                 ' PAUSE SOUND
3002 LOCATE 10,23 : PRINT "PAUSE";

3010 GOSUB 1000
3011 IF K% = 27 THEN 30                                           ' RESTART GAME
3012 IF B% = 0 AND C% = 0 THEN 3010
3013 CMD PLYSONG SG% : CMD PLYPLAY
3014 LOCATE 10,23 : PRINT "     ";
3015 GOTO 100                                                     ' MAIN LOOP

'==== MOVE PLAYER
3100 IF TIME < PT% THEN PT% = TIME
3101 DI% = TIME - PT%
3102 IF DI% < PI% THEN RETURN
3103 PT% = TIME
3104 IF A% = 0 AND B% = 0 THEN RETURN
3105 GOSUB 3110                                     ' check arrows
3106 GOTO 3150                                      ' check button

' check arrows
3110 SS% = 0
3111 IF A% >= 2 AND A% <= 4 THEN 3130
3112 IF A% >= 6 AND A% <= 8 THEN 3120
3113 RETURN

3120 X% = PX%(0) - 4
3121 Y% = PY%(1) - 7                                ' check obstacle at left of flubber down
3122 GOSUB 5100 : IF O% = 0 THEN RETURN
3123 Y% = PY%(0) + 1                                ' check obstacle at left of flubber up
3124 GOSUB 5100 : IF O% = 0 THEN RETURN
3125 SS% = 4
3126 GOSUB 3600                                     ' do playfield level left scrolling
3127 GOTO 3140

3130 X% = PX%(0) + 4
3131 Y% = PY%(1) - 7                                ' check obstacle at right of flubber down
3132 GOSUB 5110 : IF O% = 0 THEN RETURN
3133 Y% = PY%(0) + 1                                ' check obstacle at right of flubber up
3134 GOSUB 5110 : IF O% = 0 THEN RETURN
3135 SS% = 2
3136 GOSUB 3500                                     ' do playfield level right scrolling

3140 PX%(0) = X% : PX%(1) = X%
3141 IF PJ%(0) > 0 OR PS%(0) >= 6 THEN PS%(0) = SS% + 6 ELSE PS%(0) = SS%
3142 IF PJ%(1) > 0 OR PS%(1) >= 18 THEN PS%(1) = SS% + 18 ELSE PS%(1) = SS% + 12
3143 GOTO 2100                                      ' show sprites

' check button
3150 IF PJ%(0) > 0 THEN RETURN
3151 IF PS%(0) >= 6 THEN RETURN
3152 IF B% <> 0 THEN PM%(0) = 4 : JS% = 4 : GOTO 3160
3153 IF A% = 8 OR A% = 1 OR A% = 2 THEN PM%(0) = 2 : JS% = 1 : GOTO 3160
3154 RETURN

3160 PJ%(0) = 12                                    ' activate jump
3161 IF A% = 1 THEN PS%(0) = 6 ELSE PS%(0) = PS%(0) + 6
3162 CMD PLYSOUND JS%, 2
3163 GOTO 2100                                      ' show sprites

'==== GRAVITY AND JUMPING
3200 IF TIME < PMT% THEN PMT% = TIME
3201 DI% = TIME - PMT%
3202 IF DI% < PMI% THEN RETURN
3203 PMT% = TIME
3204 IF PJ%(0) = 0 THEN GOSUB 3210 ELSE GOSUB 3230  ' choose between gravity or jumping for flubber up
3205 IF PJ%(1) = 0 THEN GOTO 3250 ELSE GOTO 3270    ' choose between gravity or jumping for flubber down

' gravity for flubber up
3210 X% = PX%(0) : Y% = PY%(0) + PM%(0)
3212 GOSUB 5130                                     ' check obstacle down
3213 IF O% = 1 THEN 3240

3220 IF PS%(0) < 6 THEN RETURN
3221 PS%(0) = PS%(0) - 6                            ' end of fall
3222 CMD PLYSOUND 1, 2
3223 IF PJ%(1) > 0 THEN 2100
3224 PM%(1) = PM%(0)                                ' activate flubber down jump
3225 PJ%(1) = 12
3226 PS%(1) = PS%(0) + 18
3227 GOTO 2100                                      ' show sprites

' jumping for flubber up
3230 PJ%(0) = PJ%(0) - 1
3231 X% = PX%(0) : Y% = PY%(0) - PM%(0)
3232 GOSUB 5120                                     ' check obstacle up
3233 IF O% = 1 THEN 3240
3234 PJ%(0) = 0
3235 RETURN

3240 PY%(0) = Y%
3241 GOTO 2100                                      ' show sprites

' gravity for flubber down
3250 X% = PX%(1) : Y% = PY%(1) - PM%(1) - 6
3251 GOSUB 5120                                     ' check obstacle up
3252 IF O% = 1 THEN 3280

3260 IF PS%(1) < 18 THEN RETURN
3261 PS%(1) = PS%(1) - 6                            ' end of fall
3262 CMD PLYSOUND 1, 2
3263 GOTO 2100                                      ' show sprites

' jumping for flubber down
3270 PJ%(1) = PJ%(1) - 1
3271 X% = PX%(1) : Y% = PY%(1) + PM%(1) - 6
3272 GOSUB 5130                                     ' check obstacle down
3273 IF O% = 1 THEN 3280
3274 PJ%(1) = 0
3275 RETURN

3280 PY%(1) = Y% + 6
3281 GOTO 2100                                      ' show sprites

'==== MOVE CLOUDS
3300 IF TIME < T1% THEN T1% = TIME
3301 DI% = TIME - T1%
3302 IF DI% < TS% THEN RETURN           ' 10x/s
3303 T1% = TIME

3310 CF% = (CF% + 1) MOD 3
3311 I% = CBP%                          ' move cloud 1
3312 IF CF% = 0 THEN I% = I% + 64       ' else, move cloud 2
3313 X% = PEEK(I%) : Y% = PEEK(I%+32)   ' cloud scrolling
3314 CMD RAMTORAM I%+1, I%, 64
3315 POKE I%+31, X% : POKE I%+63, Y%
3320 GOTO 4250                          ' copy buffer to clouds vram

'==== SPLASH ANIMATION
3400 IF TIME < PT% THEN PT% = TIME
3401 DI% = TIME - PT%
3402 IF DI% < TS% THEN RETURN
3403 PT% = TIME

3410 IF PC%(0) = 12 THEN GOSUB 3421 : GOSUB 3420 ELSE GOSUB 3420 : GOSUB 3421
3411 PC%(0) = PC%(0) XOR 12 : PC%(1) = PC%(1) XOR 12
3412 W% = (W% + 1) MOD 5
3413 IF W% > 0 THEN RETURN
3414 PC%(2) = PC%(2) XOR 10
3415 FOR I% = 130 TO 138 : SET TILE COLOR I%, PC%(2), 4 : NEXT
3416 RETURN

3420 SET TILE COLOR 139, PC%(0), 1 : SET TILE COLOR 144, PC%(0), 1 : SET TILE COLOR 145, PC%(0), 1 : SET TILE COLOR 146, PC%(0), 1 : RETURN
3421 SET TILE COLOR 192, PC%(1), 1 : SET TILE COLOR 193, PC%(1), 1 : SET TILE COLOR 194, PC%(1), 1 : SET TILE COLOR 195, PC%(1), 1 : RETURN

'==== SCROLL PLAYFIELD LEVEL TO RIGHT
3500 IF PX%(0) < 168 THEN RETURN
3501 IF LSS% >= 96 THEN RETURN
3502 LSS% = LSS% + 1 : X% = X% - 4
3503 GOTO 4800                          ' copy playfield level to screen

'==== SCROLL PLAYFIELD LEVEL TO LEFT
3600 IF PX%(0) > 80 THEN RETURN
3601 IF LSS% = 0 THEN RETURN
3602 LSS% = LSS% - 1 : X% = X% + 4
3603 GOTO 4800                          ' copy playfield level to screen

'==== TIME UPDATE
3700 IF TIME < T0% THEN T0% = TIME
3701 DI% = TIME - T0%
3702 IF DI% < TM% THEN RETURN           ' 1x/min
3703 T0% = TIME
3710 IF LT% > 0 THEN LT% = LT% - 1
3711 IF MT% = 0 THEN IF LT% = 12 THEN CMD PLYSONG 5 : CMD PLYPLAY
3712 GOTO 4100                          ' score update

'==== TIME'S UP OR LIFE'S END
3800 IF PL% > 0 THEN PL% = PL% - 1 : GOSUB 4100
3801 IF PL% = 0 THEN 3850
3810 LOCATE 4, 10 : PRINT "+-----------------------+";
3811 LOCATE 4, 11 : PRINT "+  T I M E  I S  U P !  +";
3812 LOCATE 4, 12 : PRINT "+-----------------------+";
3813 CMD PLYSONG 3 : CMD PLYPLAY
3814 GOSUB 5300                         ' player is dead
3815 NC% = TM% : GOSUB 2400             ' wait NC% cycles
3820 GOSUB 1000 : IF B% = 0 THEN 3820
3821 GOSUB 2200                         ' wait player to release space or button
3822 CMD PLYMUTE
3823 GOTO 70

'==== A KNIFE WAS HITTED
3830 IF PL% > 0 THEN PL% = PL% - 1 : GOSUB 4100
3831 IF PL% = 0 THEN 3850
3840 LOCATE 4, 10 : PRINT "+-----------------------+";
3841 LOCATE 4, 11 : PRINT "+ OOPS! TRY IT AGAIN... +";
3842 LOCATE 4, 12 : PRINT "+-----------------------+";
3843 CMD PLYSONG 3 : CMD PLYPLAY
3844 GOSUB 5300                         ' player is dead
3845 NC% = TM% : GOSUB 2400             ' wait NC% cycles
3846 GOSUB 1000 : IF B% = 0 THEN 3846
3847 GOSUB 2200                         ' wait player to release space or button
3848 CMD PLYMUTE
3849 GOTO 70

'==== GAME OVER
3850 LOCATE 4, 10 : PRINT "+-----------------------+";
3851 LOCATE 4, 11 : PRINT "+   G A M E   O V E R   +";
3852 LOCATE 4, 12 : PRINT "+-----------------------+";
3853 CMD PLYSONG 3 : CMD PLYPLAY
3854 GOSUB 5300                         ' player is dead
3855 NC% = TM% : GOSUB 2400             ' wait NC% cycles
3860 GOSUB 1000 : IF B% = 0 THEN 3860
3861 GOSUB 2200                         ' wait player to release space or button
3862 CMD PLYMUTE
3863 GOTO 30

'==== LEVEL VICTORY
3900 LOCATE 4, 10 : PRINT "+-----------------------+";
3901 LOCATE 4, 11 : PRINT "+ Y O U  M A D E  I T ! +";
3902 LOCATE 4, 12 : PRINT "+-----------------------+";
3903 CMD PLYSONG 2 : CMD PLYPLAY
3904 GOSUB 2000                         ' clear sprites
3905 GOSUB 3930                         ' add points
3906 NC% = TM% : GOSUB 2400             ' wait NC% cycles
3910 GOSUB 1000 : IF B% = 0 THEN 3910
3911 GOSUB 2200                         ' wait player to release space or button
3912 LV% = LV% + 1
3920 CMD PLYMUTE
3921 IF LV% > 15 THEN 3950
3922 GOTO 70

' add points
3930 IF LT% = 0 THEN RETURN
3931 PP# = PP# + 10 : PN% = PN% + 1 : LT% = LT% - 1                     ' add 10 points for each second left
3932 IF PN% >= 100 AND PL% < 99 GOSUB 3940                              ' add 1 life for each 1000 points
3933 IF HS# < PP# THEN HS# = PP#                                        ' hi-score control
3934 GOSUB 4100                                                         ' update score
3935 CMD PLYSOUND 6, 2                                                  ' countdown sound
3936 NC% = 1 : GOSUB 2400                                               ' wait NC% cycles
3937 GOTO 3930

3940 CMD PLYSOUND 3, 0
3941 PN% = PN% - 100
3942 PL% = PL% + 1
3943 RETURN

'==== CONGRATULATIONS
3950 CMD DISSCR
3951 GOSUB 2000                         ' clear sprites
3952 SCREEN LOAD 4
3953 GOSUB 5200                         ' set game font
3954 CMD ENASCR
3960 LOCATE 2, 11 : PRINT "    CONGRATULATIONS!!!!";
3961 LOCATE 2, 12 : PRINT "    -------------------";
3962 LOCATE 2, 14 : PRINT "    YOU BRAVELY REACHED";
3963 LOCATE 2, 16 : PRINT "      THE LAST LEVEL!";
3964 LOCATE 2, 18 : PRINT "  YOUR FINAL SCORE: "; USING$("00000",PP#);
3965 LOCATE 1, 22 : PRINT "SPACE OR TRIGGER TO PLAY AGAIN";
3970 CMD PLYSONG 6 : CMD PLYPLAY
3971 NC% = TM% * 2 : GOSUB 2400         ' wait NC% cycles
3972 GOSUB 1000 : IF B% = 0 THEN 3972
3973 GOSUB 2200                         ' wait player to release space or button
3974 CMD PLYMUTE
3975 GOTO 30

'==== SHOW PLAYFIELD
4000 CMD DISSCR
4001 COLOR 15, 0, 0
4002 GOSUB 2000                         ' clear sprites
4003 SCREEN LOAD 3
4004 SPRITE LOAD 5
4005 GOSUB 5200                         ' set game font
4006 LOCATE 0,  0 : PRINT "LIVES: 00 TIME: 000 SCORE: 00000";
4007 LOCATE 0, 23 : PRINT "LEVEL: 00        HI-SCORE: 00000";
4008 GOSUB 4100                         ' show score panel

4010 GOSUB 4200                         ' copy vram clouds to buffer
4011 GOSUB 4300                         ' load playfield level data
4012 GOSUB 4800                         ' copy playfield level to screen

4020 CMD ENASCR
4021 RETURN

'==== SHOW SCORE PANEL
4100 LOCATE  7,  0 : PRINT USING$("##", PL%);
4101 LOCATE 16,  0 : PRINT USING$("###", LT%);
4102 LOCATE 27,  0 : PRINT USING$("00000", PP#);
4103 LOCATE  7, 23 : PRINT USING$("##", LV%);
4104 LOCATE 27, 23 : PRINT USING$("00000", HS#);
4105 RETURN

'==== COPY VRAM CLOUDS TO BUFFER
4200 CMD VRAMTORAM SVP%, CBP%, 64
4201 CMD VRAMTORAM SVP%+640, CBP%+64, 64
4202 RETURN

'==== COPY BUFFER TO VRAM CLOUDS
4250 CMD RAMTOVRAM CBP%, SVP%, 64
4251 CMD RAMTOVRAM CBP%+64, SVP%+640, 64
4252 RETURN

'==== LOAD PLAYFIELD LEVEL DATA
4300 RESTORE LV% - 1
4301 GOSUB 4500                         ' up part
4302 GOSUB 4600                         ' down part
4303 GOTO 4700                          ' initialize screen buffer

4410 POKE Y%, C% : POKE Y%+1, C% : POKE Y%+128, C% : POKE Y%+129, C% : RETURN
4420 POKE Y%, C% : POKE Y%+1, C%+1 : POKE Y%+128, C%+2 : POKE Y%+129, C%+3 : RETURN

' Level data scheme:
'         X   X   X   W   X           = 0
'         X   X           W           = 1
'     X   X                       M   = 2
' X   X                       M   X   = 3
' --- --- --- --- --- --- --- --- ---
' 1   2   3   4   5   w   W   m   M
' 049 050 051 052 053 119 087 109 077

'====  LOAD PLAYFIELD LEVEL DATA - UP PART
4500 READ LS$                           ' up level
4510 Y% = LBP%                          ' level buffer pointer
4511 FOR K% = 0 TO 3
4512   X% = VARPTR(LS$) + 1             ' level data pointer
4513   FOR I% = 0 TO 63
4514     W% = PEEK(X%)
4515     C% = 128
4516     ON K% GOTO 4530, 4540, 4550
'        row 0
4520        IF W% >= 51 AND W% <= 53 OR W% = 87 THEN C% = 138 : GOTO 4560  ' 3, 4, 5, W
4521        IF W% = 119 THEN C% = 144                                      ' w
4522        GOTO 4560
'        row 1
4530        IF W% = 51 OR W% = 52 THEN C% = 138 : GOTO 4560                ' 3, 4
4531        IF W% = 87 THEN C% = 144                                       ' W
4532        GOTO 4560
'        row 2
4540        IF W% = 50 OR W% = 51 THEN C% = 138 : GOTO 4560                ' 2, 3
4541        IF W% = 77 THEN C% = 142                                       ' M
4542        GOTO 4560
'        row 3
4550        IF W% = 49 OR W% = 50 OR W% = 77 THEN C% = 138 : GOTO 4560     ' 1, 2, M
4551        IF W% = 109 THEN C% = 142 : GOTO 4560                          ' m
4552        IF W% = 60 THEN C% = 130 : GOTO 4560                           ' <
4553        IF W% = 62 THEN C% = 134                                       ' >
'        continue
4560     IF C% = 128 THEN GOSUB 4410 ELSE GOSUB 4420
4561     X% = X% + 1
4562     Y% = Y% + 2
4570   NEXT
4571   Y% = Y% + 128
4580 NEXT
4581 RETURN

'====  LOAD PLAYFIELD LEVEL DATA - DOWN PART
4600 READ LS$                           ' down level
4610 FOR K% = 0 TO 3
4611   X% = VARPTR(LS$) + 1             ' level data pointer
4613   FOR I% = 0 TO 63
4614     W% = PEEK(X%)
4615     C% = 160
4616     ON K% GOTO 4630, 4640, 4650
'        row 0
4620        IF W% = 49 OR W% = 50 OR W% = 77 THEN C% = 170 : GOTO 4660     ' 1, 2, M
4621        IF W% = 109 THEN C% = 172 : GOTO 4660                          ' m
4622        IF W% = 60 THEN C% = 162 : GOTO 4660                           ' <
4623        IF W% = 62 THEN C% = 166                                       ' >
4624        GOTO 4660
'        row 1
4630        IF W% = 50 OR W% = 51 THEN C% = 170 : GOTO 4660                ' 2, 3
4631        IF W% = 77 THEN C% = 172                                       ' M
4632        GOTO 4660
'        row 2
4640        IF W% = 51 OR W% = 52 THEN C% = 170 : GOTO 4660                ' 3, 4
4641        IF W% = 87 THEN C% = 176                                       ' W
4642        GOTO 4660
'        row 3
4650        IF W% >= 51 AND W% <= 53 OR W% = 87 THEN C% = 170 : GOTO 4660  ' 3, 4, 5, W
4651        IF W% = 119 THEN C% = 176                                      ' w
'        continue
4660     IF C% = 160 THEN GOSUB 4410 ELSE GOSUB 4420
4661     X% = X% + 1
4662     Y% = Y% + 2
4670   NEXT
4671   Y% = Y% + 128
4680 NEXT
4681 RETURN

'==== INITIALIZE SCREEN BUFFER
4700 DST% = SBP%
4701 FOR I% = 0 TO 767
4702    POKE DST%, 138
4703    DST% = DST% + 1
4704 NEXT
4705 RETURN

'==== COPY PLAYFIELD LEVEL DATA TO SCREEN VRAM
4800 SRC% = LBP% + LSS%                   ' level buffer + level scroll skip
4801 DST% = SBP% + 32                     ' screen buffer
4802 FOR I% = 1 TO 16
4803   CMD RAMTORAM SRC%, DST%, 32
4804   SRC% = SRC% + 128
4805   DST% = DST% + 32
4806   IF I% = 8 THEN DST% = DST% + 32
4807 NEXT
4810 CMD RAMTOVRAM SBP%+32, SVP%+64, 256
4811 CMD RAMTOVRAM SBP%+320, SVP%+384, 256
4812 RETURN

'==== CHECK OBSTACLE CALCULATIONS
5000 IK% = ((Y%-24) / 8) * 32 + (X% / 8)
5001 W% = PEEK(SBP%+IK%+32)
5002 RETURN

5010 IF W% = 128 OR W% = 160 THEN O% = 1 : RETURN                                               ' the way is clear to go
5011 IF W% >= 134 AND W% <= 137 OR W% >= 166 AND W% <= 169 THEN LST% = 1                        ' finish flag reached
5012 RETURN

5020 IF W% = 142 OR W% = 143 OR W% = 146 OR W% = 147 OR W% >= 174 AND W% <= 177 THEN LST% = 2   ' knife hitted
5021 RETURN

'==== CHECK OBSTACLE ON LEFT
5100 O% = 0
5101 IF X% < 5 THEN RETURN
5102 GOSUB 5000                     ' get tile (left up)
5103 GOSUB 5010                     ' is it a free position?
5104 IF O% = 0 THEN RETURN
5105 O% = 0
5106 Y% = Y% + 15 : GOSUB 5000      ' get tile (left down)
5107 Y% = Y% - 15 : GOTO 5010       ' is it a free position?

'==== CHECK OBSTACLE ON RIGHT
5110 O% = 0
5111 IF X% >= 240 THEN RETURN
5112 X% = X% + 15 : GOSUB 5000      ' get tile (right up)
5113 GOSUB 5010                     ' is it a free position?
5114 IF O% = 0 THEN X% = X% - 15 : RETURN
5115 O% = 0
5116 Y% = Y% + 15 : GOSUB 5000      ' get tile (right down)
5117 Y% = Y% - 15 : X% = X% - 15
5118 GOTO 5010                      ' is it a free position?

'==== CHECK OBSTACLE ON UP
5120 O% = 0
5121 GOSUB 5000                     ' get tile (left up)
5122 GOSUB 5020                     ' test if knife was hitted
5123 GOSUB 5010                     ' is it a free position?
5124 IF O% = 0 THEN RETURN
5125 O% = 0
5126 X% = X% + 15 : GOSUB 5000      ' get tile (right up)
5127 GOSUB 5020                     ' test if knife was hitted
5128 X% = X% - 15 : GOTO 5010       ' is it a free position?

'==== CHECK OBSTACLE ON DOWN
5130 O% = 0
5131 Y% = Y% + 15 : GOSUB 5000      ' get tile (left down)
5132 GOSUB 5020                     ' test if knife was hitted
5133 GOSUB 5010                     ' is it a free position?
5134 IF O% = 0 THEN Y% = Y% - 15 : RETURN
5135 O% = 0
5136 X% = X% + 15 : GOSUB 5000      ' get tile (right down)
5137 GOSUB 5020                     ' test if knife was hitted
5138 X% = X% - 15 : Y% = Y% - 15
5139 GOTO 5010                      ' is it a free position?

'==== SET GAME FONT
5200 SET FONT 2
5201 FOR I% = 33 TO 122
5202    SET TILE COLOR I%, (12,12,3,3,3,12,12,12), (0,0,0,0,0,0,0,0)
5203 NEXT
5204 RETURN

'==== PLAYER IS DEAD
5300 PS%(0) = 6 : PS%(1) = 18
5310 O% = 0 : GOSUB 2100            ' show sprites
5311 IF PY%(0) > 10 THEN PY%(0) = PY%(0) - 4 : O% = 1
5312 IF PY%(1) < 166 THEN PY%(1) = PY%(1) + 4 : O% = 1
5313 NC% = TS% : GOSUB 2400         ' wait NC% cycles
5314 IF O% = 1 THEN 5310
5320 GOTO 2000                      ' clear sprites

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-flubber-readme", "", "MSXBAS2ROM: Jogo: Flubber", "", "README.md: Flubber is a side-scrolling bouncing ball game where the goal is to get...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Flubber: README.md

FLUBBER IN THE UPSIDE DOWN WORLD (V1.2)
Flubber is a side-scrolling bouncing ball game where the goal is to get through an obstacle course and reach the finish flag in less than a minute.

You can jump over obstacles by pressing up (small jump) or space (big jump). There is an inverted version of flubber that will mirror all movements of the original with a slight delay. Furthermore, the lower world will have a different path than the upper one.

You will earn 10 points for every second remaining at the end of each successfully completed level. For every 1000 points earned, you will gain 1 new life. Also, when time runs out or you touch a blade, you will lose 1 life.

While playing the game, you can also press ESC to PAUSE/EXIT the game or press "M" to turn the music on/off.

There are a total of 15 fixed levels in the game, each with increasing difficulty from the previous one.

Written in MSX Basic from scratch (~587 lines of code) and compiled using MSXBAS2ROM compiler, this is a MSX1 compatible single player game (standard ROM BASIC required). Joysticks are recommended, but not required.

Good Luck!

GAME INFO

- Created by Amaury Carvalho (2021-2023);
- All songs are variations of Perez Prado's Mambos;
- Published on MSXDEV23;
- ExecROM is not compatible with this 48kb ROM type, so use SofaRUN or ODO to load this game from MSX-DOS.

HISTORY

- 1.2 Speed fix on 50hz machines;
- 1.1 Graphics and game over mechanics improvements;
- 1.0 Initial release.

SOURCE CODE

Compiling

       msxbas2rom flubber.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-fortknox-disk-autoexec", "", "MSXBAS2ROM: Jogo: Fortknox", "", "disk\\AUTOEXEC.BAS: 30 REM (C)2025 GameCast",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Fortknox: disk\AUTOEXEC.BAS

10 REM AUTOEXEC.BAS
20 REM FORT KNOX
30 REM (C)2025 GameCast
40 REM
50 RUN"FORTKNOX.LDR"


FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-fortknox-disk-fortknox", "", "MSXBAS2ROM: Jogo: Fortknox", "", "disk\\FORTKNOX.BAS: 30 REM (C)2025 GameCast",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Fortknox: disk\FORTKNOX.BAS

5 REM SAVE "FORTKNOX.BAS
10 REM FORT KNOX
20 REM
30 REM (C)2025 GameCast
40 REM
50 REM VERSION 2.0
60 REM
100 REM set list defusr
110 READ N,X$,T$:IF N=255 THEN GOTO130
111 W=&HF39A+N*2:POKE W,VAL("&h"+RIGHT$(X$,2)):POKEW+1,VAL("&h"+LEFT$(X$,2)):GOTO 110
112 DATA 1,"DA00",SFX
113 DATA 2,"0090",RESET PSG
114 DATA 3,"D000",MAIN LOOP
115 DATA 4,"D003",RESET ALL
129 DATA 255,"",""
130 DEFUSR5=&HDE00:REM CTRLSPR
140 CLEAR500,&HBFFF
150 DEFINTA-Y:DEFSTRZ:DIMZ(28)
160 HI=512:VDP(1)=98
170 REM INIT INTRO MUSIC!
180 U$="T150"
190 V1$="M5000S1V12R1R8R8R4R4L8O5BAL4GL8GAL4GO4L8BO5CDEDO4BO5L4DL8GAL4BBL8BAGAL4BAAL8BAL4GL8GAL4GO4L8BO5CDEDO4BO5L4DL8GAL4BBBL8GAL4BAL2G"
200 V2$="V10O4R4.L8DD-DD-DR4.DD-DD-DR4.DD-DD-DR4.DD-DD-DR4.DD-DD-DR4.R8R8R8L8D-DR4.DD-DD-DR4.DD-DD-DR4.DD-DD-DR1"
210 V3$="V10L2O2GO3DO2GO3DO2GO3DO2GO3DO2GO3DO2DAGO3DO2GO3DO2GO3DL4O2DF#GR4"
225 REM INTRO
230 CLS
240 FOR A=0 TO31:PUT SPRITE A,,,0:NEXTA
250 PRINT" SCORE        HI SCORE"
260 PRINTSC*100;SPC(10);HI*100
270 LOCATE0,8
280 LOCATE11,5:PRINTCHR$(200)+CHR$(202):LOCATE11,6:PRINTCHR$(201)+CHR$(203)
290 LOCATE13,5:PRINTCHR$(204)+CHR$(206):LOCATE13,6:PRINTCHR$(205)+CHR$(207)
300 LOCATE16,5:PRINTCHR$(208)+CHR$(210):LOCATE16,6:PRINTCHR$(209)+CHR$(211)
310 LOCATE18,5:PRINTCHR$(212)+CHR$(214):LOCATE18,6:PRINTCHR$(213)+CHR$(215)
320 LOCATE6,8:PRINT"the bagman return"
330 LOCATE 1,12:PRINT"1UP BONUS EVERY FOR 30000 PTS"
340 LOCATE 10,14:PRINT"INTRO MUSIC BY"
350 LOCATE 2,16:PRINT"CLAUDIO LUPO & DANIEL TKACH"
360 LOCATE 0,18:PRINT"   @ 1982 VALADON AUTOMATION"
370 LOCATE 2,20:PRINT"PUSH SPACE OR FIRE TO START"
380 LOCATE 0,22:PRINT" @ 2025 GAMECAST & danysoft"
410 IF STRIG(0)=-1 THENPOKE&HDA03,5:A=USR1(0):GOTO660
420 IF STRIG(1)=-1 THENPOKE&HDA03,5:A=USR1(0):GOTO660
430 TI=TI+1:IFTI>1500THENTI=0:GOTO450
440 GOTO 410
450 CLS:SP=1:X=16:Y=32:XC=16:YC=64:CN=31
460 PUTSPRITE6,(X,Y),6,SP+K:PUTSPRITE7,(X,Y),10,SP+1+K:PUTSPRITE8,(X,Y),15,SP+2+K
470 PUTSPRITE2,(XC,YC),15,CN+M:PUTSPRITE3,(XC,YC),6,CN+1+M:PUTSPRITE4,(XC+16,YC),15,CN+2+M:PUTSPRITE5,(XC+16,YC),6,CN+3+M
480 LOCATE2,12:PRINTCHR$(144)+CHR$(145)
490 LOCATE2,13:PRINTCHR$(146)+CHR$(147)
500 LOCATE2,16:PRINTCHR$(162)+CHR$(163)
510 LOCATE6,4:PRINT"player1"
520 LOCATE6,8:PRINT"bird condor"
530 LOCATE6,13:PRINT"bag"
540 LOCATE6,16:PRINT"whellbarrow"
550 LOCATE0,0:PRINT" SCORE        HI SCORE"
560 PRINTSC*100;SPC(10);HI*100
570 LOCATE2,20:PRINT"TAKE ALL THE BAGS AND CARRY"
580 LOCATE4,21:PRINT"THEM ON THE WHEELBARROW"
590 PLAY U$,U$,U$
600 PLAY V1$,V2$,V3$
610 IF STRIG(0)=-1 THEN230
620 IF STRIG(1)=-1 THEN230
630 TI=TI+1:IFTI>1500THENTI=0:GOTO230
640 GOTO610
650 REM SCORE,LIVES
660 CLS
670 LI=2:SC=0:BO=300:LV=1:ST=1:SS=0:PG=0
680 REM LOADER LEVELS
690 IF LV=1 THEN BLOAD"sfbank00.vrm",S
700 IF LV=2 THEN BLOAD"sfbank01.vrm",S
710 IF LV>2THENLV=1:GOTO690
720 CLS:FORI=0TO31:PUTSPRITEI,,,0:NEXT:LOCATE 11,10:PRINT"STAGE";ST
730 LOCATE 12,12:PRINT"READY"
732 A=USR2(0)
735 PLAY"V14T180O5L8DC#DEDC#O4BA#BO5CO4BA","V14T180O2F#V13O3F#O4CV15O2GV13O3GO4D#","T180R4O4CDR4O3BA"
737 PLAY"T180S0M9600GBAGF#AEF#GEC#O3AV15O4DL8V14O3D","T180V14O2EV13O3GF#V14GV13O3GV14O2AD","T180R4BAR4BAF#"
740 FORA=0TO6000:NEXT
750 CLS:LOCATE 1,0:PRINT"SCORE":LOCATE12,0:PRINT"HI-SCORE"
760 LOCATE0,1:PRINTSC*100:LOCATE11,1:PRINTHI*100
770 LF=LI:IFLF>6THENLF=6
780 FOR A=1 TO LF
790 LOCATE21,0:PRINTSTRING$(LF,134);
800 NEXT
810 L=PG:L=L+1:C$="00":GOSUB1610:A$=B$:L=LV:GOSUB1610
820 LOCATE 27,0:PRINT "SCENE":LOCATE 27,1:PRINT A$;"-";B$
830 REM INIT VAR
840 X=48:Y=16:SP=1:SA=0:CR=0
850 GA=0
854 FORJ=0TO31:PUTSPRITEJ,(0,0),0,0:NEXT
855 GOSUB 1348:V=2048:DEFUSR=&HD012:A=USR(0):V=2048:REM
860 XA=144-24:YB=16:REM cariola
865 DEFUSR=&HD003:A=USR(0)
866 POKE&HD016,0:REMuovo
867 POKE&HD017,0:REMcondor
868 POKE&HD015,0
870 REM
875 A=USR4(1+PG)
880 A=USR3(0)
885 INTERVALON:ONINTERVAL=200GOSUB1730
900 ON STRIG GOSUB 1210,1210,1210
910 STRIG(0)ON:STRIG(1)ON
920 ONSTICK(0)ORSTICK(1)GOSUB1100,1100,1110,1110,1120,1120,1130,1130
930 IFSTICK(0)ORSTICK(1)>0THENK=K+1ANDK<1
940 X1=X:Y1=Y
955 POKE&HDE03,Y:POKE&HDE04,X:POKE&HDE05,SP+K
956 A=USR5(0)
960 IFP=1THENPUTSPRITE10,(X+16,Y+7),6,21
970 IFX>240THENIFPG=7THENX=X-8ELSEIFPG<7THEN PG=PG+1:A=USR4(1+PG):A=USR3(0):X=8:GOSUB1180:GOSUB1620:POKE&HDA03,8:A=USR1(0):FORI=2TO5:PUTSPRITEI,,0,0:NEXTI:PUTSPRITE9,,0,0:TI=0:CR=0:XC=0:YC=0:XO=0:YO=0
980 IFX<8THENIFPG=0THENX=X+8ELSEIFPG>0THEN PG=PG-1:A=USR4(1+PG):A=USR3(0):X=240:GOSUB1190:GOSUB1620:POKE&HDA03,9:A=USR1(0):FORI=2TO5:PUTSPRITEI,,0,0:PUTSPRITE9,,0,0:NEXTI:TI=0:CR=0:XC=0:YC=0:XO=0:YO=0
990 IFSC=>BOTHENBO=BO+300:GOSUB1680:LI=LI+1:LF=LI:IFLF>6THENFORI=0TO1000:NEXTIELSELOCATE20+LF,0:PRINTCHR$(134):FORI=0TO1000:NEXTI
1000 IFSP=7ANDVPEEK(6144+INT((Y+16)/8)*32+((X+8)/8))=32THENGOSUB1070
1010 IFSP=1ANDVPEEK(6144+INT((Y+16)/8)*32+((X)/8))=32THENGOSUB1070
1020 IFTIME>2000THENTIME=0:CR=1
1030 IFCR=1THENA=USR3(0)
1041 REM check condor sopra 1up
1042 A=INT(PEEK(&HD0C6)/8):B=INT(X/8):IF PEEK(&HD017)=1 THEN IF A=B THEN POKE&HD015,1
1043 Q1=VPEEK(6144+INT((Y+16)/8)*32+((X+8)/8)):IF Q1=193 OR Q1=195 THENGOSUB1390:REM MORTE CON UOVO
1050 GOTO920
1060 REM CADUTA 1UP
1070 K=0:SP=22:PUTSPRITE6,(X,Y),6,SP:PUTSPRITE7,(X,Y),10,SP+1:PUTSPRITE8,(X,Y),15,SP+2:Y=Y+4:FORI=0TO5:NEXTI
1080 IFVPEEK(6144+INT((Y+16)/8)*32+((X)/8))=128THENGOTO1380ELSEGOTO1070
1090 REM ANIMAZIONE 1UP
1100 GOSUB1160:IFUP=136THENPS=1:SP=4:Y=Y-8:GOSUB1200:RETURNELSEPS=0:RETURN
1110 IFPS=0THENSP=0:GOSUB1140:X=X+8:GOSUB1180:RETURNELSERETURN
1120 GOSUB1170:IFDW=136THENPS=1:SP=4:Y=Y+8:GOSUB1200:RETURNELSEPS=0:RETURN
1130 IFPS=0THENSP=2:GOSUB1150:X=X-8:GOSUB1190:RETURNELSERETURN
1140 IFX=240THENRETURNELSEIFVPEEK(6144+INT(INT((Y)/8)*32+((X+16)/8)))=128THENX=X-8:RETURNELSERETURN
1150 IF VPEEK(6144+INT(INT((Y)/8)*32+((X-8)/8)))=128 THENX=X+8:RETURNELSERETURN
1160 UP=VPEEK(6144+INT(INT((Y)/8)*32+((X)/8))):RETURN
1170 DW=VPEEK(6144+INT(INT((Y+16)/8)*32+((X)/8))):RETURN
1180 IFSA=1THENSP=6:RETURNELSERETURN
1190 IFSA=1THENSP=8:RETURNELSERETURN
1200 IFSA=1THENSP=10:RETURNELSERETURN
1210 REM METTE SACCO SULLA CARIOLA
1211 A=USR3(0)
1220 IFSA=1ANDX=XA+24ANDY=YBTHENSA=0:SP=0:SC=SC+5:LOCATE0,1:PRINTSC*100:LOCATE25,1:POKE&HDA03,9:A=USR1(0):SS=SS+1
1225 IFSS=65THENFORI=0TO500:NEXTI:FORJ=0TO4:LOCATE 8,10+J:PRINTSTRING$(16,32):NEXTJ:LOCATE 8+2,12:PRINT " stage clear ":GOSUB1660:FORI=0TO1000:NEXTI:SS=0:LV=LV+1:ST=ST+1:GOTO690
1230 IF SA=1 THEN RETURN
1240 REM PRENDI SACCO
1250 Q2=VPEEK(6144+INT(INT((Y)/8)*32+((X)/8)))
1260 Q1=6144+INT(INT((Y)/8)*32+((X)/8))
1261 Q3=8192+PG*704+INT(Y/8)*32+INT(X/8)
1270 IFP=1THENP=0:PUTSPRITE10,,0,0:VPOKEQ3+35,162:VPOKEQ3+36,163:XA=X:YB=Y:POKE&HDA03,7:A=USR1(0):A=USR4(1+PG):A=USR3(0)
1280 IF Q2=144 OR Q2=146 THEN GOSUB 1341:SA=1:POKE&HDA03,10:A=USR1(0):IFSP=1THENGOSUB1180:RETURNELSEIFSP=7THENGOSUB1190:RETURN
1290 H2=VPEEK(6144+INT(INT((Y+8)/8)*32+((X+16)/8)))
1300 H1=6144+INT(INT((Y+8)/8)*32+((X+16)/8))
1301 H3=8192+PG*704+INT(INT((Y+8)/8)*32+((X+16)/8))
1310 IFH2=162THENVPOKEH2,32:VPOKEH2+1,32:VPOKE H3,32:VPOKE H3+1,32:A=USR(1+PG):A=USR3(0):P=1:XA=X:YB=Y:POKE&HDA03,4:A=USR1(0)
1320 REM PRESA ALLA MANIGLIA PER CARRELLO
1330 IFVPEEK(6144+INT((Y-8)/8)*32+((X+8)/8))=132THENSP=13:K=0:POKE&HDA03,6:A=USR1(0)
1340 RETURN
1341 REM clear one sacco
1342 VPOKE Q3,32:VPOKEQ3+1,32:VPOKEQ3+32,32:VPOKEQ3+33,32
1343 A=USR4(1+PG):A=USR3(0)
1344 B$=RIGHT$("0000"+HEX$(Q3),4)
1345 VPOKE V,VAL("&H"+LEFT$(B$,2))
1346 VPOKE V+1,VAL("&H"+RIGHT$(B$,2)):V=V+2
1347 RETURN
1348 REM clear vram 2048
1349 DEFUSR=&HD00F:A=USR(0):RETURN
1380 REM MORTE 1UP
1390 POKE&HDE05,14:A=USR5(0)
1400 SOUND7,56:SOUND12,25:SOUND11,0:SOUND10,0
1401 SOUND0,50:SOUND1,6:SOUND8,16
1402 SOUND2,50:SOUND3,7:SOUND9,16
1403 SOUND4,50:SOUND5,5:SOUND10,16
1404 FORI=0TO2:SOUND13,0:FORJ=0TO300:NEXTJ:NEXTI
1405 SOUND0,200:SOUND1,7:SOUND2,200:SOUND3,8:SOUND4,200:SOUND5,6
1406 SOUND12,35:SOUND13,0
1410 FORI=0TO2000:NEXTI
1420 LI=LI-1:IF LI<0 THEN1560
1430 LF=LI:IFLI>6THENLF=6:GOTO1450
1440 LOCATE 21+LF,0:PRINTCHR$(32)
1450 SP=1:X=48:Y=16:PUTSPRITE9,,0,0
1460 FORI=2TO5:PUTSPRITEI,,0,0:PUTSPRITE9,,0,0:NEXTI:TI=0:CR=0
1470 IFSA=1THENSS=SS+1:SA=0:PUTSPRITE5,,0,0
1480 POKE&HD017,0:POKE&HD016,0:A=USR3(1+PG):A=USR3(0):A=USR1(0):RETURN
1490 REM BONUS 1UP
1500 GOSUB 1680
1510 LI=LI+1
1520 LF=LI:IFLF>6THENLF=6
1530 LOCATE 20+LF,0:PRINTCHR$(134)
1540 RETURN
1550 REM GAME OVER
1560 LOCATE 10,10:PRINT" GAME OVER "
1570 GOSUB1580:GOSUB1680
1580 FORI=1 TO 3000:NEXTI
1590 GOTO 230
1600 REM PRINT SCENE
1610 X$=STR$(L):B$=RIGHT$(C$+RIGHT$(X$,LEN(RIGHT$(X$,LEN(X$)-1))),LEN(C$)):RETURN
1620 L=PG:L=L+1:C$="00":GOSUB1610:A$=B$:L=LV:GOSUB1610
1630 LOCATE 27,0:PRINT "SCENE":LOCATE 27,1:PRINT A$;"-";B$
1640 RETURN
1650 REM SFX SOUND
1660 POKE&HDE05,0:A=USR5(0)
1661 PLAY"V15T180O4S3M8000L4B-O6DCO5L8B-A-GA-CDS0M31000L1E-","V15T180S3M8000O2L4B-O3L8A-.R16A-.R16O2L4B-O4L8C.R16O3B-.O2L4E-GB-O3L8E-","V15T180S3M8000R4O4L8F.R16D.R16R4O3G.R16G.R16L2G."
1662 FORI=0TO2500:NEXTI
1665 FOR I=250 TO 50 STEP-5:FOR O=I TO I-50 STEP-8
1667 SOUND8,13:SOUND0,O:NEXT:NEXT:SOUND0,0
1668 RETURN
1670 REM
1680 RESTORE1710:GOSUB1690:IFI3=0THENPLAYZ(0),Z(1),Z(2):RETURNELSEFORI3=I3TO0STEP-1:READI0,I1,I2:PLAYZ(I0),Z(I1),Z(I2):NEXT:RETURN
1690 READI0,Z0,I1,I3:SOUND7,I0:PLAYZ0,Z0,Z0:FORI0=0TOI1:READZ(I0):NEXT:RETURN
1700 RETURN
1710 DATA&O70,t220s1m4096l8,2,0,O5EEEG+V12ES1BBBO6EV12O5BO6EV8O5BO6EV6O5BO6E,O5G+G+G+BV12G+S1O6D+D+D+G+V12D+G+V8D+G+V6D+G+,O4BBBO5EV12O4BS1O5G+G+G+BV12G+BV8G+BV6G+B
1720 REM CONDOR
1730 POKE&HD017,1:RETURN


FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-fortknox-fortknox", "", "MSXBAS2ROM: Jogo: Fortknox", "", "fortknox.bas: ' Game Info:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Fortknox: fortknox.bas

'-----------------------------------------------------------------
' FORTKNOX - MSXBAS2ROM DEMO
'-----------------------------------------------------------------
' Game Info:
'   FORT KNOX is a remake of Bagman, an arcade video game released in 1982 by the French Valadon
'   Released as an Open source Freeware License (c) 2025 by GAMECAST Entertainment:
'     https://www.msxdev.org/2025/01/31/msxdev24-30-fort-knox/
'   Rewritten for MSXBAS2ROM by Amaury Carvalho (2025):
'     https://github.com/amaurycarvalho/msxbasic/Fortknox
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'   https://launchpad.net/nmsxtiles
'   https://github.com/pipagerardo/nMSXtiles
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom -x fortknox.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/fortknox.akm"                               ' 0 - songs exported from ArkosTracker 3 (Music.aks)
FILE "music/effects.akx"                                ' 1 - sound effects exported from ArkosTracker 3 (SoundEffects.aks)
FILE "levels/SFBANK00.VRM"                              ' 2 - scenes from stage type 1
FILE "levels/SFBANK01.VRM"                              ' 3 - scenes from stage type 2
FILE "img/INTRO.SC1"                                    ' 4 - tile bank (edited via nMSXTiles)
FILE "img/fortknox1.spr"                                ' 5 - sprites bank - player handsfree (plain text, edited via Tiny Sprite)
FILE "img/fortknox2.spr"                                ' 6 - sprites bank - player holding the bag (plain text, edited via Tiny Sprite)
FILE "img/fortknox3.spr"                                ' 7 - sprites bank - player pushing the car (plain text, edited via Tiny Sprite)

' Initialization
1 DEFINT A-Z
2 DIM SCA(351,7)                                        ' scenes data buffer (screen data=704 bytes, 8 scenes)
3 CMD PLYLOAD 0, 1                                      ' load music and effects from resources 0/1
4 SCREEN 1,2,0 : COLOR 15,0,0
5 HI=512 : SC = 0 : TI = TIME : SN = 0 : MUS = 99       ' HI = high score, SC = score
6 WIDTH 32
7 IF NTSC() THEN TS = 60 : TD = 6 ELSE TS = 50 : TD = 5

' Intro screen
10 GOSUB 9040                                           ' clear sprites
11 GOSUB 9050                                           ' load tiles bank
12 GOSUB 8020                                           ' print score and hi score
13 N = 200 : X = 11 : Y = 5 : GOSUB 8000                ' draw tile 2x2
14 N = 204 : X = 13 : GOSUB 8000
15 N = 208 : X = 16 : GOSUB 8000
16 N = 212 : X = 18 : GOSUB 8000
20 LOCATE  6, 8 : PRINT "the bagman return"
21 LOCATE  1,12 : PRINT "1UP BONUS EVERY FOR 30000 PTS"
22 LOCATE 10,14 : PRINT "INTRO MUSIC BY"
23 LOCATE  2,16 : PRINT "CLAUDIO LUPO & DANIEL TKACH"
24 LOCATE  0,18 : PRINT "   @ 1982 VALADON AUTOMATION"
25 LOCATE  2,20 : PRINT "PUSH SPACE OR FIRE TO START"
26 LOCATE  0,22 : PRINT "    @ 2025 GAMECAST & A.C."
27 IF MUS <> 0 THEN MUS = 0 : GOSUB 9030                ' play intro song

30 GOSUB 9000                                           ' get player input
31 IF K = 0 THEN 40
32   ST = 1 : SCN = 1                                   ' ST = stage number, SCN = scene number
33   SC = 0 : LV = 3                                    ' SC = score, LV = lives
34   TI = TIME
35   GOTO 100                                           ' start gameplay

40 IF TIME < TI THEN TI = TIME
41 DI = TIME - TI
42 IF DI < (TS*6) THEN 30
43   TI = TIME
44   SN = (SN + 1) MOD 2
45   IF SN = 0 THEN 10

50 GOSUB 9050                                           ' load tiles bank
51 PX = 16 : PY = 28 : PS = 0 : GOSUB 8050              ' show player sprite
52 CX = 10 : CY = 60 : CS = 0 : GOSUB 8060              ' show condor sprite
54 N = &H90 : X = 2 : Y = 11 : GOSUB 8010               ' draw bag top (tile 2x1)
55 N = &H92 : Y = 12 : GOSUB 8010                       ' draw bag bottom (tile 2x1)
56 N = &HA2 : Y = 15 : GOSUB 8010                       ' draw car
57 LOCATE 6, 4 : PRINT "player1"
58 LOCATE 6, 8 : PRINT "bird condor"
59 LOCATE 6,12 : PRINT "bag"
60 LOCATE 6,15 : PRINT "whellbarrow"
61 GOSUB 8020                                           ' print score and hi score
62 LOCATE 2,18 : PRINT "TAKE ALL THE BAGS AND CARRY"
63 LOCATE 2,20 : PRINT "  THEM ON THE WHEELBARROW"
64 LOCATE 2,22 : PRINT "   POWERED BY MSXBAS2ROM"
65 GOTO 30

' Stage initialization
100 GOSUB 9040                                           ' clear sprites
101 MUS = 1 : GOSUB 9030                                 ' stage start song
102 GOSUB 9050                                           ' load tiles bank
103 LOCATE 13,10 : PRINT "STAGE";ST
104 LOCATE 14,12 : PRINT "READY"
105 I = 4 : GOSUB 9020                                   ' wait 4 seconds
106 BF = 0 : GOSUB 8200                                  ' player action flag (0=handsfree, 1=holding the bag, 2=pushing the car)
107 BC = 65                                              ' bags count
108 GOSUB 8100                                           ' load stage scenes bank

' Scene initialization
110 PX = 0 : PY = 15 : PS = 1                            ' player x, y and sprite
111 CX = 0 : CY = 212 : CS = 0                           ' condor x, y and sprite
112 EX = 0 : EY = 212                                    ' egg x, y
113 PT = TIME : CT = TIME : ET = TIME                    ' timers for player, condor and egg logic
114 GOSUB 9040                                           ' clear sprites
115 GOSUB 8030                                           ' draw stage/scene on screen
116 GOSUB 8050                                           ' show player sprite
117 GOSUB 8060                                           ' show condor sprite
118 GOSUB 8070                                           ' show egg sprite
119 GOSUB 8080                                           ' show lives

' Gameplay loop
120 GOSUB 200                                            ' player logic
121 GOSUB 300                                            ' condor logic
122 GOSUB 500                                            ' egg logic
123 IF BC = 0 THEN 900                                   ' if no more bags: stage clear
124 IF LV = 0 THEN 950                                   ' if nothing left: game over
125 GOTO 120

' Player movement logic
200 IF TIME < PT THEN PT = TIME
201 DI = TIME - PT
202 IF DI < (TD*2) THEN RETURN                           ' time step = 5x per second
203   PT = TIME

210 GOSUB 9000                                           ' get player input
211 IF K <> 0 GOSUB 800                                  ' player press the button?
212 ON J GOTO 220, 213, 230, 213, 240, 213, 250, 213     ' player press the stick?
213   RETURN

220 IF PS = 2 THEN PS = 6 ELSE PS = 2                    ' --> player moves UP
221 IF PY <= 15 THEN RETURN
222   GOSUB 270                                          ' get up/down tiles
223   IF TU1 <> &H88 OR BF = 2 THEN RETURN               ' if not a ladder (or pushing the car), return
224     PY = PY - 8
225     GOTO 8050                                        ' show player sprite

230 IF PS = 0 THEN PS = 1 ELSE PS = 0                    ' --> player moves RIGHT
231 IF PX < 240 THEN 236                                 ' if hit right border: next scene
232   IF SCN >= 8 THEN RETURN
233     SCN = SCN + 1
234     PX = 0 : IF PS = 0 THEN PS = 1 ELSE PS = 0       ' player x, y and sprite
235     RETURN 111                                       ' next scene
236 GOSUB 260                                            ' get left/right tiles
237 IF TR1 = &H80 OR TR2 = &H80 THEN RETURN              ' if a wall, return
238   PX = PX + 8
239   GOTO 8050                                          ' show player sprite

240 IF PS = 2 THEN PS = 6 ELSE PS = 2                    ' --> player moves DOWN
241 IF PY >= 170 THEN RETURN
242   GOSUB 270                                          ' get up/down tiles
243   IF TD1 <> &H88 OR BF = 2 THEN RETURN               ' if not a ladder (or pushing the car), return
244     PY = PY + 8
245     GOTO 8050                                        ' show player sprite

250 IF PS = 4 THEN PS = 5 ELSE PS = 4                    ' --> player moves LEFT
251 IF PX > 7 THEN 256                                   ' if hit left border: previous scene
252   IF SCN <= 1 THEN RETURN
253     SCN = SCN - 1
254     PX = 240 : IF PS = 4 THEN PS = 5 ELSE PS = 4     ' player x, y and sprite
255     RETURN 111                                       ' previous scene
256 GOSUB 260                                            ' get left/right tiles
257 IF TL1 = &H80 OR TL2 = &H80 THEN RETURN              ' if a wall, return
258   PX = PX - 8
259   GOTO 8050                                          ' show player sprite

260 X=PX/8 : Y=PY/8                                      ' get LEFT/RIGHT tiles from player position
261 TL1=TILE(X-1,Y+1)                                    ' tile position in text mode coords
262 TL2=TILE(X-1,Y+2)
263 TR1=TILE(X+2,Y+1)
264 TR2=TILE(X+2,Y+2)
265 RETURN

270 X=PX/8 : Y=PY/8                                      ' get UP/DOWN tiles from player position
271 TU1=TILE(X,Y+1)                                      ' tile position in text mode coords
272 TD1=TILE(X,Y+3)
273 RETURN

280 X=PX/8 : Y=PY/8+1                                    ' get inner tiles from player position
281 TP1=TILE(X,Y)                                        ' tile position in text mode coords
282 TP2=TILE(X,Y+1)
283 TP3=TILE(X+1,Y)
284 TP4=TILE(X+1,Y+1)
285 TPS=TP1 OR TP2 OR TP3 OR TP4
286 RETURN

' Condor appearence logic
300 IF TIME < CT THEN CT = TIME
301 DI = TIME - CT
302 IF DI < (TD*3) THEN RETURN                           ' time step = 3x per second
303   CT = TIME

310 IF CY < 212 THEN 400                                 ' if condor already in action: movement logic
311 GOSUB 9005                                           ' next random number
312 IF (R MOD 20) > ST THEN RETURN                       ' random condor appearence: higher stages = more appearences
313   CY = 0 : CX = 0
314   CS = 0
315   GOTO 8060                                          ' show condor sprite

' Condor movement logic
400 CS = (CS + 1) MOD 2
401 CX = CX + 8
402 IF CX > 220 THEN CY = 212                            ' if right border: hide condor
403 GOSUB 8060                                           ' show condor sprite

' Egg appearence logic
500 IF TIME < ET THEN ET = TIME
501 DI = TIME - ET
502 IF DI < TD THEN RETURN                               ' time step = 10x per second
503   ET = TIME

510 IF EY < 212 THEN 600                                 ' if egg already in action: movement logic
511   IF CY >= 212 THEN RETURN                           ' if condor not in action: return
512     GOSUB 9005                                       ' next random number
513     IF (R MOD 40) > ST THEN RETURN                   ' random egg appearence: higher stages = more appearences
514       EY = CY : EX = CX + 8
515       GOTO 8070                                      ' show egg sprite

' Egg movement logic
600 IF EY >= 212 THEN RETURN
601   EY = EY + 6
602   GOSUB 8070                                         ' show egg sprite

' Egg vs player collision logic
700 IF COLLISION(7,0) < 0 THEN RETURN                    ' if egg sprite collided with player sprite
701   IF PS < 4 THEN PS = 3 ELSE PS = 7                  ' player is dead
702     GOSUB 8050                                       ' show player sprite
703     CMD PLYSOUND 1                                   ' play egg collision sound effect
704     IF LV > 0 THEN LV = LV - 1 : GOSUB 8080          ' show lives
705     EY = 212
706     GOTO 8070                                        ' hide egg

' Player trying to get an item
800 GOSUB 280                                            ' get inner tiles from player position
801 ON BF GOTO 820, 860                                  ' player action status (0=handsfree, 1=holding the bag, 2=pushing the car)
802   IF TP2 = &HA2 THEN 850                             ' is it the car?
803     IF TP1 <> &H90 THEN RETURN                       ' if not a bag, return

' Getting the bag logic
810       GOSUB 8015                                     ' clear bag tile
811       GOSUB 8110                                     ' register getting the bag on the scene buffer
812       CMD PLYSOUND 4                                 ' play getting an item sound effect
813       BF = 1 : GOSUB 8200                            ' player holding the bag
814       GOTO 8050                                      ' show player sprite

' Releasing the bag logic
820 IF TP2 = &HA2 OR TP2 = &HA3 THEN 830                 ' if car tile, release the bag
821   IF TPS <> 32 AND TPS <> 0 THEN RETURN              ' if not an empty space, return
822     N = &H90 : GOSUB 8090                            ' release the bag on the floor
823     GOSUB 8120                                       ' register releasing the bag on the scene buffer
824     BF = 0 : GOSUB 8200                              ' player handsfree flag
825     CMD PLYSOUND 4                                   ' play release an item sound effect
826     GOTO 8050                                        ' show player sprite

' Release the bag in the car logic
830 BF = 0 : GOSUB 8200                                  ' player handsfree flag
831 GOSUB 8050                                           ' show player sprite
832 CMD PLYSOUND 4                                       ' play release an item sound effect

' Add score and reduce bags count
840 SC = SC + 1                                          ' add to score points
841 IF BC > 0 THEN BC = BC - 1                           ' reduce bags count
842 GOTO 8020                                            ' print score/hi-score

' Getting the car logic
850 GOSUB 8015                                           ' clear car tile
851 GOSUB 8110                                           ' register getting the car on the scene buffer
852 CMD PLYSOUND 4                                       ' play getting an item sound effect
853 BF = 2 : GOSUB 8200                                  ' player holding the bag
854 GOTO 8050                                            ' show player sprite

' Releasing the car logic
860 IF TPS <> 32 AND TPS <> 0 THEN RETURN                ' if not an empty space, return
861   N = &HA2 : Y = Y + 1 : GOSUB 8010                  ' release the car on the floor
862   GOSUB 8130                                         ' register releasing the car on the scene buffer
863   BF = 0 : GOSUB 8200                                ' player handsfree flag
864   CMD PLYSOUND 4                                     ' play release an item sound effect
865   GOTO 8050                                          ' show player sprite

' Stage clear
900 ST = ST + 1 : SCN = 1
901 IF LV < 5 THEN LV = LV + 1 : GOSUB 8080              ' show lives
902 LOCATE 3,10 : PRINT "+----------------------+"
903 LOCATE 3,11 : PRINT "+ S T A G E  C L E A R +"
904 LOCATE 3,12 : PRINT "+----------------------+"
905 MUS = 3 : GOSUB 9030                                 ' stage clear song
906 I = 3 : GOSUB 9020                                   ' wait 3 seconds
907 GOTO 100                                             ' go to next stage

' Game over
950 IF SC > HI THEN HI = SC                              ' new high score
951 LOCATE 5,10 : PRINT "+--------------------+"
952 LOCATE 5,11 : PRINT "+  G A M E  O V E R  +"
953 LOCATE 5,12 : PRINT "+--------------------+"
954 MUS = 2 : GOSUB 9030                                 ' game over song
955 GOSUB 9010                                           ' wait for player hit a button
956 SN = 0 : TI = TIME
957 GOTO 10                                              ' return to initialize game

'-------------------------------------------
' KNOX SUPPORT ROUTINES
'-------------------------------------------

' Put tile block 2x2 started by N on X,Y (vertical way)
8000 PUT TILE N,(X,Y)
8001 PUT TILE N+1,(X,Y+1)
8002 PUT TILE N+2,(X+1,Y)
8003 PUT TILE N+3,(X+1,Y+1)
8004 RETURN

' Put tile block 2x1 started by N on X,Y
8010 PUT TILE N,(X,Y)
8011 PUT TILE N+1,(X+1,Y)
8012 RETURN

' Clear tile block 2x2 on X,Y
8015 PUT TILE 32,(X,Y)
8016 PUT TILE 32,(X,Y+1)
8017 PUT TILE 32,(X+1,Y)
8018 PUT TILE 32,(X+1,Y+1)
8019 RETURN

' Print score and hi score
8020 LOCATE 0, 0 : PRINT " SCORE        HI SCORE"
8021 PRINT USING$("######", SC*100.0); SPC(10); USING$("######", HI*100.0)
8022 RETURN

' Draw stage ST - scene SCN
8030 BUF = VARPTR(SCA(0,SCN-1))                          ' start address of element SCENE,SCREEN_INTEGER
8031 CMD RAMTOVRAM BUF, BASE(5)+64, 704                  ' copy screen from RAM buffer to VRAM (name table position, line 2)
8032 GOSUB 8020                                          ' print score/hi-score

' Print stage/scene
8040 LOCATE 27,0 : PRINT "SCENE";
8041 LOCATE 27,1 : PRINT USING$("00", ST);"-";USING$("00", SCN);
8042 RETURN

' Show player sprite
8050 SS = (PS * 3) + 9
8051 PUT SPRITE 0,(PX,PY),11,SS
8052 PUT SPRITE 1,(PX,PY),8,SS+1
8053 PUT SPRITE 2,(PX,PY),15,SS+2
8054 RETURN

' Show condor sprite
8060 SS = CS*4 + 1
8061 PUT SPRITE 3,(CX,CY),15,SS
8062 PUT SPRITE 4,(CX+16,CY),15,SS+1
8063 PUT SPRITE 5,(CX,CY),8,SS+2
8064 PUT SPRITE 6,(CX+16,CY),8,SS+3
8065 RETURN

' Show egg sprite
8070 PUT SPRITE 7,(EX,EY),15,0
8071 RETURN

' Show lives
8080 X = 23 : Y = 0
8081 FOR I = 1 TO 4
8082   IF I < LV THEN C = &H86 ELSE C = 32
8083   PUT TILE C,(X, Y)
8084   X = X + 1
8085 NEXT
8086 RETURN

' Put tile block 2x2 started by N on X,Y (horizontal way)
8090 PUT TILE N,(X,Y)
8091 PUT TILE N+1,(X+1,Y)
8092 PUT TILE N+2,(X,Y+1)
8093 PUT TILE N+3,(X+1,Y+1)
8094 RETURN

' Load stage scenes bank to RAM buffer
8100 CMD RESTORE 2 + ((ST - 1) MOD 2)                    ' load stage data file
8101 IRESTORE 39                                         ' set initial read position on stage data file
8102 FOR Y = 0 TO 7                                      ' scenes data loop
8103   FOR X = 0 TO 351                                  ' screen from scene data loop
8104     IREAD SCA(X,Y)                                  ' read next integer data to scenes buffer (screen_data,scene)
8105   NEXT
8106 NEXT
8107 RETURN

' Register getting the bag in the scene buffer
8110 N = ((Y - 2) * 32 + X)                              ' element on scenes buffer
8111 BUF = VARPTR(SCA(0, SCN-1)) + N
8112 POKE BUF, &H20
8113 POKE BUF+1, &H20
8114 POKE BUF+32, &H20
8115 POKE BUF+33, &H20
8116 RETURN

' Register release the bag in the scene buffer
8120 N = ((Y - 2) * 32 + X)                             ' element on scenes buffer
8121 BUF = VARPTR(SCA(0, SCN-1)) + N
8122 POKE BUF, &H90
8123 POKE BUF+1, &H91
8124 POKE BUF+32, &H92
8125 POKE BUF+33, &H93
8126 RETURN

' Register release the car in the scene buffer
8130 N = ((Y - 2) * 32 + X)                             ' element on scenes buffer
8131 BUF = VARPTR(SCA(0, SCN-1)) + N
8132 POKE BUF, &HA2
8133 POKE BUF+1, &HA3
8134 RETURN

' Load sprite bank (player handsfree, holding the bag, pushing the car)
8200 SPRITE LOAD 5 + BF
8201 RETURN

'-------------------------------------------
' GENERAL SUPPORT ROUTINES
'-------------------------------------------

' Get player input (out: K, J)
9000 K = STRIG(0) OR STRIG(1)
9001 J = STICK(0) OR STICK(1)

' Next random number
9005 R = RND(1) * 100
9006 RETURN

' Wait for player hit a button
9010 GOSUB 9000                                          ' get player input
9011 IF K=0 THEN 9010
9012 RETURN

' Wait I seconds
9020 T1 = TIME : I = I * TS
9021 IF T1 > TIME THEN T1 = TIME
9022 DT = TIME - T1
9023 IF DT < I THEN 9021
9024 RETURN

' Play song MUS
9030 CMD PLYSONG MUS
9031 CMD PLYPLAY
9032 RETURN

' Clear sprites
9040 FOR A=0 TO 31
9041   PUT SPRITE A,,0,0
9042 NEXT
9043 RETURN

' Load tile and sprite bank
9050 SCREEN LOAD 4
9051 SPRITE LOAD 5
9052 RETURN

' Clear input buffer
9060 GOSUB 9000
9061 IF K <> 0 THEN 9060
9062 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-fortknox-readme", "", "MSXBAS2ROM: Jogo: Fortknox", "", "README.md: Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Fortknox: README.md

FORTKNOX
Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- FORT KNOX is a remake of Bagman), an arcade video game released in 1982 by the French Valadon;
- It was released as an Open source Freeware License (c) 2025 by GAMECAST Entertainment for MSXDev24 contest;
- Source code rewritten to be compiled with MSXBAS2ROM by Amaury Carvalho (2025).

SOURCE CODE

Compiling

       msxbas2rom -x fortknox.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-kwirk-kwirk", "", "MSXBAS2ROM: Jogo: Kwirk", "", "kwirk.bas: ' Game Info:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Kwirk: kwirk.bas

'-----------------------------------------------------------------
' KWIRK - MSXBAS2ROM DEMO
'-----------------------------------------------------------------
' Game Info:
'   Kwirk is a 1989 game created by Atlus and published by Acclaim for Gameboy (1990)
'   Levels adapted from TI89 version created by Ludvig Strigeus and Jordan Clifford (2011)
'   Remake to MSX2 created by Amaury Carvalho (2025):
'     https://github.com/amaurycarvalho/msxbasic/kwirk
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom -x kwirk.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/kwirk.akm"                                  ' 0 - songs exported from ArkosTracker 3 (Music.aks)
FILE "music/effects.akx"                                ' 1 - sound effects exported from ArkosTracker 3 (SoundEffects.aks)
FILE "levels/kEasy.89s"                                 ' 2 - levels easy
FILE "levels/kMed.89s"                                  ' 3 - levels medium
FILE "levels/kHard.89s"                                 ' 4 - levels hard
FILE "img/splash.SC5"                                   ' 5 - splash screen
FILE "img/skill.SC5"                                    ' 6 - skill screen
FILE "img/floor.SC5"                                    ' 7 - floor screen
FILE "img/start.SC5"                                    ' 8 - start screen
FILE "img/wall.SC5"                                     ' 9 - wall screen
FILE "img/abandon.SC5"                                  ' 10 - abandon screen
FILE "img/finish.SC5"                                   ' 11 - finish screen
FILE "img/tiles.SC5"                                    ' 12 - tiles

' Initialization
1 DEFINT A-Z
2 DIM MAZE(24,32)
3 IF MSX() = 0 THEN PRINT "Sorry, not for MSX1 machines" : END
4 CMD PLYLOAD 0, 1                                      ' load music and effects from resources 0/1
5 SCREEN 5,,0
6 COLOR 4,0,0

' Intro screen
10 CLS
11 COPY(0,0)-(255,212),0 TO (0,0),1
12 SC = 5 : GOSUB 9020                                  ' show splash screen
13 MU = 0 : GOSUB 9030                                  ' play intro song
14 GOSUB 9010                                           ' wait for player hit a button
15 CMD PLYMUTE                                          ' stops song

' Skill screen
20 SC = 6 : GOSUB 9020                                  ' show skill screen
21 MU = 1 : GOSUB 9030                                  ' play parameters screen song
22 GOSUB 9010                                           ' wait for player hit a button

' Floor screen
30 SC = 7 : GOSUB 9021                                  ' show floor screen
31 GOSUB 9010                                           ' wait for player hit a button

' Start screen
40 SC = 8 : GOSUB 9021                                  ' show start screen
41 GOSUB 9010                                           ' wait for player hit a button
42 CMD PLYMUTE

' Wall screen
50 SC = 9 : GOSUB 9020                                  ' show wall screen
51 MU = 2 : GOSUB 9030                                  ' maze song
52 GOSUB 9010                                           ' wait for player hit a button

' Abandon screen
60 SC = 10 : GOSUB 9020                                 ' show abandon screen
61 MU = 2 : GOSUB 9030                                  ' maze song
62 GOSUB 9010                                           ' wait for player hit a button

' Finish screen
70 SC = 11 : GOSUB 9020                                 ' show finish screen
71 MU = 2 : GOSUB 9030                                  ' maze song
72 GOSUB 9010                                           ' wait for player hit a button

' Tiles screen
80 GOSUB 8000
81 SET PAGE 1,1
82 GOSUB 9010

' End demo
90 SCREEN 0 : PRINT "GAME FINISHED" : END

' Load tiles on page 1
8000 SET PAGE 0,1
8001 SC = 12 : GOSUB 9021
8002 RETURN

' Print tile N on X, Y

' Build level maze

' Show level maze

'-------------------------------------------
' SUPPORT ROUTINES
'-------------------------------------------

' Get player input (out: K, J)
9000 K = STRIG(0) OR STRIG(1)
9001 J = STICK(0) OR STICK(1)
9002 RETURN

' Wait for player hit a button
9010 GOSUB 9000                                         ' get player input
9011 IF K=0 THEN 9010
9012 RETURN

' Load resource screen (in: SC)
9020 SET PAGE 1,0
9021 SCREEN LOAD SC
9022 SET PAGE 0,0
9023 RETURN

' Play song (in: MU)
9030 CMD PLYSONG MU
9031 CMD PLYPLAY
9032 RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-kwirk-readme", "", "MSXBAS2ROM: Jogo: Kwirk", "", "README.md: Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Kwirk: README.md

KWIRK
Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- Kwirk is a 1989 game created by Atlus and published by Acclaim for Gameboy (1990);
- Levels adapted from TI89 version created by Ludvig Strigeus and Jordan Clifford (2011);
- Remake to MSX2 created by Amaury Carvalho (2025).

SOURCE CODE

Compiling

       msxbas2rom -x kwirk.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-pickinx-pickinx", "", "MSXBAS2ROM: Jogo: PickinX", "", "pickinx.bas: ' PICKINX - v1.2",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: PickinX: pickinx.bas

'-----------------------------------------------------------------
' PICKINX - v1.2
'-----------------------------------------------------------------
' Game Info:
'   Pickin'X is a MSX clone of the arcade game of similar name developed by Valadon Automation in 1983.
'     https://www.arcade-museum.com/Videogame/pickin
'   Remake to MSX1 created by Amaury Carvalho (2021):
'     https://github.com/amaurycarvalho/msxbasic/PickinX
'     https://www.msxdev.org/2021/05/16/msxdev21-15-pickinx/
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'   https://launchpad.net/nmsxtiles
'   https://github.com/pipagerardo/nMSXtiles
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom pickinx.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/pickinx.akm"                            ' 0
FILE "music/pickinx.akx"                            ' 1
FILE "img/screen_0_0.plet5"                         ' 2
FILE "img/screen_1_0.plet5"                         ' 3
FILE "img/screen_2_0.plet5"                         ' 4
FILE "img/screen_3_0.plet5"                         ' 5
FILE "img/screen_4_0.plet5"                         ' 6
FILE "img/screen_5_0.plet5"                         ' 7
FILE "img/screen_6_0.plet5"                         ' 8
FILE "img/screen_7_0.plet5"                         ' 9
FILE "img/tiles.chr.plet5"                          ' 10
FILE "img/tiles.clr.plet5"                          ' 11
FILE "img/sprites.spr"                              ' 12 - plain text file (load on TinySprite)

1 DATA "[ G O O D  W O R K ! ]"
2 DATA "[ WAS IT JUST LUCK?? ]"
3 DATA "[ YOU DID IT AGAIN ! ]"
4 DATA "[    Y E A H ! ! !   ]"
5 DATA "[  YOU'RE THE BEST ! ]"
6 DATA "[   W  O  W  !  !  ! ]"
7 DATA "[ YOU ARE THE KING ! ]"

'==== GAME START
10 SCREEN 2, 2, 0 : COLOR 15, 0, 0
11 CMD DISSCR
12 CMD WRTCHR 10
13 CMD WRTCLR 11
14 SPRITE LOAD 12
15 CMD SETFNT 1
17 CMD WRTSCR 2
18 CMD PLYLOAD 0,1

20 LOCATE 07,  9 : PRINT "v1.2 BY AC (2021)"
21 LOCATE 13, 12 : PRINT "PICKER";
22 LOCATE 13, 14 : PRINT "TRANSPOSER";
23 LOCATE 13, 16 : PRINT "DISTURBER";
24 LOCATE 13, 18 : PRINT "KILLER";
25 CMD ENASCR

30 PUT SPRITE 0, (60,90), 15, 0
31 PUT SPRITE 1, (60,107), 13, 7
32 PUT SPRITE 2, (60,125), 11, 8
33 PUT SPRITE 3, (60,140),  8, 10
34 PUT SPRITE 4,,0,4
35 PUT SPRITE 5,,0,4

40 GOSUB 1510
41 GOSUB 1000         ' GET PLAYER INPUT
42 IF B% = 0 THEN 41
43 GOSUB 1100         ' CLEAR PLAYER INPUT
44 CMD PLYMUTE

50 DIM BX%(2,20), BY%(2,20), BT%(2,20), BC%(2)
51 AC% = 1 : SS% = 3 : SC% = 500 : TT% = 0 : MR% = HEAP() : MV% = BASE(10)
52 IF NTSC() THEN L1% = 60 : L2% = 30 : L3% = 4 ELSE L1% = 50 : L2% = 25 : L3% = 2

'==== INITIALIZE LEVEL
60 CMD DISSCR
61 CMD WRTSCR SS%
62 GOSUB 2000 : GOSUB 1300
63 CMD ENASCR
64 GOSUB 1520

70 PX% = 120 : PY% = 175 : PS% = 0 : PB% = 0 : GO% = -1
71 DX% = 120 : DY% = 17 : DS% = 8 : DB% = 0 : DM% = 0 : DT% = 0
72 IF AC% > 3 THEN DL% = 5 ELSE DL% = 10
73 IF (RR% MOD 2) = 0 THEN DK% = 3 : KK% = 7 ELSE DK% = 7 : KK% = 3
74 TX% = 0 : TY% = 0 : TS% = 5 : TXI% = 6 : TYI% = 6
75 KX% = 0 : KY% = 0 : KS% = 10 : WI% = 0 : SM% = 0
76 T1% = TIME : T2% = TIME : T3% = TIME : TT% = 0 : BP% = 0 : TF% = -1
77 GOSUB 1200 : GOSUB 1615 : GOSUB 1420 : GOSUB 1430

'==== MAIN LOOP
100 GOSUB 1400                                 ' SHOW SPRITES

110 GOSUB 1600                                 ' TIMERS

115 IF GO% = 0 THEN 3000                       ' GAME OVER
116 IF WI% > 0 THEN 3100                       ' VICTORY

120 GOSUB 1000                                 ' GET PLAYER INPUT
121 IF X% <> 0 OR Y% = 27 THEN GOSUB 3200      ' PAUSE/RESTART
122 IF K% = 0 AND B% = 0 THEN 110

125 IF TF% = 0 THEN 140

130 IF K% <> 0 THEN GOSUB 2200

140 IF B% <> 0 THEN GOSUB 2300

150 GOTO 100

'==== GET PLAYER INPUT
1000 K% = STICK(0) OR STICK(1)
1001 X% = STRIG(3)
1002 B% = STRIG(0) OR STRIG(1)
1003 Y% = INKEY
1004 GOTO 1900

'==== WAIT UNTIL INPUT
1100 IF strig(0) OR strig(1) OR stick(0) OR stick(1) OR strig(3) THEN 1100
1101 IF INKEY > 0 THEN 1101
1102 RETURN

'==== SHOW SCORE
1200 LOCATE 0, 0  : PRINT "LEFT:";USING$("####",SC%);
1201 LOCATE 25, 0 : PRINT "ACT:";USING$("###",AC%);
1202 LOCATE 11, 0 : PRINT "ELAPSED:";USING$("####",TT%);
1203 RETURN

'==== CLEAR SPRITES
1300 FOR I% = 0 TO 3 : PUT SPRITE I%,,0 : NEXT : RETURN

'==== SHOW SPRITES
1400 PUT SPRITE 0,(PX%, PY%),15,PS%
1401 IF PB% > 0 THEN GOSUB 1420
1402 PUT SPRITE 2,(DX%, DY%),11,DS%
1403 IF DB% > 0 THEN GOSUB 1430
1404 IF TY% > 0 THEN PUT SPRITE 1,(TX%, TY%),13,TS%
1405 IF KY% > 0 THEN PUT SPRITE 3,(KX%, KY%),8,KS%
1406 RETURN

1420 PUT SPRITE 4,(PX%, PY%),PB% : RETURN

1430 PUT SPRITE 5,(DX%, DY%),DB% : RETURN

'==== SPLASH SCREEN
1510 LOCATE  3, 20 : PRINT "     ARE YOU READY ?     ";
1511 LOCATE  3, 21 : PRINT "SPACE OR TRIGGER TO START";
1512 CMD PLYSONG 1 : CMD PLYPLAY
1513 RETURN

'==== LEVEL START MELODY
1520 CMD PLYSONG 2 : CMD PLYPLAY
1521 RETURN

'==== TIMERS
1600 D1% = ABS(TIME - T1%) : D2% = ABS(TIME - T2%) : D3% = ABS(TIME - T3%)
1601 IF D1% > L1% THEN GOSUB 1610
1602 IF D2% > L2% THEN GOSUB 1630
1603 IF D3% > L3% THEN GOTO  1650
1604 RETURN

1610 T1% = TIME : TT% = TT% + 1 : LOCATE 19, 0 : PRINT USING$("####",TT%);
1611 SC% = SC% - 1 : LOCATE 5, 0 : PRINT USING$("####",SC%); : IF SC% <= 0 THEN GO% = 0 : RETURN
1612 DI% = TT% - DT% : IF DI% > DL% THEN DT% = TT% : DM% = DM% XOR 1
1613 IF TT% > 120 AND TY% = 0 THEN CMD PLYSONG 3 : CMD PLYPLAY : GOSUB 1620
1614 IF TT% > 180 AND KY% = 0 THEN KY% = 17 : KX% = 120 : CMD PLYSONG 4 : CMD PLYPLAY

1615 'CMD VRAMTORAM MV%, MR%, 768
1616 RETURN

1620 GOSUB 1900 : TY% = RR% MOD 150 + 20 : GOSUB 1900 : TX% = RR% MOD 200 : RETURN

1630 T2% = TIME
1631 IF DS% = 8 THEN DS% = 9 ELSE DS% = 8
1632 TS% = TS% + 1 : IF TS% > 7 THEN TS% = 5
1633 IF KS% = 10 THEN KS% = 11 ELSE KS% = 10
1634 IF DM% = 1 THEN 1950
1635 GOTO 1980

1640 IF SM% = 0 THEN RETURN
1641 CMD PLYSOUND SM%, 2
1642 IF SM% = 2 OR SM% >= 4 THEN SM% = 0 : RETURN
1643 SM% = SM% + 1
1644 RETURN

1650 GOSUB 1640 : T3% = TIME
1651 IF TY% > 0 THEN GOSUB 1840
1652 IF KY% > 0 THEN GOSUB 1870
1653 GOTO 1800

1700 NX% = X% : NY% = Y% : NS% = S% : O% = 0
1701 IF K% = 3 THEN NX% = NX% + NI% : NS% = 1
1702 IF K% = 7 THEN NX% = NX% - NI% : NS% = 3
1703 IF K% = 5 THEN NY% = NY% + NI% : NS% = 2
1704 IF K% = 1 THEN NY% = NY% - NI% : NS% = 0
1705 IF NX% < 0 OR NX% > 240 THEN RETURN
1706 IF NY% < 17 OR NY% > 175 THEN RETURN
1707 AX% = NX% + 2  : AY% = NY% + 2 : IF K% = 1 OR K% = 7 GOSUB 1720
1708 AX% = NX% + 14 : IF K% = 1 OR K% = 3 GOSUB 1720
1709 AY% = NY% + 14 : IF K% = 3 OR K% = 5 GOSUB 1720
1710 AX% = NX% + 2  : IF K% = 5 OR K% = 7 GOSUB 1720
1711 X% = NX% : Y% = NY% : S% = NS% : O% = 1
1712 RETURN

1720 GOSUB 1750 : IF A% > 127 THEN RETURN 1712
1721 IF P% = 0 THEN IF PB% > 0 AND A% <> 0 THEN RETURN 1712
1722 RETURN

1750 XT% = AX% / 8 : YT% = AY% / 8
1751 A% = TILE( XT%, YT% )
1752 RETURN

1800 GOSUB 1900 : RR% = RR% MOD 2 : NI% = 6
1801 X% = DX% : Y% = DY% : P% = 1
1802 IF DK% = 3 OR DK% = 7 THEN IF RR% = 0 THEN K% = 1 ELSE K% = 5
1803 IF DK% = 1 OR DK% = 5 THEN IF RR% = 0 THEN K% = 3 ELSE K% = 7
1804 GOSUB 1700
1805 IF O% = 1 THEN 1820
1806 X% = DX% : Y% = DY% : K% = DK% : GOSUB 1700
1807 IF O% = 1 THEN 1820

1810 IF DK% = 3 THEN DK% = 7 : GOTO 1806
1811 IF DK% = 7 THEN DK% = 3 : GOTO 1806
1812 IF DK% = 1 THEN DK% = 5 : GOTO 1806
1813 DK% = 1 : GOTO 1806

1820 DX% = X% : DY% = Y% : DK% = K%
1821 GOTO 1400

1840 TX% = TX% + TXI% : IF TX% < 0 OR TX% > 240 THEN TXI% = -TXI% : GOTO 1840
1841 TY% = TY% + TYI% : IF TY% < 17 OR TY% > 175 THEN TYI% = -TYI% : GOTO 1841
1842 IF TF% <> 0 THEN TF% = COLLISION(1, 0)
1843 IF TF% <> 0 THEN RETURN
1844 PX% = TX% : PY% = TY%
1845 RETURN

1870 GOSUB 1900 : RR% = RR% MOD 2 : NI% = 6
1871 X% = KX% : Y% = KY% : P% = 2
1872 IF KK% = 3 OR KK% = 7 THEN IF RR% = 0 THEN K% = 1 ELSE K% = 5
1873 IF KK% = 1 OR KK% = 5 THEN IF RR% = 0 THEN K% = 3 ELSE K% = 7
1874 GOSUB 1700
1875 IF O% = 1 THEN 1890
1876 X% = KX% : Y% = KY% : K% = KK% : GOSUB 1700
1877 IF O% = 1 THEN 1890

1880 IF KK% = 3 THEN KK% = 7 : GOTO 1876
1881 IF KK% = 7 THEN KK% = 3 : GOTO 1876
1882 IF KK% = 1 THEN KK% = 5 : GOTO 1876
1883 KK% = 1 : GOTO 1876

1890 KX% = X% : KY% = Y% : KK% = K% : GO% = COLLISION(3, 0)
1891 RETURN

'==== GET NEXT RANDOM NUMBER
1900 RR% = RND(1) * 100 : RETURN

1950 IF DB% > 0 THEN RETURN
1951 GOTO 2600

1980 IF DB% = 0 THEN RETURN
1981 GOTO 2600

'==== CHECK MAZE STATUS
2000 FOR I% = 0 TO 2 : BC%(I%) = 0 : NEXT
2010 FOR X% = 0 TO 31 STEP 2
2011   FOR Y% = 0 TO 22 STEP 2
2020     A% = TILE(X%, Y%) : I% = 3
2021     IF A% >= 1 AND A% <= 4  THEN I% = 0
2022     IF A% >= 5 AND A% <= 8  THEN I% = 1
2023     IF A% >= 9 AND A% <= 12 THEN I% = 2
2024     IF I% = 3 THEN 2030
2025     A% = BC%(I%)
2026     BX%(I%,A%) = X% : BY%(I%,A%) = Y% : BT%(I%,A%) = 0
2027     BC%(I%) = A% + 1
2030   NEXT
2031 NEXT
2040 FOR I% = 0 TO 2
2041   FOR A% = 0 TO BC%(I%) - 1
2042     GOSUB 1900 : RR% = RR% MOD 3 : IF RR% = I% THEN 2042
2043     K% = A% MOD BC%(RR%)
2044     IF BT%(I%,A%) > 0 OR BT%(RR%,K%) > 0 THEN 2060
2045     X% = BX%(I%,A%) : Y% = BY%(I%,A%) : BT%(I%,A%) = 1
2046     B% = RR% : GOSUB 2100
2047     X% = BX%(RR%,K%) : Y% = BY%(RR%,K%) : BT%(RR%,K%) = 1
2048     B% = I% : GOSUB 2100
2060   NEXT
2061 NEXT
2062 RETURN

2100 IF B% = 0 THEN W% = 1
2101 IF B% = 1 THEN W% = 5
2102 IF B% = 2 THEN W% = 9
2103 PUT TILE W%, (X%, Y%)
2104 PUT TILE W%+1, (X%+1, Y%)
2105 PUT TILE W%+2, (X%, Y%+1)
2106 PUT TILE W%+3, (X%+1, Y%+1)
2107 RETURN

2200 X% = PX% : Y% = PY% : S% = PS% : NI% = 2 : P% = 0 : GOSUB 1700
2201 IF O% = 1 THEN 2250
2202 IF K% = 3 OR K% = 7 THEN 2220

2210 FOR I% = 1 TO 8
2211    X% = PX% + I% : GOSUB 1700 : IF O% = 1 THEN 2250
2212    X% = PX% - I% : GOSUB 1700 : IF O% = 1 THEN 2250
2213 NEXT
2214 RETURN

2220 FOR I% = 1 TO 8
2221   Y% = PY% + I% : GOSUB 1700 : IF O% = 1 THEN 2250
2222   Y% = PY% - I% : GOSUB 1700 : IF O% = 1 THEN 2250
2223 NEXT
2224 RETURN

2250 PX% = X% : PY% = Y% : PS% = S%
2251 RETURN

'==== GET OBJECT
2300 IF ABS(TIME - BP%) < L2% THEN RETURN
2301 BP% = TIME
2302 AX% = PX%+8 : AY% = PY%+8 : GOSUB 2400
2303 IF TF% = 0 AND A% = 0 THEN TF% = -1 : GOTO 1620
2304 IF PB% > 0 THEN 2350
2305 IF BS% = 0 THEN 2360
2306 PB% = BS% : SM% = 1
2307 GOTO 2500

'==== RELEASE OBJECT
2350 IF A% <> 0 THEN 2360
2351 BS% = PB% : GOSUB 2550
2352 IF W% > 0 THEN PB% = 0 : SM% = 3 : GOSUB 1420 : GOTO 2700
2353 RETURN

'==== WRONG OBJECT
2360 SM% = 5 : RETURN

2400 BS% = 0 : GOSUB 1750
2401 IF A% = 0 OR A% > 12 THEN RETURN
2402 IF A% <= 4 THEN BS% = 2 : RETURN
2403 IF A% <= 8 THEN BS% = 4 : RETURN
2404 BS% = 8
2405 RETURN

2500 A% = (A% - 1) MOD 4
2501 IF A% = 1 OR A% = 3 THEN XT% = XT% - 1
2502 IF A% = 2 OR A% = 3 THEN YT% = YT% - 1
2503 PUT TILE 0, (XT%, YT%)
2504 PUT TILE 0, (XT%+1, YT%)
2505 PUT TILE 0, (XT%, YT%+1)
2506 PUT TILE 0, (XT%+1, YT%+1)
2507 RETURN

2550 W% = 0 : X% = (AX% - 4) / 8 : Y% = (AY% - 4) / 8
2551 IF TILE(X%+1,Y%+1) <> 0 OR TILE(X%+1,Y%) <> 0 OR TILE(X%,Y%+1) <> 0 OR TILE(X%,Y%) <> 0 THEN 2360
2552 IF BS% = 2 THEN W% = 1 : GOTO 2103
2553 IF BS% = 4 THEN W% = 5 : GOTO 2103
2554 W% = 9
2555 GOTO 2103

2600 AX% = DX%+8 : AY% = DY%+8 : GOSUB 2400
2601 IF DB% > 0 THEN 2650
2602 IF BS% = 0 THEN RETURN
2603 DB% = BS%
2604 GOTO 2500

2650 IF A% <> 0 THEN RETURN
2651 BS% = DB% : GOSUB 2550
2652 IF W% > 0 THEN DB% = 0 : GOTO 1430
2653 RETURN

2700 WI% = 1
2701 FOR I% = 0 TO 2
2702   FOR A% = 0 TO BC%(I%) - 1
2703     X% = BX%(I%,A%) : Y% = BY%(I%,A%)
2704     W% = TILE(X%,Y%)
2705     IF I% = 0 THEN IF W% < 1 OR W% > 4 THEN WI% = 0 : RETURN
2706     IF I% = 1 THEN IF W% < 5 OR W% > 8 THEN WI% = 0 : RETURN
2707     IF I% = 2 THEN IF W% < 9 OR W% > 12 THEN WI% = 0 : RETURN
2720   NEXT
2721 NEXT
2722 RETURN

'==== WAIT X% SECONDS
2800 Y% = TIME : X% = X% * L1%
2801 IF TIME < Y% THEN Y% = TIME
2802 DI% = TIME - Y%
2803 IF DI% < X% THEN 2801
2804 RETURN

'==== GAME OVER
3000 LOCATE 03,01
3001 IF SC% = 0 THEN PRINT "[   G A M E   O V E R   ]"; ELSE PRINT "[  YOU GOT CAUGHT !!!!! ]";
3002 CMD PLYSONG 5 : CMD PLYPLAY
3003 GOSUB 1100                                                 ' CLEAR KEY BUFFER
3004 X% = 2 : GOSUB 2800                                        ' WAIT X% SECONDS
3010 GOSUB 1000 : IF B% = 0 THEN 3010
3011 CMD PLYMUTE : GOSUB 1100
3012 IF SC% = 0 THEN 10 ELSE 60

'==== VICTORY AND NEXT LEVEL
3100 IF SS% = 3 THEN RESTORE 1
3101 READ MS$ : LOCATE 05,01 : PRINT MS$;
3102 CMD PLYSONG 6 : CMD PLYPLAY
3103 GOSUB 1100                                                 ' CLEAR KEY BUFFER
3104 X% = 2 : GOSUB 2800                                        ' WAIT X% SECONDS
3110 GOSUB 1000 : IF B% = 0 THEN 3110
3111 CMD PLYMUTE : GOSUB 1100
3120 SS% = AC% MOD 7 + 3 : AC% = AC% + 1
3121 SC% = SC% + 300
3122 GOTO 60

'==== PAUSE/RETURN
3200 LOCATE 0,0 : PRINT ">>>>>>>>>[ P A U S E ]<<<<<<<<<<";
3201 CMD PLYSONG 0 : CMD PLYPLAY : CMD PLYSOUND 6               ' PAUSE SOUND
3202 GOSUB 1100                                                 ' CLEAR KEYBOARD BUFFER
3203 X% = 1 : GOSUB 2800                                        ' WAIT X% SECONDS
3210 IF STRIG(0) OR STRIG(1) THEN 3220
3211 IF INKEY = 27 OR STRIG(3) THEN GOSUB 1100 : RETURN 10
3212 GOTO 3210
3220 GOSUB 1100                                                 ' CLEAR KEYBOARD BUFFER
3221 CMD PLYSOUND 6                                             ' PAUSE SOUND
3222 LOCATE 0,0 : PRINT "                                ";
3223 GOTO 1200                                                  ' SHOW SCORE

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-pickinx-readme", "", "MSXBAS2ROM: Jogo: PickinX", "", "README.md: Pickin'X is a MSX clone of the arcade game of similar name developed by...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: PickinX: README.md

PICKINX
Pickin'X is a MSX clone of the arcade game of similar name developed by Valadon Automation in 1983.

With gameplay loosely based on the original game, with notable changes in sound, graphics, maps and game rules, in this version you are the Picker and you need to get all of the like-colored blocks into their matching places while avoiding hitting the enemys (Transposer and Killer).

If you hit the Transposer, you will be captured by him until you press the joystick button (or space bar).

Hit the Killer and you will get game over immediately.

During the game the Disturber will randomly capture some blocks and move them from their original places.

Hitting the Disturber will not harm you, but it cant be stopped so pay very close attention in its moves.

You have a limited time to complete each stage (named "Act"). Complete it in time and you will get more time for the next round.

Written in MSX Basic from scratch and compiled using MSXBAS2ROM compiler, this is a MSX1 compatible single player game (standard ROM BASIC required). Joysticks are recommended, but not required.

Good Luck and have fun, Pickers!

GAME INFO

- PickinX is a MSX clone of the arcade game of similar name developed by Valadon Automation in 1983;
- Remake to MSX1 created by Amaury Carvalho (2021);
- Published on MSXDEV21;
- ExecROM is not compatible with this 48kb ROM type, so use SofaRUN or ODO to load this game from MSX-DOS.

HISTORY

- 1.2 Current release;
- 1.1 Bug fix and sound/graphics improvements;
- 1.0 MSXDEV21 release.

SOURCE CODE

Compiling

       msxbas2rom pickinx.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-qbertdemo-qbertdem", "", "MSXBAS2ROM: Jogo: QbertDemo", "", "qbertdem.bas: ' Game Info:",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: QbertDemo: qbertdem.bas

'-----------------------------------------------------------------
' QBERT - MSXBAS2ROM DEMO
'-----------------------------------------------------------------
' Game Info:
'   Q*bert (also known as Qbert) is an arcade video game released by Gottlieb in 1982
'   Ported to MSX2 by Jelle Roggen (2024):
'     https://www.msxdev.org/2025/01/11/msxdev24-21-qbert/
'   MSXBAS2ROM rough draft adaptation by Amaury Carvalho from Roggen original code (2025):
'     https://github.com/amaurycarvalho/msxbasic/QbertDemo
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom -x qbertdem.bas
'-----------------------------------------------------------------

' Resources (when compiling on Windows, change slashs to backslashs: / to \ )
FILE "music/qbert.akm"                          ' 0 - songs exported from ArkosTracker 3 (Music.aks)
FILE "music/effects.akx"                        ' 1 - sound effects exported from ArkosTracker 3 (SoundEffects.aks)
FILE "levels/Q-BERT1.STA"                       ' 2 - stages
FILE "img/INTRO.SC5"                            ' 3 - intro screen
FILE "img/TILES.SC5"                            ' 4 - tiles

' Initialization
1 DEFINT A-Z
2 DIM A(63,2), J(10), Q(7,3), M(6,12), S(11,8), P(12,3), B(3), D(1,7)
3 IF MSX() = 0 THEN PRINT "Sorry, not for MSX1 machines" : END
4 CMD PLYLOAD 0, 1                              ' load music and effects from resources 0/1

' Intro screen
10 SCREEN 5,,0
11 SET PAGE 0,1
12 SCREEN LOAD 3                                ' load screen from resource 3 (INTRO.SC5)
13 FOR AA=0 TO 15 : COLOR=(AA,0,0,0) : NEXT AA
14 SET PAGE 1,1
15 PP=0 : GOSUB 40                              ' screen fade in
16 CMD PLYSONG 0 : CMD PLYPLAY                  ' play intro song
17 IF MSX() < 3 THEN GOSUB 32 ELSE GOSUB 60     ' wave effect or Turbo R special effect

' Game start
20 CMD PLYMUTE                                  ' stops song
21 PP=1 : GOSUB 40                              ' screen fade out
22 GOSUB 100                                    ' level initialization
23 GOSUB 200                                    ' stage screen
27 GOSUB 30 : IF K=0 THEN 27
28 CMD PLYSOUND 1                               ' play sound effect 1
29 SCREEN 0 : PRINT "GAME FINISHED" : END

' Get trigger status
30 K = STRIG(0) OR STRIG(1) OR STRIG(2)
31 RETURN

' Screen wave effect
32 COPY(0,0)-(255,212),1 TO (1,1),0
33 CMD PAGE 1,1                                 ' enable wave effect (cadari bit)
34 GOSUB 30 : IF K=0 THEN 34
35 CMD PAGE 0,0                                 ' disable wave effect
36 RETURN

' Screen fade in/out (PP=0 or PP=1)
39 DATA 0,0,4,0,0,0,0,0,5,0,0,6,1,1,7,2,1,7,7,0,0,2,2,7,7,2,2,7,3,3,7,4,2,6,6,3,5,3,3,5,3,5,5,5,5,7,7,7
40 IF PP=0 THEN R=0 : R1=7 : R2=1 ELSE R=7 : R1=0 : R2=-1
41 RESTORE 39
42 FOR AA=0 TO 15
43   FOR BB=0 TO 2
44     READ A(AA,BB)
45   NEXT BB
46 NEXT AA
50 FOR BB=R TO R1 STEP R2
51   E=0
52   FOR AA=0 TO 15
53     RED=INT(((A(AA,0)/7.0)*BB))*16 : GREEN=(A(AA,1)/7.0)*BB : BLUE=(A(AA,2)/7.0)*BB
54     VPOKE &H7680+E,RED+BLUE : VPOKE &H7681+E,GREEN : E=E+2
55   NEXT AA
56   COLOR=RESTORE
57 NEXT BB
58 RETURN

' Turbo R special effect
60 CMD TURBO 1                                                      ' activate Turbo R mode
61 FOR AA=0 TO 2 STEP 2
62   COPY(0,0)-(255,212),1 TO (0,0),AA
63 NEXT AA
64 P1=0 : P2=2 : SET PAGE P1,P2
70 F=F+1 : DD=50.0*SIN(F/10.0)
71 GOSUB 80 : IF K<>0 AND DD > -4 AND DD < 4 THEN RETURN
72 GOTO 70
80 C=0 : GOSUB 90
81 FOR AA=62 TO 0 STEP -1
82   GOSUB 30 : IF K<>0 THEN RETURN
83   A(AA,P1/2)=DD*SIN(C/10.0)+59
84   COPY (59,AA)-(205,AA),1 TO (A(AA,P1/2),AA)
85   C=C+1
86 NEXT AA
87 SET PAGE P1,P2 : SWAP P1,P2
88 RETURN
90 FOR AA=0 TO 62
91   LINE(A(AA,P1/2),AA)-STEP(205-61+10,0),0
92 NEXT AA
93 RETURN

' level initialization
100 PP=50*RND(-TIME)
101 SCREEN 5 : COLOR=(3,0,3,6) : COLOR=(7,6,1,1) : COLOR=(14,0,3,6)
102 FOR AA=0 TO 3 : COPY(0,0)-STEP(255,44),0 TO (0,213),AA : NEXT AA  ' clear screen pages
103 SET PAGE 0, 1 : SCREEN LOAD 4 : SET PAGE 0, 0                     ' load sprites map on page 1
104 RESTORE 104 : FOR C=0 TO 10 : READ BB : J(C)=BB : NEXT C : DATA 0,3,5,8,9,9,9,8,5,3,0
105 FOR AA=0 TO 7 : FOR BB=0 TO 3: READ C : Q(AA,BB)=C : NEXT BB,AA : DATA0,76,12,18,13,76,12,20,26,76,14,18,41,76,14,20,56,76,14,20,71,76,14,20,86,76,12,20,99,76,12,20
106 RETURN

' build stage screen
200 FOR AA=0 TO PP : BB=RND(1) : NEXT AA                                ' randomize
201 P1=0 : P2=2 : L=3
202 CMD RESTORE 2                                                       ' load binary DATA on resource 2 (stage data)
203 IRESTORE 7                                                          ' set binary DATA initial read position to byte 7
210 H=4 : T=0 : T1=0
211 FOR Y=0 TO 8                                                        ' read stage data
212   IREAD BB                                                          ' read next integer data
213   W = ((BB SHR 8) AND &h0F) OR ((BB SHL 4) AND &h0FF0)              ' reverse it to a modified big endian
214   FOR X=11 TO 0 STEP -1
215     S(X,Y)= W AND 1
216     W = W SHR 1
217   NEXT
218 NEXT
220 SET PAGE ,3 : CLS
221 FOR Y=0 TO 8
222   FOR X=0 TO 11
223     IF S(X,Y)=1 THEN COPY(0,0)-(20,34),1 TO (X*20+(Y AND 1)*10,2+Y*24),,TPSET : T=T+1
224   NEXT
225 NEXT
230 FOR BB=0 TO 2 STEP 2
231   SET PAGE ,BB : CLS
232   COPY(0,0)-(255,212),3 TO (0,0),BB
233 NEXT
234 SET PAGE P1,P2
'235 GOSUB 300 : GOTO 305
290 CMD PLYSONG 1 : CMD PLYPLAY                                         ' play level song
291 RETURN

'50 W=&H8000
'51 DEFUSR=&HD000:A=USR(0):DEFUSR=&HD200:DEFUSR1=&HFA88
'52 IFPEEK(&HCF80)=0ANDPEEK(&HCF81)=1ANDPEEK(&HCF82)=2THENPRINTELSEBEEP:BEEP:SCREEN0:PRINT"Program is too big, risk of crashing !!!!!!!!!!!!!!!!":END
'60 FORA=0TOP:B=RND(1):NEXTA:P1=0:P2=2:L=3
'70 H=4:T=0:T1=0:POKE&HFAA0,5:FORY=0TO8:W5=USR1(W):A$=RIGHT$("00000000"+BIN$(W5),8):W5=USR1(W+1):B$=RIGHT$("0000"+BIN$(W5),4):W=W+2:A$=A$+B$:FORX=0TO11:S(X,Y)=VAL(MID$(A$,X+1,1)):NEXTX,Y
'80 SETPAGE,3:CLS:FORY=0TO8:FORX=0TO11:IFS(X,Y)=1THENCOPY(0,0)-(20,34),1TO(X*20+(Y-(INT(Y/2)*2))*10,2+Y*24),,TPSET:T=T+1:NEXTX,Y:ELSENEXTX,Y
'90 FORB=0TO2STEP2:SETPAGE,B:CLS:COPY(0,0)-(255,212),3TO(0,0),B:NEXTB:SETPAGEP1,P2
'-----> GOSUB100:GOTO105
'100 A=0:D=0:X=USR1(W):X2=X:Y=USR1(W+1):Y2=Y:GOSUB135:W=W+2
'101 FORS=0TO5:M(S,7)=0:GOSUB104:M(S,2)=300*RND(1)+100:M(S,3)=0:M(S,8)=1:NEXTS:RETURN
'104 M(S,1)=48:M(S,0)=INT(12*RND(1))+2:IFS(M(S,0),2)=0THENGOTO104ELSEM(S,0)=(M(S,0)*20)+5:RETURN:
'105 FORB=0TO1:D(B,0)=USR1(W):D(B,5)=D(B,0):D(B,1)=USR1(W+1):D(B,6)=D(B,1):D(B,2)=USR1(W+2):D(B,3)=USR1(W+3):D(B,4)=0:D(B,7)=20*RND(1)+20:W=W+4:NEXTB:W=W+2
'110 IFZ>0THENGOSUB145:GOSUB150:GOSUB355:FORB=0TO1:GOSUB410:NEXTB:GOSUB470:FORP=0TO500*(PEEK(&H2D)-1):NEXTP:SWAPP1,P2:GOSUB134:SETPAGEP1,P2:GOTO110
'115 IFD<10THEND=STICK(0)+PEEK(&HCFFF-4):ONDGOSUB140,170,140,170,140,170,140,170
'120 GOSUB145:GOSUB150:FORS=0TO5:GOSUB230:NEXTS:FORB=0TO1:GOSUB410:NEXTB:GOSUB180:FORS=0TO5:GOSUB231:GOSUB360:NEXTS:GOSUB470:SWAPP1,P2:GOSUB135:SETPAGEP1,P2
'130 GOTO110
'134 IF(Z>0ANDZ<100)THENRETURN
'135 IFTIME<4THEN135ELSETIME=0:T8=130+Y+J(A):IFT8<203THENT8=203
'136 T7=X+127:IFT7<200THENT7=200ELSEIFT7>280THENT7=280
'137 VDP(24)=T8:VDP(27)=T7\8:VDP(28)=7-(T7AND7):RETURN
'140 RETURN
'145 IFB(2)=0THENRETURNELSEB1=P2:FORB=0TO1:COPY(21,0)-(41,34),1TO(B(0),B(1)),B1,TPSET:B1=3:NEXTB:B(2)=B(2)-1:RETURN
'150 COPY(P(0,P1),P(0,P1+1))-STEP(14,20),3TO(P(0,P1),P(0,P1+1)):FORS=1TO6:IFP(S,P1)=0THENNEXTSELSECOPY(P(S,P1),P(S,P1+1))-STEP(13,11),3TO(P(S,P1),P(S,P1+1)):P(S,P1)=0:NEXTS
'154 FORB=0TO1:IFP(9+B,P1)=0THENNEXTBELSECOPY(P(9+B,P1),P(9+B,P1+1))-STEP(13,14),3TO(P(9+B,P1),P(9+B,P1+1)):P(9+B,P1)=0:NEXTB
'155 COPY(P(11,P1),P(11,P1+1))-STEP(32,26),3TO(P(11,P1),P(11,P1+1)):IFP(12,P1)=0THENRETURNELSECOPY(P(12,P1),P(12,P1+1))-STEP(44,21),3TO(P(12,P1),P(12,P1+1)):P(12,P1)=0:RETURN
'160 ' Q-BERT MOVEMENT
'170 XX=USR(6):X1=(D>5)-(D<5):Y1=(D=2ORD=8)-(D=4ORD=6):H=D:X2=X:Y2=Y:D=10:RETURN
'180 IFD=10THENA=A+1:X=X+X1:Y=-J(A)+Y2+Y1*(24/10)*A:GOSUB190:IFA=10THENGOSUB340:D=-(D=11)*11:A=0:X2=X:Y2=Y:RETURNELSERETURNELSEGOSUB190:RETURN
'190 IFY>190THENRETURNELSEK=(H-2)-(D=10):COPY(Q(K,0),Q(K,1))-STEP(Q(K,2),Q(K,3)),1TO(X-2,(Y-3)*-(Y-3>0)),,TPSET:P(0,P1)=X-2:P(0,P1+1)=(Y-3)*-(Y-3>0):RETURN
'200 COPY(0,35)-STEP(12,15),1TO(X,Y),,TPSET:P(0,P1)=X:P(0,P1+1)=Y:RETURN
'210 COPY((-(Q=5)*14)-((M(S,7)=5)*28),51)-STEP(13,11),1TO(M(S,0),M(S,1)+2),,TPSET:P(S+1,P1)=M(S,0):P(S+1,P1+1)=M(S,1)+2:Q=0:RETURN
'220 ' MONSTERS MOVEMENT
'230 IFD(0,4)>0ORD(1,4)>0ORY>M(S,1)ORM(S,12)=6THENM(S,12)=5:GOTO232ELSEM(S,12)=0:RETURN
'231 IF M(S,12)=5ORM(S,12)=6THENRETURN
'232 ONM(S,3)+1GOTO235,240,250,260,280,265
'235 IFM(S,2)=0THENGOTO236ELSEM(S,2)=M(S,2)-1:IFM(S,2)<30THENM(S,12)=6:COPY(16,98)-STEP(7,5),1TO(M(S,0)+2,M(S,1)+10),,TPSET:P(S+1,P1)=M(S,0)+2:P(S+1,P1+1)=M(S,1)+10:M(S,11)=M(S,1):IFM(S,2)=0THENXX=USR(2):RETURNELSERETURNELSERETURN
'236 M(S,1)=M(S,1)-M(S,11):GOSUB210:M(S,1)=M(S,1)+M(S,11):M(S,11)=M(S,11)-2:IFM(S,11)<0THENM(S,3)=2:RETURNELSERETURN
'240 ' m(s,2)=1  lege sprong regel
'250 B=INT(2*RND(1)):M(S,4)=(B=0)-(B>0):M(S,5)=1:M(S,6)=M(S,1):M(S,3)=3:M(S,2)=0:IFS(((M(S,0)+M(S,4)*10)-5)/20,((M(S,1)+24))/24)=0THENM(S,4)=M(S,4)*-1:IFS(((M(S,0)+M(S,4)*10)-5)/20,(M(S,1)+24)/24)=0THENIFS=0THENM(S,7)=5:GOTO280:'ELSERETURNELSERETURN
'260 M(S,2)=M(S,2)+1:M(S,0)=M(S,0)+M(S,4):M(S,1)=-J(M(S,2))+M(S,6)+M(S,5)*(24/10)*M(S,2):GOSUB210:IFM(S,2)=10THENIFM(S,7)=5THENGOSUB261:M(S,3)=5:M(S,2)=2:RETURNELSE270ELSERETURN
'261 IFP5=-1THENCOLOR=(0,7,7,7):W=W-10:GOSUB101:W=W+10:XX=USR(1):COLOR=(0,0,0,0):RETURNELSERETURN
'265 M(S,2)=M(S,2)-1:Q=5:GOSUB210:IFM(S,2)>0THENRETURNELSEIFM(S,7)=5THENM(S,3)=4:RETURNELSEM(S,3)=2:RETURN:'wacht pauze
'270 IFS((M(S,0)-5)/20,(M(S,1))/24)>0THENM(S,3)=5:RETURNELSEGOSUB104:M(S,2)=500*RND(1):M(S,3)=0:RETURN:'m volgt q-bert>>>
'280 M(S,2)=0:M(S,3)=3:M(S,6)=M(S,1):IFX2>M(S,0)THENM(S,4)=1ELSEM(S,4)=-1
'290 IF Y2>M(S,1)THENM(S,5)=1ELSEM(S,5)=-1
'291 X3=M(S,0)+(10*M(S,4)):Y3=M(S,1)+(24*M(S,5)):IF(D(0,4)>1ORD(1,4)>1)ANDX2-9X3ANDY2-9Y3THENP5=-1ELSEP5=0
'292 IFM(S,8)=1THENGOSUB311ELSEGOSUB320:IFP=0THENM(S,5)=M(S,5)*-1:GOSUB320:IFP=0THENM(S,5)=M(S,5)*-1:M(S,4)=M(S,4)*-1:GOSUB320:IFP=0THENM(S,5)=M(S,5)*-1
'293 IFM(S,4)=M(S,9)*-1ANDM(S,5)=M(S,10)*-1THENM(S,8)=M(S,8)*-1ELSEM(S,9)=M(S,4):M(S,10)=M(S,5)
'310 RETURN
'311 GOSUB320:IFP=0THENM(S,4)=M(S,4)*-1:GOSUB320:IFP=0THENM(S,4)=M(S,4)*-1:M(S,5)=M(S,5)*-1:GOSUB320:IFP=0THENM(S,4)=M(S,4)*-1
'312 RETURN
'320 IFP5=-1THENP=5:RETURNELSEIFM(S,1)+(M(S,5)*24)<0THENP=0:RETURNELSEP=S(((M(S,0)+M(S,4)*10)-5)/20,((M(S,1)+M(S,5)*24))/24):RETURN
'330 ' Check routines
'340 S1=(X-5)/20:S2=(Y)/24:IFS(S1,S2)=1THENB(0)=X-5:B(1)=Y+2:B(2)=2:S(S1,S2)=2:T1=T1+1:IFT=T1THENZ=1:Y1=Y:RETURN:ELSERETURN
'350 IF S(S1,S2)=0THENFORB=0TO1:IFD(B,0)-10XANDD(B,1)-10YANDD(B,4)<>-1THEND(B,4)=D(B,7):D=11:XX=USR(10):RETURNELSENEXTB:L=L-1:Z=150:XX=USR(8):RETURNELSERETURN
'355 IFZ>99THEN356ELSED=10:Z=Z+1:Y=(-J(ZMOD10)*2)+Y1:D=10*-(Z<50):GOSUB190:IFZ=51THENZ=0:RETURN70ELSERETURN:'als alles gesprongen is.
'356 IFZ=150THENGOTO358ELSEGOSUB190:GOSUB357:Z=Z+1:IFZ=105THENXX=USR(255):RETURNELSEIFZ=125THENW=W-10:GOSUB101:W=W+10:Z=0:X=X2:Y=Y2:D=0:A=0:RETURNELSERETURN
'357 COPY(0,151)-STEP(44,21),1TO(X-20,Y-22),,TPSET:P(12,P1)=X-20:P(12,P1+1)=Y-22:RETURN
'358 Y=Y+5:X=X+X1:D=10:GOSUB190:COPY(X-2,Y-3)-STEP(30,30),3TO(X-2,Y-3),,TPSET:IFY>190THENZ=0:W=W-12:GOSUB100:W=W+10:RETURNELSERETURN
'360 ' Check Hit q-bert with monster
'370 IFZ=0ANDD(0,4)<1ANDD(1,4)<1ANDM(S,3)>0ANDX-5M(S,0)ANDY-5M(S,1)THENL=L-1:Z=100:RETURNELSERETURN
'399 '
'400 '                               Disk
'401 '
'410 IFD(B,4)=-1THENRETURNELSEIFD(B,4)=0THENGOSUB430:RETURNELSED=11:D(B,5)=(((D(B,7)+1-D(B,4))/D(B,7))*(D(B,2)-D(B,0)))+D(B,0):D(B,6)=(((D(B,7)+1-D(B,4))/D(B,7))*(D(B,3)-D(B,1)))+D(B,1):D(B,4)=D(B,4)-1
'420 GOSUB430:X=D(B,5):Y=D(B,6):GOSUB190:IFD(B,4)=0THENX2=X:Y2=Y:D=0:D(B,4)=-1:GOTO340ELSERETURN
'430 POKE&HFAA0,3:Y9=(USR1(&HB900+Y8)-5)*-(D(B,4)=0)*((B=0)+B):POKE&HFAA0,5:Y8=(Y8+1)MOD32:COPY(0,95)-STEP(13,13),1TO(D(B,5)-2,D(B,6)+7+Y9),,TPSET:P(9+B,P1)=D(B,5)-2:P(9+B,P1+1)=D(B,6)+7+Y9:RETURN
'440 '
'450 '                     Amount of life remaining
'460 '
'470 L=L*-(L>0):COPY(0,120)-STEP(22,16),1TO(20+T7-100,T8-127),,TPSET:COPY(L*8,109)-STEP(7,10),1TO(45+T7-100,T8-127+5),,TPSET:P(11,P1)=20+T7-100:P(11,P1+1)=T8-127:RETURN

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-qbertdemo-readme", "", "MSXBAS2ROM: Jogo: QbertDemo", "", "README.md: Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: QbertDemo: README.md

Q*BERT
Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- Q*bert (also known as Qbert) is an arcade video game released by Gottlieb in 1982;
- Ported to MSX2 by Jelle Roggen (2024) for MSXDev24 contest;
- MSXBAS2ROM rough draft adaptation by Amaury Carvalho from Roggen original code (2025).

SOURCE CODE

Compiling

       msxbas2rom -x qbertdem.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-readme", "", "MSXBAS2ROM: Jogos MSX BASIC (msxbasic)", "", "README.md: MSX BASIC demonstration games compiled with MSXBAS2ROM",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogos MSX BASIC (msxbasic): README.md

MSXBASIC
MSX BASIC demonstration games compiled with MSXBAS2ROM

NOTES

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;

- All .BAS files are in plain text format, so they can be opened by any text editor;

- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;

- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;

- .AKS files are Arkos Tracker 3 projects;

- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;

- .AKX files are sound effects exported by Arkos Tracker 3;

- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-spider-readme", "", "MSXBAS2ROM: Jogo: Spider", "", "README.md: Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM...",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Spider: README.md

SHOOT THE SPIDER
Game demo source code written in MSX BASIC to be compiled with MSXBAS2ROM compiler

GAME INFO

- Shoot the spider to earn points!
- Created by Amaury Carvalho (2022-2025);

SOURCE CODE

Compiling

       msxbas2rom spider.bas

Running online

- Game emulation via WebMSX

Repository

- Source code on Github

DEVELOPMENT TOOLS

- MSXBAS2ROM
- Arkos Tracker 3
- MSX Screen Converter
- MSX Tiny Sprite
- nMSXTiles for Linux
- nMSXTiles for Windows

Footnotes

- MSXBAS2ROM is a command line tool, so you need to use a terminal shell on your PC to compile these projects;
- All .BAS files are in plain text format, so they can be opened by any text editor;
- .SCn are MSX BASIC "BSAVE,S" native binary format screen files exported from MSX Screen Converter;
- .SPR files are in plain text format and can be opened by any text editor and edited by Tiny Sprite as well;
- .AKS files are Arkos Tracker 3 projects;
- .AKM files are songs exported by Arkos Tracker 3 in minimalist format;
- .AKX files are sound effects exported by Arkos Tracker 3;
- You can find a MSXBAS2ROM reference guide here.

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-spider-spider", "", "MSXBAS2ROM: Jogo: Spider", "", "spider.bas: ' SPIDER - MSXBAS2ROM DEMO (v.1.0)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Spider: spider.bas

'-----------------------------------------------------------------
' SPIDER - MSXBAS2ROM DEMO (v.1.0)
'-----------------------------------------------------------------
' Game Info:
'   Shoot the spider !
'   By Amaury Carvalho (2022-2025):
'     https://github.com/amaurycarvalho/msxbasic/Spider
'-----------------------------------------------------------------
' Stack:
'   https://github.com/amaurycarvalho/msxbas2rom/
'   https://julien-nevo.com/at3test/index.php/download/
'   https://msx.jannone.org/conv/
'   https://msx.jannone.org/tinysprite/tinysprite.html
'   https://launchpad.net/nmsxtiles
'   https://github.com/pipagerardo/nMSXtiles
'-----------------------------------------------------------------
' Compile:
'   msxbas2rom spider.bas
'-----------------------------------------------------------------

' Resources
FILE "music/spider.akm"         ' 0 - Music (exported from Arkos Tracker 3)
FILE "music/effects.akx"        ' 1 - Sound Effects (exported from Arkos Tracker 3)
FILE "img/spider.SC2"           ' 2 - Game screen (exported from nMSXTiles, same as BSAVE "filename",S)
FILE "img/spider.spr"           ' 3 - Sprites (exported from TinySprite)

' Game initialization
05 DEFINT A-Z                   ' All variables are integers
10 SCREEN 2, 2, 0               ' Screen mode 2
11 COLOR 15, 0, 0               ' Set screen front, back and border colors

' Screen initialization
20 SCREEN OFF                   ' Disable screen
21 SET TILE ON                  ' Activate tiled text mode (print text in the same way SCREEN modes 0 and 1 do)
22 SCREEN LOAD 2                ' Load game screen resource (similar to BLOAD "file.SC2",S, created with NMSXTILES)
23 SPRITE LOAD 3                ' Load game sprites (TXT file created with TinySprite)
24 SET FONT 0                   ' Load system default font

30 CMD PLYLOAD 0, 1             ' load music and effects from resources 0 and 1 (Arkos Tracker exported files)
31 CMD PLYSONG 0                ' Load game song theme (Bach Invention 14)
32 CMD PLYPLAY                  ' Play theme

40 LOCATE 00, 00 : PRINT "SHOOT THE SPIDER!";
41 LOCATE 22, 00 : PRINT "HITS:";
42 SCREEN ON                                ' Enable screen

' Player start position and score
50 PX = 8
51 PY = 24
52 PP = 0

' Show Spider
60 SX = RND(1) * 100 MOD 192 + 32           ' Select a random X position to spider
61 SY = RND(1) * 100 MOD 128 + 32           ' Select a random Y position to spier
62 PUT SPRITE 1, (SX, SY), 8

' Show player / Game loop
70 PUT SPRITE 0, (PX, PY), 15, 0

' Get player input
80 PS = STICK(0) OR STICK(1)                ' Keyboard or Joystick movement
81 PB = STRIG(0) OR STRIG(1)                ' Keyboard or Joystick trigger

' Check for trigger
90 IF PB = 0 THEN 200                       ' Player not press the trigger? So, check for movement
91     PUT SPRITE 0,,,2                     ' else, change player's sprite pattern
92     IF COLLISION(0, 1) < 0 THEN 110      ' Sprite 0 not collided with sprite 1? So, do miss the spider logic
                                            ' else, do hit the spider logic

' Player hit the spider
100 CMD PLYSOUND 2                          ' Play sound effect from instrument 2
101 PP = PP + 1
102 LOCATE 28, 00 : PRINT PP;
103 PB = STRIG(0) OR STRIG(1)
104 IF PB <> 0 THEN 103
105 GOTO 60

' Player miss the spider
110 CMD PLYSOUND 3                          ' Play sound effect from instrument 3
111 PB = STRIG(0) OR STRIG(1)
112 IF PB <> 0 THEN 111
113 GOTO 70

' Player movement logic
200 IF PS = 0 THEN 80
201   DX = 0 : DY = 0

210   IF PS = 1 THEN DY = -1 : GOTO 220
211   IF PS = 2 THEN DX = 1 : DY = -1 : GOTO 220
212   IF PS = 3 THEN DX = 1 : GOTO 220
213   IF PS = 4 THEN DX = 1 : DY = 1 : GOTO 220
214   IF PS = 5 THEN DY = 1 : GOTO 220
215   IF PS = 6 THEN DX = -1 : DY = 1 : GOTO 220
216   IF PS = 7 THEN DX = -1 : GOTO 220
217   IF PS = 8 THEN DX = -1 : DY = -1

220 DX = DX + PX
221 DY = DY + PY
222 IF DX < 8 OR DX > 232 OR DY < 24 OR DY > 160 THEN 80

230 PX = DX
231 PY = DY
232 GOTO 70                                 ' Repeat game loop

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    add("msxbas2rom-games-superman-superman", "", "MSXBAS2ROM: Jogo: Superman", "", "superman.bas: ' https://en.wikipedia.org/wiki/Superman_(1979_video_game)",
        R"MAN(MSXBAS2ROM (Amaury Carvalho)  -  Jogo: Superman: superman.bas

' https://en.wikipedia.org/wiki/Superman_(1979_video_game)

10 PRINT "Superman game draft"
20 END

FONTE
  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom)MAN");

    return v;
}

} // namespace man
