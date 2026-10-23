OCM-MegaSDHC drivers for Nextor
===============================

This folder contains the OCM-MegaSDHC disk-driver sources, the two Nextor
kernel bases used by this package, and the Windows build scripts required to
generate all four ROM images.

Driver releases
---------------
- v1.4: 2026-09-01 (file date code: 20260901)
- v1.5: 2026-09-02 (file date code: 20260902)

ROM targets
-----------
- Nextor kernel 2.1.4 + OCM-MegaSDHC driver v1.4 (filename: nextsd14.rom)
- Nextor kernel 2.1.4 + OCM-MegaSDHC driver v1.5 (filename: nextsd15.rom)
- Nextor kernel 3.0 beta 2 + OCM-MegaSDHC driver v1.4 (filename: n3xtsd14.rom)
- Nextor kernel 3.0 beta 2 + OCM-MegaSDHC driver v1.5 (filename: n3xtsd15.rom)

Folder layout
-------------
base\
  Nextor kernel bases. The Nextor 3 base already contains the documented
  silent-boot patch. See nextor3_silent_boot_patch.txt.

asm\
  Four Z80 assembler sources. They are assembled with Sjasm v0.42c. The
  512-LDI transfer blocks use the native REPEAT / ENDREPEAT loop syntax.

output\
  Generated 128 KB ROM images. This folder is created/updated by
  make-roms.cmd.

make-roms.cmd / make-roms.ps1
  Windows build entry point and ROM-composition script. The entry point calls
  the supplied dev\sjasm\make-rom.cmd helper for Z80 assembly.

Assembler
---------
The build uses the bundled original Sjasm v0.42c executable at the fixed path:

  ..\..\dev\sjasm\sjasm.exe

and the supplied assembler helper at:

  ..\..\dev\sjasm\make-rom.cmd

The assembler path is intentionally not changed from the supplied OCM-PLD
development-tree layout.

Build on Windows
----------------
1. Keep the dev and src folders in their supplied relative positions.
2. Open 'src\20261002 ocm-megasdhc disk drivers for nextor\'.
3. Run:

     make-roms.cmd

Windows PowerShell is used only for ROM composition and validation. Z80 source
assembly is performed exclusively by Sjasm v0.42c.

The builder rejects Sjasm source diagnostics before ROM composition, then
checks the 128 KB ROM size, the 48-byte OCM ASCII8 bank-switch module in all
eight 16 KB banks, the FF mapper padding, and the Nextor 3 silent-boot patch.

Bank switching
--------------
The OCM-MegaSDHC ROMs use ASCII8 bank switching. Each logical 16 KB Nextor
bank is selected as two consecutive 8 KB pages through 6000h and 6800h.
The canonical 48-byte bank-switch tail starts with:

  07 32 00 60 3C 32 00 68 C9

and the remaining 39 bytes are FF.


______________
KdL 2026.10.02
