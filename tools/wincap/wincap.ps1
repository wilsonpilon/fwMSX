<#
  Ferramenta de verificacao da janela do fwMSX (so' Windows, so' PowerShell): manda teclas a UMA janela e grava uma captura SO' DELA.

    $p = Start-Process .\distwMSX.exe -ArgumentList "--msx","--bios","distomsios\MSX.ROM","--no-splash","--mute" -PassThru
    .	ools\wincap\wincap.ps1 -ProcId $p.Id -Out captura.png -Keys @("F10","ENTER","DOWN","B") -WaitMs 5000
    Stop-Process -Id $p.Id -Force

  - As teclas vao por PostMessage (WM_KEYDOWN/WM_KEYUP com o scancode certo) -- a janela NAO precisa de foco e nada e' digitado na
    area de trabalho. Nomes: F10 ENTER ESC BS UP DOWN LEFT RIGHT (as setas saem como teclas estendidas) A..F G D M S B C J R Q N0..N9.
  - A captura e' do conteudo da janela (PrintWindow com PW_RENDERFULLCONTENT), nao da tela: nenhum outro programa aparece.
  - Serve para conferir o splash, o painel Z80 Debugger (F10) e o que mais for visual sem depender de olhar a tela.
#>
param([int]$ProcId, [string]$Out, [string[]]$Keys, [int]$WaitMs = 1500)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public class W {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
"@
$p = Get-Process -Id $ProcId
for ($i = 0; $i -lt 40 -and $p.MainWindowHandle -eq [IntPtr]::Zero; $i++) { Start-Sleep -Milliseconds 250; $p.Refresh() }
$h = $p.MainWindowHandle
# teclas: nome:VK:scancode
$map = @{ F10=@(0x79,0x44); ENTER=@(0x0D,0x1C); DOWN=@(0x28,0x50); UP=@(0x26,0x48); RIGHT=@(0x27,0x4D); LEFT=@(0x25,0x4B); G=@(0x47,0x22); D=@(0x44,0x20); M=@(0x4D,0x32); S=@(0x53,0x1F); B=@(0x42,0x30); C=@(0x43,0x2E); J=@(0x4A,0x24); R=@(0x52,0x13); Q=@(0x51,0x10); ESC=@(0x1B,0x01); BS=@(0x08,0x0E); A=@(0x41,0x1E); E=@(0x45,0x12); F=@(0x46,0x21); N0=@(0x30,0x0B); N1=@(0x31,0x02); N2=@(0x32,0x03); N3=@(0x33,0x04); N4=@(0x34,0x05); N5=@(0x35,0x06); N6=@(0x36,0x07); N7=@(0x37,0x08); N8=@(0x38,0x09); N9=@(0x39,0x0A) }
Start-Sleep -Milliseconds $WaitMs
foreach ($k in $Keys) {
  $vk = $map[$k][0]; $sc = $map[$k][1]
  $ext = 0; if (@("DOWN","UP","LEFT","RIGHT") -contains $k) { $ext = (1 -shl 24) }   # setas: tecla estendida (senao vira o teclado numerico)
  $down = [IntPtr](1 -bor ($sc -shl 16) -bor $ext); $up = [IntPtr](1 -bor ($sc -shl 16) -bor (1 -shl 30) -bor (1 -shl 31) -bor $ext)
  [W]::PostMessage($h, 0x100, [IntPtr]$vk, $down) | Out-Null   # WM_KEYDOWN
  Start-Sleep -Milliseconds 120
  [W]::PostMessage($h, 0x101, [IntPtr]$vk, $up) | Out-Null     # WM_KEYUP
  Start-Sleep -Milliseconds 350
}
Start-Sleep -Milliseconds 500
$r = New-Object W+RECT; [W]::GetClientRect($h, [ref]$r) | Out-Null
$w = $r.R - $r.L; $hh = $r.B - $r.T
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp); $dc = $g.GetHdc()
[W]::PrintWindow($h, $dc, 2) | Out-Null
$g.ReleaseHdc($dc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
"$w x $hh -> $Out"
