#ifndef LIBAU_DEF_H
#define LIBAU_DEF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#ifdef __DJGPP__
#ifndef ZDM
#include <sys/nearptr.h>
#else
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <dpmi.h>
#include <go32.h>
#define __djgpp_conventional_base 0
extern unsigned short au_map_selector; /* dpmi.c */
#endif
#endif

extern char libau_istr[100]; /* au.c */

struct dosmem_t{
 unsigned short selector;
 char *linearptr;
};

#define NEWFUNC_ASM

typedef long long		mpxp_int64_t;
typedef unsigned long long	mpxp_uint64_t;
typedef long			mpxp_int32_t;
typedef unsigned long           mpxp_uint32_t;
typedef short                   mpxp_int16_t;
typedef unsigned short          mpxp_uint16_t;
typedef signed char             mpxp_int8_t;
typedef unsigned char           mpxp_uint8_t;
typedef float                   mpxp_float_t;
typedef double                  mpxp_double_t;
typedef char                    mpxp_char_t;
typedef long                    mpxp_filesize_t;
typedef unsigned long           mpxp_ptrsize_t; // !!! on 32 bits

#if defined(NEWFUNC_ASM) && defined(__WATCOMC__)
 typedef char mpxp_float80_t[10];
 void pds_float80_put(mpxp_float80_t *ptr,double val);
 #pragma aux pds_float80_put parm[esi][8087] = "fstp tbyte ptr [esi]"
 float pds_float80_get(mpxp_float80_t *ptr);
 #pragma aux pds_float80_get parm[esi] value[8087] = "fld tbyte ptr [esi]"

#define ENTER_CRITICAL IRQ_PUSH_OFF()
 void IRQ_PUSH_OFF (void);
#pragma aux IRQ_PUSH_OFF = \
	"cli"              \
	"pushfd"           \
	modify [esp];

#define LEAVE_CRITICAL IRQ_POP()
 void IRQ_POP (void);
#pragma aux IRQ_POP =   \
		"popfd" \
		"sti"   \
		modify [esp];

#else
 typedef long double mpxp_float80_t; // !!! double (64bit) in WATCOMC
 #define pds_float80_put(p,v) *(p)=(v)
 #define pds_float80_get(p)   (*(p))
#endif

#ifndef __WATCOMC__

 #ifndef min
 #define min(a,b) (((a) < (b))? (a) : (b))
 #endif
 #define outb(reg,val) outportb(reg,val)
 #define outw(reg,val) outportw(reg,val)
 #define outl(reg,val) outportl(reg,val)
 #define inb(reg) inportb(reg)
 #define inw(reg) inportw(reg)
 #define inl(reg) inportl(reg)

 #define ENTER_CRITICAL _disable()
 #define LEAVE_CRITICAL _enable()

#endif

#if defined(NEWFUNC_ASM) && defined(__WATCOMC__)
 /*void pds_ftoi(mpxp_double_t,mpxp_int32_t *); // rather use -zro option at wcc386
 #pragma aux pds_ftoi parm [8087][esi] = "fistp dword ptr [esi]"
 void pds_fto64i(mpxp_double_t,mpxp_int64_t *);
 #pragma aux pds_fto64i parm [8087][esi] = "fistp qword ptr [esi]"*/

 #define pds_ftoi(ff,ii)   (*(ii)=(mpxp_int32_t)(ff))
 #define pds_fto64i(ff,ii) (*(ii)=(mpxp_int64_t)(ff))
 mpxp_uint32_t pds_bswap16(mpxp_uint32_t);
 #pragma aux pds_bswap16 parm [eax] value [eax] = "xchg al,ah"
 mpxp_uint32_t pds_bswap32(mpxp_uint32_t);
 #if (_CPU_ >= 486) || (_M_IX86>=400)
  #pragma aux pds_bswap32 parm [eax] value [eax] = "bswap eax"
 #else
  #pragma aux pds_bswap32 parm [eax] value [eax] = "xchg al,ah" "rol eax,16" "xchg al,ah"
 #endif
 void pds_cpu_hlt(void);
 #pragma aux pds_cpu_hlt ="hlt"

#else
 #define pds_ftoi(ff,ii)   (*ii=(mpxp_int32_t)ff)
 #define pds_fto64i(ff,ii) (*ii=(mpxp_int64_t)ff)
 #define pds_bswap16(a) (((a&0xff)<<8)|((a&0xff00)>>8))
 #define pds_bswap32(a) ((a<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|((a>>24)&0xff))
 #define pds_cpu_hlt
#endif

// note LE: lowest byte first, highest byte last
#define PDS_GETB_8S(p)	  *((mpxp_int8_t *)p)               // signed 8 bit (1 byte)
#define PDS_GETB_8U(p)    *((mpxp_uint8_t *)p)              // unsigned 8 bit (1 byte)
#define PDS_GETB_LE16(p)  *((mpxp_int16_t *)p)              // 2bytes LE to short
#define PDS_GETB_LEU16(p) *((mpxp_uint16_t *)p)             // 2bytes LE to unsigned short
#define PDS_GETB_BE16(p) pds_bswap16(*((mpxp_uint16_t *)p))// 2bytes BE to unsigned short
#define PDS_GETB_LE32(p)  *((mpxp_int32_t *)p)              // 4bytes LE to long
#define PDS_GETB_BE32(p) pds_bswap32(*((mpxp_uint32_t *)p))// 4bytes BE to unsigned long
#define PDS_GETB_LE24(p) ((PDS_GETB_LE32(p))&0x00ffffff)
#define PDS_GETB_BE24(p) ((PDS_GETB_BE32(p))&0x00ffffff)
#define PDS_GETB_LE64(p)  *((mpxp_int64_t *)p)              // 8bytes LE to int64
#define PDS_GET4C_LE32(a,b,c,d) ((mpxp_uint32_t)a | ((mpxp_uint32_t)b << 8) | ((mpxp_uint32_t)c << 16) | ((mpxp_uint32_t)d << 24))

#define PDS_PUTB_LE16(p,v) *((mpxp_int16_t *)p)=v              //
#define PDS_PUTB_BE16(p,v) *((mpxp_int16_t *)p)=pds_bswap16(v) //
#define PDS_PUTB_LE32(p,v) *((mpxp_int32_t *)p)=v              // long to 4bytes LE
#define PDS_PUTB_BE32(p,v) *((mpxp_int32_t *)p)=pds_bswap32(v) // long to 4bytes BE

#define funcbit_test(var,bit)       ((var)&(bit))
#define funcbit_enable(var,bit)     ((var)|=(bit))
#define funcbit_disable(var,bit)    ((var)&=~(bit))
#define funcbit_inverse(var,bit)    ((var)^=(bit))
#define funcbit_copy(var1,var2,bit) ((var1)|=(var2)&(bit))

#define funcbit_smp_test(var,bit)       funcbit_test(var,bit)
#define funcbit_smp_enable(var,bit)     funcbit_enable(var,bit)
#define funcbit_smp_disable(var,bit)    funcbit_disable(var,bit)
#define funcbit_smp_inverse(var,bit)    funcbit_disable(var,bit)
#define funcbit_smp_copy(var1,var2,bit) funcbit_copy(var1,var2,bit)
#define funcbit_smp_value_get(var)       (var)
#define funcbit_smp_pointer_get(var)     (var)
#define funcbit_smp_value_put(var,val)   (var)=(val)
#define funcbit_smp_int64_put(var,val)   (var)=(val)
#define funcbit_smp_filesize_put(var,val) funcbit_smp_value_put(var,val)
#define funcbit_smp_pointer_put(var,val) (var)=(val)
#define funcbit_smp_value_increment(var) var++
#define funcbit_smp_value_decrement(var) var--

//dpmi.c
extern struct dosmem_t *pds_dpmi_dos_allocmem(unsigned int size);
extern void pds_dpmi_dos_freemem(void);
extern unsigned long pds_dpmi_map_physical_memory(unsigned long phys_addr,unsigned long memsize);
extern void pds_dpmi_unmap_physycal_memory(unsigned long linear_address);

//dmairq.c
#define AUCARDS_DMABUFSIZE_NORMAL 32768
#define AUCARDS_DMABUFSIZE_MAX    131072
#define AUCARDS_DMABUFSIZE_BLOCK  4096   // default page (block) size

#define DMAMODE_AUTOINIT_OFF 0
#define DMAMODE_AUTOINIT_ON  0x10

struct mpxplay_audioout_info_s;
extern unsigned int MDma_get_max_pcmoutbufsize(unsigned int pagesize,unsigned int samplesize);
extern unsigned int MDma_init_pcmoutbuf(struct mpxplay_audioout_info_s *aui,unsigned int maxbufsize,unsigned int pagesize);
extern void MDma_clearbuf(struct mpxplay_audioout_info_s *aui);
extern void MDma_writedata(struct mpxplay_audioout_info_s *aui,const char *src,unsigned long left);
extern unsigned int MDma_bufpos(struct mpxplay_audioout_info_s *aui);

// out pcm defs
#define PCM_OUTSAMPLES    1152     // at 44100Hz
#define PCM_MIN_CHANNELS  1
#ifdef MPXPLAY_WIN32
#define PCM_MAX_CHANNELS  8        // au_mixer output (au_card input) limit
#else
#define PCM_MAX_CHANNELS  2        // au_mixer output (au_card input) limit
#endif
#define PCM_MIN_BITS      1
#define PCM_MAX_BITS      32
#define PCM_MIN_FREQ      512
#define PCM_MAX_FREQ      192000   // program can play higher freq too
#define PCM_MAX_SAMPLES   (((PCM_OUTSAMPLES*PCM_MAX_FREQ)+22050)/44100*PCM_MAX_CHANNELS) // only the pcm buffer is limited (in one frame)
#define PCM_MAX_BYTES     (PCM_MAX_SAMPLES*(PCM_MAX_BITS/8))  // in one frame
#define PCM_BUFFER_SIZE   (2*PCM_MAX_BYTES) // *2 : speed control expansion

//timer settings
#define MPXPLAY_TIMER_INT      0x08
#define INT08_DIVISOR_DEFAULT  65536
#define INT08_CYCLES_DEFAULT   (1000.0/55.0)  // 18.181818
#ifdef __DOS__
#define INT08_DIVISOR_NEW      10375  // = 18.181818*65536 / (3 * 44100/1152)
//#define INT08_DIVISOR_NEW      1194   // for 1ms (1/1000 sec) refresh
#else
#define INT08_DIVISOR_NEW      20750  // 60fps (XP requires this low value)
#endif
#define INT08_CYCLES_NEW       (INT08_CYCLES_DEFAULT*INT08_DIVISOR_DEFAULT/INT08_DIVISOR_NEW) // = 114.8

//#define REFRESH_DELAY_JOYMOUSE (INT08_CYCLES_NEW/36) // 38 char/s

//wave (codec) IDs at input/output
#define MPXPLAY_WAVEID_UNKNOWN   0x0000
#define MPXPLAY_WAVEID_PCM_SLE   0x0001 // signed little endian
#define MPXPLAY_WAVEID_PCM_FLOAT 0x0003 // 32-bit float le
#define MPXPLAY_WAVEID_AAC       0x706D
#define MPXPLAY_WAVEID_AC3       0x2000
#define MPXPLAY_WAVEID_DTS       0x2001
#define MPXPLAY_WAVEID_MP2       0x0050
#define MPXPLAY_WAVEID_MP3       0x0055
#define MPXPLAY_WAVEID_WMAV1     0x0160 // 7.0
#define MPXPLAY_WAVEID_WMAV2     0x0161 // 8.0
#define MPXPLAY_WAVEID_WMAV3     0x0162 // 9.0 (not implemented)
#define MPXPLAY_WAVEID_WMAV4     0x0163 // lossless (not implemented)
#define MPXPLAY_WAVEID_FLAC      0xF1AC
// non-standard (internal) wave-ids
#define MPXPLAY_WAVEID_PCM_SBE   0x00017001 // signed big endian pcm
#define MPXPLAY_WAVEID_PCM_F32BE 0x00017003 // 32-bit float big endian
#define MPXPLAY_WAVEID_PCM_F64LE 0x00017004 // 64-bit float little endian
#define MPXPLAY_WAVEID_PCM_F64BE 0x00017005 // 64-bit float big endian
#define MPXPLAY_WAVEID_VORBIS    0x00018000 // ??? Vorbis has 3 official IDs
#define MPXPLAY_WAVEID_SPEEX     0x00018002
#define MPXPLAY_WAVEID_ALAC      0x00018005

//------------------------------------------------------------------------
//au_infos->card_controlbits
#define AUINFOS_CARDCNTRLBIT_TESTCARD         1
#define AUINFOS_CARDCNTRLBIT_DOUBLEDMA        2
#define AUINFOS_CARDCNTRLBIT_MIDASMANUALCFG   4
#define AUINFOS_CARDCNTRLBIT_DMACLEAR         8 // run AU_clearbuffs
#define AUINFOS_CARDCNTRLBIT_DMADONTWAIT     16 // don't wait for card-buffer space (at seeking)
#define AUINFOS_CARDCNTRLBIT_BITSTREAMOUT    32 // enable bitstream out
#define AUINFOS_CARDCNTRLBIT_BITSTREAMNOFRH  64 // no-frame-header output is supported by decoder
#define AUINFOS_CARDCNTRLBIT_BITSTREAMHEAD  128 // write main header (at wav out only)
#define AUINFOS_CARDCNTRLBIT_AUTOTAGGING    256 // copy tags (id3infos) from infile to outfile (if possible)
#define AUINFOS_CARDCNTRLBIT_AUTOTAGLFN     512 // create filename from id3infos (usually "NN. Artis - Title.ext"

//au_infos->card_infobits
#define AUINFOS_CARDINFOBIT_PLAYING          1
#define AUINFOS_CARDINFOBIT_IRQRUN           2
#define AUINFOS_CARDINFOBIT_IRQSTACKBUSY     4
#define AUINFOS_CARDINFOBIT_DMAUNDERRUN      8 // dma buffer is empty (set by dma-monitor)
#define AUINFOS_CARDINFOBIT_DMAFULL         16 // dma buffer is full (set by AU_writedata)
#define AUINFOS_CARDINFOBIT_HWTONE          32
#define AUINFOS_CARDINFOBIT_BITSTREAMOUT    64 // bitstream out enabled/supported
#define AUINFOS_CARDINFOBIT_BITSTREAMNOFRH 128 // no frame headers (cut)

//one_sndcard_info->infobits
#define SNDCARD_SELECT_ONLY     1 // program doesn't try to use automatically (ie: wav output)
#define SNDCARD_INT08_ALLOWED   2 // use of INT08 (and interrupt decoder) is allowed
#define SNDCARD_CARDBUF_SPACE   4 // routine gives back the cardbuf space, not the bufpos
#define SNDCARD_SPECIAL_IRQ     8 // card has a special (long) irq routine (requires stack & irq protection)
#define SNDCARD_SETRATE        16 // always call setrate before each song (special wav-out and test-out flag!)
#define SNDCARD_LOWLEVELHAND   32 // native soundcard handling (PCI)
#define SNDCARD_IGNORE_STARTUP 64 // ignore startup (do not restore songpos) (ie: wav out)
#define SNDCARD_FLAGS_DISKWRITER (SNDCARD_SELECT_ONLY|SNDCARD_SETRATE|SNDCARD_IGNORE_STARTUP)

//au_cards mixer channels
#define AU_AUTO_UNMUTE 1 // automatically unmute (switch off mute flags) at -scv

#define MIXER_SETMODE_RELATIVE 0
#define MIXER_SETMODE_ABSOLUTE 1
#define MIXER_SETMODE_RESET    2

#define AU_MIXCHAN_MASTER       0  // master out
#define AU_MIXCHAN_PCM          1  // pcm out
#define AU_MIXCHAN_HEADPHONE    2  // headphone out
#define AU_MIXCHAN_SPDIFOUT     3  // digital out
#define AU_MIXCHAN_SYNTH        4  // midi/synth out
#define AU_MIXCHAN_MICIN        5  // MIC input
#define AU_MIXCHAN_LINEIN       6  // LINE in
#define AU_MIXCHAN_CDIN         7  // CD in
#define AU_MIXCHAN_AUXIN        8  // AUX in
#define AU_MIXCHAN_BASS         9  // !!! default: -1 in au_cards, +50 in au_mixer
#define AU_MIXCHAN_TREBLE      10  // -"-
#define AU_MIXCHANS_NUM        11

// aucards_mixchandata_s->channeltype
#define AU_MIXCHANFUNC_VOLUME 0  // volume control (of master,pcm,etc.)
#define AU_MIXCHANFUNC_MUTE   1  // mute switch (of master,pcm,etc.)
#define AU_MIXCHANFUNCS_NUM   2  // number of mixchanfuncs
#define AU_MIXCHANFUNCS_FUNCSHIFT 8
#define AU_MIXCHANFUNCS_FUNCMASK  0xff
#define AU_MIXCHANFUNCS_CHANMASK  ((1<<AU_MIXCHANFUNCS_FUNCSHIFT)-1)
#define AU_MIXCHANFUNCS_PACK(chan,func) (((func)<<AU_MIXCHANFUNCS_FUNCSHIFT)|(chan))
#define AU_MIXCHANFUNCS_GETCHAN(c) ((c) & AU_MIXCHANFUNCS_CHANMASK)
#define AU_MIXCHANFUNCS_GETFUNC(c) (((c) >> AU_MIXCHANFUNCS_FUNCSHIFT)&AU_MIXCHANFUNCS_FUNCMASK)

//for verifying
#define AU_MIXERCHAN_MAX_SUBCHANNELS    8 // this is enough for a 7.1 setting too :)
#define AU_MIXERCHAN_MAX_REGISTER   65535 // check this again at future cards (2^bits)
#define AU_MIXERCHAN_MAX_BITS          32 //
#define AU_MIXERCHAN_MAX_VALUE 0xffffffff // 2^32-1

//aucards_submixerchan_s->infobits
#define SUBMIXCH_INFOBIT_REVERSEDVALUE  1 // reversed value
#define SUBMIXCH_INFOBIT_SUBCH_SWITCH   2 // set register if value!=submixch_max

//soundcard mixer structures
typedef struct aucards_submixerchan_s{
 unsigned long submixch_register; // register-address of channel
 unsigned long submixch_max;      // max value (and mask) (must be 2^n-1 (ie:1,3,7,15,31,63,127,255))
 unsigned long submixch_shift;    // bit-shift from 0. bit
 unsigned long submixch_infobits; //
}aucards_submixerchan_s;

typedef struct aucards_onemixerchan_s{
 unsigned long mixchan;       // master,pcm,etc. & volume,mute-sw
 unsigned long subchannelnum; // sub-channels (mono (1) or left&right (2))
#if defined(__WATCOMC__) || (defined(__GNUC__) && __GNUC__ > 2)
 aucards_submixerchan_s submixerchans[]; // infos of 1 or 2 subchannels (reg,max,shift,flag)
#else
 aucards_submixerchan_s submixerchans[0]; // infos of 1 or 2 subchannels (reg,max,shift,flag)
#endif
}aucards_onemixerchan_s;

typedef struct aucards_onemixerchan_s* aucards_allmixerchan_s;

struct mpxplay_audioout_info_s;

typedef struct one_sndcard_info{
 const char *shortname;
 unsigned long infobits;

 int  (*card_config)(struct mpxplay_audioout_info_s *); // not used yet
 int  (*card_init)(struct mpxplay_audioout_info_s *);   // read the environment variable and try to init the card
 int  (*card_detect)(struct mpxplay_audioout_info_s *); // try to autodetect the card
 void (*card_info)(struct mpxplay_audioout_info_s *);   // show card infos
 void (*card_start)(struct mpxplay_audioout_info_s *);  // start playing
 void (*card_stop)(struct mpxplay_audioout_info_s *);   // stop playing (immediately)
 void (*card_close)(struct mpxplay_audioout_info_s *);  // close soundcard
 void (*card_setrate)(struct mpxplay_audioout_info_s *);// set freqency,channels,bits

 void (*cardbuf_writedata)(struct mpxplay_audioout_info_s *); // write output data into the card's buffer
 long (*cardbuf_pos)(struct mpxplay_audioout_info_s *);  // get the buffer (playing) position (usually the DMA buffer get-position)(returns negative number on error)
 void (*cardbuf_clear)(struct mpxplay_audioout_info_s *);// clear the soundcard buffer (usually the DMA buffer)
 void (*cardbuf_int_monitor)(struct mpxplay_audioout_info_s *); // interrupt (DMA) monitor function
 void (*irq_routine)(struct mpxplay_audioout_info_s *);  // as is

 void (*card_writemixer)(struct mpxplay_audioout_info_s *,unsigned long mixreg,unsigned long value);
 unsigned long (*card_readmixer)(struct mpxplay_audioout_info_s *,unsigned long mixreg);
 aucards_allmixerchan_s *card_mixerchans;
}one_sndcard_info;

//card and mixer data
typedef struct mpxplay_audioout_info_s{
 /* members shared with struct auinfo_s:
  * _must_ overlap with struct auinfo_s */
 char          infostr[96];
 char         *card_DMABUFF;
 unsigned long card_dmasize;
 unsigned int  bytespersample_card;
 unsigned int  freq_card;
 unsigned int  bits_set;
 unsigned int  chan_set;
 /* members private to libau */
 unsigned int  samplenum;
 unsigned int  freq_set;
 unsigned int  chan_card;
 unsigned int  bits_card;
 unsigned int   card_wave_id;    // 0x0001,0x0003,0x0055,0x2000,etc.
 unsigned long  card_controlbits;  // card control flags
 unsigned long  card_infobits;     // card info flags
 unsigned long  card_outbytes;     // samplenum*bytespersample_card
 unsigned long  card_dmalastput;
 unsigned long  card_dmaspace;
 unsigned long  card_dmafilled;
 unsigned long  card_dma_lastgoodpos;
 unsigned int   card_bytespersign; // bytespersample_card*chan_card
 unsigned int   card_bufprotect;
 unsigned int	card_select_config;	//STEREO_SPEAKER_OUT or LINE_HP_OUT
 struct dosmem_t *card_dma_dosmem;
 one_sndcard_info *card_handler; // function structure of the card
 void *card_private_data;        // extra private datas can be pointed here (with malloc)
 int card_master_volume;
 int card_mixer_values[AU_MIXCHANS_NUM]; // -1, 0-100
}mpxplay_audioout_info_s;

extern void pds_delay_10us(unsigned int ticks);
extern mpxp_uint64_t pds_gettimeu(void); // usec

#ifdef MPXPLAY_USE_DEBUGF
#include <stdarg.h>
static void mpxplay_debugf(FILE *fp,const char *format, ...)
{
 va_list ap;
 char sout[1024];

 va_start(ap,format);
 vsprintf(sout, format, ap );
 va_end(ap);

 if(fp){
  fprintf(fp,"%s\n",sout);
  fflush(fp);
  fclose(fp);
 }else  printf(sout);
}
#elif defined (__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define mpxplay_debugf(fmt, args...)	do {} while (0)
#else
#define mpxplay_debugf(...)		do {} while (0)
#endif

#endif /* LIBAU_DEF_H */
