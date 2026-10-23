FM PCM Player
=============

Copyright 2020 Laurens Holst

Thanks go to D.S.A., Nuke.YKT and WouterV for support.

Project information
-------------------

Plays back PCM files on the YM2413 OPLL MSX-MUSIC sound chip.

* Author: Laurens Holst <laurens@grauw.nl>
* Site: <http://www.grauw.nl/projects/fmpcm/>
* Source: <https://hg.sr.ht/~grauw/fmpcm>
* Support: <http://www.msx.org/forum/msx-talk/software/fm-pcm-player>
* License: Simplified BSD License

This player plays 15.7 kHz or 7.85 kHz PCM on the MSX-MUSIC. The PCM file must
be 8-bit raw PCM data at one of the aforementioned frequencies. By default the
file is interpreted signed, but there is an option to provide unsigned data.
Since 15.7 kHz requires nearly 1 MB per minute, you do need sufficient memory
to fit the sample. If the sample doesn’t fit it will be truncated.

It works by forcing the phase generator to hold on a specified phase using test
register bit 2. This allows it to index into the sine table and produce
accurate near 8-bit sample playback with a simple inverse lookup table.

The playback is timed by synchronizing to the VDP line drawing frequency, which
provides a good high-frequency timer on MSX2 that is CPU speed independent.
Additionally it lets us render a nice visualisation during playback.


System requirements
-------------------

  * MSX2, MSX2+ or MSX turboR
  * 128K main RAM (several MB recommended)
  * 64K video RAM
  * MSX-DOS 2
  * MSX-MUSIC with real YM2413, or very accurate emulation


Usage instructions
------------------

Run FMPCM from MSX-DOS 2, specifying a raw 8-bit PCM file on the command line.
Wildcards and multiple files are supported, they are played in sequence.

Usage:

    fmpcm [options] <file.raw>...

Options:

  * `/l` Number of playback loops. Default: 1.

    How many times the sample should be repeated. The amount is specified like
    `/L15`. Use `/L` or `/L0` to loop infinitely.

  * `/q` Suppress messages and visualisation.

    Completely prevents any visual output, except for errors.

  * `/v` Disable visualisation.

    Does not show the visualisation during playback.

  * `/r` Prefer TurboR PCM.

    Plays the PCM data on turboR PCM if available. For testing and for use with
    incompletely emulated YM2413s which do not support the test register.

  * `/h` Half frequency (7850 Hz).

    Plays the PCM data at a frequency of 7850 Hz, half the base frequency.

  * `/u` Unsigned PCM.

    Interprets the provided PCM data as unsigned.

To configure Multi Mente to play PCM files, add the following lines to
MMRET.DAT:

    .RAW    FMPCM $M
    .PCM    FMPCM $M


Development information
-----------------------

FM PCM Player is free and open source software. If you want to contribute to
the project you are very welcome to. Please contact me at any one of the
places mentioned in the project information section.

You are also free to re-use code for your own projects, provided you abide by
the license terms.

FM PCM Player depends on the library project Neonlib; it is recommended to
clone the project with Mercurial so they will automatically be pulled in at the
correct version, otherwise you have to download them manually:

    hg clone https://hg.sr.ht/~grauw/fmpcm

Building the project is really easy on all modern desktop platforms. On MacOS
and Linux, simply invoke `make` to build the binary and symbol files into the
`bin` directory:

    make

Windows users can open the `Makefile` and build by pasting the line in the `all`
target into the Windows command prompt.

To launch the build in openMSX after building, put a copy of `MSXDOS2.SYS` and
`COMMAND2.COM` and some PCM files to test with in the `bin` directory, and then
invoke the `make run` command.

Note that the [glass](http://www.grauw.nl/projects/glass/) assembler which is
embedded in the project requires [Java 8](http://java.com/getjava). To check
your Java version, invoke the `java -version` command.


Patched Build: 2025.05.04 by KdL
--------------------------------

Base Commit: 08c41681ce5c
https://hg.sr.ht/~grauw/fmpcm/rev/08c41681ce5c87ef23a7d67a37cbd1d9a247dba5

[src/Application.asm]
Application_welcome:
    db "FM PCM Player 1.0 by Grauw",13,10,10
    db "Patched Build: 2025.05.04 by KdL",13,10,10,0

Application_usageInstructions:
    db "Usage: fmpcm [options] <file.raw>...",13,10
    db 10
    db "Options:",13,10
    db "  /q  No messages and visualisation.",13,10
    db "  /v  Disable visualisation.",13,10
    db "  /r  Prefer TurboR PCM (also MSX++).",29,13,10     ; MODE 37 compliant
    db "  /h  Half frequency (7850 Hz).",13,10
    db "  /u  Unsigned PCM.",13,10,0

[src/drivers/TurboRPCM.asm]
; ix = this
TurboRPCM_Construct:
;   call TurboRPCM_Detect
;   jp nc,Driver_SetNotFound
    ld a,80H
    out (TurboRPCM_DATA),a
    ld a,00000011B
    out (TurboRPCM_CONTROL),a
    ret

; f <- c: found
;TurboRPCM_Detect:
;   ld hl,IDBYT2
;   call System_ReadBIOS
;   add a,-3
;   ret
