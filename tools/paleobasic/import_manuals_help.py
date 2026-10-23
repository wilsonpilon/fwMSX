#!/usr/bin/env python3
"""Converte MsxSoftwareHelpData.pbi e MsxManualsHelpData.pbi do paleobasic nas paginas do `man` do fwMSX.

Uso:  python tools/paleobasic/import_manuals_help.py          (gera src/man/cpp/man_pages_manuals.cpp)

Fontes (third-party/paleobasic/src/editor/help/), geradas pelo paleobasic a partir dos CHM do RuMSX (Lex Lechz):
  MsxSoftwareHelpData.pbi  (SOFTWARE.CHM) comandos do MSX-BASIC (1, 2, 2+, Turbo-R, Disk-BASIC), extensoes CALL de firmware, MSX-DOS 1, CP/M e referencias
                           (tabela de cores, codigos de erro, modos de tela, vozes do MSX-Music...)
  MsxManualsHelpData.pbi   (MANUALS.CHM) MSX-DOS 2 (referencia de comandos, interface de programa, codigos de funcao), Z80, R800, Turbo-Basic, FM-PAC.
                           Os 8 capitulos do MSX2 Technical Handbook que ele traz NAO sao importados: o man ja' tem o livro inteiro (import_books_help.py).
Os manuais grandes sao PARTIDOS: o MSX-DOS 2 em um topico por comando (dos2-copy) e por chamada de funcao (dos2-fn-09h, apelido _STROUT).
Importacao unica: o resultado (man_pages_manuals.cpp) e' que vai para o git e para o executavel.
"""
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('imp', os.path.join(HERE, 'import_basic_help.py'))
imp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(imp)

OUT = os.path.join(imp.ROOT, 'src', 'man', 'cpp', 'man_pages_manuals.cpp')
STUB = 'Ainda sem pagina propria'


def slug(title, limit=60):
    s = re.sub(r'[^a-z0-9]+', '-', imp.ascii_only(title).lower()).strip('-')
    return s[:limit].strip('-') or 'topico'


def parse(fname, prefix):
    calls = imp.parse_calls(imp.read(fname), [prefix + '_Begin', prefix + '_L', prefix + '_Commit'])
    topics, cur = [], []
    for name, a in calls:
        if name.endswith('_Begin'):
            cur = []
        elif name.endswith('_L'):
            cur.append(a[0])
        else:
            topics.append((a[0], a[1], cur))
            cur = []
    return topics


def clean(lines):
    body = '\n'.join(lines) if isinstance(lines, list) else lines
    body = imp.ascii_only(body).replace('\r', '').replace('\t', '    ')
    body = re.sub(r'^\*\*(.+?):\*\*\s*$', lambda m: m.group(1).upper(), body, flags=re.M)  # **Sintaxe:** -> SINTAXE
    body = body.replace('**', '')
    body = re.sub(r'(?<!`)`(?!`)', '', body)
    lines = [l.rstrip() for l in body.split('\n') if not l.strip().startswith('```')]
    out = '\n'.join(lines)
    out = re.sub(r'\n{3,}', '\n\n', out)
    return out.strip('\n')


def first_text(body):
    for l in body.split('\n'):
        t = l.strip()
        if t and not t.startswith(('#', '(', '|', '-', '+', '=')) and t.upper() != t:
            return t
    return ''


def label_value(body, labels):
    """Primeira linha de texto depois de um rotulo (SINTAXE, FORMATO, FUNCAO, FINALIDADE...)."""
    lines = body.split('\n')
    for i, l in enumerate(lines):
        if l.strip() in labels:
            for x in lines[i + 1:]:
                if x.strip():
                    return x.strip()
    return ''


def main():
    topics = []  # dicts: name, aliases, category, usage, summary, page

    # ------------------------------------------------------------------ SOFTWARE.CHM
    by_name, order = {}, []
    for titulo, grupo, lines in parse('MsxSoftwareHelpData.pbi', 'SoftwareHelp'):
        raw_body = '\n'.join(lines)
        if STUB in raw_body:
            continue  # "ainda sem pagina propria": nada a importar
        body = clean(lines)
        body = re.sub(r'^## .*\n+', '', body)  # o titulo ja' e' o nome do topico
        t = titulo.strip()
        if grupo == 'MSX-DOS':
            name, cat, label = 'dos-' + slug(t), 'MSX-DOS 1', 'COMANDO DO MSX-DOS (RuMSX)'
        elif grupo == 'CP/M':
            name, cat, label = 'cpm', 'MSX-DOS 1', 'CP/M (RuMSX)'
        elif grupo == 'MSX-BASIC - Referencia':
            name, cat, label = 'ref-' + slug(t), 'Referencia BASIC', 'REFERENCIA (RuMSX)'
        else:
            name, cat, label = t.lower().rstrip(':').strip(), 'MSX-BASIC', 'REFERENCIA EM INGLES (RuMSX, ' + grupo + ')'
        if name not in by_name:
            by_name[name] = dict(cat=cat, parts=[])
            order.append(name)
        by_name[name]['parts'].append((label, body))
    for name in order:
        parts = by_name[name]['parts']
        page = []
        for label, body in parts:
            if page:
                page.append('')
            page += [label, '', body]
        page += ['', 'FONTE', '  SOFTWARE.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.']
        body0 = parts[0][1]
        usage = label_value(body0, ('SINTAXE', 'FORMATO'))
        summary = label_value(body0, ('FUNCAO', 'FINALIDADE')) or first_text(body0) or name
        topics.append(dict(name=name, aliases='', category=by_name[name]['cat'], usage=usage[:80], summary=imp.first_sentence(summary, 90),
                           page='\n'.join(page)))
    n_soft = len(order)

    # ------------------------------------------------------------------ MANUALS.CHM
    man = {t[0]: (t[1], clean(t[2]), t[2]) for t in parse('MsxManualsHelpData.pbi', 'ManualsHelp')}
    n_man = 0

    def add(name, cat, summary, body, book, aliases='', usage=''):
        nonlocal n_man
        topics.append(dict(name=name, aliases=aliases, category=cat, usage=usage, summary=imp.first_sentence(summary, 90),
                           page='\n'.join([book, '', body, '', 'FONTE', '  MANUALS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.'])))
        n_man += 1

    # --- MSX-DOS 2: referencia de comandos, partida por capitulo e por comando
    _, _, raw = man['MSX-DOS 2 - Referencia']
    L = [imp.ascii_only(x).replace('\t', '    ').rstrip() for x in raw]
    heads = [i for i, x in enumerate(L) if re.match(r'^\d+\.\s+[A-Z]', x) and not x.startswith(' ') and i + 1 < len(L)]
    chapters = []
    for k, i in enumerate(heads):
        end = heads[k + 1] if k + 1 < len(heads) else len(L)
        chapters.append((i, end, re.sub(r'^\d+\.\s+', '', L[i]).strip()))
    book = 'MSX-DOS 2 (versao 2.20) - Command Specification'
    front = '\n'.join(L[:heads[0]]).strip('\n')
    add('dos2-contents', 'MSX-DOS 2: manual', 'MSX-DOS 2: indice do manual de comandos', front, book)
    for i, end, title in chapters:
        if title.upper().startswith('COMMANDS'):
            # capitulo 4: introducao + um topico por comando ("NOME" sozinho numa linha, seguido de "Format")
            cmd_idx = []
            for j in range(i, end):
                if re.match(r'^[A-Z][A-Z0-9]*$', L[j]) and j > 0 and L[j - 1] == '':
                    k = j + 1
                    while k < end and L[k] == '':
                        k += 1
                    if k < end and L[k].strip() == 'Format':
                        cmd_idx.append(j)
            add('dos2-commands', 'MSX-DOS 2: manual', 'Comandos do MSX-DOS 2: introducao ao capitulo', clean(L[i:cmd_idx[0]]), book)
            for n, j in enumerate(cmd_idx):
                stop = cmd_idx[n + 1] if n + 1 < len(cmd_idx) else end
                body = clean(L[j:stop])
                cmd = L[j]
                purpose = ''
                for q, x in enumerate(L[j:stop]):
                    if x.strip() == 'Purpose':
                        purpose = next((y.strip() for y in L[j + q + 1:stop] if y.strip()), '')
                        break
                fmt = ''
                for q, x in enumerate(L[j:stop]):
                    if x.strip() == 'Format':
                        fmt = next((y.strip() for y in L[j + q + 1:stop] if y.strip()), '')
                        break
                add('dos2-' + cmd.lower(), 'MSX-DOS 2: comandos', cmd + ': ' + purpose, body, book, usage=fmt[:80])
        else:
            add('dos2-' + slug(title), 'MSX-DOS 2: manual', title + ' (MSX-DOS 2)', clean(L[i:end]), book)

    # --- MSX-DOS 2: interface de programa (uma pagina) e codigos de funcao (uma por chamada)
    _, body, _ = man['MSX-DOS 2 - Interface de Programa']
    add('dos2-program-interface', 'MSX-DOS 2: manual', 'MSX-DOS 2: especificacao da interface de programa (handles, fileinfo, ambiente)', body,
        'MSX-DOS 2 - Program Interface Specification')
    _, _, raw = man['MSX-DOS 2 - Codigos de Funcao']
    L = [imp.ascii_only(x).replace('\t', '    ').rstrip() for x in raw]
    fheads = [i for i, x in enumerate(L) if re.match(r'^\s*3\.\d+\s+\S.*\(([0-9A-F]{2})H\)\s*$', x)]
    book = 'MSX-DOS 2 (versao 2.20) - Function Specification'
    add('dos2-functions', 'MSX-DOS 2: manual', 'Chamadas de funcao do MSX-DOS 2: introducao e lista', clean(L[:fheads[0]]), book)
    for n, i in enumerate(fheads):
        stop = fheads[n + 1] if n + 1 < len(fheads) else len(L)
        m = re.match(r'^\s*3\.(\d+)\s+(.*?)\s*\(([0-9A-F]{2})H\)\s*$', L[i])
        title, code = m.group(2).strip(), m.group(3).lower() + 'h'
        sym = ''
        for x in L[i + 1:i + 8]:
            q = re.search(r'\((_[A-Za-z0-9]+)\)', x)
            if q:
                sym = q.group(1).lower()
                break
        aliases = ','.join(a for a in (sym, 'dos2-fn-' + slug(title)) if a)
        add('dos2-fn-' + code, 'MSX-DOS 2: funcoes', 'Funcao %s (%s)%s: %s' % (code.upper(), sym.upper() or '-', '', title.title()), clean(L[i:stop]), book,
            aliases=aliases, usage='C = %s%s' % (code.upper().replace('H', 'H'), ('  (' + sym.upper() + ')') if sym else ''))

    # --- Z80, R800, Turbo-Basic, FM-PAC (paginas inteiras)
    _, body, _ = man['Z80 - Visao Geral / Extensoes R800']
    add('z80-overview', 'Z80', 'Z80: visao geral e extensoes do R800', body, 'Z80')
    _, body, _ = man['Z80 - Conjunto de Instrucoes (por codigo)']
    add('z80-opcodes', 'Z80', 'Z80: conjunto de instrucoes por codigo (tabela)', body, 'Z80', aliases='z80-instruction-set')
    _, body, _ = man['Z80 - Conjunto de Comandos (por mnemonico)']
    add('z80-mnemonics', 'Z80', 'Z80: conjunto de instrucoes por mnemonico (tabela)', body, 'Z80')
    _, body, _ = man['R800 - Instrucoes']
    add('r800-instructions', 'R800', 'R800: quadro das instrucoes (Appendix A)', body, 'R800')
    _, body, _ = man['R800 - Especifico MSX']
    add('r800-msx', 'R800', 'R800: particularidades no MSX turbo R', body, 'R800')
    _, body, _ = man['Turbo-Basic Compiler']
    add('turbo-basic-compiler', 'Turbo-Basic', 'Turbo-Basic Compiler (compilador de MSX-BASIC)', body, 'Turbo-Basic Compiler', aliases='turbo-basic')
    _, body, _ = man['FM-PAC']
    add('fmpac-manual', 'FM-PAC', 'FM-PAC: manual do cartucho FM Pana Amusement Cartridge', body, 'FM-PAC (Homesoft)', aliases='fmpac-manual')

    # ------------------------------------------------------------------ saida
    lines = []
    w = lines.append
    w('// fwMSX -- MAN: MSX-DOS 2, MSX-DOS 1/CP/M, Z80, R800, FM-PAC, Turbo-Basic e referencias do MSX-BASIC em ingles. ARQUIVO GERADO por')
    w('// tools/paleobasic/import_manuals_help.py a partir de third-party/paleobasic/src/editor/help/Msx{Software,Manuals}HelpData.pbi -- nao editar a mao.')
    w('//')
    w('// Origem: SOFTWARE.CHM e MANUALS.CHM do RuMSX (Lex Lechz, lexlechz.at), que reunem material publico (boa parte digitada por Konamiman a partir dos')
    w('// manuais oficiais: MSX-DOS 2, MSX2 Technical Handbook, Turbo-Basic Compiler). Texto em ingles, como no original. Ver doc/man-spec.md.')
    w('#include "man_internal.h"')
    w('')
    w('namespace man {')
    w('')
    w('std::vector<Topic> ManualTopics() {')
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

    used = set()
    for t in topics:
        base, k = t['name'], 2
        while t['name'] in used and False:
            t['name'] = '%s-%d' % (base, k)
            k += 1
        used.add(t['name'])
        w('    add(%s, %s, %s, %s, %s,' % (cstr(t['name']), cstr(t['aliases']), cstr(t['category']), cstr(t['usage']), cstr(t['summary'])))
        w('        ' + raw_str(t['page']) + ');')
        w('')
    w('    return v;')
    w('}')
    w('')
    w('} // namespace man')
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    cats = {}
    for t in topics:
        cats[t['category']] = cats.get(t['category'], 0) + 1
    print('software: %d topicos; manuais: %d topicos; total %d -> %s (%d KB)' % (n_soft, n_man, len(topics), OUT, os.path.getsize(OUT) // 1024))
    for c, n in sorted(cats.items()):
        print('  %-24s %d' % (c, n))


if __name__ == '__main__':
    sys.exit(main())
