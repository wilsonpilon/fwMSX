----------------------------------------------------------------
    M File Manager v4.7 for MSX-DOS2 (2025-06-25)
        By Yeongman Seo <sharksym@hitel.net>
----------------------------------------------------------------

* UPDATE

  @ Changes/Improvements

  - In Normal mode, memory‐mapper usage has increased from 192KB to 224KB.
    Lite mode remains unchanged.

  - Removed IMS BNK setting from M.INI.
    The default BNK file is STANDARD.BNK—no change from before.

  - When entering text, pressing the TAB key inserts the path of the target window.

  - After Rename or Mkdir operations, the cursor moves to the affected file/directory.

  - LFN mode improvements:
    The left window is displayed wider so you can view all long filenames at once.


  @ New Features

  - WAU Browser (Tiny Browser)
    Browse the following sites via built-in commands:
    #WAUB GitHub <account>
    #WAUB MSX-Archive

    A CACHE.WAU subdirectory is created under the M directory.
    It stores temporary files and site cache.
    To delete temporary and GitHub cache files, use:
    #WAUB Cleanup

    MSX-Archive cache files must be deleted manually.
    If you have sufficient disk space, it’s recommended to leave them.


----------------------------------------------------------------
* M File Manager

  A file manager for MSX-DOS2 inspired by Mdir and NC (Norton Commander),
   both MS-DOS programs from the 90s.

  If the remaining capacity of the memory mapper is 224KB or more, it runs in normal mode.
  If the remaining capacity is between 128KB and 192KB, it runs in Lite mode.
  In Lite mode, some functions are disabled:
  Interlace mode, Korean processing, LFN display, IMS player, MPX F/W.
  If the remaining capacity is 112KB or less, M does not run.

  It supports copying, moving, deleting, changing attributes of files and directories.
  When copying/moving, the source directory is the panel with the cursor,
  and the target directory is the other panel without the cursor.
  (e.g., if you place the cursor on a file in the left panel and issue a copy command,
       the file is copied to the directory in the right panel.)

  It has various program launcher functions.
  You can select the execution program by function key or extension.
  For more details, refer to the "* M.INI settings" section.

  In interlace mode, it can output complete Korean (KS-5601),
   and in non-interlace mode, it outputs only ASCII characters.
  You can use resident fonts for special characters and Chinese characters.
  (e.g., KS_SPE.COM, KS_HANJA.COM)

  If you use the Korean FontPack cartridge,
   you can also display extended complete Korean (CP949) and Chinese characters.

  It can display VFAT's LFN (long file names).
  To handle Unicode, you need the FontPack V1/V2 cartridge.
  English (U+0000 ~ U+00FF) can be handled without the FontPack.

  You can use the built-in sound and graphic-related functions.
  (e.g., IMS Player, MP3/WMA Player, Graphic Viewer)
  When playing music, it displays ID3v1, ID3v2, WMA, GD3, PT2/3 tags.
  (FontPack is required for ID3v2 processing)

  When streaming module music (MOD, S3M, XM, etc.) from the WAU Remote Drive,
  displays the title tags.

  Includes the WAU Tiny Browser.
  Supports GitHub, MSX-Archive, and File-Hunter sites.


----------------------------------------------------------------
* Requirements

  Common: MSX2, MSX-DOS2, VRAM 128KB

  Normal mode: Uses 224KB of main memory mapper
               All functions available

  Lite mode: Uses 128KB of main memory mapper

  Note: Does not support auxiliary memory mapper.
        Therefore, even if you install a memory mapper cartridge in FS-A1ST,
        M will display a memory shortage error and will not run.
        In this case, you need to expand the built-in memory to 512KB or more.


* Optional Items

  Korean FontPack V1/V2 cartridge (CP949, Unicode display)
  MPX R1.x/R2.0 and WAU cartridge (MP3, WMA audio player)
  MoonSound (IMS audio player)
  PAC-V/XII-V cartridge (LED visualizer)
  WAU Cartridge (PC Remote Drive, Tiny Browser)  


----------------------------------------------------------------
* Installation Method

  1. Unzip the compressed file into the M directory.
  2. Edit the M.INI file to suit your environment.
  3. Add the M directory to the PATH setting in AUTOEXEC.BAT.


* Execution Method

  Type M [RETURN] at the DOS2 prompt to execute.

  M operates in a resident state in RAM by default.
  To release the resident state, type M /R at the DOS2 prompt.
  In the M.INI file, you can turn off the resident function by setting TSR:0.

  For an explanation of the operation keys in the file viewer,
   execute #HELP in M. (default registered to F1 key)

  The shortcut commands combined with the CTRL key,
   are briefly displayed at the bottom when you press the CTRL key in M.


----------------------------------------------------------------
* Shortcut List

  The ^ symbol in the key list means CTRL key, and # means SHIFT key.

  #ESC    : Exit M and return to DOS2
   ESC    : View DOS2 prompt screen / Cancel command
   HOME   : Quick-Dir menu

   TAB    : Move cursor between two panels
   UP     : Move cursor to the previous item
  ^UP     : Move cursor to the first item
   DOWN   : Move cursor to the next item
  ^DOWN   : Move cursor to the last item
   LEFT   : Move cursor to the previous page
   RIGHT  : Move cursor to the next page

   RETURN : Execute file / Open subdirectory / Open drive
  ^RETURN : Open in target panel (subdirectory, drive)
  #RETURN : Display file or directory attributes

  F1 ~ F5 : Execute function key menu (#:F6~F10, #^:F11~F15)
  #A ~ #H : Quickly open drive (A,B,C ... H)
   \      : Move to root directory
  #\      : Move to M directory
   BS     : Move to parent directory
   /      : Open DOS command line
  #/      : Open DOS command line and paste selected file

   SPACE  : Select or deselect file (hold down to continuously select downward)
  #SPACE  : Select or deselect all files
  #UP     : Select current file and move cursor to the previous item
  #DOWN   : Select current file and move cursor to the next item

  ^C | INS: Copy file or directory
  ^M      : Move file or directory
  ^S      : Change list sorting method
  ^D | DEL: Delete file or directory (excluding ReadOnly attribute)
  #DEL    : Delete file or directory (including ReadOnly attribute)
  ^K      : Create directory
  ^R      : Rename file or directory

  ^B      : Display 'about' window
  ^A      : Compress selected file to LZH
  ^E      : Decompress LZH file
  ^I      : Switch between interlace and non-interlace mode
  ^N      : Switch between black and white, color mode
  ^V      : Switch V-Sync 60Hz, 50Hz mode
  ^L      : Switch LFN (long file name) mode


* File/Directory Name Search

  When you type a name on the keyboard,
   the cursor moves to the item that starts with that name.

  To cancel the current name being typed, press the ESC key.
   Or move the cursor with the UP/DOWN/TAB keys, etc.


----------------------------------------------------------------
* M.INI Settings

  User environment settings are saved in the M.INI file.
  Set the execution program by function key and file extension.
  For detailed explanations, refer to the comments inside the M.INI file.

  You can specify the text color (using hexadecimal) by extension,
   and you can also change the background color.
  The default color of the file is gray (E), and the background color is blue (BG:4).

  When setting the program, %s is replaced with the selected file/directory name,
  and %f is replaced with the name excluding the extension.
  You can list multiple commands with a semicolon.
  e.g., TXT:5:HV %s;APED %s
        MP3:9:#MPXP %s;MPXTOOL %s /v2

  The TSR (resident) option is enabled by default.
  If the main memory is 256KB or less, it is better to disable TSR.
  To disable the TSR function, set TSR:0 in the <OPT> group.

  You can check the memory usage of M.INI by pressing CTRL + B.
  The maximum setting data of M.INI is 4608 bytes.


* Temporary File Creation

  When processing LZH compressed files, a _M$.BAT file is temporarily created.
  It is deleted after normal operation, so please note.


* Command Prefix

  *: When executing a command/executable file,
     M releases the memory it occupies and then runs the program.
     (Useful in environments with insufficient memory.)
     When returning to M, it resides in memory again.
     e.g., *MEMORY

  ]: When you enter a command/executable file with Full Path,
     it moves to the directory and then runs the program.
     When returning to M, it returns to the original directory.
     e.g., ]B:\M\APED M.INI

  /: When executing a command/executable file,
     it does not switch to DOS text mode and maintains the M screen.
     Convenient when running programs that do not use graphics.
     e.g., /MMCSD MUSIC.WAV


* Built-in Programs

  #HELP: Display help (shows M.HLP)

  #CDD : Move drive/directory
         It is convenient to register frequently used directories to function keys.
         e.g., #CDD B:\UTILS;#CDD C:\GAME\ROM

  #CPU0: Change turboR to Z80 mode
  #CPU1: Change turboR to R800 ROM mode
  #CPU2: Change turboR to R800 DRAM mode

  #WAUB: Tiny browser using the WAU cartridge.
         You can browse the following sites.
         #WAUB GitHub <account>
         #WAUB MSX-Archive

  #MPXP: MP3/WMA player using MPX R1.x/R2.0 and WAU cartridges.
         Displays MP3/WMA progress bar at the bottom of the screen,
         Supports MP3 ID3v1/v2 tags and WMA info tags.
         Supports PAC-V/XII-V LED visualizer.

  #IMSP: IMS music player using MoonSound cartridge.
         (Not available in Lite mode)
         Supports PAC-V/XII-V LED visualizer.
         Lyrics (ISS) display is also possible on turboR.

  #VGMP: VGM player using external program VGMPlay.
         Supports GD3 tags.

  #PT3P: PT3 player using external program PT3Play.
         Supports displaying song information.

  #GV  : Graphic viewer
         Supports GE5/7, SRn, Snn, SCn, PIC, YJK, ILn, SGX formats.


----------------------------------------------------------------
* Finally

  Enjoy a convenient MSX-DOS2 environment with M File Manager!
