/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (c) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * binwrap.h - Binary file I/O wrapper, using standard iostreams library
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_BINWRAP
#define H_BINWRAP

#include <iostream.h>

#include "binio.h"

class biniwstream: public binistream
{
public:
  biniwstream(istream &istr);

  virtual ~biniwstream();

  virtual void seek(unsigned long pos, Offset offs);
  virtual unsigned long read(void *buf, unsigned long length);

protected:
  virtual Byte getByte();

private:
  istream *in;
};

class binowstream: public binostream
{
public:
  binowstream(ostream &ostr);

  virtual ~binowstream();

  virtual void seek(unsigned long pos, Offset offs);
  virtual void write(const void *buf, unsigned long length);

protected:
  virtual void putByte(Byte);

private:
  ostream *out;
};

class binwstream
{
};

#endif
