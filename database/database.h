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
 * database.h - AdPlug database class
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_DATABASE
#define H_DATABASE

#include <string>

#include "filetype.h"
#include "binio.h"

class CAdPlugDatabase
{
public:
  class CKey
  {
  public:
    unsigned short crc16;
    unsigned long crc32;

    CKey() {};
    CKey(binistream &in);

    bool operator==(const CKey &key);

  private:
    void make(binistream &in);
  };

  class CRecord
  {
  public:
    typedef enum { Plain, SongInfo, ClockSpeed } RecordType;

    RecordType		type;
    CKey		key;
    CFileType::FileType	filetype;

    static CRecord *factory(RecordType type);
    static CRecord *factory(binistream &in);

    CRecord();
    virtual ~CRecord();

    void write(binostream &out);

  protected:
    virtual void read_own(binistream &in) {}
    virtual void write_own(binostream &out) {}
    virtual unsigned long get_size();
  };

  CAdPlugDatabase();

  ~CAdPlugDatabase();

  bool	load(const char *db_name);
  bool	load(binistream &f);
  bool	save(const char *db_name);
  bool	save(binostream &f);

  bool	insert(CRecord *record);

  void	wipe(CRecord *record);
  void	wipe();

  CRecord	*search(CKey const &key);
  bool		lookup(CKey const &key);

  CRecord *get_record();

  bool	go_forward();
  bool	go_backward();

  void	goto_begin();
  void	goto_end();

 private:
  static const unsigned short hash_radix = 0xfff1;	// should be prime

  class DB_Bucket
  {
  public:
    unsigned long	index;
    bool		deleted;
    DB_Bucket		*chain;

    CRecord		*record;

    DB_Bucket(CRecord *newrecord, DB_Bucket *newchain = 0);
    ~DB_Bucket();

    static unsigned long linear_length();

  private:
    static unsigned long mainindex;
  };

  DB_Bucket	*db_linear[hash_radix];
  DB_Bucket	*db_hashed[hash_radix];

  unsigned long	linear_index, linear_logic_length;

  unsigned long make_hash(CKey const &key);
};

class CInfoRecord: public CAdPlugDatabase::CRecord
{
public:
  std::string	title;
  std::string	author;

  CInfoRecord();

protected:
  virtual void read_own(binistream &in);
  virtual void write_own(binostream &out);
  virtual unsigned long get_size();
};

class CClockRecord: public CAdPlugDatabase::CRecord
{
public:
  float	clock;

  CClockRecord();

protected:
  virtual void read_own(binistream &in);
  virtual void write_own(binostream &out);
  virtual unsigned long get_size();
};

#endif
