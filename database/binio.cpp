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
 * binio.cpp - Binary stream I/O classes
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include <string>
#include <string.h>

#include "binio.h"

/***** Defines *****/

// String buffer size for std::string readString() method
#define STRINGBUFSIZE	256

/***** binio *****/

binio::Flags binio::system_flags = binio::detect_system_flags();

binio::binio()
  : order(Norm), my_flags(system_flags)
{
}

binio::~binio()
{
}

binio::Flags binio::detect_system_flags()
{
  Flags f = 0;
  union {
    Word word;
    Byte byte;
  } endian_test;

  // Endian test
  endian_test.word = 0x0102;
  switch(endian_test.byte) {
  case 0x01: f |= BigEndian; break;
  case 0x02: f &= !BigEndian; break;
  default: /* Error! Throw exception... */ break;
  }

  return f;
}

void binio::set_flag(Flag f, bool set)
{
  if(set) my_flags |= f; else my_flags &= !f;

  /*** Evaluate new flag ***/
  switch(f) {
  case BigEndian:
    if(set == system_flags & BigEndian) order = Norm; else order = Swap;
    break;
  }
}

bool binio::get_flag(Flag f)
{
  return (my_flags & f);
}

/***** binistream *****/

binistream::binistream()
{
}

binistream::~binistream()
{
}

binio::Byte binistream::readByte()
{
  return getByte();
}

binio::Word binistream::readWord()
{
  Byte first = getByte(), last = getByte();
  Word final;

  switch(order) {
  case Norm: final = ((Word)first << 8) + last; break;
  case Swap: final = ((Word)last << 8) + first; break;
  }

  return final;
}

binio::DWord binistream::readDWord()
{
  Word first = readWord(), last = readWord();
  DWord final;

  switch(order) {
  case Norm: final = ((DWord)first << 16) + last; break;
  case Swap: final = ((DWord)last << 16) + first; break;
  }

  return final;
}

binio::QWord binistream::readQWord()
{
  DWord first = readDWord(), last = readDWord();
  QWord final;

  switch(order) {
  case Norm: final = ((QWord)first << 32) + last; break;
  case Swap: final = ((QWord)last << 32) + first; break;
  }

  return final;
}

binio::Float binistream::readFloat()
{
  DWord dw = readDWord();
  return *(Float *)&dw;
}

binio::Double binistream::readDouble()
{
  QWord qw = readQWord();
  return *(Double *)&qw;
}

unsigned long binistream::readString(char *str, unsigned long maxlen,
				     char delim)
{
  unsigned long i;

  for(i = 0; i < maxlen; i++) {
    str[i] = (char)getByte();
    if(str[i] == delim) {
      str[i] = '\0';
      return i;
    }
  }

  return maxlen;
}

std::string binistream::readString(char delim)
{
  char buf[STRINGBUFSIZE];
  std::string tempstr;
  unsigned long read;

  do {
    read = readString(buf, STRINGBUFSIZE, delim);
    tempstr.append(buf, read);
  } while(read == STRINGBUFSIZE);

  return tempstr;
}

void binistream::ignore(unsigned long amount)
{
  unsigned long i;

  for(i = 0; i < amount; i++)
    getByte();
}

/***** binostream *****/

binostream::binostream()
{
}

binostream::~binostream()
{
}

void binostream::writeByte(Byte b)
{
  putByte(b);
}

void binostream::writeWord(Word w)
{
  switch(order) {
  case Norm: putByte(w >> 8); putByte(w & 0xff); break;
  case Swap: putByte(w & 0xff); putByte(w >> 8); break;
  }
}

void binostream::writeDWord(DWord dw)
{
  switch(order) {
  case Norm: writeWord(dw >> 16); writeWord(dw & 0xffff); break;
  case Swap: writeWord(dw & 0xffff); writeWord(dw >> 16); break;
  }
}

void binostream::writeQWord(QWord dw)
{
  switch(order) {
  case Norm: writeDWord(dw >> 32); writeDWord(dw & 0xffffffff); break;
  case Swap: writeDWord(dw & 0xffffffff); writeDWord(dw >> 32); break;
  }
}

void binostream::writeFloat(Float f)
{
  writeDWord(*(DWord *)&f);
}

void binostream::writeDouble(Double d)
{
  writeQWord(*(QWord *)&d);
}

void binostream::writeString(const char *str)
{
  write(str, strlen(str));
}

void binostream::writeString(const std::string &str)
{
  write(str.c_str(), str.length());
}
