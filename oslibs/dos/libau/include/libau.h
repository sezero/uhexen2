#ifndef LIBAU_H
#define LIBAU_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void au_context;

struct auinfo_s {
 char          infostr[96];
 char         *card_DMABUFF;
 unsigned long card_dmasize;
 unsigned int  bytespersample_card;
 unsigned int  freq_card;
 unsigned int  bits_set;
 unsigned int  chan_set;
};

extern au_context *AU_search(unsigned int config);
extern const struct auinfo_s *AU_getinfo(au_context *ctx);
extern unsigned int AU_cardbuf_space(au_context *ctx);
extern void AU_start(au_context *ctx);
extern void AU_stop(au_context *ctx);
extern void AU_close(au_context *ctx);
extern void AU_setrate(au_context *ctx, unsigned int *fr, unsigned int *bt, unsigned int *ch);
extern void AU_setmixer_all(au_context *ctx, unsigned int vol);
extern void AU_writedata(au_context *ctx, const char *pcm, long len);

#ifdef __cplusplus
}
#endif

#endif /* LIBAU_H */
