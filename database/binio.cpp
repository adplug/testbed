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

binio::binio()
  : endian(Little)
{
}

binio::~binio()
{
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

  switch(endian) {
  case Little: final = (Word)((last << 8) + first); break;
  case Big: final = (Word)((first << 8) + last); break;
  }

  return final;
}

binio::DWord binistream::readDWord()
{
  Word first = readWord(), last = readWord();
  DWord final;

  switch(endian) {
  case Little: final = (DWord)((last << 16) + first); break;
  case Big: final = (DWord)((first << 16) + last); break;
  }

  return final;
}

binio::Double binistream::readDouble()
{
  DWord dw = readDWord();

  return *(Double *)&dw;
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
  switch(endian) {
  case Little: putByte(w & 0xff); putByte(w >> 8); break;
  case Big: putByte(w >> 8); putByte(w & 0xff); break;
  }
}

void binostream::writeDWord(DWord dw)
{
  switch(endian) {
  case Little: writeWord(dw & 0xffff); writeWord(dw >> 16); break;
  case Big: writeWord(dw >> 16); writeWord(dw & 0xffff); break;
  }
}

void binostream::writeDouble(Double d)
{
  writeDWord(*(DWord *)&d);
}

void binostream::writeString(const char *str)
{
  write(str, strlen(str));
}

void binostream::writeString(const std::string &str)
{
  write(str.c_str(), str.length());
}
