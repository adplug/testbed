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
 * binfile.h - Binary file I/O
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_BINFILE
#define H_BINFILE

#include <stdio.h>

#include "binio.h"

class binfbase: virtual public binio
{
public:
  binfbase();

  virtual ~binfbase();

  virtual void open(const char *filename) = 0;
  void close();

  bool is_open();

  virtual void seek(unsigned long pos, Offset offs);

protected:
  FILE *f;
};

class binifstream: public binistream, public binfbase
{
public:
  binifstream();
  binifstream(const char *filename);

  virtual ~binifstream();

  virtual void open(const char *filename);
  virtual unsigned long read(void *buf, unsigned long length);

protected:
  virtual Byte getByte();
};

class binofstream: public binostream, public binfbase
{
public:
  binofstream();
  binofstream(const char *filename);

  virtual ~binofstream();

  virtual void open(const char *filename);
  virtual void write(const void *buf, unsigned long length);

protected:
  virtual void putByte(Byte b);
};

#endif
