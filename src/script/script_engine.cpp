// fwMSX -- scripts em Lua no console (C++). Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver script_engine.h e doc/script-spec.md.
#include "script_engine.h"

#include <cstdio>
#include <cstring>
#include <fstream>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include "../machine/machine.h"

namespace script {

namespace {

constexpr const char *kSelfKey = "fwmsx.engine";
constexpr int kInstructionLimit = 20000000; // por chamada (um laco infinito vira erro, nao trava o emulador)

}

struct Engine::Impl {
    struct Co {
        lua_State *th = nullptr;
        int ref = LUA_NOREF; // ancora a corrotina contra o coletor de lixo
        int wait = 0;        // quadros que faltam para acordar
    };
    struct ChangeHook {
        uint16_t addr = 0;
        int fn = LUA_NOREF;
        uint8_t last = 0;
    };

    EngineHost host;
    lua_State *L = nullptr;
    std::vector<Co> cos;
    std::vector<int> frame_hooks;
    std::vector<ChangeHook> change_hooks;
    std::vector<int> stop_hooks;
    bool was_stopped = false;
    int last_nres = 0;       // quantos valores a ultima corrotina devolveu ao acabar
    int depth = 0;           // quantas corrotinas/ganchos estao rodando agora (um script pode chamar `lua stop` por msx.cmd)
    bool stop_pending = false;
    std::string out; // o que print() escreveu desde que o trecho comecou

    explicit Impl(EngineHost h) : host(std::move(h)) {}
    ~Impl() {
        if (L) lua_close(L);
    }

    machine::Machine &M() { return host.machine(); }
    static Impl *Self(lua_State *L);
    void Install();
    void Say(const std::string &s) { out += s; }
    void DoStop();

    // roda f(...) na pilha de `th`; erro vira texto em `out` (e a corrotina acaba)
    static void Guard(lua_State *L, lua_Debug *) { luaL_error(L, "script took too long (more than %d instructions without waiting)", kInstructionLimit); }
    void Arm(lua_State *th) { lua_sethook(th, Guard, LUA_MASKCOUNT, kInstructionLimit); }

    // retoma uma corrotina; true se ela ainda existe (esperando)
    bool Resume(Co &co, std::string &err) {
        Arm(co.th);
        int nres = 0;
        ++depth;
        const int st = lua_resume(co.th, L, 0, &nres);
        --depth;
        if (st == LUA_YIELD) {
            int n = 1;
            if (nres >= 1 && lua_isinteger(co.th, -nres)) n = static_cast<int>(lua_tointeger(co.th, -nres));
            co.wait = n < 1 ? 1 : n;
            lua_pop(co.th, nres);
            return true;
        }
        if (st != LUA_OK) {
            const char *m = lua_tostring(co.th, -1);
            err = m ? m : "script error";
        } else {
            last_nres = nres;
        }
        return false;
    }
    void Release(Co &co) {
        luaL_unref(L, LUA_REGISTRYINDEX, co.ref);
        co.ref = LUA_NOREF;
        co.th = nullptr;
    }
    // chama uma funcao-gancho guardada no registro; erro vai para a saida assincrona
    void CallHook(int fn, const std::function<int(lua_State *)> &push) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, fn);
        const int n = push ? push(L) : 0;
        Arm(L);
        ++depth;
        const int st = lua_pcall(L, n, 0, 0);
        --depth;
        if (st != LUA_OK) {
            const char *m = lua_tostring(L, -1);
            host.event(std::string("error in a hook: ") + (m ? m : "?"));
            lua_pop(L, 1);
        }
    }
    void Flush() {
        if (out.empty()) return;
        std::string line;
        for (const char c : out) {
            if (c == '\n') {
                host.event(line);
                line.clear();
            } else {
                line.push_back(c);
            }
        }
        if (!line.empty()) host.event(line);
        out.clear();
    }
};

// ---------------------------------------------------------------------------------------------------------------
// a tabela msx
// ---------------------------------------------------------------------------------------------------------------

Engine::Impl *Engine::Impl::Self(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, kSelfKey);
    Impl *p = static_cast<Impl *>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    return p;
}

namespace {

using Impl = Engine::Impl;

lua_Integer Addr(lua_State *L, int idx) {
    const lua_Integer a = luaL_checkinteger(L, idx);
    if (a < 0 || a > 0xFFFF) luaL_error(L, "address out of range (0-65535): %d", static_cast<int>(a));
    return a;
}

int L_print(lua_State *L) {
    Impl *e = Impl::Self(L);
    const int n = lua_gettop(L);
    for (int i = 1; i <= n; ++i) {
        size_t len = 0;
        const char *s = luaL_tolstring(L, i, &len);
        if (i > 1) e->Say("\t");
        e->Say(std::string(s, len));
        lua_pop(L, 1);
    }
    e->Say("\n");
    return 0;
}

int L_cmd(lua_State *L) {
    Impl *e = Impl::Self(L);
    const char *line = luaL_checkstring(L, 1);
    std::string text;
    const bool ok = e->host.command(line, text);
    lua_pushboolean(L, ok);
    lua_pushlstring(L, text.data(), text.size());
    return 2;
}

// msx.vdp(n) le o registrador n do VDP; msx.vdp(n, v) grava como o OUT do Z80 (o mesmo do comando `vdp`). O limite do n depende do VDP (MSX1: 0-7).
int L_vdp(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer r = luaL_checkinteger(L, 1);
    const lua_Integer top = e->M().vdp_state().model == 0 ? 7 : 46;
    if (r < 0 || r > top) luaL_error(L, "vdp: register 0-%d for this VDP", static_cast<int>(top));
    if (lua_gettop(L) >= 2) {
        const lua_Integer v = luaL_checkinteger(L, 2);
        if (v < 0 || v > 0xFF) luaL_error(L, "vdp: 8-bit value (0-255)");
        e->M().WriteVdpRegister(static_cast<int>(r), static_cast<uint8_t>(v));
    }
    lua_pushinteger(L, e->M().vdp_state().regs[r]);
    return 1;
}

// msx.screen() devolve o texto da tela do MSX (SCREEN 0/1) como uma string, uma linha por linha da tela; nil + motivo nos outros modos.
int L_screen(lua_State *L) {
    Impl *e = Impl::Self(L);
    std::string text;
    if (!e->host.command("screen", text)) {
        lua_pushnil(L);
        lua_pushlstring(L, text.data(), text.size());
        return 2;
    }
    lua_pushlstring(L, text.data(), text.size());
    return 1;
}

int L_peek(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer a = Addr(L, 1);
    const lua_Integer n = luaL_optinteger(L, 2, 1);
    if (n < 1 || n > 256) luaL_error(L, "peek: count must be 1-256");
    luaL_checkstack(L, static_cast<int>(n), "too many values");
    for (lua_Integer i = 0; i < n; ++i) lua_pushinteger(L, e->M().ReadMemory(static_cast<uint16_t>(a + i)));
    return static_cast<int>(n);
}

int L_poke(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer a = Addr(L, 1);
    const int n = lua_gettop(L);
    if (n < 2) luaL_error(L, "poke: address and at least one value");
    for (int i = 2; i <= n; ++i) {
        const lua_Integer v = luaL_checkinteger(L, i);
        if (v < 0 || v > 255) luaL_error(L, "poke: byte value out of range (0-255)");
        e->M().WriteMemory(static_cast<uint16_t>(a + i - 2), static_cast<uint8_t>(v));
    }
    return 0;
}

int L_peek16(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer a = Addr(L, 1);
    lua_pushinteger(L, e->M().ReadMemory(static_cast<uint16_t>(a)) | (e->M().ReadMemory(static_cast<uint16_t>(a + 1)) << 8));
    return 1;
}

int L_poke16(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer a = Addr(L, 1);
    const lua_Integer v = luaL_checkinteger(L, 2);
    if (v < 0 || v > 0xFFFF) luaL_error(L, "poke16: value out of range (0-65535)");
    e->M().WriteMemory(static_cast<uint16_t>(a), static_cast<uint8_t>(v & 0xFF));
    e->M().WriteMemory(static_cast<uint16_t>(a + 1), static_cast<uint8_t>(v >> 8));
    return 0;
}

struct RegRef {
    const char *name;
    Z80Pair *p16;
    uint8_t *p8;
};

// msx.reg("pc") le; msx.reg("a", 5) escreve. af bc de hl ix iy sp pc af' bc' de' hl' e a f b c d e h l i r.
int L_reg(lua_State *L) {
    Impl *e = Impl::Self(L);
    Z80State &s = e->M().cpu().state();
    const RegRef regs[] = {{"af", &s.af, nullptr},  {"bc", &s.bc, nullptr},       {"de", &s.de, nullptr},       {"hl", &s.hl, nullptr},
                           {"ix", &s.ix, nullptr},  {"iy", &s.iy, nullptr},       {"sp", &s.sp, nullptr},       {"pc", &s.pc, nullptr},
                           {"af'", &s.af_alt, nullptr}, {"bc'", &s.bc_alt, nullptr}, {"de'", &s.de_alt, nullptr}, {"hl'", &s.hl_alt, nullptr},
                           {"a", nullptr, &s.af.b.hi}, {"f", nullptr, &s.af.b.lo}, {"b", nullptr, &s.bc.b.hi}, {"c", nullptr, &s.bc.b.lo},
                           {"d", nullptr, &s.de.b.hi}, {"e", nullptr, &s.de.b.lo}, {"h", nullptr, &s.hl.b.hi}, {"l", nullptr, &s.hl.b.lo},
                           {"i", nullptr, &s.i},       {"r", nullptr, &s.r}};
    char name[8] = {0}; // sem objetos com destrutor: luaL_error salta com longjmp
    const char *given = luaL_checkstring(L, 1);
    for (size_t i = 0; i + 1 < sizeof(name) && given[i]; ++i) name[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(given[i])));
    for (const RegRef &r : regs) {
        if (std::strcmp(name, r.name) != 0) continue;
        if (lua_gettop(L) >= 2) {
            const lua_Integer v = luaL_checkinteger(L, 2);
            if (r.p16) {
                if (v < 0 || v > 0xFFFF) luaL_error(L, "reg: 16-bit value (0-65535)");
                r.p16->w = static_cast<uint16_t>(v);
            } else {
                if (v < 0 || v > 0xFF) luaL_error(L, "reg: 8-bit value (0-255)");
                *r.p8 = static_cast<uint8_t>(v);
            }
            return 0;
        }
        lua_pushinteger(L, r.p16 ? r.p16->w : *r.p8);
        return 1;
    }
    return luaL_error(L, "reg: unknown register '%s'", given);
}

int L_frame(lua_State *L) {
    lua_pushinteger(L, static_cast<lua_Integer>(Impl::Self(L)->M().frame_count()));
    return 1;
}

// msx.wait(n): entrega o controle por n quadros (so' dentro de um script; num gancho e' erro)
int L_wait(lua_State *L) {
    const lua_Integer n = luaL_optinteger(L, 1, 1);
    if (n < 1 || n > 100000) return luaL_error(L, "wait: frames must be 1-100000");
    lua_pushinteger(L, n);
    return lua_yield(L, 1);
}

int AddHook(lua_State *L, std::vector<int> &list, int arg) {
    luaL_checktype(L, arg, LUA_TFUNCTION);
    lua_pushvalue(L, arg);
    list.push_back(luaL_ref(L, LUA_REGISTRYINDEX));
    return static_cast<int>(list.size());
}

int L_on_frame(lua_State *L) {
    lua_pushinteger(L, AddHook(L, Impl::Self(L)->frame_hooks, 1));
    return 1;
}

int L_on_stop(lua_State *L) {
    lua_pushinteger(L, AddHook(L, Impl::Self(L)->stop_hooks, 1));
    return 1;
}

// msx.on_change(addr, fn): fn(novo, antigo, addr) no quadro em que o byte mudou (olha uma vez por quadro; mudancas no meio do quadro que
// voltam ao valor antigo nao aparecem)
int L_on_change(lua_State *L) {
    Impl *e = Impl::Self(L);
    const lua_Integer a = Addr(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    Impl::ChangeHook h;
    h.addr = static_cast<uint16_t>(a);
    h.fn = luaL_ref(L, LUA_REGISTRYINDEX);
    h.last = e->M().ReadMemory(h.addr);
    e->change_hooks.push_back(h);
    lua_pushinteger(L, static_cast<lua_Integer>(e->change_hooks.size()));
    return 1;
}

int L_clear_hooks(lua_State *L) {
    Impl *e = Impl::Self(L);
    for (const int f : e->frame_hooks) luaL_unref(L, LUA_REGISTRYINDEX, f);
    for (const int f : e->stop_hooks) luaL_unref(L, LUA_REGISTRYINDEX, f);
    for (const Impl::ChangeHook &h : e->change_hooks) luaL_unref(L, LUA_REGISTRYINDEX, h.fn);
    e->frame_hooks.clear();
    e->stop_hooks.clear();
    e->change_hooks.clear();
    return 0;
}

} // namespace

void Engine::Impl::Install() {
    L = luaL_newstate();
    // so' as bibliotecas que nao tocam no computador: nada de io, os, package ou debug
    const luaL_Reg libs[] = {{LUA_GNAME, luaopen_base},   {LUA_TABLIBNAME, luaopen_table},   {LUA_STRLIBNAME, luaopen_string},
                             {LUA_MATHLIBNAME, luaopen_math}, {LUA_COLIBNAME, luaopen_coroutine}, {LUA_UTF8LIBNAME, luaopen_utf8}};
    for (const luaL_Reg &l : libs) {
        luaL_requiref(L, l.name, l.func, 1);
        lua_pop(L, 1);
    }
    for (const char *g : {"dofile", "loadfile", "require"}) {
        lua_pushnil(L);
        lua_setglobal(L, g);
    }
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, kSelfKey);
    lua_pushcfunction(L, L_print);
    lua_setglobal(L, "print");
    const luaL_Reg api[] = {{"cmd", L_cmd},         {"peek", L_peek},         {"poke", L_poke},         {"peek16", L_peek16}, {"poke16", L_poke16},
                            {"reg", L_reg},         {"vdp", L_vdp},           {"screen", L_screen},         {"frame", L_frame},       {"wait", L_wait},         {"on_frame", L_on_frame},
                            {"on_change", L_on_change}, {"on_stop", L_on_stop}, {"clear_hooks", L_clear_hooks}, {nullptr, nullptr}};
    luaL_newlib(L, api);
    lua_setglobal(L, "msx");
    Arm(L);
}

// ---------------------------------------------------------------------------------------------------------------
// o Engine
// ---------------------------------------------------------------------------------------------------------------

Engine::Engine(EngineHost host) : impl_(new Impl(std::move(host))) { impl_->Install(); }
Engine::~Engine() = default;

bool Engine::RunString(const std::string &code, const std::string &chunk_name, std::string &out, bool echo) {
    Impl &e = *impl_;
    e.out.clear();
    lua_State *th = lua_newthread(e.L);
    Impl::Co co;
    co.th = th;
    co.ref = luaL_ref(e.L, LUA_REGISTRYINDEX); // tira a thread da pilha e a prende
    bool as_expr = false;
    if (echo) {
        const std::string expr = "return " + code;
        as_expr = luaL_loadbuffer(th, expr.data(), expr.size(), chunk_name.c_str()) == LUA_OK;
        if (!as_expr) lua_settop(th, 0);
    }
    if (!as_expr && luaL_loadbuffer(th, code.data(), code.size(), chunk_name.c_str()) != LUA_OK) {
        const char *m = lua_tostring(th, -1);
        out = m ? m : "syntax error";
        e.Release(co);
        return false;
    }
    std::string err;
    e.last_nres = 0;
    const bool alive = e.Resume(co, err);
    out = e.out;
    e.out.clear();
    if (as_expr && !alive && err.empty()) { // os valores da expressao, separados por tab
        for (int k = 0; k < e.last_nres; ++k) {
            size_t len = 0;
            const char *s = luaL_tolstring(co.th, lua_gettop(co.th) - e.last_nres + 1 + k, &len);
            if (!out.empty() && out.back() != '\n') out += '\t';
            out.append(s, len);
            lua_pop(co.th, 1);
        }
    }
    if (!err.empty()) {
        if (!out.empty() && out.back() != '\n') out += '\n';
        out += err;
        e.Release(co);
        return false;
    }
    if (alive) e.cos.push_back(co);
    else e.Release(co);
    if (e.stop_pending && e.depth == 0) e.DoStop();
    return true;
}

bool Engine::RunFile(const std::string &path, std::string &out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        out = "cannot open " + path;
        return false;
    }
    std::string code((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if (code.size() >= 3 && static_cast<unsigned char>(code[0]) == 0xEF && static_cast<unsigned char>(code[1]) == 0xBB && static_cast<unsigned char>(code[2]) == 0xBF) code.erase(0, 3);
    return RunString(code, "@" + path, out);
}

void Engine::Tick() {
    Impl &e = *impl_;
    // 1) as corrotinas que estavam esperando
    // (o script pode chamar msx.cmd("lua ...") e criar outra corrotina: nada de referencia para dentro do vetor durante o Resume)
    const size_t n_before = e.cos.size();
    size_t i = 0;
    while (i < n_before && i < e.cos.size()) {
        Impl::Co co = e.cos[i];
        if (--co.wait > 0) {
            e.cos[i].wait = co.wait;
            ++i;
            continue;
        }
        std::string err;
        const bool alive = e.Resume(co, err);
        if (i >= e.cos.size() || e.cos[i].th != co.th) break; // um `lua stop` esvaziou a lista
        if (alive) {
            e.cos[i] = co;
            ++i;
            continue;
        }
        if (!err.empty()) e.host.event("error: " + err);
        e.Release(co);
        e.cos.erase(e.cos.begin() + static_cast<std::ptrdiff_t>(i));
    }
    e.Flush();
    // 2) os ganchos
    for (const int fn : std::vector<int>(e.frame_hooks)) {
        e.CallHook(fn, [&e](lua_State *L) {
            lua_pushinteger(L, static_cast<lua_Integer>(e.M().frame_count()));
            return 1;
        });
    }
    for (size_t i = 0; i < e.change_hooks.size(); ++i) {
        Impl::ChangeHook &h = e.change_hooks[i];
        const uint8_t now = e.M().ReadMemory(h.addr);
        if (now == h.last) continue;
        const uint8_t old = h.last;
        h.last = now;
        const uint16_t addr = h.addr;
        e.CallHook(h.fn, [now, old, addr](lua_State *L) {
            lua_pushinteger(L, now);
            lua_pushinteger(L, old);
            lua_pushinteger(L, addr);
            return 3;
        });
    }
    const bool stopped = e.M().debugger().stopped();
    if (stopped && !e.was_stopped) {
        const std::string why = e.M().debugger().last_stop();
        for (const int fn : std::vector<int>(e.stop_hooks)) {
            e.CallHook(fn, [&why](lua_State *L) {
                lua_pushlstring(L, why.data(), why.size());
                return 1;
            });
        }
    }
    e.was_stopped = stopped;
    e.Flush();
    if (e.stop_pending && e.depth == 0) e.DoStop();
}

void Engine::Stop() {
    Impl &e = *impl_;
    if (e.depth > 0) { // chamado de dentro de um script: acaba quando ele voltar
        e.stop_pending = true;
        return;
    }
    e.DoStop();
}

void Engine::Impl::DoStop() {
    Impl &e = *this;
    e.stop_pending = false;
    for (Impl::Co &co : e.cos) e.Release(co);
    e.cos.clear();
    for (const int f : e.frame_hooks) luaL_unref(e.L, LUA_REGISTRYINDEX, f);
    for (const int f : e.stop_hooks) luaL_unref(e.L, LUA_REGISTRYINDEX, f);
    for (const Impl::ChangeHook &h : e.change_hooks) luaL_unref(e.L, LUA_REGISTRYINDEX, h.fn);
    e.frame_hooks.clear();
    e.stop_hooks.clear();
    e.change_hooks.clear();
    lua_gc(e.L, LUA_GCCOLLECT);
}

size_t Engine::Coroutines() const { return impl_->cos.size(); }
size_t Engine::Hooks() const { return impl_->frame_hooks.size() + impl_->change_hooks.size() + impl_->stop_hooks.size(); }

std::string Engine::Status() const {
    return "lua: " + std::to_string(Coroutines()) + " script(s) waiting, " + std::to_string(impl_->frame_hooks.size()) + " on_frame, " +
           std::to_string(impl_->change_hooks.size()) + " on_change, " + std::to_string(impl_->stop_hooks.size()) + " on_stop";
}

} // namespace script
