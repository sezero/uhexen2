#include "libaudef.h"

unsigned int MDma_get_max_pcmoutbufsize(unsigned int pagesize, unsigned int samplesize)
{
	unsigned int bufsize;

	if(!pagesize)
		pagesize=AUCARDS_DMABUFSIZE_BLOCK;

	if(samplesize<2)
		samplesize=2;

	bufsize=AUCARDS_DMABUFSIZE_NORMAL;
	// 2x bufsize at 32-bit output (1.5x at 24)
	bufsize*=samplesize;
	bufsize+=(pagesize-1);		// rounding up to pagesize
	bufsize-=(bufsize%pagesize);

	if(bufsize>AUCARDS_DMABUFSIZE_MAX)
		bufsize=AUCARDS_DMABUFSIZE_MAX-(AUCARDS_DMABUFSIZE_MAX%pagesize);

	return bufsize;
}

unsigned int MDma_init_pcmoutbuf(struct mpxplay_audioout_info_s *aui, unsigned int maxbufsize, unsigned int pagesize)
{
	unsigned int dmabufsize, bit_width, tmp;

	switch(aui->card_wave_id)
	{
	case MPXPLAY_WAVEID_PCM_FLOAT:
		bit_width=32;
		break;

	default:
		bit_width=aui->bits_card;
		break;
	}

	dmabufsize=maxbufsize;

	dmabufsize+=(pagesize-1);		// rounding up to pagesize
	dmabufsize-=(dmabufsize%pagesize);

	if(dmabufsize<(pagesize*2))
		dmabufsize=(pagesize*2);

	if(dmabufsize>maxbufsize)
		dmabufsize=maxbufsize-(maxbufsize%pagesize);

	funcbit_smp_value_put(aui->card_bytespersign,aui->chan_card*((bit_width+7)/8));
	funcbit_smp_value_put(aui->card_dmasize,dmabufsize);
	funcbit_smp_value_put(aui->card_dma_lastgoodpos,0); // !!! the soundcard also must to do this
	tmp = aui->card_dmasize/2;
	tmp-= aui->card_dmalastput%aui->card_bytespersign; // round down to pcm_samples
	funcbit_smp_value_put(aui->card_dmalastput,tmp);
	funcbit_smp_value_put(aui->card_dmafilled,aui->card_dmalastput);
	funcbit_smp_value_put(aui->card_dmaspace,aui->card_dmasize-aui->card_dmafilled);

	return dmabufsize;
}

void MDma_clearbuf(struct mpxplay_audioout_info_s *aui)
{
#ifdef ZDM
	unsigned long addr = (unsigned int)aui->card_DMABUFF;
	int i;

	_farsetsel(_dos_ds);
	for (i = 0; i < aui->card_dmasize; i++, addr++)
	{
		_farnspokeb(addr, 0);
	}
#else
	memset(aui->card_DMABUFF,0,aui->card_dmasize);
#endif /* ZDM */
}

unsigned int MDma_bufpos(struct mpxplay_audioout_info_s *aui)
{
	unsigned int bufpos = aui->card_handler->cardbuf_pos(aui);

	if(bufpos>=aui->card_dmasize)
		bufpos=0;		// checking
	else {
		bufpos-=(bufpos%aui->card_bytespersign);	// round
	}

#ifndef SDR
	if(aui->card_infobits&AUINFOS_CARDINFOBIT_DMAUNDERRUN)
	//sets a new put-pointer in this case
#endif
	{
		if(bufpos>=aui->card_outbytes) {
			aui->card_dmalastput=bufpos-aui->card_outbytes;
		}
		else {
			aui->card_dmalastput=aui->card_dmasize+bufpos-aui->card_outbytes;
		}

		funcbit_smp_disable(aui->card_infobits,AUINFOS_CARDINFOBIT_DMAUNDERRUN);
	}

	return bufpos;
}

void MDma_writedata(struct mpxplay_audioout_info_s *aui,const char *src,unsigned long left)
{
	unsigned int todo;

	aui->card_outbytes = left;

#ifdef SDR
//	MDma_bufpos();
#endif

	todo = aui->card_dmasize-aui->card_dmalastput;

	if(todo<=left) {
#ifdef ZDM
		dosmemput(src,todo,(unsigned int)aui->card_DMABUFF+aui->card_dmalastput);
#else
		memcpy(aui->card_DMABUFF+aui->card_dmalastput,src,todo);
#endif /* ZDM */

		aui->card_dmalastput=0;
		left-=todo;
		src+=todo;
	}

	if(left) {
#ifdef ZDM
		dosmemput(src,left,(unsigned int)aui->card_DMABUFF+aui->card_dmalastput);
#else
		memcpy(aui->card_DMABUFF+aui->card_dmalastput,src,left);
#endif
		aui->card_dmalastput+=left;
	}

	if(aui->card_handler->cardbuf_writedata) {
		aui->card_handler->cardbuf_writedata(aui);
	}
}
