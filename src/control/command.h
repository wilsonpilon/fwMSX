// fwMSX -- ponte de controle externo: despachante de comandos (C++). Codigo ORIGINAL do fwMSX
// (BSD-3-Clause). Ver doc/control-spec.md.
//
// UMA camada de comandos para tudo: a TUI, a linha de comando e o servidor TCP (server.h) chamam
// o mesmo Commander::Execute(), entao o que um editor/depurador/montador remoto faz e' exatamente
// o que a TUI faz. O protocolo e' de texto, uma linha por comando:
//
//   > peek 0xC000 4        < ok 00 01 02 03
//   > poke 0xC000 0x41     < ok
//   > cart nao-existe.rom  < err nao foi possivel abrir ...
#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "../machine/machine.h"

namespace control {

struct Result {
    bool ok = true;
    std::string text; // sem quebra de linha; vazio = so' "ok"

    // Linha pronta para o fio: "ok [texto]" ou "err texto".
    std::string Line() const { return (ok ? "ok" : "err") + (text.empty() ? std::string() : " " + text); }
};

// O que o Commander precisa do dono da maquina (a janela, a TUI ou um teste). A maquina pode ser
// RECRIADA pelo host (ex.: trocar de cartucho reinicia), por isso nunca e' guardada aqui.
class Host {
public:
    virtual ~Host() = default;
    virtual machine::Machine &machine() = 0;
    virtual bool paused() const = 0;
    virtual void set_paused(bool paused) = 0;
    virtual void quit() = 0;
    // Troca o cartucho (reinicia a maquina). `path` vazio = retira. `mapper` "" = automatico.
    virtual bool LoadCartridge(const std::string &path, const std::string &mapper, std::string &error) = 0;
    // Grava a tela em `path` (PNG na janela; o formato e' do host).
    virtual bool SaveScreenshot(const std::string &path, std::string &error) = 0;
};

class Commander {
public:
    explicit Commander(Host &host) : host_(host) {}

    // Executa UMA linha de comando. Nunca lanca excecao.
    Result Execute(const std::string &line);

    // Chamar uma vez por quadro emulado: avanca a fila de digitacao do comando `type`
    // (tecla pressionada por 3 quadros, solta por 3).
    void Tick();

    // Quebra a linha em palavras (aspas duplas agrupam; dentro delas so' \" e' escape -- a barra
    // invertida dos caminhos do Windows e' literal). O \n do comando "type" e' tratado la'.
    static std::vector<std::string> Tokenize(const std::string &line, std::string &error);
    // Aceita 123, 0x7B, $7B e 7Bh. false se nao e' um numero.
    static bool ParseNumber(const std::string &text, uint32_t &value);

    bool typing() const { return !typing_.empty() || key_phase_ != 0; }

private:
    struct TypedKey {
        std::string key;
        bool shift;
    };

    Host &host_;
    std::deque<TypedKey> typing_;
    TypedKey current_{};
    int key_phase_ = 0; // 0 = ocioso, 1 = tecla pressionada, 2 = solta (intervalo)
    int key_ticks_ = 0;
};

} // namespace control
