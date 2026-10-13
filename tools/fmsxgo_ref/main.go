// Gerador de quadro de referencia: roda uma ROM no fmsxgo (port em Go do fMSX) e grava a tela em PPM.
// Uso: fmsxref -model 0|1|2 -roms <dir das BIOS> -cart <rom> -frames N -out <arq.ppm> [-disk <dsk>]
// O fmsxgo entra por "replace" no go.mod (nao e' copiado nem alterado).
package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	"fmsxgo/pkg/msx"
)

func main() {
	model := flag.Int("model", 0, "0=MSX1 1=MSX2 2=MSX2+")
	roms := flag.String("roms", "", "diretorio com MSX.ROM/MSX2.ROM/MSX2EXT.ROM/DISK.ROM")
	cart := flag.String("cart", "", "cartucho (slot 1)")
	disk := flag.String("disk", "", "disquete em A:")
	frames := flag.Int("frames", 300, "quadros a rodar")
	out := flag.String("out", "ref.ppm", "saida PPM (quadro inteiro, 288x240)")
	flag.Parse()

	cfg := msx.DefaultConfig()
	cfg.Model = *model
	if *model == msx.ModelMSX1 {
		cfg.RAMPages, cfg.VRAMPages = 4, 2
	}
	cfg.ROMDir = *roms
	cfg.DiskAPath = *disk
	m, err := msx.NewMachine(cfg)
	if err != nil {
		fmt.Fprintln(os.Stderr, "NewMachine:", err)
		os.Exit(2)
	}
	if *cart != "" {
		if err := m.LoadCartridge(1, *cart); err != nil {
			fmt.Fprintln(os.Stderr, "LoadCartridge:", err)
			os.Exit(2)
		}
	}
	m.Reset()
	for i := 0; i < *frames; i++ {
		m.StepFrame()
	}
	fb := m.GetFrameBuffer() // RGBA 576x240 (x dobrado)
	const w, h = 576, 240
	f, err := os.Create(*out)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(2)
	}
	defer f.Close()
	bw := bufio.NewWriter(f)
	defer bw.Flush()
	// quadro inteiro (com bordas), x reduzido a metade: 288x240. O alinhamento com o fwMSX e' feito pelo compare.py.
	const ow, oh = w / 2, h
	fmt.Fprintf(bw, "P6\n%d %d\n255\n", ow, oh)
	for y := 0; y < oh; y++ {
		for x := 0; x < ow; x++ {
			p := (y*w + x*2) * 4
			bw.Write(fb[p : p+3])
		}
	}
}
