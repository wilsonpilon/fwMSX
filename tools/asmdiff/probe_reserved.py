#!/usr/bin/env python3
"""Descobre (caixa-preta, no asMSX de verdade) quais palavras sao reservadas, isto e, nao podem ser usadas
como rotulo. Saida: lista das reservadas, uma por linha, em tests/asm/golden/reserved.txt."""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from oracle import run_many

WORDS = """
a b c d e h l f i r af bc de hl sp ix iy ixh ixl iyh iyl af'
nz z nc po pe p m
adc add and bit call ccf cp cpd cpdr cpi cpir cpl daa dec di djnz ei ex exx halt im in inc ind indr ini inir
jp jr ld ldd lddr ldi ldir neg nop or otdr otir out outd outi pop push res ret reti retn rl rla rlc rlca rld
rr rra rrc rrca rrd rst sbc scf set sla sll sra srl sub xor
db dw ds dt defb defw defs deft byte word
org page equ zilog bios biosvars rom megarom basic msxdos start search subpage select phase dephase
callbios calldos size include incbin random debug break breakpoint rept endr macro endm
printtext printstring print printdec printhex printfix cas cassette wav filename sinclair skip
if else endif ifdef ifndef endif
sin cos tan acos asin atan sqr sqrt exp pow log ln abs fix fixmul fixdiv int pi not
konami konamiscc ascii8 ascii16 at
mod and or xor shl shr
chkram synchr rdslt chrgtr wrslt outdo calslt dcompr enaslt getypr callf keyint initio inifnk disscr enascr wrtvdp
rdvrm wrtvrm setrd setwrt filvrm ldirmv ldirvm chgmod chgclr nmi clrspr initxt init32 inigrp inimlt settxt sett32
setgrp setmlt calpat calatr gspsiz grpprt gicini wrtpsg rdpsg strtms chsns chget chput lptout lptstt cnvchr pinlin
inlin qinlin breakx iscntc ckcntc beep cls posit fnksb erafnk dspfnk totext gtstck gttrig gtpad gtpdl tapion tapin
tapiof tapoon tapout tapoof stmotr lftq putq rightc leftc upc tupc downc tdownc scalxy mapxyc fetchc storec setatr
readc setc nsetcx gtaspc pntini scanr scanl chgcap chgsnd rslreg wslreg rdvdp snsmat phydio format isflio outdlp
getvcp getvc2 kilbuf calbas subrom extrom chkslz chknew eol bigfil nsetrd nstwrt nrdvrm nwrvrm rdbtst wrbtst
chgcpu getcpu pcmply pcmrec
cgtabl vdp_dr vdp_dw msxid1 rdprim wrprim clprim linl40 txtnam clikfl jiffy exptbl scrmod
""".split()


def main():
    words = sorted(set(WORDS))
    res = run_many(["%s: nop\n" % w for w in words])
    reserved = [w for w, r in zip(words, res) if any("reserved" in m for m in r["msgs"]) or r["rc"] != 0]
    other_err = {}
    for w, r in zip(words, res):
        if r["rc"] != 0:
            other_err[w] = (r["rc"], r["msgs"][:2])
    out = os.path.join(os.path.dirname(__file__), "..", "..", "tests", "asm", "golden", "reserved.txt")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", newline="\n") as f:
        for w in words:
            r = res[words.index(w)]
            f.write("%s rc=%d %s\n" % (w, r["rc"], " | ".join(r["msgs"][:1])))
    print("reservadas/erro:", len(reserved), "de", len(words))


if __name__ == "__main__":
    main()
