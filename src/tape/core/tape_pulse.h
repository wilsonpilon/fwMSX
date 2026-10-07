// fwMSX -- cursor de pulsos de fita (modo normal, com som). Codigo
// ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/tape-spec.md.
//
// Um "pulso" e' um meio-periodo (convencao do TZX, ver TZX_format.md,
// secao 2): o nivel comeca em baixo (0) e alterna a cada pulso consumido.
// Avancar o cursor em T-states de Z80 e' o mesmo servico que o PSG/SCC/FM
// ja' tem (Advance()), so' que aqui o "som" e' so' o nivel atual (onda
// quadrada), sem envelope nem mistura de canais -- isso fica na camada
// C++ (src/tape/cpp/tape_device.h).
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TapePulseCursor {
    const uint32_t *durations; // nao copiado -- o chamador mantem o buffer vivo
    uint32_t count;
    uint32_t index;   // pulso atual (posicao na fita, p/ barra de progresso)
    long remaining;   // T-states restantes do pulso atual
    int level;        // nivel atual (0/1)
    int finished;     // 1 = chegou ao fim da lista (fica no ultimo nivel)
} TapePulseCursor;

// Liga o cursor a um nova lista de pulsos (NAO copia `durations`) e
// rebobina. `durations`/`count` podem ser NULL/0 (fita sem pulsos nenhum
// -- TAPION do modo rapido so', por exemplo).
void tape_cursor_set(TapePulseCursor *c, const uint32_t *durations, uint32_t count);

// Volta ao primeiro pulso da lista atual (mesmo `durations`/`count`).
void tape_cursor_rewind(TapePulseCursor *c);

// Avanca `cycles` T-states de Z80; devolve o nivel (0/1) depois de
// avancar. Ao chegar ao fim da lista, para' (fica no ultimo nivel,
// silencio) e marca `finished`.
int tape_cursor_advance(TapePulseCursor *c, long cycles);

// Pula DIRETO para o pulso `index` (sem gerar som nem consumir ciclos --
// usado por TapeEngine::SeekToFile() para marcar o ponto de carga). O
// nivel sai certo (a paridade de quantos pulsos foram "consumidos" desde
// o inicio, ja' que cada um alterna o nivel uma vez).
void tape_cursor_seek(TapePulseCursor *c, uint32_t index);

#ifdef __cplusplus
}
#endif
