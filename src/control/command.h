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
#include <memory>
#include <string>
#include <vector>

#include "../debug/cpp/debug_view.h"
#include "../machine/machine.h"

namespace mamute {
class Monitor;
} // namespace mamute

namespace prj {
class Project;
} // namespace prj

namespace script {
class Engine;
} // namespace script

namespace control {

struct Result {
    bool ok = true;
    std::string text; // sem quebra de linha; vazio = so' "ok"

    // Resposta pronta para o fio. Uma linha: "ok [texto]" ou "err texto". Com varias linhas de texto (dis,
    // mem, reg...): "ok +N" seguido das N linhas -- o cliente le N linhas a mais (doc/control-spec.md).
    std::string Line() const {
        if (ok && text.find('\n') != std::string::npos) {
            size_t n = 1;
            for (const char c : text) n += c == '\n';
            return "ok +" + std::to_string(n) + "\n" + text;
        }
        return (ok ? "ok" : "err") + (text.empty() ? std::string() : " " + text);
    }
};

// O texto da tela do MSX (SCREEN 0 ou 1), uma linha por linha da tela; vazio se a tela esta' em outro modo. Comando `screen`.
std::string ScreenText(machine::Machine &m);

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
    // O caminho da ROM do kernel do MSX-DOS 2 (a do banco de ROMs); "" = nao achou (e `error` diz o que fazer).
    virtual std::string FindDos2Rom(std::string &error) {
        error = "o MSX-DOS 2 nao esta' disponivel neste modo";
        return "";
    }
    // Reinicia a maquina como um MSX-DOS 2: `rom` no slot 1 (mapper msxdos2), interface de disquete ligada e, se `disk` nao e' vazio, ele em A:.
    // Mantem o resto da configuracao (discos, fita). `rom` vazio = so' desliga o cartucho (a maquina volta ao que era).
    virtual bool BootDos2(const std::string &rom, const std::string &disk, std::string &error) {
        (void)rom;
        (void)disk;
        error = "o MSX-DOS 2 nao esta' disponivel neste modo";
        return false;
    }
    // A ROM do Nextor (Sunrise IDE) do banco de ROMs; "" = nao achou (e `error` diz o que fazer).
    virtual std::string FindNextorRom(std::string &error) {
        error = "o disco rigido nao esta' disponivel neste modo";
        return "";
    }
    // Reinicia a maquina com a interface Sunrise IDE (Nextor) e as imagens de disco rigido `master`/`slave` (vazias = sem); as duas vazias
    // tiram a interface. Mantem o resto da configuracao (discos, fita, cartucho).
    virtual bool BootIde(const std::string &nextor, const std::string &master, const std::string &slave, std::string &error) {
        (void)nextor;
        (void)master;
        (void)slave;
        error = "o disco rigido nao esta' disponivel neste modo";
        return false;
    }
    // A ROM do Nextor com o driver MegaSD (nextsd15.rom, do pacote SDBIOS copiado para <pasta de ROMs>/ocm/roms); "" = nao achou.
    virtual std::string FindMegaSdRom(std::string &error) {
        error = "o cartao SD nao esta' disponivel neste modo";
        return "";
    }
    // Reinicia a maquina com o MegaSD e a imagem de cartao SD `image` (vazia = tira o MegaSD). Mantem o resto da configuracao.
    virtual bool BootSd(const std::string &rom, const std::string &image, std::string &error) {
        (void)rom;
        (void)image;
        error = "o cartao SD nao esta' disponivel neste modo";
        return false;
    }
    // Grava a tela em `path` (PNG na janela; o formato e' do host).
    virtual bool SaveScreenshot(const std::string &path, std::string &error) = 0;
};

class Commander {
public:
    explicit Commander(Host &host);
    ~Commander();

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

    // Eventos assincronos do depurador ("! break pc=4010 ..."), na ordem em que aconteceram; esvazia a fila.
    // Quem hospeda a maquina (servidor TCP, TUI, modo terminal) chama depois de cada quadro/comando.
    std::vector<std::string> TakeEvents();

    // Uma "digitacao": as teclas descem juntas (na ordem) e sobem juntas (na ordem inversa) -- {ctrl}+{c} e' uma so'.
    struct TypedKey {
        std::vector<std::string> keys;
    };
    // Interpreta o argumento do "type" (texto entre aspas ou solto, {tecla}, {a}+{b}); false + erro se invalido.
    // literal = true e' o modo do --keys: aspas e espacos sao texto comum, so' {tecla} e a barra-n tem sentido especial.
    static bool ParseTypeSpec(machine::Machine &m, const std::string &spec, bool literal, std::vector<TypedKey> &out, std::string &error);

private:
    // asmrun: monta um fonte .asm do disco, grava as saidas ao lado dele, carrega o resultado na maquina e entrega os simbolos e as linhas ao depurador.
    Result AsmRun(const std::vector<std::string> &t);
    // badig / badigrun: converte um .dmx (Basic Dignified) em .amx/.bmx; badigrun tambem monta um disco com o .bmx e o AUTOEXEC.BAS, insere em A: e reinicia.
    Result BadigCmd(const std::vector<std::string> &t, bool run);
    // obi build|check [Obifile] / obirun [Obifile]: a receita que monta o .dsk (src/obi; doc/obi-spec.md); obirun tambem insere o disco em A: e reinicia.
    Result ObiCmd(const std::vector<std::string> &t, bool run);
    // lua <codigo|arquivo.lua|stop|status> e source <arquivo>: scripts (src/script; doc/script-spec.md).
    Result LuaCmd(const std::string &line);
    Result SourceCmd(const std::vector<std::string> &t, int depth);
    script::Engine &Lua();
    std::unique_ptr<script::Engine> lua_;
    int source_depth_ = 0;
    // project: o .msxproject (SQLite) e as copias soltas numa pasta (src/control/project_cmd.cpp; doc/project-spec.md).
    Result ProjectCmd(const std::vector<std::string> &t);
    std::unique_ptr<prj::Project> project_;
    std::string project_root_; // a pasta das copias soltas do projeto aberto (vazio = a pasta do .msxproject)

    Host &host_;
    // O monitor Mamute (comando `mon`): criado na primeira vez que alguem o usa; a maquina so' e' ligada a ele DURANTE
    // cada comando (ela pode ser recriada pelo host entre um comando e outro).
    std::unique_ptr<mamute::Monitor> mon_;
    // Painel "Z80 Debugger" por texto (comando `dbgui`): o MESMO modelo da janela, para a TUI e qualquer cliente.
    dbgui::DebugView dbgview_;
    std::deque<TypedKey> typing_;
    TypedKey current_{};
    int key_phase_ = 0; // 0 = ocioso, 1 = tecla pressionada, 2 = solta (intervalo)
    int key_ticks_ = 0;
};

} // namespace control
