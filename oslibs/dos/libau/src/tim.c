#include "libaudef.h"
#include <time.h>

void pds_delay_10us(unsigned int ticks) // each tick is 10us
{
  unsigned int i, oldtsc, tsctemp, tscdif;

  _disable();

  for (i = 0; i < ticks; i++) {
    outb(0x43, 64); // chanel 1 only read (NOT modify!) chanels 0 & 2 is free!
    oldtsc = inb(0x41);

    do {
      outb(0x43, 64);
      tsctemp = inb(0x41);
      if (tsctemp <= oldtsc)
          tscdif = oldtsc - tsctemp; // handle overflow
      else tscdif = 18 + oldtsc - tsctemp;
    } while (tscdif < 12); // wait for 10us 12/1193180 sec
  }

  _enable();
}

int64_t pds_gettimeu(void)
{
  return (int64_t)clock() * (int64_t)1000000 / (int64_t)CLOCKS_PER_SEC; // clock time in usec
}
