// fwMSX -- depurador basico do emulador (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver
// doc/debug-spec.md.
//
// O que existe: breakpoints de execucao (PC), watchpoints de leitura/escrita de memoria, um ponto de
// parada de uso unico ("until": base de `next`, `until` e `go <fim>`), parada manual e uma FILA DE EVENTOS
// ("! break pc=4010 ...") que a ponte de controle entrega aos clientes inscritos. Nao conhece a maquina:
// quem roda as instrucoes (Machine) pergunta CheckPc() antes de cada uma e entrega os acessos de memoria por
// OnAccess() (via DebugBus). Tudo roda na thread do emulador -- sem travas.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dbg {

struct Breakpoint {
    int id = 0;
    uint16_t addr = 0;
    uint32_t hits = 0;
};

struct Watchpoint {
    int id = 0;
    uint16_t addr = 0;
    uint16_t len = 1;
    bool on_read = false;
    bool on_write = true;
    uint32_t hits = 0;
};

class Debugger {
public:
    // ---- configuracao -------------------------------------------------------------------------
    // Devolve o id (o mesmo, se ja havia um ponto nesse endereco).
    int AddBreak(uint16_t addr);
    bool RemoveBreak(uint16_t addr);
    void ClearBreaks() { breaks_.clear(); }
    const std::vector<Breakpoint> &breaks() const { return breaks_; }

    // `len` 1..65535. Pelo menos um de `on_read`/`on_write`.
    int AddWatch(uint16_t addr, uint16_t len, bool on_read, bool on_write);
    bool RemoveWatch(uint16_t addr);
    void ClearWatches() { watches_.clear(); }
    const std::vector<Watchpoint> &watches() const { return watches_; }

    // Ponto de parada de uso unico (some quando atingido ou quando a maquina para por outro motivo).
    void SetUntil(uint16_t addr) {
        until_active_ = true;
        until_addr_ = addr;
    }
    bool until_active() const { return until_active_; }

    // ---- estado de execucao -------------------------------------------------------------------
    bool stopped() const { return stopped_; }
    // Para a maquina agora (sem evento: quem pediu ja tem a resposta do comando). `why` fica em last_stop().
    void Stop(const std::string &why) {
        stopped_ = true;
        until_active_ = false;
        last_stop_ = why;
    }
    // Volta a rodar. A primeira instrucao e executada sem testar os breakpoints (senao parar de novo no
    // mesmo PC seria inevitavel).
    void Resume() {
        stopped_ = false;
        skip_once_ = true;
    }
    const std::string &last_stop() const { return last_stop_; }

    // Alguma coisa pode parar a execucao? (teste barato, feito antes de CADA instrucao)
    bool armed() const { return !breaks_.empty() || until_active_; }
    bool watching() const { return !watches_.empty(); }

    // ---- ganchos da maquina -------------------------------------------------------------------
    // Antes de executar a instrucao em `pc`: true se a maquina deve parar (ja marca stopped e enfileira o
    // evento). `skip_once_` pula um teste logo apos Resume().
    bool CheckPc(uint16_t pc);
    // PC da instrucao que esta para executar (para relatar quem tocou a memoria).
    void set_inst_pc(uint16_t pc) { inst_pc_ = pc; }
    // Cada leitura (write=false) ou escrita (write=true) de memoria do Z80.
    void OnAccess(uint16_t addr, uint8_t value, bool write);
    // Depois de cada instrucao: true se um watchpoint disparou durante ela (a maquina para).
    bool TakeWatchHit(uint16_t pc_after);

    // ---- eventos ------------------------------------------------------------------------------
    void PostEvent(const std::string &line);
    size_t EventCount() const { return events_.size(); }
    // Descarta os eventos a partir do `mark`-esimo (usado pelos comandos sincronos, que ja devolvem o
    // resultado na propria resposta).
    void TruncateEvents(size_t mark) {
        if (events_.size() > mark) events_.resize(mark);
    }
    // Entrega e esvazia a fila.
    std::vector<std::string> TakeEvents();

    // Esquece tudo (breakpoints, watchpoints, parada) -- troca de maquina.
    void Reset();

private:
    std::vector<Breakpoint> breaks_;
    std::vector<Watchpoint> watches_;
    int next_id_ = 1;
    bool stopped_ = false;
    bool skip_once_ = false;
    bool until_active_ = false;
    uint16_t until_addr_ = 0;
    uint16_t inst_pc_ = 0;
    std::string last_stop_;

    // Watch disparado durante a instrucao em curso (so o primeiro conta).
    bool watch_hit_ = false;
    int watch_id_ = 0;
    uint16_t watch_addr_ = 0;
    uint8_t watch_value_ = 0;
    bool watch_write_ = false;
    uint16_t watch_pc_ = 0;

    std::vector<std::string> events_;
};

} // namespace dbg
