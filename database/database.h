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

#define HASH_RADIX	0xfff1		// should be prime

class CAdPlugDatabase
{
public:
	class CRecord
	{
	public:
	  typedef unsigned char	Key[6];
	  typedef enum { Plain, SongInfo, ClockSpeed } RecordType;

	  RecordType		type;
	  unsigned long		size;
	  Key			key;
	  CFileType::FileType	filetype;

	  static CRecord *factory(RecordType type);
	  static CRecord *read(istream &in);

	  CRecord();
	  virtual ~CRecord();

	  void write(ostream &out);

	protected:
	  virtual void read_own(istream &in) {}
	  virtual void write_own(ostream &out) {}
	};

	static void make_key(unsigned char *buffer, long buffer_size,
			     CRecord::Key key);

	CAdPlugDatabase();
	~CAdPlugDatabase();

	bool	load(const char *db_name);
	bool	load(istream &f);
	bool	save(const char *db_name);
	bool	save(ostream &f);

	bool    insert(CRecord *record);

	void	wipe(CRecord *record);
	void	wipe();

	CRecord *search(CRecord::Key key);
	bool	lookup(CRecord::Key key);

	CRecord	*get_record();

	bool	go_forward();
	bool	go_backward();

	void	goto_begin();
	void	goto_end();

private:
	class DB_Bucket
	{
	public:
	  unsigned long	index;
	  bool		deleted;
	  DB_Bucket	*chain;

	  CRecord	*record;

	  DB_Bucket(CRecord *newrecord, DB_Bucket *newchain = 0);
	  ~DB_Bucket();

	  static unsigned long linear_length();

	private:
	  static unsigned long mainindex;
	};

	DB_Bucket	*db_linear[HASH_RADIX];
	DB_Bucket	*db_hashed[HASH_RADIX];

	unsigned long	linear_index, linear_logic_length;

	unsigned long make_hash(CRecord::Key key);
};

class CInfoRecord: public CAdPlugDatabase::CRecord
{
public:
  std::string	title;
  std::string	author;

  CInfoRecord();

protected:
  virtual void read_own(istream &in);
  virtual void write_own(ostream &out);
};

class CClockRecord: public CAdPlugDatabase::CRecord
{
public:
  float	clock;

  CClockRecord();

protected:
  virtual void read_own(istream &in);
  virtual void write_own(ostream &out);
};

#endif
