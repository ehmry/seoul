/**
 * Directly-assigned PCI device.
 *
 * Copyright (C) 2007-2009, Bernhard Kauer <bk@vmmon.org>
 *
 * This file is part of Vancouver.
 *
 * Vancouver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vancouver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#include "nul/motherboard.h"
#include "host/hostpci.h"
#include "host/hostvf.h"
#include "model/pci.h"


/**
 * Directly assign a host PCI device to the guest.
 *
 * State: testing
 * Features: pcicfgspace, ioport operations, memory read/write, host irq, mem-alloc, DMA remapping
 * Missing: MSI, MSI-X
 * Documentation: PCI spec v.2.2
 */
class DirectPciDevice : public StaticReceiver<DirectPciDevice>, public HostVfPci
{
  enum {
    PCI_CFG_SPACE_DWORDS = 1024,
  };

  struct MsiXTableEntry {
    unsigned long long address;
    unsigned data;
    unsigned control;
  };

  Motherboard &_mb;
  unsigned  _bdf;
  unsigned  _irq_count;
  unsigned *_host_irqs;
  MsiXTableEntry *_msix_table;
  MsiXTableEntry *_msix_host_table;
  unsigned  _cfgspace[PCI_CFG_SPACE_DWORDS];
  unsigned  _bar_count;
  unsigned  _msi_cap;
  bool      _msi_64bit;
  unsigned  _msix_cap;
  unsigned  _msix_bar;
  struct {
    unsigned long size;
    char *   ptr;
    bool     io;
    unsigned short port;
  } _barinfo[MAX_BAR];

  const char *debug_getname() { return "DirectPciDevice"; }


  /**
   * Map the bars.
   */
  void map_bars(unsigned long long *bases, unsigned long long *sizes) {
    for (unsigned i=0; i < _bar_count; i++) {
      Logging::printf("%s %llx %llx\n", __func__, bases[i], sizes[i]);
      _barinfo[i].size = sizes[i];
      if (!bases[i]) continue;
      if ((bases[i] & 1) == 1) {
	_barinfo[i].io   = true;
	_barinfo[i].port = bases[i] & BAR_IO_MASK;

	MessageHostOp msg(MessageHostOp::OP_ALLOC_IOIO_REGION, (_barinfo[i].port << 8) |  Cpu::bsr(sizes[i] | 0x3));
	_mb.bus_hostop.send(msg);
      } else {
	_barinfo[i].io  = false;

	MessageHostOp msg(MessageHostOp::OP_ALLOC_IOMEM, bases[i] & ~0x1f, 1 << Cpu::bsr(((sizes[i] - 1) | 0xfff) + 1));
	if (_mb.bus_hostop.send(msg) && msg.ptr)
	  _barinfo[i].ptr = msg.ptr + (bases[i] & 0x10);
	else
	  Logging::panic("can not map IOMEM region %lx+%x %p", msg.value, msg.len, msg.ptr);
      }
    }
  }


  bool match_iobars(unsigned short port, unsigned short &newport) {

    // optimize access
    if (port < 0x100) return false;

    // check whether io decode is disabled
    if (~_cfgspace[1] & 1) return false;
    for (unsigned i=0; i < _bar_count; i++) {
      if (!_barinfo[i].io || (_cfgspace[4 + i] ^ port) & ~0x3u) continue;
      newport = _barinfo[i].port;
      return true;
    }
    return false;
  }

  /**
   * Check whether the guest mem address matches and translate to host pointer
   * address.
   */
  unsigned match_bars(unsigned long address, unsigned size, char *&ptr) {

    COUNTER_INC("PCIDirect::match");

    // mem decode is disabled?
    if (~_cfgspace[1] & 2)  return 0;


    for (unsigned i=0; i < _bar_count; i++) {

      // we assume that accesses with a size larger than the bar will never happen
      if (_barinfo[i].size < size || _barinfo[i].io || !in_range(address, _cfgspace[4 + i] & BAR_MEM_MASK, _barinfo[i].size - size + 1))
	continue;
      ptr = _barinfo[i].ptr + address - (_cfgspace[4 + i] & BAR_MEM_MASK);
      if (_msix_host_table && ptr >= reinterpret_cast<char *>(_msix_host_table) && ptr < reinterpret_cast<char *>(_msix_host_table + _irq_count))
	ptr = reinterpret_cast<char *>(_msix_table) + (reinterpret_cast<char *>(_msix_host_table) - ptr);
      return 4 + i;
    }
    return 0;
  }

 public:


  bool receive(MessageIOIn &msg)
  {
    unsigned old_port = msg.port;
    if (!match_iobars(old_port, msg.port))  return false;
    bool res = _mb.bus_hwioin.send(msg);
    msg.port = old_port;
    return res;
  }


  bool receive(MessageIOOut &msg)
  {
    unsigned old_port = msg.port;
    unsigned new_port = msg.port;
    if (!match_iobars(old_port, msg.port))  return false;
    msg.port = new_port;
    bool res = _mb.bus_hwioout.send(msg);
    msg.port = old_port;
    return res;
  }


  bool receive(MessagePciConfig &msg)
  {
    if (!msg.bdf)
      {
	assert(msg.dword < PCI_CFG_SPACE_DWORDS);
	if (msg.type == MessagePciConfig::TYPE_READ)
	  {
	    bool internal = in_range(msg.dword, 0x0, BAR0 + MAX_BAR);
	    if (_msi_cap)
	      internal = internal || in_range(msg.dword, _msi_cap, (_msi_64bit ? 4 : 3));

	    if (internal)
	      msg.value = _cfgspace[msg.dword];
	    else
	      msg.value = conf_read(_bdf, msg.dword);

	    // disable multi-function devices
	    if (msg.dword == 3)   msg.value &= ~0x800000;
	    //Logging::printf("%s:%x -- %8x,%8x\n", __PRETTY_FUNCTION__, _bdf, msg.dword, msg.value);
	    return true;
	  }
	else
	  {
	    unsigned mask = ~0u;
	    if (!msg.dword) mask = 0;
	    if (in_range(msg.dword, BAR0, BAR0 + MAX_BAR)) mask = ~(_barinfo[msg.dword - 4].size - 1);
	    if (_msi_cap) {
	      if (msg.dword == _msi_cap) mask = 0x710000;
	      if (msg.dword == (_msi_cap + 1)) mask = ~3u;
	      if (msg.dword == (_msi_cap + 2)) mask = ~0u;
	      if (msg.dword == (_msi_cap + (_msi_64bit ? 3 : 2))) mask = 0xffff;
	    }
	    if (~mask)
	      _cfgspace[msg.dword] = (_cfgspace[msg.dword] & ~mask) | (msg.value & mask);
	    else {
	      //write through
	      conf_write(_bdf, msg.dword, _cfgspace[msg.dword]);
	      _cfgspace[msg.dword] = conf_read(_bdf, msg.dword);
	    }
	    //Logging::printf("%s:%x -- %8x,%8x value %8x mask %x msi %x\n", __PRETTY_FUNCTION__, _bdf, msg.dword, _cfgspace[msg.dword], msg.value, mask, _msi_cap);
	  return true;
	  }
      }
    return false;
  }


  bool receive(MessageIrq &msg)
  {
    for (unsigned i = 0; i < _irq_count; i++)
      if (_host_irqs[i] == msg.line) {
	//Logging::printf("Irq message #%x  %x\n", msg.line, msg.type);
	unsigned long long msi_address = 0;
	unsigned           msi_data   = 0;

	// MSI enabled?
	if (_cfgspace[_msi_cap] & 0x10000) {
	  unsigned idx = _msi_cap;
	  msi_address = _cfgspace[++idx];
	  if (_cfgspace[_msi_cap] & 0x800000)
	    msi_address |= static_cast<unsigned long long>(_cfgspace[++idx]) << 32;
	  msi_data = _cfgspace[++idx] & 0xffff;
	  unsigned multiple_msgs = 1 << ((_cfgspace[_msi_cap] >> 20) & 0x7);
	  if (i < multiple_msgs) msi_data |= i;
	  //Logging::printf("Direct MSI %llx %x\n", msi_address, msi_data);
	}
	// MSI-X enabled?
	else if (_cfgspace[_msix_cap] >> 31 && _msix_table) {
	  msi_address = _msix_table[i].address;
	  msi_data = _msix_table[i].data;
	}
	else if (!i) {
	  MessageIrq msg2(msg.type, _cfgspace[15] & 0xff);
	  return _mb.bus_irqlines.send(msg2);
	}
	if (msi_address) {
	  // XXX FSB delivery
	  MessageIrq msg2(msg.type, msi_data & 0xff);
	  return _mb.bus_irqlines.send(msg2);
	}
      }
    return false;
  }


  bool receive(MessageIrqNotify &msg)
  {
    // XXX adopt for _msi cases
    unsigned irq = _cfgspace[15] & 0xff;
    if (in_range(irq, msg.baseirq, 8) && msg.mask & (1 << (irq & 0x7))) {
      //Logging::printf("Notify irq message #%x  %x -> %x\n", msg.mask, msg.baseirq, _host_irqs[0]);
      MessageHostOp msg2(MessageHostOp::OP_NOTIFY_IRQ, _host_irqs[0]);
      return _mb.bus_hostop.send(msg2);
    }
    return false;
  }


  bool receive(MessageMemAlloc &msg)
  {
    COUNTER_INC("PCIDirect::alloc");
    char *ptr;
    unsigned ofs = match_bars(msg.phys1 & ~0xfff, 0x1000, ptr);
    unsigned bar = _cfgspace[ofs];
    // there is a bar and it does not cross multiple pages and it is prefetchable?
    if (!ofs || msg.phys2 != ~0xffful || ~bar & 0x8)  return false;
    *msg.ptr = ptr + (msg.phys1 & 0xfff);
    return true;

  }

  bool receive(MessageMemWrite &msg)
  {

    char *ptr;
    if (!match_bars(msg.phys, msg.count, ptr))  return false;
    COUNTER_INC("PCIDirect::write");
    Logging::printf("%s %lx %p count %d\n", __PRETTY_FUNCTION__, msg.phys, ptr, msg.count);
    switch (msg.count) {
    case 4:
      *reinterpret_cast<unsigned       *>(ptr) = *reinterpret_cast<unsigned       *>(msg.ptr);
       // write msix control trough
       if (_msix_host_table && ptr >= reinterpret_cast<char *>(_msix_table) && ptr < reinterpret_cast<char *>(_msix_table + _irq_count) && (msg.phys & 0xf) == 0xc)
	_msix_host_table[(ptr - reinterpret_cast<char *>(_msix_table)) / 16].control = *reinterpret_cast<unsigned *>(msg.ptr);
      break;
      case 2:
	*reinterpret_cast<unsigned short *>(ptr) = *reinterpret_cast<unsigned short *>(msg.ptr);
	break;
      case 1:
	*reinterpret_cast<unsigned char  *>(ptr) = *reinterpret_cast<unsigned char  *>(msg.ptr);
	break;
      default:
	memcpy(ptr, msg.ptr, msg.count);
      }
    return true;
  }


  bool receive(MessageMemRead &msg)
  {
    char *ptr;
    if (!match_bars(msg.phys, msg.count, ptr))  return false;

    Logging::printf("%s %lx %p count %d\n", __PRETTY_FUNCTION__, msg.phys, ptr, msg.count);
    COUNTER_INC("PCIDirect::read");
    switch (msg.count)
      {
      case 4:
	*reinterpret_cast<unsigned       *>(msg.ptr) = *reinterpret_cast<unsigned       *>(ptr);
	break;
      case 2:
	*reinterpret_cast<unsigned short *>(msg.ptr) = *reinterpret_cast<unsigned short *>(ptr);
	break;
      case 1:
	*reinterpret_cast<unsigned char  *>(msg.ptr) = *reinterpret_cast<unsigned char  *>(ptr);
	break;
      default:
	memcpy(msg.ptr, ptr, msg.count);
      }
    return true;
  }

  bool  receive(MessageMemMap &msg) {
    char *ptr;
    unsigned ofs = match_bars(msg.phys & ~0xfff, 0x1000, ptr);
    if (!ofs) return false;
    if (_msix_host_table && ptr >= reinterpret_cast<char *>(_msix_table) && ptr < reinterpret_cast<char *>(_msix_table + _irq_count))
      return false;

    // XXX fix _msix clashes
    msg.ptr = ptr;
    msg.count = _barinfo[ofs - 4].size;
    Logging::printf(" MAP %lx+%x from %p\n", msg.phys, msg.count, msg.ptr);
    return true;
  }

  DirectPciDevice(Motherboard &mb, unsigned bdf, unsigned dstbdf, unsigned parent_bdf = 0, unsigned vf_no = 0)
    : HostVfPci(mb.bus_hwpcicfg, mb.bus_hostop), _mb(mb), _bdf(bdf), _msix_table(0), _msix_host_table(0), _bar_count(count_bars(_bdf))
  {
    if (parent_bdf)
      _bdf = bdf = vf_bdf(parent_bdf, vf_no);
    for (unsigned i=0; i < PCI_CFG_SPACE_DWORDS; i++) _cfgspace[i] = conf_read(_bdf, i);
    if (parent_bdf) _cfgspace[0] = vf_device_id(parent_bdf);
    Logging::printf("Our device ID is %04x.\n", _cfgspace[0]);

    MessageHostOp msg4(MessageHostOp::OP_ASSIGN_PCI, parent_bdf ? parent_bdf : _bdf, parent_bdf ? _bdf : 0);
    check0(!mb.bus_hostop.send(msg4), "DPCI: could not directly assign %x via iommu", bdf);

    unsigned long long bases[HostPci::MAX_BAR];
    unsigned long long sizes[HostPci::MAX_BAR];
    if (parent_bdf)
      read_all_vf_bars(parent_bdf, vf_no, bases, sizes);
    else
      read_all_bars(_bdf, bases, sizes);

    map_bars(bases, sizes);

    _msi_cap  = find_cap(_bdf, CAP_MSI);
    _msi_64bit = false;
    _msix_cap = find_cap(_bdf, CAP_MSIX);
    _msix_bar = ~0;
    _irq_count = 1;
    if (_msi_cap)  {
      _irq_count = 1 << ((_cfgspace[_msi_cap] >> 1) & 0x7);
      _msi_64bit = _cfgspace[_msi_cap] & 0x800000;
      // disable MSI
      _cfgspace[_msi_cap] &= ~0x10000;
    }
    if (_msix_cap) {
      unsigned msix_irqs = 1 + ((_cfgspace[_msix_cap] >> 16) & 0x7ff);
      if (_irq_count < msix_irqs) _irq_count = msix_irqs;
      _msix_table = new MsiXTableEntry[_irq_count];
      _msix_bar  = _cfgspace[1 + _msix_cap] & 0x7;
      _msix_host_table = reinterpret_cast<MsiXTableEntry *>(_barinfo[_msix_bar].ptr + (_cfgspace[1 + _msix_cap] & ~0x7));
      // disable MSIX
      _cfgspace[_msix_cap] = 0x7fffffff;
    }

    _host_irqs = new unsigned[_irq_count];
    for (unsigned i=0; i < _irq_count; i++)
      // XXX when do we need level?
      _host_irqs[i] = get_gsi(mb.bus_hostop, mb.bus_acpi, _bdf, i, i ? false : true, _msix_host_table);
    dstbdf = (dstbdf == 0) ? bdf : PciHelper::find_free_bdf(mb.bus_pcicfg, dstbdf);
    mb.bus_pcicfg.add(this, &DirectPciDevice::receive_static<MessagePciConfig>, dstbdf);
    mb.bus_ioin.add(this, &DirectPciDevice::receive_static<MessageIOIn>);
    mb.bus_ioout.add(this, &DirectPciDevice::receive_static<MessageIOOut>);
    mb.bus_memread.add(this, &DirectPciDevice::receive_static<MessageMemRead>);
    mb.bus_memwrite.add(this, &DirectPciDevice::receive_static<MessageMemWrite>);
    mb.bus_memmap.add(this, &DirectPciDevice::receive_static<MessageMemMap>);
    mb.bus_hostirq.add(this, &DirectPciDevice::receive_static<MessageIrq>);
    mb.bus_irqnotify.add(this, &DirectPciDevice::receive_static<MessageIrqNotify>);
  }
};


PARAM(dpci,
      {
	HostPci  pci(mb.bus_hwpcicfg, mb.bus_hostop);
	unsigned bdf = pci.search_device(argv[0], argv[1], argv[2]);

	check0(!bdf, "search_device(%lx,%lx,%lx) failed", argv[0], argv[1], argv[2]);
	Logging::printf("search_device(%lx,%lx,%lx) bdf %x \n", argv[0], argv[1], argv[2], bdf);

	new DirectPciDevice(mb, bdf, argv[3]);
	Logging::printf("dpci done\n");
      },
      "dpci:class,subclass,instance,bdf - makes the specified hostdevice directly accessible to the guest.",
      "Example: Use 'dpci:2,,0,0x21' to attach the first network controller to 00:04.1.",
      "If class or subclass is ommited it is not compared. If the instance is ommited the last instance is used.",
      "If bdf is zero the very same bdf as in the host is used, if it is ommited a free bdf is used.");


#include "host/hostvf.h"

PARAM(vfpci,
      {
	HostVfPci pci(mb.bus_hwpcicfg, mb.bus_hostop);
	unsigned parent_bdf = argv[0];
	unsigned vf_no      = argv[1];

	// Check if VF exists, before creating the object.
	unsigned vf_bdf = pci.vf_bdf(parent_bdf, vf_no);
	check0(!vf_bdf, "XXX VF%d does not exist in parent %x.", vf_no, parent_bdf);
	Logging::printf("VF is at %04x.\n", vf_bdf);

	new DirectPciDevice(mb, 0, argv[2], parent_bdf, vf_no);
      },
      "vfpci:parent_bdf,vf_no,guest_bdf - directly assign a given virtual function to the guest.",
      "if no guest_bdf is given, a free one is used.");
