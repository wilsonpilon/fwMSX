#!/usr/bin/env python3
"""Converte a ajuda de BIOS/BDOS/RAM e de hardware do paleobasic nas paginas do `man` do fwMSX.

Uso:  python tools/paleobasic/import_bios_help.py          (gera src/man/cpp/man_pages_msxbios.cpp)

Fontes (resource/paleobasic/src/editor/help/): BiosCallsHelpData.pbi (rotinas nomeadas de MainROM/SubROM/Disk-ROM/Hangul-ROM, BDOS e variaveis
de RAM), BiosDocHelpData.pbi (documentacao: sequencias ESC de impressora...) e HardwareHelpData.pbi (RTC, PSG, SCC, OPLL, MSX-Audio, MoonSound,
V9958, V9990, teclado, mapa de portas, Kanji, MIDI, SCSI...). O paleobasic as gerou do MSXBIOS.CHM do RuMSX (Lex Lechz, lexlechz.at), que por sua
vez reune material publico (MSX Assembly Page, manuais do blueMSX e do openMSX, livros da Aleph). O texto e' em INGLES e vai como no original.
Importacao unica: o resultado (man_pages_msxbios.cpp) e' que vai para o git e para o executavel.
"""
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('imp', os.path.join(HERE, 'import_basic_help.py'))
imp = importlib.util.module_from_spec(spec)
spec.loader.exec_module(imp)

OUT = os.path.join(imp.ROOT, 'src', 'man', 'cpp', 'man_pages_msxbios.cpp')

CATEGORY = {
    'MainROM': 'BIOS-ROM', 'SubROM': 'BIOS-ROM', 'Disk-ROM': 'BIOS-ROM', 'Hangul-ROM': 'BIOS-ROM', 'MSX-JE': 'BIOS-ROM', 'EXTBIO': 'BIOS-ROM',
    'BDOS': 'BDOS', 'RAM-variables': 'Variaveis RAM',
}
GROUP_NAME = {
    'MainROM': 'BIOS principal (MainROM)', 'SubROM': 'Sub-ROM do MSX2', 'Disk-ROM': 'ROM de disco (Disk-ROM)', 'Hangul-ROM': 'Hangul-ROM',
    'MSX-JE': 'MSX-JE', 'EXTBIO': 'EXTBIO', 'BDOS': 'BDOS (MSX-DOS)', 'RAM-variables': 'Variavel da area de trabalho (RAM)',
}
# apelidos curtos para o primeiro topico de cada grupo de hardware: man hw-psg, man hw-rtc...
HW_ALIAS = {
    'Clockchip (RTC)': 'hw-rtc,rtc', 'PSG': 'hw-psg', 'SCC': 'hw-scc', 'MSX-Music (OPLL, FM-PAC)': 'hw-opll,opll,msx-music,fmpac', 'MSX-Audio': 'hw-msx-audio,y8950',
    'MoonSound': 'hw-moonsound,opl4', 'MSX-Video (V9958)': 'hw-v9958,v9958', 'V9990 (VideoChip do futuro)': 'hw-v9990,v9990', 'Keyboard-Matrix': 'hw-keyboard',
    'I/O-Port map': 'hw-ports,hw-io', "Kanji ROM's": 'hw-kanji,kanji', 'PCM': 'hw-pcm', 'MIDI': 'hw-midi', 'SCSI Controller (WD33C93A)': 'hw-scsi,scsi',
    'Lightpen (Sanyo compatible)': 'hw-lightpen', 'Tape I/O': 'hw-tape', 'ROM (geral)': 'hw-rom',
}


def parse_topics(fname, prefix):
    calls = imp.parse_calls(imp.read(fname), [prefix + '_Begin', prefix + '_L', prefix + '_Commit'])
    topics, cur = [], []
    for name, a in calls:
        if name.endswith('_Begin'):
            cur = []
        elif name.endswith('_L'):
            cur.append(a[0])
        else:
            topics.append((a[0], a[1], '\n'.join(cur)))
            cur = []
    return topics


def clean(body):
    body = imp.ascii_only(body).replace('\r', '').replace('\t', '    ')
    return '\n'.join(l.rstrip() for l in body.split('\n')).strip('\n')


def slug(title):
    s = title.split(' - ')[-1] if ' - ' in title else title
    s = re.sub(r'^msx-bios:\s*', '', s, flags=re.I)
    s = re.sub(r'[^a-z0-9.$_]+', '-', s.lower()).strip('-')
    return s or 'topico'


def main():
    lines = []
    w = lines.append
    w('// fwMSX -- MAN: rotinas e variaveis da BIOS/BDOS e hardware do MSX. ARQUIVO GERADO por tools/paleobasic/import_bios_help.py a partir de')
    w('// resource/paleobasic/src/editor/help/{BiosCalls,BiosDoc,Hardware}HelpData.pbi -- nao editar a mao.')
    w('//')
    w('// Origem: MSXBIOS.CHM do RuMSX (Lex Lechz, lexlechz.at), que reune material publico (MSX Assembly Page, manuais do blueMSX e do openMSX,')
    w('// livros da Aleph). Texto em ingles, como no original. Ver doc/man-spec.md, secao "Origem e direitos".')
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

    # ------------------------------------------------------------ BIOS / BDOS / RAM
    w('std::vector<Topic> BiosTopics() {')
    w('    std::vector<Topic> v;')
    w('    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {')
    w('        v.push_back(Topic{name, aliases, category, usage, summary, page, false});')
    w('    };')
    w('')
    by_name, order = {}, []
    for titulo, grupo, corpo in parse_topics('BiosCallsHelpData.pbi', 'BiosCallsHelp'):
        key = titulo.lower()
        if key not in by_name:
            by_name[key] = []
            order.append(key)
        by_name[key].append((titulo, grupo, clean(corpo)))
    n_bios = 0
    for key in sorted(order):
        parts = by_name[key]
        first_title, first_group, first_body = parts[0]
        page = []
        for titulo, grupo, corpo in parts:
            if page:
                page.append('')
            page.append('AREA: ' + GROUP_NAME.get(grupo, grupo))
            page.append('')
            page.append(corpo)
        page.append('')
        page.append('FONTE')
        page.append('  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.')
        page.append('')
        page.append('VER TAMBEM\n  bios, sysvars, hooks')
        head = first_body.split('\n')[0].strip()
        rest = [l.strip() for l in first_body.split('\n')[1:] if l.strip()]
        summary = imp.first_sentence(rest[0]) if rest else imp.first_sentence(head)
        if len(head) > 70:
            head = head[:70].rsplit(' ', 1)[0]
        w('    add(%s, "", %s, %s, %s,' % (cstr(key), cstr(CATEGORY.get(first_group, 'BIOS-ROM')), cstr(head), cstr(summary)))
        w('        ' + raw('\n'.join(page)) + ');')
        w('')
        n_bios += 1
    for titulo, grupo, corpo in parse_topics('BiosDocHelpData.pbi', 'BiosDocHelp'):
        name = 'bios-doc-' + slug(titulo)
        first = clean(corpo).split('\n')[0].strip()
        w('    add(%s, "", "Doc BIOS", "", %s,' % (cstr(name), cstr(imp.first_sentence(first))))
        w('        ' + raw(clean(corpo) + '\n\nFONTE\n  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.') + ');')
        w('')
        n_bios += 1
    w('    return v;')
    w('}')
    w('')

    # ------------------------------------------------------------ hardware
    w('std::vector<Topic> HardwareTopics() {')
    w('    std::vector<Topic> v;')
    w('    auto add = [&](const char *name, const char *aliases, const char *category, const char *usage, const char *summary, const char *page) {')
    w('        v.push_back(Topic{name, aliases, category, usage, summary, page, false});')
    w('    };')
    w('')
    used, seen_group, n_hw = set(), set(), 0
    for titulo, grupo, corpo in parse_topics('HardwareHelpData.pbi', 'HardwareHelp'):
        name = slug(titulo)
        base, k = name, 2
        while name in used:
            name = '%s-%d' % (base, k)
            k += 1
        used.add(name)
        body = clean(corpo)
        aliases = ''
        if grupo not in seen_group:
            seen_group.add(grupo)
            aliases = HW_ALIAS.get(grupo, '')
        first = ''
        for l in body.split('\n'):
            if l.strip():
                first = l.strip()
                break
        page = ['HARDWARE: ' + grupo, titulo, '', body, '', 'FONTE', '  MSXBIOS.CHM do RuMSX (Lex Lechz); texto em ingles, como no original.']
        w('    add(%s, %s, "Hardware", "", %s,' % (cstr(name), cstr(aliases), cstr(imp.first_sentence(titulo + ': ' + first, 90))))
        w('        ' + raw('\n'.join(page)) + ');')
        w('')
        n_hw += 1
    w('    return v;')
    w('}')
    w('')
    w('} // namespace man')
    with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')
    print('bios/bdos/ram: %d topicos (de %d verbetes), hardware: %d -> %s' % (n_bios, sum(len(v) for v in by_name.values()), n_hw, OUT))
    print('hardware:', ' | '.join(sorted(used)))


if __name__ == '__main__':
    sys.exit(main())
