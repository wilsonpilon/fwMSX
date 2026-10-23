# MegaSD partition tool

In 8.3 naming: `MSDPAR.COM`.

Based on `PARSET` v1.1 & `PARLIST` by Konamiman, 1998-1999.


## Purpose
On OCM/MSX++ with MegaSD: set a partition, determine which one is set, or list the disk's partitions.

## Support, compatibility & limitations
Works on both MSX-DOS1 and MSX-DOS2.
DOS1 only supports FAT12.

FAT16 partition support in MegaSD is limited - see section _"FAT12 vs. FAT16"_.
This tool cannot improve upon that.

This tool will NOT create partition tables.
For that, use Nextor `_FDISK` in BASIC (after booting to Nextor) or any modern computer and a partition tool capable of
partitioning MBRs & formatting FAT12.
(Note that I only could get MSX-DOS1 to start on a FAT12 partition formatted by Nextor `_FDISK`.
Linux `mkfs.fat -F 12` creates an incompatible boot sector.)

This tool is NOT compatible with MegaSCSI (because it uses MMC SPI)
nor with MegaFlashROM SCC+ SD (because OCM's ESE-RAM and MegaFlashROM have incompatible ways of enabling writes).

This tool does NOT work on Nextor. For Nextor, use its own tool `MAPDRV`.

### Disclaimer
This program comes with ABSOLUTELY NO WARRANTY.
See COPYING for more information.


## Usage
Issue one of the following commands:
```
TYPE MSDPAR.COM                  # show name & version
MSDPAR [/?|H]                    # show usage
MSDPAR [<drive>]:                # show mapped partition
MSDPAR [<drive>:]a|p-<0|e> [/Q]  # map partition
MSDPAR /L                        # list disk partition
MSDPAR [<drive>:] /Ei            # Enable i MegaSD drives (i=1-8)
```

Partition selection:
* `a`: absolute partition number 1-255
* `p-0`: primary partition 1-4
* `p-e`: logical partition, e.g., `3-2` to select second extended partition in primary partition 3.

Options:
* `/Q`: quiet - don't print anything
* `/Ei`: enable _i_ drives, with _i_ being a number between 1 and 8 (inclusive).
    The drive parameter, when specified, must indicate a drive managed by MegaSD.
  * by default, 1 or 2 drives (`A:` & `B:`) are enabled, depending on the EPBIOS/SDBIOS in use.
  * per drive, just 21 Bytes extra memory is used (both in BASIC - `PRINT FRE(0)` and in DOS - [`TPAMEM`](https://www.msx.org/wiki/TPAMEM))
    * even with 8 drives enabled, there is still more memory available than 'classic' DOS1-with-Ctrl-held-at-boot
  * then execute `RESET` (warm boot) to activate the drive change.
    E.g., `msdpar /e6^reset` performs a warm boot directly after enabling 6 drives.
    Don't power off or cold boot (don't use the reset button); all MegaSD configuration will then revert to default.
  * until the newly enabled drives are mapped to a partition,
    `MSDPAR <drive>:` will show `*** MegaSD error: Device not ready` and return user error 34.
    Reason is that, by default, each drive points to a successive MegaSD device
    (`B:` => 1, `C:` => 2, .., `H:` => 7) and OCM-PLD supports only device ID 0.
    That id is set when _mapping_ a partition.
  * this can also be done with the original `ESET.COM`, but that's more cumbersome

### Default mapping
When starting up, MegaSD scans the partition table and maps the first partition.
When that is not FAT12 or FAT16, the system will not boot (`Disk error reading drive A`)!
Note: MegaSD sets the partition length to the maximum value of FF.FFFFh, which (with a sector size of 512 Bytes) is almost 8GiB.
And not even possible with FAT16, which supports 4GiB max!

`MSDPAR A:` will therefore print `WARNING: Drive size and partition size are not equal`.
This only happens for the drive automapped by MegaSD; not for partitions mapped by `MSDPAR`.

It doesn't seem to be harmful.
If partition 1 is FAT12, the warning can be resolved by `MSDPAR A:1-0`.

### FAT12 vs. FAT16
Mapping multiple partitions, all FAT12, works fine.
Remember, FAT12 partitions can be max. 32MiB in size.

However, when mapping any FAT16 partition (4GiB max) and then accessing it, MegaSD behaves in an unexpected way:
both the mapped drive and `A:` will point to the _first partition on disk_ (`1-0`),
irrespective whether it's FAT12 or FAT16.
This means that if your first partition is FAT12, you cannot access any FAT16 partition at all, since the mapping is undone by MegaSD.

Recommended usage patterns:
* One 4GiB FAT16 partition (mapped to `A:` at boot), followed but as many FAT12's as you need (well, 254 are mappable to `A:`~`H:` after boot using `MSDPAR`)
* Only FAT12 partitions (max 255 of 'em) - if you want to be able to boot to MSX-DOS1

This means it's cumbersome to use much more space than 4GiB.
Use Nextor if you need more flexibility or storage.

### MSX-DOS1
How to boot to and use MSX-DOS1:
1. format a micro-SD card using Nextor BASIC `_FDISK` with all partitions you want to access max 16MiB in size (32MiB appears unsupported)
2. copy `MSXDOS.SYS` & `COMMAND.COM` to the first partition
3. boot - DOS1 should start and show the `A>` prompt
4. use `MSDPAR /Ei` with i=2~6 to support the required drives (`B:`~`F:`)
    * 7 or 8 drives seems unsupported
    * under DOS1, each drive uses not 21 but 1558 Bytes of RAM! For 6 drives, only 17.204 KiB is left in BASIC.
5. do a soft reset using a 4-Byte tool you can create yourself in MSX BASIC using [`OPEN"RESET.COM"FOROUTPUTAS#1:?#1,CHR$(247);CHR$(128);STRING$(2,0):CLOSE`](https://www.msx.org/forum/msx-talk/general-discussion/soft-reset)
  (code by NYYRIKKI - also included in this repo, minus the superfluous trailing CR/LF/EOF)
6. use `MSDPAR` to map any other partitions to drive `B:`~`F:`

Note: if `MSXDOS2.SYS` & `COMMAND2.COM` are also on the partition,
then do all the steps above **in MSX-DOS2**. Finally:
7. execute `RESET`, then hold the '1'-key at boot to force MSX-DOS1

See section 'Bugs' for an explanation.

Remember: DOS1 supports max. 112 files per partition, and no subdirectories!

### Hardware
The Device ID is always 0 for MegaSD.
(Line 231 of OCM-PLD `megasd.vhd.390`: `.. and MmcMod(0) = '0' ..` although line 251 (`MmcMod := dbo(1 downto 0)`) seems to accept id 0-3?)

The MegaSD slot is autodetected based on the specified drive.
For those interested: it'll be slot 1-3 or 2-3 using openMSX, and 3-2 using OCM-PLD.

### Reset & reboot
OCM/MSX++ does not have persistent memory like MegaSCSI's ESE-RAM.
Any mappings or configuration made will only survive a soft reset,
e.g. by invoking [`RESET` in `COMMAND2.COM` v2.40](https://www.msx.org/wiki/MSX-DOS_Commands),
or by pressing left-CTRL + F12 on OCM-PLD 3.9.2.
Except drive `A:`: that's always reinitialized to the first partition on disk, even after soft reset.

Mappings (`B:`~`H:`) & config will, however, _not_ survive a cold boot/hard reset or power down.
MegaSD, like Nextor, maps the first supported partition it finds to `A:` at boot.
No other partitions are automapped.

Consider invoking `MSDPAR` with option `/Q` in `AUTOEXEC.BAT` to (quietly) map additional partitions at boot.

### Error handling
Errors are printed to screen, unless option `/Q` is provided.

Under DOS2, a user error code is also returned:
* 1 (silent): an error situation that led to showing usage instructions
* 2 (silent): invalid input or mismatch with disk configuration
* 3 (silent): internal error
* 32 or higher (printed by DOS): MegaSCSI API error

Use `ECHO %_ERROR%` to show the error code, or use it in a batch file.


## Using original `PARSET` & `PARLIST` on OCM/MSX++ with MegaSD
`PARLIST` isn't usable:
* `PARLIST 0` shows: `*** MEGASCSI ERROR: Unknown error (code #C0)`
* `PARLIST 1` through 7 shows
  * garbage for device name & manufacturer
  * after partitions header: `*** MEGASCSI ERROR: Device not ready`

`PARSET` works somewhat:
* `PARSET :`: mentions slot 3-2, device id 0, name/manufacturer: garbage, partition 1-0 & warning about drive and partition size not equal
* `PARSET B:` through `H:` shows (as expected): `*** ERROR: The specified drive does not exist`
* `PARSET B:2-1 0` shows `Partition set successfully!`
  * though this must be preceded with either `MSDPAR /E2^RESET` or by using `ESET.COM` to enable drive `B:`
  * ... don't forget the final `0` to indicate device ID 0!
  * afterwards, `PARSET B:` shows: slot 3-2, id 0, garbage name+manuf, partition 2-1
  * ... and indeed, the partition is accessible


## Bugs
* `MSDPAR` hangs if DOS1 & DOS2 files are both present
   on the boot partition and '1' is pressed to force booting to MSX-DOS1.
   If anyone knows why this happens, let me know.
  * workaround: remove or rename `MSXDOS2.SYS` (and optionally `COMMAND2.COM`)
  * the hang occurs when printing text to screen -
    invocation of _DOS (#0005) function _STROUT (#09)


## Changelog
* 23-Aug-2024: MSDPAR v1.0d
  * proper bugfix for reading of CID at `MSDPAR <drive>:`
* 23-Jun-2024: MSDPAR v1.0c
  * bugfix for reading of CID at `MSDPAR <drive>:`
  * bug description for usage on DOS1
* 7-Jun-2024: MSDPAR v1.0b
  * support DOS1
* 5-Jun-2024: MSDPAR v1.0a
  * bugfix for MSDPAR sector start & count formatting
* 2-Jun-2024: MSDPAR v1.0
  * Translate Spanish to English (comments, macros & most labels)
  * port to sjasm v0.42c+GNU make
  * require MegaSD signature with version: "MEGASCSI ver2.15"
  * default device ID 0; the only one supported on OCM/MSX++ with MegaSD BIOS
  * drop ESE-RAMdisk handling; not implemented on OCM/MSX++
  * merge in PARLIST for partition listing
  * replace SCSI Inquiry by MMC command 10 (SEND_CID)
  * define constants & add URLs to information
  * support `TYPE MSDPAR.COM`
  * support LBA partition types, in sync with Nextor 2.1.2
  * some command line options
  * option to set # drives
  * size grows from 4494 to about 6K
* 1999: PARSET for Mega-SCSI v1.1 created by Néstor Soriano (Konami Man)
* 1998: PARLIST for Mega-SCSI created by Néstor Soriano (Konami Man)


## Nice to have
These features might be added in the future:
* when mapping `A:`, print a warning that user should ensure `COMMAND(2).COM` is present on newly mapped partition
* show warning when mapping a FAT16 partition
* detect mapping a partition twice; print a warning
* `/L`: indicate drive for partitions that are mapped
* starting `MSDPAR` from BASIC via `BLOAD` or `CALL`.
* option to show all mapped drives at once
* support `/[R]eset` option for `/Ei` (drive count adjust)
* report # active drives


## License
Everything in this repository is GPLv3.
See COPYING for more information.


## Developing
MSDPAR is developed on Linux using Sjasm 0.42c (patched to compile on Linux) and GNU make.

Execute `make` to compile.

Any feedback on my Z80 assembly code style?
Please send PRs, with comments.
It's almost 35 years back since I coded Z80, and didn't really develop a code style back then.
I focused on understandability, hence the equs & URLs.

## Testing
With some modifications, openMSX extension 'MegaFlashROM_SCC+_SD' will behave like OCM/MSX++ MegaSD.
This enables testing & debugging on that emulator.
See [OpenMSX MegaSD extension for OCM](../OpenMSX-MegaSD-extension/readme.md) in this repository.

File `test-images.zip` contains two 1-MiB files with skeleton 'SD card' images with various (tiny & unformatted) partitions to verify `MSDPAR /L`.

Execute `make run` to start openMSX.
In the default setup, your local folder is mapped to drive `C:`, so `C:MSDPAR` should start your compiled code.

⚠️ With the standard openMSX extension 'MegaFlashROM_SCC+_SD', MegaSD will not boot at all.
So you will need to compile openMSX yourself.

`MSDPAR` is tested on a modified openMSX emulator,
and on a real SX1-mini+ with OCM-PLD 3.9
and a real SX2 with OCM-PLD 3.9.1, both with their predelivered EPBIOSes.
