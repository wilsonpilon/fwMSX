#!/usr/bin/env python3
"""Converte as ajudas de NestorBASIC, SEE Tracker e MSXBAS2ROM do paleobasic nas paginas do `man` do fwMSX.

Uso:  python tools/paleobasic/import_tools_help.py          (gera src/man/cpp/man_pages_tools_help.cpp)

Fontes (third-party/paleobasic/):
  src/editor/basic/NestorBasicHelpData.pbi          NestorBASIC (Nestor Soriano / Konamiman): biblioteca de 87 funcoes chamadas por USR do MSX-BASIC
  src/editor/visual_editors/SeeTrackerHelpData.pbi  Sound Effect Editor (SEE) 3.10 (Fuzzy Logic): editor de efeitos sonoros do PSG e o formato .SEE
  resource/tools/msxbas2rom/help/*.md + _index.json MSXBAS2ROM (Amaury Carvalho): compilador de MSX-BASIC para ROM (a "turbo" do paleobasic)
O fasmm (asMSX) e o N80 NAO entram aqui: o montador do fwMSX tem o manual proprio (man_pages_fwasm.cpp).
Importacao unica: o resultado (man_pages_tools_help.cpp) e' que vai para o git e para o executavel.
"""
import importlib.util
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('imp', os.path.join(HERE, 'import_basic_help.py'))
imp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(imp)

PALEO = os.path.join(imp.ROOT, 'third-party', 'paleobasic')
OUT = os.path.join(imp.ROOT, 'src', 'man', 'cpp', 'man_pages_tools_help.cpp')


def rd(path):
    with open(os.path.join(PALEO, path), encoding='utf-8-sig') as f:
        return f.read()


def slug(title, limit=48):
    s = re.sub(r'[^a-z0-9]+', '-', imp.ascii_only(title).lower()).strip('-')
    return s[:limit].strip('-') or 'topico'


def md_to_text(md):
    """Markdown (wiki do MSXBAS2ROM, mini-Markdown do paleobasic) -> texto de terminal, ASCII puro."""
    s = imp.ascii_only(md).replace('\r', '').replace('\t', '    ')
    s = re.sub(r'<!--.*?-->', '', s, flags=re.S)
    s = re.sub(r'!\[([^\]]*)\]\([^)]*\)', lambda m: '[' + m.group(1) + ']' if m.group(1) else '', s)   # imagens
    s = re.sub(r'\[([^\]]+)\]\([^)]*\)', r'\1', s)                                                      # links
    s = re.sub(r'</?[a-zA-Z][^>]*>', '', s)                                                              # tags HTML
    out = []
    fence = False
    for line in s.split('\n'):
        t = line.strip()
        if t.startswith('```'):
            fence = not fence
            continue
        if not fence:
            m = re.match(r'^(#{1,6})\s+(.*)$', line)
            if m:
                head = m.group(2).strip().strip('#').strip()
                out.append(head.upper() if len(m.group(1)) <= 2 else head)
                continue
            if re.fullmatch(r'[-*_]{3,}', t):
                continue
            line = line.replace('**', '')
            line = re.sub(r'(?<!`)`(?!`)', '', line)
            line = re.sub(r'^(\s*)[*+] ', r'\1- ', line)
        out.append(line.rstrip())
    text = '\n'.join(out)
    text = re.sub(r'\n{3,}', '\n\n', text)
    return text.strip('\n')


def main():
    topics = []

    def add(name, aliases, category, usage, summary, page):
        topics.append(dict(name=name, aliases=aliases, category=category, usage=usage, summary=imp.first_sentence(summary, 90), page=page))

    def first_line(text):
        for l in text.split('\n'):
            t = l.strip()
            if t and t.upper() != t:
                return t
        return ''

    # ------------------------------------------------------------------ NestorBASIC
    nb = imp.parse_calls(rd('src/editor/basic/NestorBasicHelpData.pbi'), ['NBHelp_Add'])
    n_nb = 0
    for _, a in nb:
        numero, titulo, grupo, wrapper, corpo = a[0], a[1], a[2], a[3], a[4]
        chamada = a[5] if len(a) > 5 else ''
        body = md_to_text(corpo)
        if numero < 0:
            if titulo.startswith('Introducao'):
                add('nestorbasic', 'nestor-basic,nb', 'NestorBASIC: introducao', 'USR(n)', 'NestorBASIC: biblioteca de 87 funcoes (RAM mapeada, disco, VRAM, som) para o MSX-BASIC',
                    '\n'.join(['NESTORBASIC (Nestor Soriano / Konamiman)', '', body, '', 'FONTE',
                               '  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.']))
                n_nb += 1
            continue  # os topicos de grupo so' tem o titulo
        page = ['FUNCAO %d do NestorBASIC  (grupo: %s)' % (numero, grupo), '']
        if wrapper:
            page += ['WRAPPER (Basic Dignified do paleobasic): ' + wrapper]
        if chamada:
            page += ['CHAMADA'] + ['    ' + l for l in imp.ascii_only(chamada).replace('\r', '').split('\n')]
        if wrapper or chamada:
            page.append('')
        page += [body, '', 'VER TAMBEM', '  nestorbasic', '', 'FONTE',
                 '  Ajuda do NestorBASIC do paleobasic (portugues); manual original nbas111e.txt do NestorBASIC 1.11.']
        aliases = ['nb-' + slug(titulo)]
        if wrapper:
            aliases.append(wrapper.lstrip('.').lower())
        add('nb-%d' % numero, ','.join(aliases), 'NestorBASIC: ' + grupo, 'USR(%d)' % numero, titulo + ': ' + first_line(body), '\n'.join(page))
        n_nb += 1

    # ------------------------------------------------------------------ SEE Tracker
    se = imp.parse_calls(rd('src/editor/visual_editors/SeeTrackerHelpData.pbi'), ['SeeHelp_Add'])
    n_see = 0
    used = set()
    for _, a in se:
        titulo, grupo, corpo = a
        if grupo.startswith('Rumo a um tracker'):
            continue  # plano de trabalho do editor do paleobasic, nao e' documentacao do SEE
        name = 'see-' + slug(titulo)
        base, k = name, 2
        while name in used:
            name = '%s-%d' % (base, k)
            k += 1
        used.add(name)
        body = md_to_text(corpo)
        page = ['SEE TRACKER (Sound Effect Editor 3.10, Fuzzy Logic 1991/95)  -  ' + grupo, '', body, '', 'VER TAMBEM', '  seetracker', '', 'FONTE',
                '  Ajuda do SEE Tracker do paleobasic (portugues), a partir do manual SEE3HELP.TXT e do driver SEE3PLAY.']
        add(name, 'seetracker,see' if titulo.startswith('O que e o SEE') else '', 'SEE Tracker: ' + grupo, '', titulo + ': ' + first_line(body), '\n'.join(page))
        n_see += 1
    # a primeira pagina vira o ponto de entrada "seetracker"
    for t in topics:
        if t['name'] == 'see-o-que-e-o-see':
            t['name'] = 'seetracker'
            t['aliases'] = 'see,see-tracker'

    # ------------------------------------------------------------------ MSXBAS2ROM
    help_dir = os.path.join(imp.ROOT, 'third-party', 'msxbas2rom', 'help')
    index = json.load(open(os.path.join(help_dir, '_index.json'), encoding='utf-8-sig'))
    n_b2r = 0
    for item in index:
        fname = item['file']
        with open(os.path.join(help_dir, fname), encoding='utf-8-sig') as f:
            md = f.read()
        body = md_to_text(md)
        stem = os.path.splitext(fname)[0]
        name = 'msxbas2rom' if stem == 'home' else 'msxbas2rom-' + slug(stem)
        page = ['MSXBAS2ROM (Amaury Carvalho)  -  ' + item['group'] + ': ' + item['title'], '', body, '', 'FONTE',
                '  Wiki do MSXBAS2ROM (texto em ingles); projeto: github.com/amaurycarvalho/msxbas2rom']
        aliases = 'msx-turbo-basic,msxbas2rom-home' if stem == 'home' else ''
        add(name, aliases, 'MSXBAS2ROM: ' + item['group'], '', item['title'] + ': ' + first_line(body), '\n'.join(page))
        n_b2r += 1

    # ------------------------------------------------------------------ saida
    lines = []
    w = lines.append
    w('// fwMSX -- MAN: NestorBASIC, SEE Tracker e MSXBAS2ROM. ARQUIVO GERADO por tools/paleobasic/import_tools_help.py a partir de third-party/paleobasic -- nao editar a mao.')
    w('//')
    w('// NestorBASIC: Nestor Soriano (Konamiman). SEE: Sound Effect Editor 3.10, Fuzzy Logic 1991/95. MSXBAS2ROM: Amaury Carvalho (GPL).')
    w('// Ver doc/man-spec.md, secao "Origem e direitos".')
    w('#include "man_internal.h"')
    w('')
    w('namespace man {')
    w('')
    w('std::vector<Topic> ToolsHelpTopics() {')
    w('    std::vector<Topic> v;')
    w('    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {')
    w('        v.push_back(Topic{name, aliases, category, usage, summary, page, false});')
    w('    };')
    w('')

    def cstr(s):
        s = imp.ascii_only(s)
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

    def raw_str(s):
        s = imp.ascii_only(s)
        if ')MAN"' in s:
            raise ValueError('texto contem o delimitador )MAN"')
        return 'R"MAN(' + s + ')MAN"'

    for t in topics:
        w('    add(%s, %s, %s, %s, %s,' % (cstr(t['name']), cstr(t['aliases']), cstr(t['category']), cstr(t['usage']), cstr(t['summary'])))
        w('        ' + raw_str(t['page']) + ');')
        w('')
    w('    return v;')
    w('}')
    w('')
    w('} // namespace man')
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print('NestorBASIC: %d, SEE Tracker: %d, MSXBAS2ROM: %d -> %s (%d KB)' % (n_nb, n_see, n_b2r, OUT, os.path.getsize(OUT) // 1024))


if __name__ == '__main__':
    sys.exit(main())
