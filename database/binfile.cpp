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

#include <stdio.h>

#include "binfile.h"

/***** binfbase *****/

binfbase::binfbase()
  : f(0)
{
}

binfbase::~binfbase()
{
  close();
}

void binfbase::close()
{
  if(f) fclose(f);
}

void binfbase::seek(unsigned long pos, Offset offs)
{
  switch(offs) {
  case Start: fseek(f, pos, SEEK_SET); break;
  case Add: fseek(f, pos, SEEK_CUR); break;
  case End: fseek(f, pos, SEEK_END); break;
  }
}

bool binfbase::eof()
{
  return feof(f);
}

bool binfbase::is_open()
{
  return (f ? true : false);
}

/***** binifstream *****/

binifstream::binifstream()
{
}

binifstream::binifstream(const char *filename)
{
  open(filename);
}

binifstream::~binifstream()
{
}

void binifstream::open(const char *filename)
{
  f = fopen(filename, "rb");
}

unsigned long binifstream::read(void *buf, unsigned long length)
{
  return fread(buf, length, 1, f);
}

binio::Byte binifstream::getByte()
{
  return (Byte)fgetc(f);
}

/***** binofstream *****/

binofstream::binofstream()
{
}

binofstream::binofstream(const char *filename)
{
  open(filename);
}

binofstream::~binofstream()
{
}

void binofstream::open(const char *filename)
{
  f = fopen(filename, "wb");
}

void binofstream::write(const void *buf, unsigned long length)
{
  fwrite(buf, length, 1, f);
}

void binofstream::putByte(Byte b)
{
  fputc(b, f);
}
