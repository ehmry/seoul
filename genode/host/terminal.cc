/** @file
 * Serial terminal via Genode Terminal session.
 *
 * Copyright (C) 2019, Emery Hemingway <ehmry@posteo.net>
 *
 * This file is part of Seoul.
 *
 * Seoul is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Vancouver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#include <nul/motherboard.h>

/* Genode includes */
#include <terminal_session/connection.h>
#include <base/thread.h>

/* VMM utils */
#include <vmm/utcb_guard.h>

extern Genode::Env *_seoul_env;

namespace Host { class Terminal; }

/**
 * A Terminal sends and receives data over a serial bus
 * via a Genode Terminal session.
 *
 * State: experimental
 */
class Host::Terminal : public StaticReceiver<Terminal>
{
	DBus<MessageSerial> & _bus_serial;
	unsigned const _hdev;

	::Terminal::Connection _terminal {
		*_seoul_env, Genode::String<8>((Genode::Hex)_hdev).string() };
	Genode::Io_signal_handler<Terminal> _host_handler {
		_seoul_env->ep(), *this, &Terminal::_handle_host };

	void _handle_host()
	{
		while (_terminal.avail()) {
			MessageSerial msg(_hdev-1, 0);
			_terminal.read(&msg.ch, 1);
			_bus_serial.send(msg);
		}
	}

private:

	/*
	 * Noncopyable
	 */
	Terminal(Terminal const &);
	Terminal &operator = (Terminal const &);

public:

	bool  receive(MessageSerial &msg)
	{
		typedef Vmm::Utcb_guard::Utcb_backup Utcb_backup;
		static Utcb_backup _utcb_backup;

		if (msg.serial != _hdev)   return false;

		_utcb_backup = *(Utcb_backup *)Genode::Thread::myself()->utcb();
		_terminal.write(&msg.ch, 1);
		*(Utcb_backup *)Genode::Thread::myself()->utcb() = _utcb_backup;

		return true;
	}

	Terminal(DBus<MessageSerial> &bus_serial, unsigned hdev)
	: _bus_serial(bus_serial), _hdev(hdev)
	{
		_terminal.read_avail_sigh(_host_handler);
	}
};

PARAM_HANDLER(terminal,
	"terminal:hostdevnr - provide an output for a serial port.",
	"Example: 'terminal:0x4712'.")
{
	mb.bus_serial.add(
		new Host::Terminal(mb.bus_serial, argv[0]),
		Host::Terminal::receive_static<MessageSerial>);
}
