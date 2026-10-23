#!/usr/bin/env python3
"""Converte os livros de referencia do paleobasic nas paginas do `man` do fwMSX.

Uso:  python tools/paleobasic/import_books_help.py          (gera src/man/cpp/man_pages_books.cpp)

Fontes (third-party/paleobasic/src/editor/help/):
  RedBookHelpData.pbi      "The MSX Red Book" (Avalon Software / Kuma Computers, 1985), versao Markdown de Gustavo Seidler
  Th2HandbookHelpData.pbi  "MSX2 Technical Handbook" (ASCII Corporation, 1987), edicao Markdown de Konamiman (Nestor Soriano)
  BasicDignifiedHelpData.pbi  ajuda do Basic Dignified (em portugues) escrita para o paleobasic

Os dois livros vem em ingles, como no original; os links internos do Markdown (`[[[texto|||ancora]]]`) viram texto e, no fim da pagina, uma
lista "VER TAMBEM" com os topicos para onde os links apontavam (`man <nome>`). Importacao unica: o resultado (man_pages_books.cpp) e' que vai
para o git e para o executavel.
"""
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('imp', os.path.join(HERE, 'import_basic_help.py'))
imp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(imp)

OUT = os.path.join(imp.ROOT, 'src', 'man', 'cpp', 'man_pages_books.cpp')
LINK = re.compile(r'\[\[\[(?!\[)(.*?)\|\|\|([^\]]*)\]\]\]')
IMAGE = re.compile(r'\[\[\[\[(.*?)\|\|\|[^\]]*\]\]\]\]')

BOOKS = [
    # arquivo, prefixo do .pbi, prefixo do nome do topico, nome da categoria, titulo do livro
    ('RedBookHelpData.pbi', 'RedBookHelp', 'rb-', 'Red Book', 'The MSX Red Book (Avalon Software / Kuma Computers, 1985)'),
    ('Th2HandbookHelpData.pbi', 'Th2HandbookHelp', 'th2-', 'Handbook', 'MSX2 Technical Handbook (ASCII Corporation, 1987)'),
]


def slug(title):
    s = re.sub(r'[^a-z0-9]+', '-', imp.ascii_only(title).lower()).strip('-')
    return s[:60].strip('-') or 'topico'


def clean(body):
    body = imp.ascii_only(body).replace('\r', '').replace('\t', '    ')
    body = IMAGE.sub(lambda m: '[' + m.group(1) + ']', body)
    body = LINK.sub(lambda m: m.group(1), body)
    body = re.sub(r'</?a[^>]*>', '', body)
    body = re.sub(r'<br\s*/?>', '\n', body)
    body = re.sub(r'^@@@', '    ', body, flags=re.M)  # @@@ = linha pre-formatada (codigo, tabelas) no texto do paleobasic
    body = body.replace('**', '')
    body = re.sub(r'(?<!`)`(?!`)', '', body)
    body = body.replace('\\|', '|').replace('\\*', '*').replace('\\_', '_')
    lines = [l.rstrip() for l in body.split('\n') if not l.strip().startswith('```')]
    out = '\n'.join(lines)
    out = re.sub(r'\n{3,}', '\n\n', out)
    return out.strip('\n')


def parse_book(fname, prefix):
    calls = imp.parse_calls(imp.read(fname), [prefix + '_Begin', prefix + '_L', prefix + '_Commit', prefix + '_AddAnchor'])
    topics, anchors, cur = [], {}, []
    for name, a in calls:
        if name.endswith('_Begin'):
            cur = []
        elif name.endswith('_L'):
            cur.append(a[0])
        elif name.endswith('_Commit'):
            topics.append(dict(titulo=a[0], grupo=a[1], raw='\n'.join(cur)))
            cur = []
        else:
            anchors[a[0]] = a[1]
    return topics, anchors


def main():
    lines = []
    w = lines.append
    w('// fwMSX -- MAN: livros de referencia do MSX (The MSX Red Book, MSX2 Technical Handbook) e a ajuda do Basic Dignified. ARQUIVO GERADO por')
    w('// tools/paleobasic/import_books_help.py a partir de third-party/paleobasic/src/editor/help/*.pbi -- nao editar a mao.')
    w('//')
    w('// Red Book: Avalon Software / Kuma Computers, 1985 (versao Markdown de Gustavo Seidler). MSX2 Technical Handbook: ASCII Corporation, 1987')
    w('// (edicao Markdown de Konamiman). Texto em ingles, como no original. Ver doc/man-spec.md, secao "Origem e direitos".')
    w('#include "man_internal.h"')
    w('')
    w('namespace man {')
    w('')

    def cstr(s):
        s = imp.ascii_only(s)
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

    def raw(s):
        s = imp.ascii_only(s)
        if ')MAN"' in s:
            raise ValueError('texto contem o delimitador )MAN"')
        return 'R"MAN(' + s + ')MAN"'

    w('std::vector<Topic> BookTopics() {')
    w('    std::vector<Topic> v;')
    w('    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {')
    w('        v.push_back(Topic{name, aliases, category, usage, summary, page, false});')
    w('    };')
    w('')
    counts = {}
    for fname, prefix, nprefix, cat, book in BOOKS:
        topics, anchors = parse_book(fname, prefix)
        used, names = set(), []
        for t in topics:
            base = nprefix + slug(t['titulo'])
            name, k = base, 2
            while name in used:
                name = '%s-%d' % (base, k)
                k += 1
            used.add(name)
            names.append(name)
        for i, t in enumerate(topics):
            refs = []
            for m in LINK.finditer(t['raw']):
                anchor = m.group(2)
                idx = anchors.get(anchor)
                if idx is None or idx == i or idx >= len(names) or anchor.startswith('img:'):
                    continue
                if names[idx] not in refs:
                    refs.append(names[idx])
            body = clean(t['raw'])
            page = ['LIVRO: ' + book, 'CAPITULO: ' + t['grupo'], '', body, '', 'FONTE', '  ' + book + '; texto em ingles, como no original.']
            if refs:
                page += ['', 'VER TAMBEM'] + ['  ' + ', '.join(refs[i:i + 3]) for i in range(0, min(len(refs), 12), 3)]
            first = ''
            for l in body.split('\n'):
                if l.strip() and not l.startswith((' ', '[', '#', '|', '-')):
                    first = l.strip()
                    break
            summary = imp.first_sentence(t['titulo'] + (': ' + first if first and first != t['titulo'] else ''), 90)
            w('    add(%s, "", %s, "", %s,' % (cstr(names[i]), cstr('%s: %s' % (cat, t['grupo'])), cstr(summary)))
            w('        ' + raw('\n'.join(page)) + ');')
            w('')
        counts[cat] = (len(topics), len(set(t['grupo'] for t in topics)))

    # ------------------------------------------------------------ Basic Dignified (portugues)
    bd = imp.parse_calls(imp.read('BasicDignifiedHelpData.pbi'), ['BDHelp_Add'])
    used = set()
    for n, (call, a) in enumerate(bd):
        titulo, grupo, corpo = a
        base = 'bd-' + slug(titulo)
        name, k = base, 2
        while name in used:
            name = '%s-%d' % (base, k)
            k += 1
        used.add(name)
        body = clean(corpo)
        page = ['BASIC DIGNIFIED: ' + grupo, '', body, '', 'FONTE', "  Ajuda do Basic Dignified do paleobasic (portugues); a suite original e' em Python."]
        first = next((l.strip() for l in body.split('\n') if l.strip()), '')
        w('    add(%s, %s, %s, "", %s,' % (cstr(name), cstr('dignified' if n == 0 else ''), cstr('Basic Dignified: ' + grupo), cstr(imp.first_sentence(titulo + ': ' + first, 90))))
        w('        ' + raw('\n'.join(page)) + ');')
        w('')
    counts['Basic Dignified'] = (len(bd), len(set(a[1] for _, a in bd)))
    w('    return v;')
    w('}')
    w('')
    w('} // namespace man')
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    for k, (n, g) in counts.items():
        print('%s: %d topicos em %d capitulos/grupos' % (k, n, g))
    print('->', OUT, os.path.getsize(OUT) // 1024, 'KB')


if __name__ == '__main__':
    sys.exit(main())
