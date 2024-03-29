//**************************************************************************
//*                     This file is part of the                           *
//*                      Mpxplay - audio player.                           *
//*                  The source code of Mpxplay is                         *
//*        (C) copyright 1998-2023 by PDSoft (Attila Padar)                *
//*                http://mpxplay.sourceforge.net                          *
//*                  email: mpxplay@freemail.hu                            *
//**************************************************************************
//*  This program is distributed in the hope that it will be useful,       *
//*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
//*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
//*  Please contact with the author (with me) if you want to use           *
//*  or modify this source.                                                *
//**************************************************************************
//function: PCI-BIOS handling
//based on a code of Taichi Sugiyama (YAMAHA)

#include <string.h>
#include <dos.h>
#ifdef __WATCOMC__
#include <conio.h>
#endif

#include "pcibios.h"

struct pci_config_s libau_pci = { 0,0,0,0,0,NULL,0 };

#define PCIBIOS_DIRECTPORT_RW 1

#if PCIBIOS_DIRECTPORT_RW
#ifdef __WATCOMC__
#define pcibios_outportl(reg,val) outpd(reg,val)
#define pcibios_inportl(reg) inpd(reg)
#endif
#ifdef __DJGPP__
#define pcibios_outportl(reg,val) outportl(reg,val)
#define pcibios_inportl(reg) inportl(reg)
#endif
#define PCI_IOPORT_ADDR  0x0CF8
#define PCI_IOPORT_DATA  0x0CFC
#define PCI_ENABLE_BIT   0x80000000
#define PCI_PORTADDR_VALUE(p, a) (PCI_ENABLE_BIT | ((uint32_t)(p)->bBus << 16) | ((uint32_t)(p)->bDev << 11) | ((uint32_t)(p)->bFunc << 8) | ((uint32_t)(a) & 0xFC))
#endif

#define PCIDEVNUM(bParam)      (bParam >> 3)
#define PCIFUNCNUM(bParam)     (bParam & 0x07)
#define PCIDEVFUNC(bDev,bFunc) ((((uint32_t)bDev) << 3) | bFunc)

static uint8_t pcibios_GetBus(void)
{
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_BIOS_PRESENT;
  reg.w.cflag=1;

  int386(PCI_SERVICE, &reg, &reg);

  if (reg.w.cflag)
    return 0;

  return 1;
}

uint8_t	pcibios_FindDevice(uint16_t wVendor, uint16_t wDevice, pci_config_s *ppkey)
{
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_FIND_DEVICE;
  reg.w.cx = wDevice;
  reg.w.dx = wVendor;
  reg.w.si = 0;  //bIndex;

  int386(PCI_SERVICE, &reg, &reg);

  if (ppkey && (reg.h.ah == PCI_SUCCESSFUL)) {
    ppkey->bBus  = reg.h.bh;
    ppkey->bDev  = PCIDEVNUM(reg.h.bl);
    ppkey->bFunc = PCIFUNCNUM(reg.h.bl);
    ppkey->vendor_id = wVendor;
    ppkey->device_id = wDevice;
  }

  return reg.h.ah;
}

uint8_t pcibios_search_devices(pci_device_s devices[],pci_config_s *ppkey)
{
  if (pcibios_GetBus()) {
    unsigned int i = 0;

    while (devices[i].vendor_id) {
      if (pcibios_FindDevice(devices[i].vendor_id,devices[i].device_id,ppkey) == PCI_SUCCESSFUL) {
        if (ppkey) {
          ppkey->device_name = devices[i].device_name;
          ppkey->device_type = devices[i].device_type;
        }
        return PCI_SUCCESSFUL;
      }

      i++;
    }
  }

  return PCI_DEVICE_NOTFOUND;
}

uint8_t pcibios_ReadConfig_Byte(pci_config_s *ppkey, uint16_t wAdr)
{
#if PCIBIOS_DIRECTPORT_RW
  const int shift = ((wAdr & 3) * 8);
  const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
  pcibios_outportl(PCI_IOPORT_ADDR, val);
  return (pcibios_inportl(PCI_IOPORT_DATA) >> shift) & 0xFF;

#else
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_READ_BYTE;
  reg.h.bh = ppkey->bBus;
  reg.h.bl = PCIDEVFUNC(ppkey->bDev, ppkey->bFunc);
  reg.w.di = wAdr;

  int386(PCI_SERVICE, &reg, &reg);

  return reg.h.cl;
#endif
}

uint16_t pcibios_ReadConfig_Word(pci_config_s *ppkey, uint16_t wAdr)
{
#if PCIBIOS_DIRECTPORT_RW
  if ((wAdr & 3) <= 2) {
    const int shift = ((wAdr & 3) * 8);
    const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
    pcibios_outportl(PCI_IOPORT_ADDR, val);
    return (pcibios_inportl(PCI_IOPORT_DATA) >> shift) & 0xFFFF;
  }

  return (uint16_t)pcibios_ReadConfig_Byte(ppkey, wAdr) | ((uint16_t)pcibios_ReadConfig_Byte(ppkey, wAdr + 1) << 8);

#else
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_READ_WORD;
  reg.h.bh = ppkey->bBus;
  reg.h.bl = PCIDEVFUNC(ppkey->bDev, ppkey->bFunc);
  reg.w.di = wAdr;

  int386(PCI_SERVICE, &reg, &reg);

  return reg.w.cx;
#endif
}

uint32_t pcibios_ReadConfig_Dword(pci_config_s *ppkey, uint16_t wAdr)
{
#if PCIBIOS_DIRECTPORT_RW
  if ((wAdr & 3) == 0) {
    const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
    pcibios_outportl(PCI_IOPORT_ADDR, val);
    return pcibios_inportl(PCI_IOPORT_DATA);
  }

  return ((uint32_t)pcibios_ReadConfig_Word(ppkey, (uint8_t)(wAdr + 2)) << 16L) | pcibios_ReadConfig_Word(ppkey, wAdr);

#else
  uint32_t dwData;

  dwData  = (uint32_t)pcibios_ReadConfig_Word(ppkey, wAdr + 2) << 16;
  dwData |= (uint32_t)pcibios_ReadConfig_Word(ppkey, wAdr);

  return dwData;
#endif
}

void pcibios_WriteConfig_Byte(pci_config_s *ppkey, uint16_t wAdr, uint8_t bData)
{
#if PCIBIOS_DIRECTPORT_RW
  const int shift = ((wAdr & 3) * 8);
  const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
  pcibios_outportl(PCI_IOPORT_ADDR, val);
  pcibios_outportl(PCI_IOPORT_DATA, (uint32_t)(pcibios_inportl(PCI_IOPORT_DATA) & ~(0xFFU << shift)) | ((uint32_t)bData << shift));

#else
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_WRITE_BYTE;
  reg.h.bh = ppkey->bBus;
  reg.h.bl = PCIDEVFUNC(ppkey->bDev, ppkey->bFunc);
  reg.h.cl = bData;
  reg.w.di = wAdr;

  int386(PCI_SERVICE, &reg, &reg);
#endif
}

void pcibios_WriteConfig_Word(pci_config_s *ppkey, uint16_t wAdr, uint16_t wData)
{
#if PCIBIOS_DIRECTPORT_RW
  if ((wAdr & 3) <= 2) {
    const int shift = ((wAdr & 3) * 8);
    const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
    pcibios_outportl(PCI_IOPORT_ADDR, val);
    pcibios_outportl(PCI_IOPORT_DATA, (pcibios_inportl(PCI_IOPORT_DATA) & ~(0xFFFFU << shift)) | ((uint32_t)wData << shift));
  } else {
    pcibios_WriteConfig_Byte(ppkey, wAdr    , (uint8_t)(wData & 0xFF));
    pcibios_WriteConfig_Byte(ppkey, wAdr + 1, (uint8_t)(wData >> 8));
  }

#else
  union REGS reg;

  memset(&reg,0,sizeof(reg));
  reg.h.ah = PCI_FUNCTION_ID;
  reg.h.al = PCI_WRITE_WORD;
  reg.h.bh = ppkey->bBus;
  reg.h.bl = PCIDEVFUNC(ppkey->bDev, ppkey->bFunc);
  reg.w.cx = wData;
  reg.w.di = wAdr;

  int386(PCI_SERVICE, &reg, &reg);
#endif
}

void pcibios_WriteConfig_Dword(pci_config_s *ppkey, uint16_t wAdr, uint32_t dwData)
{
#if PCIBIOS_DIRECTPORT_RW
  if ((wAdr & 3) == 0) {
    const uint32_t val = PCI_PORTADDR_VALUE(ppkey, wAdr);
    pcibios_outportl(PCI_IOPORT_ADDR, val);
    pcibios_outportl(PCI_IOPORT_DATA, dwData);
  } else {
    pcibios_WriteConfig_Word(ppkey, wAdr    , LoW(dwData));
    pcibios_WriteConfig_Word(ppkey, wAdr + 2, HiW(dwData));
  }
#else
  pcibios_WriteConfig_Word(ppkey, wAdr    , LoW(dwData));
  pcibios_WriteConfig_Word(ppkey, wAdr + 2, HiW(dwData));
#endif
}

void pcibios_set_master(pci_config_s *ppkey)
{
  unsigned int cmd;

  cmd = pcibios_ReadConfig_Byte(ppkey, PCIR_PCICMD);
  cmd |= 0x01|0x04;
  pcibios_WriteConfig_Byte(ppkey, PCIR_PCICMD, cmd);
}

void pcibios_enable_memmap_set_master(pci_config_s *ppkey)
{
  unsigned int cmd;

  cmd = pcibios_ReadConfig_Byte(ppkey, PCIR_PCICMD);
  cmd &= ~0x01;     // disable io-port mapping
  cmd |= 0x02|0x04; // enable memory mapping and set master
  pcibios_WriteConfig_Byte(ppkey, PCIR_PCICMD, cmd);
}
