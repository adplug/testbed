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
 * binio.h - Binary stream I/O classes
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_BINIO
#define H_BINIO

#include <string>

class binio
{
public:
  typedef enum { Little, Big } Endian;
  typedef enum { Start, Add, End } Offset;

  typedef char		Byte;
  typedef short		Word;
  typedef long		DWord;
  typedef double	Double;

  Endian endian;

  binio();

  virtual ~binio();

  virtual void seek(unsigned long, Offset = Start) = 0;
};

class binistream: virtual public binio
{
public:
  binistream();

  virtual ~binistream();

  Byte readByte();
  Word readWord();
  DWord readDWord();
  Double readDouble();

  unsigned long readString(char *str, unsigned long maxlen, char delim = '\0');
  std::string readString(char delim = '\0');

  virtual unsigned long read(void *, unsigned long) = 0;

  void ignore(unsigned long amount = 1);

protected:
  virtual Byte getByte() = 0;
};

class binostream: virtual public binio
{
public:
  binostream();
  binostream(ostream &stream);

  virtual ~binostream();

  void writeByte(Byte b);
  void writeWord(Word w);
  void writeDWord(DWord dw);
  void writeDouble(Double d);

  void writeString(const char *str);
  void writeString(const std::string &str);

  virtual void write(const void *, unsigned long) = 0;

protected:
  virtual void putByte(Byte) = 0;
};

class binstream: public binistream, public binostream
{
public:

};

#endif
