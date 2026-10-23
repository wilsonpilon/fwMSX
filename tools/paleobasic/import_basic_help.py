#!/usr/bin/env python3
"""Converte a ajuda de MSX-BASIC do paleobasic (third-party/paleobasic/src/editor/help/*.pbi) nas paginas do `man` do fwMSX.

Uso:  python tools/paleobasic/import_basic_help.py          (gera src/man/cpp/man_pages_basic.cpp)

Os .pbi sao arquivos PureBasic com chamadas MSXDict_Add(...), MSXDict_Add2Plus(...) e MSXManual_Add(...), cujos argumentos sao
concatenacoes de literais ("..." + #CRLF$ + MSXQ + Chr(34)). Este script interpreta so' isso -- qualquer outra coisa e' erro, para nao
importar lixo em silencio. E' uma importacao UNICA: o resultado (man_pages_basic.cpp) e' que vai para o git e para o executavel.

Fontes originais (ver o cabecalho de cada .pbi): livro "Linguagem BASIC MSX" (Denise Santoro Cruz, Aleph/Gradiente, 1986) e
"Manual MSX 2+ FM" (Ademir Carchano / Flavio Monaco, ACVS Eletronica).
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
HELP = os.path.join(ROOT, 'third-party', 'paleobasic', 'src', 'editor', 'help')
OUT = os.path.join(ROOT, 'src', 'man', 'cpp', 'man_pages_basic.cpp')


def read(name):
    with open(os.path.join(HELP, name), encoding='utf-8-sig') as f:
        return f.read()


def split_args(body):
    """Divide o texto entre os parenteses de uma chamada nos argumentos (virgulas fora de strings/parenteses)."""
    args, cur, depth, i, n = [], [], 0, 0, len(body)
    while i < n:
        c = body[i]
        if c == '"':
            j = body.index('"', i + 1)
            cur.append(body[i:j + 1])
            i = j + 1
            continue
        if c == ';':  # comentario ate' o fim da linha
            while i < n and body[i] != '\n':
                i += 1
            continue
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        if c == ',' and depth == 0:
            args.append(''.join(cur))
            cur = []
        else:
            cur.append(c)
        i += 1
    args.append(''.join(cur))
    return [a.strip() for a in args]


def evaluate(expr):
    """Avalia uma concatenacao PureBasic de literais."""
    terms, cur, i, n = [], [], 0, len(expr)
    while i < n:
        c = expr[i]
        if c == '"':
            j = expr.index('"', i + 1)
            cur.append(expr[i:j + 1])
            i = j + 1
            continue
        if c == '+':
            terms.append(''.join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    terms.append(''.join(cur).strip())
    out = []
    for t in terms:
        if not t:
            continue
        if t.startswith('"'):
            out.append(t[1:-1])
        elif t == '#CRLF$':
            out.append('\n')
        elif t == 'MSXQ' or t == 'Chr(34)':
            out.append('"')
        elif t == '#True':
            return True
        elif t == '#False':
            return False
        elif re.fullmatch(r'-?\d+', t):
            return int(t)
        else:
            raise ValueError('termo desconhecido: %r em %r' % (t, expr[:80]))
    return ''.join(out)


def parse_calls(text, names):
    """Acha as chamadas `Nome(...)` que comecam uma linha (fora das definicoes `Procedure`)."""
    rx = re.compile(r'^[ \t]*(%s)\(' % '|'.join(names), re.M)
    calls = []
    for m in rx.finditer(text):
        i = m.end()
        depth, j, n = 1, i, len(text)
        while j < n and depth:
            c = text[j]
            if c == '"':
                j = text.index('"', j + 1)
            elif c == ';':
                while j < n and text[j] != '\n':
                    j += 1
                continue
            elif c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            j += 1
        args = [evaluate(a) for a in split_args(text[i:j - 1])]
        calls.append((m.group(1), args))
    return calls


def ascii_only(s):
    """O projeto e' ASCII puro (sem acento): troca/retira o que sobrar de fora."""
    import unicodedata
    s = s.replace('¡', '! (invertido)').replace('¿', '? (invertido)')
    s = unicodedata.normalize('NFKD', s)
    return ''.join(c for c in s if ord(c) < 128)


def plain(s):
    """O mini-Markdown do paleobasic -> texto puro (sem acento e sem marcacao)."""
    s = s.replace('\r', '')
    s = re.sub(r'\*\*(.+?)\*\*', r'\1', s)
    s = re.sub(r'`([^`]*)`', r'\1', s)
    s = re.sub(r'^## ?', '', s, flags=re.M)
    return s.strip('\n')


def indent(text, n=2):
    pad = ' ' * n
    return '\n'.join((pad + l) if l.strip() else '' for l in text.split('\n'))


def make_aliases(title):
    """Titulos compostos ("for-next", "erl/err", "sprite on/off/stop") ganham os nomes simples como apelido: `man for` acha for-next."""
    out = []
    if ' on/off' in title:
        out.append(title.split(' on/off')[0].strip())
    elif ' (' not in title and any(sep in title for sep in '/,-') or ' e ' in title:
        for tok in re.split(r'\s*[/,-]\s*|\s+e\s+', title):
            tok = tok.strip()
            if tok:
                out.append(tok)
    seen = []
    for a in out:
        if a and a != title and a not in seen and ',' not in a:
            seen.append(a)
    return seen


def first_sentence(s, limit=96):
    s = ' '.join(s.split())
    if len(s) <= limit:
        return s
    cut = s[:limit].rsplit(' ', 1)[0]
    return cut + '...'


def main():
    keywords = {}  # nome minusculo -> dict
    order = []
    for fname, names in (('MsxBasicDictData.pbi', ['MSXDict_Add']), ('MsxBasic2PlusDictData.pbi', ['MSXDict_Add2Plus'])):
        for call, a in parse_calls(read(fname), names):
            titulo, ehfunc, avanc, origem, resumo, formato, exformato, funcao, prog, pagina = a
            is_2p = call.endswith('2Plus')
            base = re.sub(r'\s*\(MSX2\+\)\s*$', '', titulo)
            key = base.lower()
            entry = dict(titulo=base, func=ehfunc, avanc=avanc, origem=origem, resumo=resumo, formato=formato,
                         exformato=exformato, funcao=funcao, prog=prog, pagina=pagina, fonte='MSX2+' if is_2p else 'MSX1')
            if key not in keywords:
                keywords[key] = {'parts': [entry]}
                order.append(key)
            else:
                keywords[key]['parts'].append(entry)

    topics = []  # prosa do manual
    for fname in ('MsxBasicManualData.pbi', 'MsxBasic2PlusManualData.pbi'):
        text = read(fname)
        for call, a in parse_calls(text, [r'MSXManual_Add\w*']):
            if len(a) != 4:
                raise ValueError('MSXManual_Add com %d argumentos em %s' % (len(a), fname))
            topics.append(dict(titulo=a[0], parte=a[1], corpo=a[2], pagina=a[3], fonte='MSX2+' if '2Plus' in fname else 'MSX1'))

    lines = []
    w = lines.append
    w('// fwMSX -- MAN: paginas do MSX-BASIC (dicionario de palavras reservadas e manual). ARQUIVO GERADO por')
    w('// tools/paleobasic/import_basic_help.py a partir de third-party/paleobasic/src/editor/help/*.pbi -- nao editar a mao.')
    w('//')
    w('// Fontes dos textos (transcricao feita no projeto paleobasic): livro "Linguagem BASIC MSX" (Denise Santoro Cruz, Editora Aleph/')
    w('// Gradiente, 4a edicao, 1986) e "Manual MSX 2+ FM" (Ademir Carchano / Flavio Monaco, ACVS Eletronica). Obras de terceiros,')
    w('// citadas como referencia do projeto; ver doc/man-spec.md, secao "Origem e direitos".')
    w('#include "man_internal.h"')
    w('')
    w('namespace man {')
    w('')
    w('std::vector<Topic> BasicTopics() {')
    w('    std::vector<Topic> v;')
    w('    // nome, apelidos, categoria, uso, resumo, pagina')
    w('    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {')
    w('        v.push_back(Topic{name, aliases, category, usage, summary, page, false});')
    w('    };')
    w('')

    def raw(s):
        s = ascii_only(s)
        if ')MAN"' in s:
            raise ValueError('texto contem o delimitador )MAN"')
        return 'R"MAN(' + s + ')MAN"'

    def cstr(s):
        s = ascii_only(s)
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

    nkw = 0
    for key in sorted(order):
        parts = keywords[key]['parts']
        main = parts[0]
        page = []
        for idx, e in enumerate(parts):
            if idx > 0:
                page.append('')
                page.append('NO MSX2+ (Manual MSX 2+ FM)' if e['fonte'] == 'MSX2+' else 'OUTRO VERBETE')
            head = []
            kind = 'FUNCAO' if e['func'] else 'INSTRUCAO'
            head.append('%s%s%s' % (kind, ' (uso avancado)' if e['avanc'] else '', ('  ' + e['origem']) if e['origem'] else ''))
            page.append(('TIPO: ' + head[0]) if idx == 0 else 'TIPO: ' + head[0])
            if e['resumo']:
                page.append('')
                page.append(indent(plain(e['resumo'])))
            if e['formato']:
                page.append('')
                page.append('FORMATO')
                page.append(indent(plain(e['formato'])))
            if e['exformato']:
                page.append('')
                page.append('EXEMPLO')
                page.append(indent(plain(e['exformato'])))
            if e['funcao']:
                page.append('')
                page.append('DESCRICAO')
                page.append(indent(plain(e['funcao'])))
            if e['prog']:
                page.append('')
                page.append('PROGRAMA EXEMPLO')
                page.append(indent(e['prog'].replace('\r', '').strip('\n'), 4))
        src = ('Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina %d' % main['pagina']) if main['fonte'] == 'MSX1' else \
              ('Manual MSX 2+ FM (ACVS), pagina %d' % main['pagina'])
        page.append('')
        page.append('FONTE')
        page.append('  ' + src + '.')
        page.append('')
        page.append('VER TAMBEM\n  basic, screens, cores')
        usage = main['formato'].split('\n')[0].strip() if main['formato'] else main['titulo']
        summary = first_sentence(plain(main['resumo'])) if main['resumo'] else main['titulo']
        aliases = make_aliases(key)
        w('    add(%s, %s, "MSX-BASIC", %s, %s,' % (cstr(key), cstr(','.join(aliases)), cstr(usage), cstr(summary)))
        w('        ' + raw('\n'.join(page)) + ');')
        w('')
        nkw += 1

    ntop = 0
    for t in topics:
        name = re.sub(r'[^a-z0-9]+', '-', re.sub(r'^apendice ([a-z]) - ', r'apendice-\1-', t['titulo'].lower())).strip('-')
        page = ['DESCRICAO' if False else t['parte'], '']
        page.append(plain(t['corpo']))
        page.append('')
        page.append('FONTE')
        page.append('  ' + ('Livro "Linguagem BASIC MSX" (Aleph/Gradiente), pagina %d' % t['pagina'] if t['fonte'] == 'MSX1'
                            else 'Manual MSX 2+ FM (ACVS), pagina %d' % t['pagina']) + '.')
        w('    add(%s, "", "Manual MSX-BASIC", "", %s,' % (cstr(name), cstr(t['titulo'] + ' (' + t['parte'] + ')')))
        w('        ' + raw('\n'.join(page)) + ');')
        w('')
        ntop += 1

    w('    return v;')
    w('}')
    w('')
    w('} // namespace man')
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print('palavras: %d (de %d verbetes), topicos de manual: %d -> %s' % (nkw, sum(len(v['parts']) for v in keywords.values()), ntop, OUT))
    print('nomes:', ' | '.join(sorted(keywords)))
    print('topicos:', ' | '.join(t['titulo'] for t in topics))


if __name__ == '__main__':
    sys.exit(main())
