// fwMSX -- Mamute: o enderecamento estendido do SUPER-X (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/mamute-spec.md, "Fase 4".
//
// Os comandos X* aceitam, depois do endereco, um sufixo que escolhe ONDE ler/gravar, ignorando o PAGE:
//   4000#3     slot primario 3 (mesmo que nao esteja comutado em pagina nenhuma agora)
//   4000#3-1   sub-slot 1 do slot 3 (slot expandido; ate' 1 MB de RAM num MSX de verdade)
//   0000#V     a VRAM (plana; enderecos de ate' 5 digitos)        (#4 e' sinonimo)
//   4000#S     o PAGE corrente, de proposito                       (#5 e' sinonimo)
// Uma "variavel de debugger" (@0-@3, @B, @E, @S) guarda um endereco COM o alvo e substitui o endereco inteiro: "XD @0".
#pragma once

#include <cstdint>
#include <string>

#include "mamute_memory.h"

namespace mamute {

struct Target {
    bool vram = false;
    bool is_explicit = false; // true = slot primario informado (#slot ou #slot-sub): nao passa pelo PAGE
    int slot = 0;
    int sub = 0;
};

struct SxAddr {
    uint32_t addr = 0;
    Target t;
};

// As 7 variaveis do SUPER-X. @0-@3 sao as normais; @B/@E/@S (Begin/End/Start-ou-Size) sao preenchidas pelos comandos de carga em disco.
class Vars {
public:
    struct Slot {
        bool has = false;
        SxAddr v;
    };
    // `name` e' o caractere depois do @ ('0'-'3', 'B', 'E', 'S'); nullptr se nao e' nenhuma das 7.
    Slot *Find(char name);
    const Slot *Find(char name) const { return const_cast<Vars *>(this)->Find(name); }
    // "Commands store base address in variable 0": XD/XA/XI/XM/XH gravam o endereco aberto em @0.
    void StoreBase(const SxAddr &a);
    Slot slots[7];
};

// "#3", "#3-1", "#V" ou "" (so' o PAGE corrente).
std::string TargetSuffix(const Target &t);
// 4 digitos, ou 5 na VRAM.
int TargetDigits(const Target &t);
// O maior endereco do alvo: FFFF, ou o tamanho da VRAM - 1.
uint32_t TargetMax(IMemory &mem, const Target &t);
// Dobra o endereco dentro do alvo (modulo 64 KB, ou modulo o tamanho da VRAM).
uint32_t TargetWrap(IMemory &mem, const Target &t, int64_t addr);

uint8_t TRead(IMemory &mem, const Target &t, uint32_t addr);
// false = nao gravou (ROM, celula vazia, fora da VRAM).
bool TWrite(IMemory &mem, const Target &t, uint32_t addr, uint8_t value);

// "ENDERECO[#sufixo]" completo, ou "@nome". false = sintaxe invalida.
bool ParseSx(IMemory &mem, const Vars &vars, const std::string &token, SxAddr &out);
// Um endereco PURO no alvo que o inicio ja escolheu (o <fim> de um bloco): 4 digitos, ou ate' 5 e dentro da VRAM.
bool ParsePure(IMemory &mem, const Target &t, const std::string &token, uint32_t &out);
// "4000#3-1" para o log.
std::string FormatSx(const SxAddr &a);
// "4000-40FF#3-1": o intervalo, com o alvo depois dele.
std::string FormatRange(const SxAddr &s, uint32_t end);

} // namespace mamute
