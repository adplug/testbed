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
 * database.cpp - AdPlug database class
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include <fstream.h>
#include <string.h>

#include "database.h"

#define DB_FILEID	"AdPlug Module Information Database\x10"

/***** CAdPlugDatabase *****/

CAdPlugDatabase::CAdPlugDatabase()
  : linear_index(0), linear_length(0), linear_logic_length(0)
{
	memset(db_linear,0,sizeof(DB_Bucket *) * HASH_RADIX);
	memset(db_hashed,0,sizeof(DB_Bucket *) * HASH_RADIX);
}

CAdPlugDatabase::~CAdPlugDatabase()
{
	for(unsigned long i=0;i<linear_length;i++)
		delete db_linear[i];
}

bool CAdPlugDatabase::load(const char *db_name)
{
	ifstream f(db_name, ios::in | ios::binary);

	if (!f.is_open()) return false;
	return load(f);
}

bool CAdPlugDatabase::load(istream &f)
{
	// read id
	char id[35];

	f.read(id,35);

	if (memcmp(id,DB_FILEID,35))
		return false;

	// read length
	unsigned long length;

	f.read((char *)&length,4);

	// read records
	for (unsigned long i=0;i<length;i++)
	  insert(CRecord::read(f));

	return true;
}

bool CAdPlugDatabase::save(const char *db_name)
{
	ofstream f(db_name, ios::out | ios::trunc | ios::binary);

	if(!f.is_open()) return false;
	return save(f);
}

bool CAdPlugDatabase::save(ostream &f)
{
	// write id
	f.write(DB_FILEID,35);

	// write length
	f.write((char *)&linear_logic_length, 4);

	// write records
	for (unsigned long i=0;i<linear_length;i++)
	  if (!db_linear[i]->deleted)
	    db_linear[i]->record->write(f);

	return true;
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::search(CRecord::Key key)
{
  lookup(key);
  return get_record();
}

bool CAdPlugDatabase::lookup(CRecord::Key key)
{
	long index = make_hash(key);

	if(!db_hashed[index]) return false;

	// immediate hit ?
	DB_Bucket *bucket = db_hashed[index];

	if (!memcmp(bucket->record->key,key,6))
	{
		linear_index = bucket->index;
		return true;
	}

	// in-chain hit ?
	bucket = db_hashed[index]->chain;

	while (bucket)
	{
		if (!memcmp(bucket->record->key,key,6))
		{
			linear_index = bucket->index;
			return true;
		}

		bucket = bucket->chain;
	}

	return false;
}

bool CAdPlugDatabase::insert(CRecord *record)
{
  if(!record) return false;
  if(linear_length == HASH_RADIX) return false;
  if(search(record->key)) return false;

	// make bucket
	DB_Bucket *bucket = new DB_Bucket;
	if(!bucket) return false;

	bucket->record	= record;
	bucket->index   = linear_length;
	bucket->deleted = false;
	bucket->chain   = NULL;

	// add to linear list
	db_linear[linear_length] = bucket;
	linear_length++; linear_logic_length++;

	// add to hashed list
	long index = make_hash(record->key);

	if (!db_hashed[index])
		db_hashed[index] = bucket;
	else
	{
		DB_Bucket *chain = db_hashed[index];

		while (chain->chain)
			chain = (DB_Bucket *)chain->chain;

		chain->chain = bucket;
	}

	return true;
}

void CAdPlugDatabase::wipe(CRecord *record)
{
  if (!linear_length || !lookup(record->key)) return;

  DB_Bucket *bucket = db_linear[linear_index];

  if (!bucket->deleted) {
    delete bucket->record;
    linear_logic_length--;
    bucket->deleted = true;
  }
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::get_record()
{
  if(!linear_length) return 0;
  return db_linear[linear_index]->record;
}

bool CAdPlugDatabase::go_forward()
{
	if (!(linear_length - linear_index - 1))
		return false;

	linear_index++;

	return true;
}

bool CAdPlugDatabase::go_backward()
{
	if (!linear_index)
		return false;

	linear_index--;

	return true;
}

void CAdPlugDatabase::goto_begin()
{	
	if (linear_length)
		linear_index = 0;
}

void CAdPlugDatabase::goto_end()
{
	if (linear_length)
		linear_index = linear_length - 1;
}

void CAdPlugDatabase::make_key(unsigned char *buffer, long buffer_size, CRecord::Key key)
/*
 * Key is CRC16:CRC32 pair. CRC16 and CRC32 calculation routines (c) Zhengxi
 *
 */
{
	static const unsigned short magic16 = 0xa001;
	static const unsigned long  magic32 = 0xedb88320;

	unsigned short crc16 =  0;
	unsigned long  crc32 = ~0;

	for (long i=0;i<buffer_size;i++)
	{
		unsigned char byte = buffer[i];

		for (int j=0;j<8;j++)
		{
			if ((crc16 ^ byte) & 1)
				crc16 = (crc16 >> 1) ^ magic16;
			else
				crc16 >>= 1;

			if ((crc32 ^ byte) & 1)
				crc32 = (crc32 >> 1) ^ magic32;
			else
				crc32 >>= 1;

			byte >>= 1;
		}
	}

	crc16 &= 0xffff;
	crc32  = ~crc32;

	*(unsigned short *)&key[4] = crc16;
	*(unsigned long  *)&key[0] = crc32;
}

unsigned long CAdPlugDatabase::make_hash(CRecord::Key key)
{
	return (((*(unsigned long *)&key[0]) + (*(unsigned short *)&key[4])) % HASH_RADIX);
}

/***** CAdPlugDatabase::CRecord *****/

CAdPlugDatabase::CRecord *CAdPlugDatabase::CRecord::factory(RecordType type)
{
  switch(type) {
  case Plain: return new CRecord;
  case SongInfo: return new CInfoRecord;
  case ClockSpeed: return new CClockRecord;
  default: return 0;
  }
}

CAdPlugDatabase::CRecord::CRecord()
  : type(Plain), size(sizeof(CRecord)), filetype(CFileType::Undefined)
{
  memset(key, 0, sizeof(key));
}

CAdPlugDatabase::CRecord::~CRecord()
{
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::CRecord::read(istream &in)
{
  RecordType	type;
  unsigned long	size;
  CRecord	*rec;

  in >> (unsigned int)type >> size;
  rec = factory(type);

  if(rec) {
    rec->size = size;
    in >> rec->key >> (unsigned int)rec->filetype;
    rec->read_own(in);
    return rec;
  } else {
    // skip this record, cause we don't know about it
    in.seekg(size, ios::cur);
    return 0;
  }
}

void CAdPlugDatabase::CRecord::write(ostream &out)
{
  out << type;
  out << size;
  out << key;
  out << filetype;

  write_own(out);
}

/***** CInfoRecord *****/

CInfoRecord::CInfoRecord()
{
  type = SongInfo;
}

void CInfoRecord::read_own(istream &in)
{
  in >> title;
  in >> author;
}

void CInfoRecord::write_own(ostream &out)
{
}

/***** CClockRecord *****/

CClockRecord::CClockRecord()
  : clock(0.0f)
{
  type = ClockSpeed;
}

void CClockRecord::read_own(istream &in)
{
  in >> clock;
}

void CClockRecord::write_own(ostream &out)
{
  out << clock;
}
