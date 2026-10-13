/** fMSX sem janela, para comparacao de quadros com o fwMSX ***/
/** Camada de plataforma minima no lugar de Unix.c: o nucleo ***/
/** (fMSX.c, MSX.c, V9938.c, Z80...) e' o ORIGINAL de       ***/
/** resource/fMSX, sem alteracao. Aqui so' ha' o que o nucleo **/
/** pede da plataforma: video em memoria (32 bpp), sem som, **/
/** sem teclado, e o disparo do quadro N para um arquivo PPM.**/
/** Uso: fmsx_ref [opcoes do fMSX] -- -frames N -out arq.ppm ***/
#include "MSX.h"
#include "EMULib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH       272                   /* Buffer width    */
#define HEIGHT      228                   /* Buffer height   */

int UseSound    = 0;
int SyncFreq    = 0;
const char *Title = "fMSX 6.0 (headless)";
const char *Disks[2][MAXDISKS+1];

static pixel *WBuf;                       /* From Wide.h     */
static pixel *XBuf;                       /* From Common.h   */
static unsigned int XPal[80];
static unsigned int BPal[256];
static unsigned int XPal0;

static unsigned int X11GetColor(unsigned char R,unsigned char G,unsigned char B)
{ return(((unsigned int)R<<16)|((unsigned int)G<<8)|B); }

void PutImage(void);
#include "CommonMux.h"

static int  ShotFrames = 300;
static const char *ShotFile = "fmsx_ref.ppm";
static int  FrameCount = 0;
static int  OldScrMode = 0;
static int  Trace = 0;                    /* -trace: imprime as mudancas de modo/registradores */
static byte OldVDP[8];

int InitMachine(void)
{
  int J;

  XBuf = (pixel *)calloc(WIDTH*HEIGHT,sizeof(pixel));
  WBuf = (pixel *)calloc(WIDTH*2*HEIGHT,sizeof(pixel));
  if(!XBuf||!WBuf) return(0);
  if(!SetScreenDepth(32)) return(0);
  for(J=0;J<80;J++) SetColor(J,0,0,0);
  for(J=0;J<256;J++)
    BPal[J]=X11GetColor(((J>>2)&0x07)*255/7,((J>>5)&0x07)*255/7,(J&0x03)*255/3);
  return(1);
}

void TrashMachine(void) { free(XBuf);free(WBuf); }

/* PutImage() e' chamado uma vez por quadro desenhado */
void PutImage(void)
{
  FILE *F;
  int W,Y,X;
  pixel *P;

  if(Trace&&((ScrMode!=OldScrMode)||memcmp(OldVDP,VDP,8)))
  {
    int R;
    fprintf(stderr,"quadro %d: SCREEN %d, R0-7:",FrameCount,ScrMode);
    for(R=0;R<8;R++) fprintf(stderr," %02X",VDP[R]);
    fprintf(stderr,"\n");
    memcpy(OldVDP,VDP,8);
  }
  OldScrMode=ScrMode;
  if(++FrameCount<ShotFrames) return;

  W = (ScrMode==6)||((ScrMode==7)&&!ModeYJK)||(ScrMode==MAXSCREEN+1)? WIDTH*2:WIDTH;
  P = W>WIDTH? WBuf:XBuf;
  if(!(F=fopen(ShotFile,"wb"))) { ExitNow=1;return; }
  fprintf(F,"P6\n%d %d\n255\n",W,HEIGHT);
  for(Y=0;Y<HEIGHT;Y++)
    for(X=0;X<W;X++)
    {
      unsigned int C=P[Y*W+X];
      fputc((C>>16)&255,F);fputc((C>>8)&255,F);fputc(C&255,F);
    }
  fclose(F);
  ExitNow=1;
}

void PlayAllSound(int uSec) { (void)uSec; }
unsigned int Joystick(void) { return(0); }
void Keyboard(void) { }
unsigned int Mouse(byte N) { (void)N;return(0); }

void SetColor(byte N,byte R,byte G,byte B)
{ if(N) XPal[N]=X11GetColor(R,G,B); else XPal0=X11GetColor(R,G,B); }

void MenuMSX(void) { }

/* som, MIDI e depurador: o nucleo chama, mas aqui nao ha' saida de audio nem depuracao */
void Sound(int Channel,int Freq,int Volume) { (void)Channel;(void)Freq;(void)Volume; }
void Drum(int Type,int Force) { (void)Type;(void)Force; }
void SetSound(int Channel,int NewType) { (void)Channel;(void)NewType; }
void SetNoise(int Seed,int OUTBit,int XORBit) { (void)Seed;(void)OUTBit;(void)XORBit; }
void SetWave(int Channel,const signed char *Data,int Length,int Rate)
{ (void)Channel;(void)Data;(void)Length;(void)Rate; }
void InitMIDI(const char *FileName) { (void)FileName; }
void TrashMIDI(void) { }
void MIDITicks(int N) { (void)N; }
byte DebugZ80(register Z80 *R) { (void)R;return(1); }

/* main() do fMSX renomeado: aqui entram so' as opcoes da comparacao, o resto segue para ele */
extern int fmsx_main(int argc,char *argv[]);

int main(int argc,char *argv[])
{
  char **A=(char **)malloc(sizeof(char *)*(argc+4));
  int N,M=0;

  for(N=0;N<argc;N++)
  {
    if(!strcmp(argv[N],"-frames")&&(N+1<argc)) ShotFrames=atoi(argv[++N]);
    else if(!strcmp(argv[N],"-trace")) Trace=1;
    else if(!strcmp(argv[N],"-out")&&(N+1<argc)) ShotFile=argv[++N];
    else A[M++]=argv[N];
  }
  /* todos os quadros sao desenhados (0% pulados) e sem mensagens */
  A[M++]="-skip";A[M++]="0";A[M]=0;
  return(fmsx_main(M,A));
}
