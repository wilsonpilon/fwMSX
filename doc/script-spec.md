# Scripts no console -- `lua` e `source`

Versao 1.43.16. Codigo em `src/script/` (`script_engine.h/.cpp`, C++ sobre o Lua 5.4); os comandos `lua` e `source` moram no `Commander` (`src/control/command.cpp`), entao valem em todo lugar onde existe o console: a TUI, a janela do emulador, o servidor TCP e a linha de comando. Testes: `controltest` (`TestLua`).

## 1. Por que Lua

O console do fwMSX tem comandos demais para uma lista de `peek`/`poke`, mas uma linguagem completa e pesada (como o Tcl do openMSX) nao era o desejado. O Lua 5.4 (C puro, MIT, ~30 mil linhas) e' o que FCEUX, BizHawk, MAME e Mesen escolheram: pequeno, rapido, com **corrotinas** (o `msx.wait(n)` abaixo) e facil de isolar. Decisoes do usuario (2026-10-22): o codigo do Lua vem por `FetchContent` no build (tarball oficial `lua-5.4.7`, hash SHA-256 fixo), e o primeiro passo entrega os degraus 1 e 2 do plano (`source` e a API `msx` com ganchos); o editor (`.lua` como tipo, Run > Run script) e o `man` completo ficam para depois.

## 2. Comandos

| Comando | O que faz |
|---|---|
| `lua <codigo>` | roda o texto como Lua. Se for uma expressao, mostra o valor (`lua 1+2` -> `3`); o `print` do script volta na resposta. O texto depois de `lua` vai CRU para o Lua (aspas, virgulas e barras sao do Lua, nao do console). |
| `lua <arquivo.lua>` | roda o arquivo. |
| `lua stop` | acaba com os scripts que esperam e com todos os ganchos. |
| `lua status` | quantos scripts esperam e quantos ganchos ha'. |
| `source <arquivo>` | roda um comando do console por linha; linhas em branco e as que comecam com `#` ou `--` sao comentarios; para na primeira que falhar (`arquivo:LINHA: erro`). Um `source` pode chamar outro, ate' 8 niveis. |

## 3. A tabela `msx`

| Funcao | Faz |
|---|---|
| `msx.cmd("linha")` | roda um comando do console; devolve `ok` (booleano) e o texto da resposta |
| `msx.peek(addr [, n])` | le `n` bytes (1 a 256) como a CPU enxerga agora; devolve `n` numeros |
| `msx.poke(addr, v1, v2...)` | grava bytes (respeita ROM) |
| `msx.peek16(addr)` / `msx.poke16(addr, v)` | palavra de 16 bits, little-endian |
| `msx.vdp(n [, valor])` | le o registrador n do VDP (grava como o OUT do Z80 se houver valor; n 0-7 no MSX1, 0-46 no MSX2) |
| `msx.screen()` | o texto da tela do MSX (SCREEN 0/1) como string; `nil` + motivo nos outros modos |
| `msx.reg(nome [, valor])` | le ou escreve um registrador: `af bc de hl ix iy sp pc af' bc' de' hl' a f b c d e h l i r` |
| `msx.frame()` | o numero do quadro emulado |
| `msx.wait(n)` | **entrega o controle e volta `n` quadros depois** (1 a 100000). Vale dentro de um script, nao dentro de um gancho |
| `msx.on_frame(fn)` | `fn(quadro)` uma vez por quadro |
| `msx.on_change(addr, fn)` | `fn(novo, antigo, addr)` no quadro em que o byte mudou |
| `msx.on_stop(fn)` | `fn(motivo)` quando a maquina para (breakpoint, watch...) |
| `msx.clear_hooks()` | tira todos os ganchos |

Exemplo: congelar as vidas de um jogo e avisar quando a fase muda.

```lua
-- vidas.lua   (lua vidas.lua)
local VIDAS, FASE = 0xE000, 0xE001
msx.on_frame(function() msx.poke(VIDAS, 9) end)
msx.on_change(FASE, function(novo, antigo) print("fase", antigo, "->", novo) end)
```

```lua
-- tela_de_titulo.lua: espera 3 segundos (180 quadros) e aperta a barra de espaco
msx.wait(180)
msx.cmd("type {space}")
print("passou do titulo no quadro", msx.frame())
```

## 4. Como roda

- Todo script roda numa **corrotina**: o `lua <arquivo>` volta logo no primeiro `msx.wait`, e o `Commander::Tick` (uma vez por quadro emulado) o acorda. O que o script imprime depois disso nao tem para onde voltar: vira o evento `! lua <texto>` (o mesmo canal dos eventos do depurador, que a TUI e o servidor ja mostram). Erros de um gancho tambem: `! lua error in a hook: ...`.
- **Nada roda por instrucao do Z80.** Os ganchos olham a maquina entre um quadro e outro; por isso `on_change` nao ve uma gravacao que volta ao valor antigo dentro do mesmo quadro. Um gancho por escrita de verdade (parar a CPU no endereco, como o `watch`) continua sendo o comando `watch` do depurador.
- **Limite de instrucoes**: cada chamada de script ou de gancho tem 20 milhoes de instrucoes Lua antes de dar erro `script took too long`; um `while true do end` nao trava o emulador.
- **Sandbox**: o Lua nasce so' com `base`, `table`, `string`, `math`, `coroutine` e `utf8`; `io`, `os`, `package`, `debug`, `dofile`, `loadfile` e `require` nao existem. Um script so' fala com o computador pelo `msx.cmd` (que roda os mesmos comandos que o usuario digitaria, incluindo `lua` e `source`).
- `lua stop` chamado de dentro de um script vale quando ele volta ou espera.

## 5. Build e licenca

`CMakeLists.txt` baixa `https://www.lua.org/ftp/lua-5.4.7.tar.gz` (SHA-256 `9fbf5e28...`), compila tudo menos `lua.c` e `luac.c` como a biblioteca estatica `fwlua` e liga nos alvos que levam o controle. Lua e' MIT (`LICENSE-THIRD-PARTY.md`).

## 6. Fora do primeiro passo

- `.lua` como tipo do projeto e **Run > Run script** no editor; a pagina `man` completa com a API;
- ganchos por escrita/leitura de endereco com custo (parar a CPU, `on_write`);
- `msx.type`, leitura de VRAM e dos registradores do VDP/PSG, `msx.screenshot`;
- bibliotecas de apoio (um `require` de arquivos da pasta de scripts, em lugar do `dofile` que foi retirado).
