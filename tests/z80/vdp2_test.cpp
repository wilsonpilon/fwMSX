// Teste do VDP MSX2 (V9938): VRAM de 128KB, modos SCREEN 3-8 e TEXT80, sprites
// de modo 2 e motor de comandos -- ver doc/vdp-spec.md (Fases 4 e 5).
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "../../src/vdp/core/vdp_render.h"
#include "../../src/vdp/core/vdp_sprites.h"
#include "../../src/vdp/core/vdp_state.h"

namespace {

int g_failures = 0;

void check(bool cond, const std::string &what) {
    if (cond) {
        std::printf("[PASS] %s\n", what.c_str());
    } else {
        std::printf("[FAIL] %s\n", what.c_str());
        ++g_failures;
    }
}

VdpState *NewVdp(int model) {
    VdpState *v = new VdpState();
    vdp_set_model(v, model);
    return v;
}

void Reg(VdpState &v, int r, uint8_t value) { vdp_write_register(&v, r, value); }

// Modos: R#0 e R#1 de cada SCREEN (com a tela ligada, R#1 bit 6).
void Screen(VdpState &v, int n) {
    uint8_t r0 = 0, r1 = 0x40;
    switch (n) {
    case 3: r1 |= 0x08; break;
    case 4: r0 = 0x04; break;
    case 5: r0 = 0x06; break;
    case 6: r0 = 0x08; break;
    case 7: r0 = 0x0A; break;
    case 8: r0 = 0x0E; break;
    case 80: r0 = 0x04; r1 |= 0x10; break; // TEXT80
    }
    Reg(v, 0, r0);
    Reg(v, 1, r1);
}

// Cor (r,g,b) da entrada de paleta idx.
bool IsPal(const VdpState &v, const VdpRgb888 &px, int idx) {
    return px.r == v.palette_r[idx] && px.g == v.palette_g[idx] && px.b == v.palette_b[idx];
}

bool IsRgb(const VdpRgb888 &px, int r, int g, int b) { return px.r == r && px.g == g && px.b == b; }

// Roda o motor de comandos ate' terminar (CE = S#2 bit 0), no maximo `limit` scanlines.
int RunCommand(VdpState &v, int limit = 5000) {
    int n = 0;
    while ((v.status[2] & 0x01) && n < limit) {
        vdp_cmd_loop(&v);
        ++n;
    }
    return n;
}

void SetXY(VdpState &v, int sx, int sy, int dx, int dy, int nx, int ny) {
    Reg(v, 32, static_cast<uint8_t>(sx)); Reg(v, 33, static_cast<uint8_t>(sx >> 8));
    Reg(v, 34, static_cast<uint8_t>(sy)); Reg(v, 35, static_cast<uint8_t>(sy >> 8));
    Reg(v, 36, static_cast<uint8_t>(dx)); Reg(v, 37, static_cast<uint8_t>(dx >> 8));
    Reg(v, 38, static_cast<uint8_t>(dy)); Reg(v, 39, static_cast<uint8_t>(dy >> 8));
    Reg(v, 40, static_cast<uint8_t>(nx)); Reg(v, 41, static_cast<uint8_t>(nx >> 8));
    Reg(v, 42, static_cast<uint8_t>(ny)); Reg(v, 43, static_cast<uint8_t>(ny >> 8));
}

// Pixel (x,y) de SCREEN 5 lido direto da VRAM.
int Pix5(const VdpState &v, int x, int y) {
    const uint8_t b = v.vram[(y << 7) + (x >> 1)];
    return (x & 1) ? (b & 0x0F) : (b >> 4);
}

} // namespace

int main() {
    // --- 1. Modelo, VRAM de 128KB e paginas ---------------------------------------
    {
        VdpState *m1 = NewVdp(VDP_MODEL_MSX1);
        check(m1->vram_mask == 0x3FFF && m1->vram_pages == 1, "MSX1: VRAM de 16KB (mascara 3FFFh), 1 pagina");
        Reg(*m1, 14, 5);
        check(m1->regs[14] == 0, "MSX1: R#14 e' ignorado (so' a pagina 0)");
        Reg(*m1, 44, 0x12);
        Reg(*m1, 46, 0xC0); // HMMV em MSX1: sem motor de comandos
        check(!(m1->status[2] & 1) && m1->engine == 0, "MSX1: R#46 nao dispara comando nenhum");
        check(m1->status[0] == 0 && m1->status[2] == 0, "MSX1: status zerados no reset (como antes)");
        delete m1;

        VdpState *m2 = NewVdp(VDP_MODEL_MSX2);
        check(m2->vram_mask == 0x1FFFF && m2->vram_pages == 8, "MSX2: VRAM de 128KB (mascara 1FFFFh), 8 paginas");
        check(m2->status[0] == 0x9F && m2->status[2] == 0x6C, "MSX2: status iniciais S#0=9Fh, S#2=6Ch (VDPSInit do fMSX)");
        Reg(*m2, 14, 0x0B);
        check(m2->regs[14] == 3, "MSX2: R#14 so' guarda 3 bits (0Bh -> 3)");

        // Escrita pela porta 98h na pagina 3
        Screen(*m2, 5);
        Reg(*m2, 14, 3);
        vdp_out(m2, 0x99, 0x10);
        vdp_out(m2, 0x99, 0x40 | 0x05); // endereco 0510h de escrita
        vdp_out(m2, 0x98, 0xAB);
        check(m2->vram[(3u << 14) | 0x0510] == 0xAB, "MSX2: a pagina R#14=3 vai para o endereco 0C510h da VRAM");

        // A volta de 16KB avanca a pagina (so' em SCREEN 4+)
        Reg(*m2, 14, 0);
        vdp_out(m2, 0x99, 0xFF);
        vdp_out(m2, 0x99, 0x40 | 0x3F);
        vdp_out(m2, 0x98, 0x11);
        vdp_out(m2, 0x98, 0x22);
        check(m2->vram[0x3FFF] == 0x11 && m2->vram[0x4000] == 0x22 && m2->regs[14] == 1, "SCREEN 5: a volta em 3FFFh avanca para a pagina 1 (R#14=1)");
        Reg(*m2, 14, 7);
        vdp_out(m2, 0x99, 0xFF);
        vdp_out(m2, 0x99, 0x40 | 0x3F);
        vdp_out(m2, 0x98, 0x33);
        vdp_out(m2, 0x98, 0x44);
        check(m2->vram[0x1FFFF] == 0x33 && m2->vram[0x0000] == 0x44 && m2->regs[14] == 0, "a pagina 7 da' a volta para a 0");

        // Leitura
        vdp_out(m2, 0x99, 0x00);
        vdp_out(m2, 0x99, 0x00 | 0x40 | 0); // escrita, pagina 0, endereco 0
        Reg(*m2, 14, 1);
        vdp_out(m2, 0x99, 0x00);
        vdp_out(m2, 0x99, 0x00); // leitura em 0 da pagina 1
        check(vdp_in(m2, 0x98) == 0x22, "leitura pela 98h respeita a pagina (4000h = 22h)");

        // Em SCREEN 1 (modo <= 3) a pagina NAO avanca na volta
        Screen(*m2, 1 == 1 ? 1 : 0);
        Reg(*m2, 14, 0);
        vdp_out(m2, 0x99, 0xFF);
        vdp_out(m2, 0x99, 0x40 | 0x3F);
        vdp_out(m2, 0x98, 0x55);
        vdp_out(m2, 0x98, 0x66);
        check(m2->regs[14] == 0 && m2->vram[0x0000] == 0x66, "SCREEN 1 em MSX2: a volta de 16KB nao muda de pagina (como no fMSX)");

        // R#10/R#11: tabela de cor/atributos de sprite com bits altos
        Reg(*m2, 10, 0xFF);
        Reg(*m2, 11, 0xFF);
        check(m2->regs[10] == 0x07 && m2->regs[11] == 0x03, "R#10 guarda 3 bits e R#11 2 bits");
        delete m2;
    }

    // --- 2. Renderizacao dos modos MSX2 --------------------------------------------
    {
        VdpState *v = NewVdp(VDP_MODEL_MSX2);
        std::vector<VdpRgb888> row(VDP_RENDER_MAX_WIDTH);

        // SCREEN 5: 256x192, 4 bits por pixel; R#2=1Fh -> pagina 0
        Screen(*v, 5);
        Reg(*v, 2, 0x1F);
        v->vram[0] = 0x2A; // pixels 0 e 1: cores 2 e 10
        v->vram[1] = 0xF1; // pixels 2 e 3: cores 15 e 1
        v->vram[128] = 0x77; // linha 1
        check(vdp_render_width(v) == 256 && vdp_render_height(v) == 192, "SCREEN 5: 256x192");
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 2) && IsPal(*v, row[1], 10) && IsPal(*v, row[2], 15) && IsPal(*v, row[3], 1), "SCREEN 5: dois pixels de 4 bits por byte (2,10,15,1)");
        vdp_render_line(v, 1, row.data());
        check(IsPal(*v, row[0], 7) && IsPal(*v, row[1], 7), "SCREEN 5: a linha 1 vem de 128 bytes adiante");

        // Cor 0 = transparente: mostra a cor de fundo (R#7), a menos que R#8 bit 5
        Reg(*v, 7, 0x04);
        vdp_render_line(v, 5, row.data()); // linha vazia (cor 0)
        check(IsPal(*v, row[0], 4), "cor 0 e' transparente: mostra a cor de fundo R#7 (4)");
        Reg(*v, 8, 0x20);
        vdp_render_line(v, 5, row.data());
        check(IsPal(*v, row[0], 0), "R#8 bit 5 (TP): cor 0 vira solida");
        Reg(*v, 8, 0x00);
        Reg(*v, 7, 0x00);

        // Pagina de R#2: 20h/40h selecionam 8000h/10000h
        Reg(*v, 2, 0x3F); // bits 5 -> pagina 1 (8000h)
        v->vram[0x8000] = 0x55;
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 5), "SCREEN 5: R#2=3Fh escolhe a pagina de 8000h");
        Reg(*v, 2, 0x1F);

        // Tela desligada: so' o fundo
        Reg(*v, 7, 0x09);
        Reg(*v, 1, 0x00);
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 9) && IsPal(*v, row[255], 9), "tela desligada (R#1 bit 6 = 0): so' a cor de fundo");
        Reg(*v, 7, 0x00);
        Screen(*v, 5);

        // 212 linhas
        Reg(*v, 9, 0x80);
        check(vdp_render_height(v) == 212, "R#9 bit 7: 212 linhas no MSX2");
        Reg(*v, 9, 0x00);

        // Rolagem vertical R#23
        Reg(*v, 23, 1);
        vdp_render_line(v, 0, row.data()); // agora mostra a linha 1 da VRAM
        check(IsPal(*v, row[0], 7), "R#23 (rolagem vertical): a linha 0 mostra a linha 1 da VRAM");
        Reg(*v, 23, 0);

        // SCREEN 6: 512x192, 2 bits por pixel
        Screen(*v, 6);
        Reg(*v, 2, 0x1F);
        v->vram[0] = 0xE4; // 11 10 01 00
        check(vdp_render_width(v) == 512, "SCREEN 6: 512 pixels de largura");
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 3) && IsPal(*v, row[1], 2) && IsPal(*v, row[2], 1) && IsPal(*v, row[3], 0), "SCREEN 6: 4 pixels de 2 bits por byte (3,2,1,0)");

        // SCREEN 7: 512x192, 4 bits por pixel, 256 bytes por linha
        Screen(*v, 7);
        Reg(*v, 2, 0x1F);
        v->vram[0] = 0x3C;
        v->vram[256] = 0x5A; // linha 1
        check(vdp_render_width(v) == 512, "SCREEN 7: 512 pixels de largura");
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 3) && IsPal(*v, row[1], 12), "SCREEN 7: 2 pixels de 4 bits por byte (3,12)");
        vdp_render_line(v, 1, row.data());
        check(IsPal(*v, row[0], 5) && IsPal(*v, row[1], 10), "SCREEN 7: a linha 1 vem de 256 bytes adiante");

        // SCREEN 8: 256x192, 1 byte por pixel, paleta GRB 3-3-2 fixa
        Screen(*v, 8);
        Reg(*v, 2, 0x1F);
        v->vram[0] = 0xFF; // branco
        v->vram[1] = 0x1C; // vermelho puro (R=7)
        v->vram[2] = 0xE0; // verde puro (G=7)
        v->vram[3] = 0x03; // azul puro (B=3)
        v->vram[4] = 0x00;
        vdp_render_line(v, 0, row.data());
        check(IsRgb(row[0], 255, 255, 255) && IsRgb(row[1], 255, 0, 0) && IsRgb(row[2], 0, 255, 0) && IsRgb(row[3], 0, 0, 255) && IsRgb(row[4], 0, 0, 0),
              "SCREEN 8: bytes GRB 3-3-2 viram branco/vermelho/verde/azul/preto (paleta fixa)");
        Reg(*v, 7, 0xFF);
        Reg(*v, 1, 0x00);
        vdp_render_line(v, 0, row.data());
        check(IsRgb(row[0], 255, 255, 255), "SCREEN 8: a borda/fundo usa R#7 como byte GRB (FFh = branco)");
        Reg(*v, 7, 0);

        // SCREEN 3: multicolor
        Screen(*v, 3);
        Reg(*v, 2, 0x02); // chr_tab = 800h
        Reg(*v, 4, 0x00); // chr_gen = 0
        v->vram[0x800] = 0x00; // caractere 0 na posicao (0,0)
        v->vram[0] = 0x4F; // linhas 0-3 do caractere: esquerda cor 4, direita cor 15
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 4) && IsPal(*v, row[3], 4) && IsPal(*v, row[4], 15) && IsPal(*v, row[7], 15), "SCREEN 3: blocos de 4x4 pixels, 2 cores por caractere");
        v->vram[1] = 0x21;
        vdp_render_line(v, 4, row.data());
        check(IsPal(*v, row[0], 2) && IsPal(*v, row[4], 1), "SCREEN 3: a linha 4 usa o 2o byte do padrao");

        // SCREEN 4 (graphics 3, como a 2): padrao + cor por linha de caractere
        Screen(*v, 4);
        Reg(*v, 2, 0x00); // chr_tab = 0
        Reg(*v, 3, 0xFF); // col_tab = 2000h
        Reg(*v, 4, 0x03); // chr_gen = 0 (R#4 & 3C)
        v->vram[0] = 0; // caractere 0
        std::fill(v->vram + 0x1800, v->vram + 0x1808, 0); // (limpa)
        v->vram[0x0000] = 0x00;
        // o caractere 0 do 1o terco: padrao em chr_gen+0, cor em col_tab+0
        // chr_tab = 0 coincide com a tabela de nomes: o byte 0 = 0 e' o codigo do caractere
        v->vram[0x2000] = 0x00;
        // padrao: usar o codigo 1 (nome na posicao 0)
        v->vram[0] = 1;
        v->vram[0x0008] = 0xF0; // glifo do codigo 1, linha 0: 4 pixels ligados a esquerda
        v->vram[0x2008] = 0x62; // cor: frente 6, fundo 2
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 6) && IsPal(*v, row[3], 6) && IsPal(*v, row[4], 2) && IsPal(*v, row[7], 2), "SCREEN 4: cor por linha de caractere (frente 6, fundo 2)");

        // TEXT80
        Screen(*v, 80);
        Reg(*v, 7, 0xF4);
        Reg(*v, 2, 0x04); // chr_tab = (04&7C)<<10 = 1000h
        Reg(*v, 4, 0x01); // chr_gen = 800h
        v->vram[0x1000] = 0x41;
        v->vram[0x800 + 0x41 * 8] = 0xFC; // glifo 'A', linha 0: 6 pixels ligados
        check(vdp_render_width(v) == 480, "TEXT80: 480 pixels (80 colunas x 6)");
        vdp_render_line(v, 0, row.data());
        check(IsPal(*v, row[0], 15) && IsPal(*v, row[5], 15) && IsPal(*v, row[6], 4), "TEXT80: o 1o caractere (6 pixels de frente 15) e o 2o de fundo 4");
        delete v;
    }

    // --- 3. Sprites de modo 2 (SCREEN 4-8) ---------------------------------------------
    {
        VdpState *v = NewVdp(VDP_MODEL_MSX2);
        std::vector<VdpRgb888> row(VDP_RENDER_MAX_WIDTH);
        Screen(*v, 5);
        Reg(*v, 2, 0x1F);
        Reg(*v, 5, 0xF4); // atributos em 7A00h; cores em 7800h (200h antes)
        Reg(*v, 6, 0x0E); // padroes em 7000h
        // fundo: cor 3
        for (int y = 0; y < 40; ++y)
            for (int x = 0; x < 128; ++x) v->vram[(y << 7) + x] = 0x33;
        std::fill(v->vram + 0x7000, v->vram + 0x7008, 0xFF); // padrao 0: 8 linhas cheias
        // sprite 0: Y=9, X=20, padrao 0; cor 10 em todas as linhas
        v->vram[0x7A00] = 9;
        v->vram[0x7A01] = 20;
        v->vram[0x7A02] = 0;
        std::fill(v->vram + 0x7800, v->vram + 0x7810, 0x0A);
        v->vram[0x7A04] = 216; // fim da lista
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[19], 3) && IsPal(*v, row[20], 10) && IsPal(*v, row[27], 10) && IsPal(*v, row[28], 3),
              "sprite de modo 2: 8 pixels de cor 10 (da tabela de cores da linha) sobre o fundo 3");
        vdp_render_line(v, 9, row.data());
        check(IsPal(*v, row[20], 3), "sprite de modo 2: aparece em Y+1 (a linha 9 nao tem sprite)");
        vdp_render_line(v, 18, row.data());
        check(IsPal(*v, row[20], 3), "sprite de modo 2: some depois de 8 linhas");

        // cor por linha: linha 1 do sprite com cor 6
        v->vram[0x7801] = 0x06;
        vdp_render_line(v, 11, row.data());
        check(IsPal(*v, row[20], 6), "cada linha do sprite tem a sua cor (tabela de 16 bytes)");
        v->vram[0x7801] = 0x0A;

        // CC: o sprite 1 tem o bit CC e cobre parte do 0 -> as cores se somam por OR
        v->vram[0x7A04] = 9;
        v->vram[0x7A05] = 24;
        v->vram[0x7A06] = 0;
        std::fill(v->vram + 0x7810, v->vram + 0x7820, 0x45); // CC + cor 5
        v->vram[0x7A08] = 216;
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[22], 10) && IsPal(*v, row[24], 15) && IsPal(*v, row[27], 15) && IsPal(*v, row[30], 5),
              "bit CC: onde os sprites 0 e 1 se sobrepoem as cores fazem OR (10|5=15)");
        // sem CC: o de menor indice cobre
        std::fill(v->vram + 0x7810, v->vram + 0x7820, 0x05);
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[24], 10), "sem CC: o sprite de menor indice cobre o outro");

        // EC (early clock): desloca 32 pixels a esquerda
        v->vram[0x7A04] = 216;
        std::fill(v->vram + 0x7800, v->vram + 0x7810, 0x8A); // EC + cor 10
        v->vram[0x7A01] = 40;
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[8], 10) && IsPal(*v, row[15], 10) && IsPal(*v, row[40], 3), "bit EC: o sprite sai 32 pixels mais a esquerda (X=40 -> 8)");
        std::fill(v->vram + 0x7800, v->vram + 0x7810, 0x0A);
        v->vram[0x7A01] = 20;

        // Cor 0 da linha: sprite invisivel
        std::fill(v->vram + 0x7800, v->vram + 0x7810, 0x00);
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[20], 3), "cor 0 na tabela: o sprite nao aparece nessa linha");
        std::fill(v->vram + 0x7800, v->vram + 0x7810, 0x0A);

        // Lista termina em Y=216: o sprite 2 (depois do marcador) nao aparece
        v->vram[0x7A04] = 216;
        v->vram[0x7A08] = 9;
        v->vram[0x7A09] = 100;
        std::fill(v->vram + 0x7820, v->vram + 0x7830, 0x0C);
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[100], 3), "Y=216 termina a lista de sprites (SCREEN 4-8)");
        v->vram[0x7A08] = 216;

        // 9o sprite na mesma linha: flag no S#0 e numero do 9o
        for (int n = 0; n < 9; ++n) {
            v->vram[0x7A00 + n * 4] = 9;
            v->vram[0x7A01 + n * 4] = static_cast<uint8_t>(n * 12);
            v->vram[0x7A02 + n * 4] = 0;
            std::fill(v->vram + 0x7800 + n * 16, v->vram + 0x7800 + n * 16 + 16, 0x01);
        }
        v->vram[0x7A00 + 9 * 4] = 216;
        v->status[0] = 0;
        vdp_sprites_update_status(v, 10);
        check((v->status[0] & 0x40) && (v->status[0] & 0x1F) == 8, "9o sprite na linha: S#0 bit 6 e numero 8");
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[96], 3), "o 9o sprite nao e' desenhado (limite de 8 por linha)");
        v->vram[0x7A00 + 8 * 4] = 216;
        v->status[0] = 0;
        vdp_sprites_update_status(v, 10);
        check(!(v->status[0] & 0x40), "8 sprites: sem flag do 9o");

        // colisao em SCREEN 5
        for (int n = 0; n < 2; ++n) {
            v->vram[0x7A00 + n * 4] = 9;
            v->vram[0x7A01 + n * 4] = static_cast<uint8_t>(20 + n * 4);
        }
        v->vram[0x7A08] = 216;
        check(vdp_sprites_check_collision(v) == 1, "colisao entre sprites sobrepostos em SCREEN 5");
        v->vram[0x7A05] = 100;
        check(vdp_sprites_check_collision(v) == 0, "sem colisao quando afastados");

        // sprites desligados (R#8 bit 1)
        Reg(*v, 8, 0x02);
        vdp_render_line(v, 10, row.data());
        check(IsPal(*v, row[20], 3), "R#8 bit 1 desliga os sprites");
        delete v;
    }

    // --- 4. Motor de comandos -----------------------------------------------------------
    {
        VdpState *v = NewVdp(VDP_MODEL_MSX2);
        Screen(*v, 5);

        // Fora de SCREEN 5-8 nao ha' comandos
        VdpState *t = NewVdp(VDP_MODEL_MSX2);
        Screen(*t, 1);
        Reg(*t, 44, 0xFF);
        Reg(*t, 46, 0xC0);
        check(!(t->status[2] & 1), "comando em SCREEN 1: ignorado");
        delete t;

        // HMMV: preenche 16x4 com 0x55 a partir de (8,10)
        SetXY(*v, 0, 0, 8, 10, 16, 4);
        Reg(*v, 44, 0x55);
        Reg(*v, 45, 0);
        Reg(*v, 46, 0xC0);
        check((v->status[2] & 1) == 1, "HMMV: S#2 bit 0 (CE) fica ligado ao disparar");
        vdp_cmd_loop(v); // 32 bytes x 439 = 14048 > 12500: precisa de uma 2a scanline
        check((v->status[2] & 1) == 1, "HMMV 16x4 (32 bytes): nao cabe no orcamento de uma scanline");
        RunCommand(*v);
        check(!(v->status[2] & 1), "HMMV pequeno termina em 2 scanlines");
        bool ok = true;
        for (int y = 10; y < 14; ++y)
            for (int x = 8; x < 24; ++x) ok = ok && Pix5(*v, x, y) == 5;
        check(ok && Pix5(*v, 7, 10) == 0 && Pix5(*v, 24, 10) == 0 && Pix5(*v, 8, 9) == 0 && Pix5(*v, 8, 14) == 0,
              "HMMV: so' o retangulo 16x4 em (8,10) foi preenchido, vizinhos intactos");
        check(v->regs[42] == 0 && v->regs[38] == 14, "HMMV: ao fim, NY=0 e DY avancou para 14");

        // Um comando grande leva varias scanlines (tempo do V9938)
        SetXY(*v, 0, 0, 0, 0, 256, 100);
        Reg(*v, 44, 0x11);
        Reg(*v, 46, 0xC0);
        vdp_cmd_loop(v);
        check((v->status[2] & 1) == 1, "HMMV 256x100: ainda em execucao apos 1 scanline");
        const int lines = RunCommand(*v);
        check(!(v->status[2] & 1) && lines > 400 && lines < 600,
              "HMMV 256x100 (12800 bytes): ~450 scanlines (~1.7 quadros), como o orcamento do fMSX (" + std::to_string(lines + 1) + ")");
        check(Pix5(*v, 0, 0) == 1 && Pix5(*v, 255, 99) == 1 && Pix5(*v, 0, 100) == 0, "HMMV 256x100: preencheu exatamente 100 linhas");

        // LMMV com operacao logica: XOR 0x0F sobre (1111...)
        SetXY(*v, 0, 0, 4, 4, 6, 2);
        Reg(*v, 44, 0x0F);
        Reg(*v, 46, 0x80 | 0x03); // LMMV, XOR
        RunCommand(*v);
        check(Pix5(*v, 4, 4) == (0x01 ^ 0x0F) && Pix5(*v, 9, 5) == (0x01 ^ 0x0F) && Pix5(*v, 10, 4) == 0x01, "LMMV com XOR: 01h ^ 0Fh = 0Eh em 6x2 pixels");
        // TIMP (LMMV com bit 3: cor 0 e' transparente)
        SetXY(*v, 0, 0, 4, 4, 2, 1);
        Reg(*v, 44, 0x00);
        Reg(*v, 46, 0x80 | 0x08); // LMMV, TSET: cor 0 nao escreve
        RunCommand(*v);
        check(Pix5(*v, 4, 4) == (0x01 ^ 0x0F), "LMMV transparente (TSET): a cor 0 nao altera o destino");

        // HMMM: copia 8x2 (pixels 0..7, linhas 10-11) para (100, 50)
        std::fill(v->vram, v->vram + 0x6000, 0);
        for (int y = 10; y < 12; ++y)
            for (int x = 0; x < 8; ++x) v->vram[(y << 7) + (x >> 1)] = static_cast<uint8_t>(0x10 * ((x & 1) ? 0 : 1) + 0);
        for (int x = 0; x < 8; ++x) { // padrao reconhecivel: pixel x = x+1
            for (int y = 10; y < 12; ++y) {
                uint8_t &b = v->vram[(y << 7) + (x >> 1)];
                b = static_cast<uint8_t>((x & 1) ? ((b & 0xF0) | (x + 1)) : ((b & 0x0F) | ((x + 1) << 4)));
            }
        }
        SetXY(*v, 0, 10, 100, 50, 8, 2);
        Reg(*v, 46, 0xD0);
        RunCommand(*v);
        ok = true;
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 8; ++x) ok = ok && Pix5(*v, 100 + x, 50 + y) == x + 1;
        check(ok && Pix5(*v, 0, 10) == 1 && Pix5(*v, 99, 50) == 0 && Pix5(*v, 108, 50) == 0, "HMMM: copia 8x2 de VRAM para VRAM, origem intacta");

        // LMMM com AND
        SetXY(*v, 0, 10, 100, 50, 8, 1);
        Reg(*v, 46, 0x90 | 0x01); // LMMM, AND: 100.. & origem = mesmo valor
        RunCommand(*v);
        check(Pix5(*v, 103, 50) == 4, "LMMM com AND: x & x = x");

        // YMMM: copia linhas inteiras (so' Y muda) de y=10 para y=60 a partir de x=0
        SetXY(*v, 0, 10, 0, 60, 0, 2);
        Reg(*v, 46, 0xE0);
        RunCommand(*v);
        check(Pix5(*v, 5, 60) == 6 && Pix5(*v, 7, 61) == 8, "YMMM: copia as linhas de y=10..11 para y=60..61 (da coluna 0 em diante)");

        // PSET / POINT
        SetXY(*v, 0, 0, 200, 150, 0, 0);
        Reg(*v, 44, 0x0C);
        Reg(*v, 46, 0x50); // PSET, SET
        check(!(v->status[2] & 1) && Pix5(*v, 200, 150) == 12, "PSET: um pixel (200,150) = 12, termina na hora");
        SetXY(*v, 200, 150, 0, 0, 0, 0);
        Reg(*v, 46, 0x40); // POINT
        check(v->regs[44] == 12 && v->status[7] == 12, "POINT: devolve a cor do pixel em R#44/S#7");

        // LINE horizontal: (10,5), comprimento 20 no eixo X, cor 7
        SetXY(*v, 0, 0, 10, 5, 20, 0);
        Reg(*v, 44, 0x07);
        Reg(*v, 45, 0x00);
        Reg(*v, 46, 0x70);
        RunCommand(*v);
        ok = true;
        for (int x = 10; x <= 30; ++x) ok = ok && Pix5(*v, x, 5) == 7;
        check(ok && Pix5(*v, 9, 5) == 0 && Pix5(*v, 31, 5) == 0 && Pix5(*v, 10, 6) == 0, "LINE horizontal: 21 pixels (10..30) na linha 5");
        // LINE diagonal: (50,50) -> eixo maior X=10 e menor Y=10 (45 graus)
        SetXY(*v, 0, 0, 50, 50, 10, 10);
        Reg(*v, 44, 0x09);
        Reg(*v, 46, 0x70);
        RunCommand(*v);
        ok = true;
        for (int i = 0; i <= 10; ++i) ok = ok && Pix5(*v, 50 + i, 50 + i) == 9;
        check(ok && Pix5(*v, 51, 50) != 9, "LINE diagonal: 11 pixels em (50+i, 50+i)");
        // LINE vertical (eixo Y maior): bit MAJ do R#45
        SetXY(*v, 0, 0, 80, 20, 12, 0);
        Reg(*v, 44, 0x0D);
        Reg(*v, 45, 0x01);
        Reg(*v, 46, 0x70);
        RunCommand(*v);
        ok = true;
        for (int y = 20; y <= 32; ++y) ok = ok && Pix5(*v, 80, y) == 13;
        check(ok && Pix5(*v, 80, 33) == 0 && Pix5(*v, 81, 20) == 0, "LINE com eixo Y maior: 13 pixels na coluna 80");
        Reg(*v, 45, 0x00);

        // SRCH: procura a cor 9 a partir de (0,3) para a direita
        std::fill(v->vram + (3 << 7), v->vram + (4 << 7), 0);
        v->vram[(3 << 7) + (40 >> 1)] = 0x90; // pixel 40 = 9
        SetXY(*v, 0, 3, 0, 0, 0, 0);
        Reg(*v, 44, 9);
        Reg(*v, 45, 0x00);
        Reg(*v, 46, 0x60);
        RunCommand(*v);
        check((v->status[2] & 0x10) && v->status[8] == 40, "SRCH: achou a cor 9 em X=40 (S#2 bit 4, S#8 = 40)");
        SetXY(*v, 100, 3, 0, 0, 0, 0); // a partir de 100 nao ha' mais pixel 9 ate' a borda
        Reg(*v, 46, 0x60);
        RunCommand(*v);
        check(!(v->status[2] & 0x10), "SRCH: nao achou ate' a borda da tela (S#2 bit 4 desligado)");

        // LMMC: CPU -> VRAM, 4 pixels em (30,70); o 1o dado vai em R#44 ANTES do comando
        SetXY(*v, 0, 0, 30, 70, 4, 1);
        Reg(*v, 44, 1);
        Reg(*v, 46, 0xB0);
        vdp_cmd_loop(v);
        check((v->status[2] & 0x80) && (v->status[2] & 1), "LMMC: apos o 1o dado, TR (S#2 bit 7) pede o proximo");
        Reg(*v, 44, 2); // com orcamento sobrando o motor consome o dado na hora e levanta o TR de novo
        check((v->status[2] & 0x80) != 0, "escrever em R#44 consome o dado e o TR volta a pedir o proximo");
        vdp_cmd_loop(v);
        Reg(*v, 44, 3);
        vdp_cmd_loop(v);
        Reg(*v, 44, 4);
        vdp_cmd_loop(v);
        check(!(v->status[2] & 1) && Pix5(*v, 30, 70) == 1 && Pix5(*v, 31, 70) == 2 && Pix5(*v, 32, 70) == 3 && Pix5(*v, 33, 70) == 4,
              "LMMC: 4 pixels (1,2,3,4) chegaram em (30..33,70) e o comando terminou");

        // HMMC: CPU -> VRAM em bytes (SCREEN 5: 2 pixels/byte): 2 bytes em (40,80)
        SetXY(*v, 0, 0, 40, 80, 4, 1);
        Reg(*v, 44, 0xAB);
        Reg(*v, 46, 0xF0);
        vdp_cmd_loop(v);
        Reg(*v, 44, 0xCD);
        vdp_cmd_loop(v);
        check(!(v->status[2] & 1) && Pix5(*v, 40, 80) == 0xA && Pix5(*v, 41, 80) == 0xB && Pix5(*v, 42, 80) == 0xC && Pix5(*v, 43, 80) == 0xD,
              "HMMC: 2 bytes (ABh, CDh) = 4 pixels em (40..43,80)");

        // LMCM: VRAM -> CPU, le 3 pixels de (30,70) pela porta de status 7
        SetXY(*v, 30, 70, 0, 0, 3, 1);
        Reg(*v, 46, 0xA0);
        vdp_cmd_loop(v);
        Reg(*v, 15, 7);
        const uint8_t p0 = vdp_in(v, 0x99);
        vdp_cmd_loop(v);
        const uint8_t p1 = vdp_in(v, 0x99);
        vdp_cmd_loop(v);
        const uint8_t p2 = vdp_in(v, 0x99);
        check(p0 == 1 && p1 == 2 && p2 == 3, "LMCM: 3 leituras do S#7 devolvem os pixels 1,2,3 (" + std::to_string(p0) + "," + std::to_string(p1) + "," + std::to_string(p2) + ")");
        Reg(*v, 15, 0);

        // ABORT
        SetXY(*v, 0, 0, 0, 0, 256, 100);
        Reg(*v, 44, 0x22);
        Reg(*v, 46, 0xC0);
        check((v->status[2] & 1) == 1, "(HMMV grande em andamento)");
        Reg(*v, 46, 0x00); // ABRT
        check(!(v->status[2] & 1), "ABORT (cmd 0): CE desligado na hora");
        delete v;
    }

    // --- 5. Modo 8 e 6: comandos com a geometria de cada modo --------------------------------
    {
        VdpState *v = NewVdp(VDP_MODEL_MSX2);
        Screen(*v, 8);
        SetXY(*v, 0, 0, 5, 3, 4, 2);
        Reg(*v, 44, 0xAB);
        Reg(*v, 46, 0xC0); // HMMV
        RunCommand(*v);
        check(v->vram[(3 << 8) + 5] == 0xAB && v->vram[(4 << 8) + 8] == 0xAB && v->vram[(3 << 8) + 9] == 0, "SCREEN 8: HMMV 4x2 bytes (1 byte por pixel, 256 por linha)");
        Screen(*v, 6);
        SetXY(*v, 0, 0, 8, 2, 8, 1);
        Reg(*v, 44, 0xE4);
        Reg(*v, 46, 0xC0);
        RunCommand(*v);
        check(v->vram[(2 << 7) + 2] == 0xE4 && v->vram[(2 << 7) + 3] == 0xE4 && v->vram[(2 << 7) + 4] == 0, "SCREEN 6: HMMV de 8 pixels = 2 bytes (4 pixels por byte)");
        Screen(*v, 7);
        SetXY(*v, 0, 0, 10, 1, 2, 1);
        Reg(*v, 44, 0x7A);
        Reg(*v, 46, 0xC0);
        RunCommand(*v);
        check(v->vram[(1 << 8) + 5] == 0x7A && v->vram[(1 << 8) + 6] == 0, "SCREEN 7: HMMV de 2 pixels = 1 byte (256 bytes por linha)");
        delete v;
    }

    // --- 9. V9958 (MSX2+): scroll, MSK, YJK/YAE em SCREEN 10-12 ----------------------
    {
        VdpState *p = NewVdp(VDP_MODEL_MSX2P);
        check((p->status[1] & 0x04) != 0, "V9958: bit 2 de S#1 ligado (ID do VDP, como o fMSX)");
        check(VDP_MODEL_IS_V9938(p->model) && p->vram_mask == 0x1FFFF, "V9958: VRAM de 128KB, como o V9938");
        Reg(*p, 5, 0x40); // tabela de sprites longe da linha 0
        Reg(*p, 8, 0x02); // sprites desligados (SPD) -- so' o desenho do fundo importa aqui
        Screen(*p, 8);
        Reg(*p, 25, 0x08); // YJK
        check(vdp_render_width(p) == 256, "V9958 YJK: a linha tem 256 pixels, como SCREEN 7/8");
        for (int x = 0; x < 256; ++x) p->vram[x] = 0x80; // Y=16, J=K=0

        // YJKColor(16,0,0): Y=16, bytes de crominancia 0 -> r=g=16, b=(80+2)/4=20; 5->8 bits
        // por replicacao: 16 -> 132, 20 -> 165. Sem bloco de fundo nos primeiros pixels.
        VdpRgb888 row[VDP_RENDER_MAX_WIDTH];
        vdp_render_line(p, 0, row);
        check(IsRgb(row[0], 132, 132, 165) && IsRgb(row[4], 132, 132, 165) && IsRgb(row[255], 132, 132, 165),
              "YJK: pixels saem da conversao (16,0,0) -> (132,132,165), sem bloco de fundo");

        // Scroll de 8: a tela le a VRAM a partir do pixel 8. O grupo 8..11 vira Y=20.
        for (int x = 8; x < 12; ++x) p->vram[x] = 0xA0; // Y=20
        Reg(*p, 26, 0x01);
        vdp_render_line(p, 0, row);
        // YJKColor(20,0,0): r=g=20 -> 165; b=(100+2)/4=25 -> 206
        check(IsRgb(row[0], 165, 165, 206) && IsRgb(row[3], 165, 165, 206),
              "scroll 8 (R#26=1): o pixel 0 da tela le o pixel 8 da VRAM (Y=20 -> 165,165,206)");
        check(IsRgb(row[4], 132, 132, 165), "scroll 8: o pixel 4 da tela le o pixel 12 da VRAM (Y=16)");
        Reg(*p, 26, 0x00);

        // Scroll fino de 1 pixel (R#27=1): a tela inteira anda um pixel.
        Reg(*p, 27, 0x01);
        vdp_render_line(p, 0, row);
        check(IsRgb(row[7], 165, 165, 206) && IsRgb(row[6], 132, 132, 165),
              "scroll fino de 1 pixel (R#27=1): a tela le VRAM[x+1]");
        Reg(*p, 27, 0x00);

        // MSK (R#25 bit 1): os 8 primeiros pixels saem na cor de fundo (R#7=0 -> preto).
        Reg(*p, 25, 0x0A);
        vdp_render_line(p, 0, row);
        // row[8] le VRAM[8] (ainda com Y=20 do teste de scroll): fora da mascara, cor normal.
        check(IsRgb(row[0], 0, 0, 0) && IsRgb(row[7], 0, 0, 0) && IsRgb(row[8], 165, 165, 206),
              "MSK (R#25 bit 1): os 8 primeiros pixels saem na cor de fundo, o 9o nao");
        Reg(*p, 25, 0x08);

        // HScroll512 (R#25 bit 0): com scroll 255, o pixel 1 da tela cai na segunda pagina (+64KB).
        Reg(*p, 25, 0x09);
        Reg(*p, 26, 0x00);
        Reg(*p, 27, 0x07); // HScroll = 7 -> pixel x le VRAM[x+7]
        p->vram[0x10000 + 0] = 0x55; // primeiro byte da segunda pagina
        vdp_render_line(p, 0, row);
        Reg(*p, 27, 0x00);
        Reg(*p, 25, 0x08);
        check(!IsRgb(row[0], 0, 0, 0), "HScroll512: a linha volta a ler a VRAM depois do scroll");

        // YAE (R#25 bit 4, SCREEN 10): pixel de Y impar usa a cor de paleta Y>>1.
        Reg(*p, 25, 0x18);
        for (int x = 0; x < 256; ++x) p->vram[x] = 0x18; // Y=3 (impar), paleta 1
        vdp_render_line(p, 0, row);
        check(IsPal(*p, row[4], 1) && IsPal(*p, row[255], 1),
              "SCREEN 10 (YAE): Y impar le a cor de paleta Y>>1 (Y=3 -> paleta 1)");
        delete p;

        // SCREEN 6 (512 pixels, 2 bits): scroll de 1 pixel anda a linha dentro de 512.
        VdpState *s6 = NewVdp(VDP_MODEL_MSX2P);
        Reg(*s6, 5, 0x40);
        Reg(*s6, 8, 0x02);
        Screen(*s6, 6);
        s6->vram[0] = 0xE4; // pixels 0..3 = 3,2,1,0
        VdpRgb888 r6[VDP_RENDER_MAX_WIDTH];
        vdp_render_line(s6, 0, r6);
        check(vdp_render_width(s6) == 512 && IsPal(*s6, r6[0], 3) && IsPal(*s6, r6[2], 1),
              "SCREEN 6 V9958 sem scroll: pixel 0 = cor 3, pixel 2 = cor 1 (512 de largura)");
        Reg(*s6, 27, 0x01);
        vdp_render_line(s6, 0, r6);
        check(IsPal(*s6, r6[0], 2) && IsPal(*s6, r6[1], 1), "SCREEN 6 com scroll 1: a tela le VRAM a partir do pixel 1");
        delete s6;

        // Diferencial: SCREEN 5/6/7/8 no V9958, sem scroll, deve bater com o V9938 (renderizador antigo).
        const int modes[4] = {5, 6, 7, 8};
        bool same = true;
        for (int m = 0; m < 4; ++m) {
            VdpState *a = NewVdp(VDP_MODEL_MSX2P), *b = NewVdp(VDP_MODEL_MSX2);
            for (VdpState *t : {a, b}) {
                Reg(*t, 5, 0x40);
                Reg(*t, 8, 0x02);
                Screen(*t, modes[m]);
            }
            uint32_t seed = 12345u + static_cast<uint32_t>(m);
            for (int i = 0; i < 0x8000; ++i) {
                seed = seed * 1103515245u + 12345u;
                const uint8_t byte = static_cast<uint8_t>(seed >> 16);
                a->vram[i] = byte;
                b->vram[i] = byte;
            }
            VdpRgb888 ra[VDP_RENDER_MAX_WIDTH], rb[VDP_RENDER_MAX_WIDTH];
            for (int y = 0; y < 16 && same; ++y) {
                vdp_render_line(a, y, ra);
                vdp_render_line(b, y, rb);
                const int w = vdp_render_width(a);
                for (int x = 0; x < w; ++x) {
                    if (!IsRgb(ra[x], rb[x].r, rb[x].g, rb[x].b)) { same = false; break; }
                }
            }
            delete a;
            delete b;
        }
        check(same, "V9958 sem scroll, SCREEN 5-8: bate com o renderizador do V9938, pixel a pixel (VRAM aleatoria)");

        // Controle: no V9938 o R#25 nao liga YJK -- SCREEN 8 normal (BPal[80h] = (0,145,0)).
        VdpState *q = NewVdp(VDP_MODEL_MSX2);
        Reg(*q, 5, 0x40);
        Reg(*q, 8, 0x02);
        Screen(*q, 8);
        Reg(*q, 25, 0x08);
        for (int x = 0; x < 256; ++x) q->vram[x] = 0x80;
        vdp_render_line(q, 0, row);
        check(IsRgb(row[4], 0, 145, 0) && vdp_render_width(q) == 256,
              "V9938 com R#25 bit 3: continua SCREEN 8 normal (R#25 nao existe nesse chip)");
        check(!(q->status[1] & 0x04), "V9938: bit 2 de S#1 desligado");
        delete q;
    }

    if (g_failures == 0) {
        std::printf("\nTodos os testes passaram.\n");
        return 0;
    }
    std::printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
