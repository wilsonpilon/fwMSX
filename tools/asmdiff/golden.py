"""Formato dos arquivos golden do montador (tests/asm/golden/*.golden).

    @case <nome>
    @args <argumentos da linha de comando>       (opcional)
    @file <nome> <hex>                            (opcional, zero ou mais: .include/.incbin)
    @src
    <linhas do fonte>
    @rc <codigo de saida do asMSX>
    @msg <uma mensagem de erro/aviso>             (zero ou mais)
    @out <arquivo> <hex>                          (uma por arquivo gerado; "z80", "sym"... = x.<ext>)
    @end

Os `.golden` sao gerados pelo asMSX de verdade (oraculo) e lidos pelo teste do fwMSX, que assim roda
sem o oraculo (e no Linux).
"""


def write_golden(path, cases):
    """cases: lista de (nome, fonte, resultado) ou (nome, fonte, resultado, files, args)."""
    with open(path, "w", newline="\n") as f:
        for case in cases:
            name, source, res = case[0], case[1], case[2]
            files = case[3] if len(case) > 3 else {}
            args = case[4] if len(case) > 4 else []
            f.write("@case %s\n" % name)
            if args:
                f.write("@args %s\n" % " ".join(args))
            for fname, data in sorted(files.items()):
                f.write("@file %s %s\n" % (fname, (data if isinstance(data, bytes) else data.encode("latin-1")).hex()))
            f.write("@src\n%s" % (source if source.endswith("\n") else source + "\n"))
            f.write("@rc %d\n" % res["rc"])
            for m in res["msgs"]:
                f.write("@msg %s\n" % m)
            for ext, data in sorted(res["outs"].items()):
                f.write("@out %s %s\n" % (ext.replace(" ", "%20"), data.hex()))
            f.write("@end\n")
