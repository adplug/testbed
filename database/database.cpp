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

#include "binio.h"
#include "binfile.h"
#include "database.h"

#define DB_FILEID	"AdPlug Module Information Database 1.0\x10"

/***** CAdPlugDatabase *****/

CAdPlugDatabase::CAdPlugDatabase()
  : linear_index(0), linear_logic_length(0)
{
  memset(db_linear,0,sizeof(DB_Bucket *) * hash_radix);
  memset(db_hashed,0,sizeof(DB_Bucket *) * hash_radix);
}

CAdPlugDatabase::~CAdPlugDatabase()
{
  for(unsigned long i=0;i<DB_Bucket::linear_length();i++)
    delete db_linear[i];
}

bool CAdPlugDatabase::load(const char *db_name)
{
  binifstream f(db_name);
  if(!f.is_open()) return false;
  return load(f);
}

bool CAdPlugDatabase::load(binistream &f)
{
  unsigned int idlen = strlen(DB_FILEID);
  char *id = new char [idlen];
  unsigned long length;

  f.read(id,idlen);
  if(memcmp(id,DB_FILEID,idlen)) {
    delete [] id;
    return false;
  }
  length = f.readDWord();

  // read records
  for (unsigned long i=0;i<length;i++)
    insert(CRecord::factory(f));

  delete [] id;
  return true;
}

bool CAdPlugDatabase::save(const char *db_name)
{
  binofstream f(db_name);
  if(!f.is_open()) return false;
  return save(f);
}

bool CAdPlugDatabase::save(binostream &f)
{
  f.writeString(DB_FILEID);
  f.writeDWord(linear_logic_length);

  // write records
  for(unsigned long i=0;i<DB_Bucket::linear_length();i++)
    if(!db_linear[i]->deleted)
      db_linear[i]->record->write(f);

  return true;
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::search(CKey const &key)
{
  lookup(key);
  return get_record();
}

bool CAdPlugDatabase::lookup(CKey const &key)
{
  long index = make_hash(key);

  if(!db_hashed[index]) return false;

  // immediate hit ?
  DB_Bucket *bucket = db_hashed[index];

  if(bucket->record->key == key)
    {
      linear_index = bucket->index;
      return true;
    }

  // in-chain hit ?
  bucket = db_hashed[index]->chain;

  while (bucket)
    {
      if(bucket->record->key == key)
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
  unsigned long linear_length = DB_Bucket::linear_length();

  if(!record) return false;			// null-pointer given
  if(DB_Bucket::linear_length() == hash_radix) return false;	// max. db size exceeded
  if(lookup(record->key)) return false;		// record already in db

  // make bucket
  DB_Bucket *bucket = new DB_Bucket(record);
  if(!bucket) return false;

  // add to linear list
  db_linear[linear_length] = bucket;
  linear_logic_length++;

  // add to hashed list
  long index = make_hash(record->key);

  if (!db_hashed[index])
    db_hashed[index] = bucket;
  else
    {
      DB_Bucket *chain = db_hashed[index];

      while (chain->chain)
	chain = chain->chain;

      chain->chain = bucket;
    }

  return true;
}

void CAdPlugDatabase::wipe(CRecord *record)
{
  if(!lookup(record->key)) return;
  wipe();
}

void CAdPlugDatabase::wipe()
{
  if (!DB_Bucket::linear_length()) return;

  DB_Bucket *bucket = db_linear[linear_index];

  if (!bucket->deleted) {
    delete bucket->record;
    linear_logic_length--;
    bucket->deleted = true;
  }
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::get_record()
{
  if(!DB_Bucket::linear_length()) return 0;
  return db_linear[linear_index]->record;
}

bool CAdPlugDatabase::go_forward()
{
	if (!(DB_Bucket::linear_length() - linear_index - 1))
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
	if (DB_Bucket::linear_length())
		linear_index = 0;
}

void CAdPlugDatabase::goto_end()
{
	if (DB_Bucket::linear_length())
		linear_index = DB_Bucket::linear_length() - 1;
}

unsigned long CAdPlugDatabase::make_hash(CKey const &key)
{
	return (key.crc32 + key.crc16) % hash_radix;
}

/***** CAdPlugDatabase::DB_Bucket *****/

unsigned long CAdPlugDatabase::DB_Bucket::mainindex = 0;

CAdPlugDatabase::DB_Bucket::DB_Bucket(CRecord *newrecord, DB_Bucket *newchain)
  : index(mainindex), deleted(false), chain(newchain), record(newrecord)
{
  mainindex++;
}

CAdPlugDatabase::DB_Bucket::~DB_Bucket()
{
  if(!deleted) delete record;
}

unsigned long CAdPlugDatabase::DB_Bucket::linear_length()
{
  return mainindex;
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
  : type(Plain), filetype(CFileType::Undefined)
{
}

CAdPlugDatabase::CRecord::~CRecord()
{
}

CAdPlugDatabase::CRecord *CAdPlugDatabase::CRecord::factory(binistream &in)
{
  RecordType	type;
  unsigned long	size;
  CRecord	*rec;

  type = (RecordType)in.readByte(); size = in.readDWord();
  rec = factory(type);

  if(rec) {
    rec->key.crc16 = in.readWord(); rec->key.crc32 = in.readDWord();
    rec->filetype = (CFileType::FileType)in.readWord();
    rec->read_own(in);
    return rec;
  } else {
    // skip this record, cause we don't know about it
    in.seek(size + 6 + 2, binio::Add);
    return 0;
  }
}

void CAdPlugDatabase::CRecord::write(binostream &out)
{
  out.writeByte(type); out.writeDWord(get_size());
  out.writeWord(key.crc16); out.writeDWord(key.crc32);
  out.writeWord(filetype);

  write_own(out);
}

unsigned long CAdPlugDatabase::CRecord::get_size()
{
  return 0;
}

/***** CAdPlugDatabase::CRecord::CKey *****/

CAdPlugDatabase::CKey::CKey(binistream &buf)
{
  make(buf);
}

bool CAdPlugDatabase::CKey::operator==(const CKey &key)
{
  return ((crc16 == key.crc16) && (crc32 == key.crc32));
}

void CAdPlugDatabase::CKey::make(binistream &buf)
// Key is CRC16:CRC32 pair. CRC16 and CRC32 calculation routines (c) Zhengxi
{
  static const unsigned short magic16 = 0xa001;
  static const unsigned long  magic32 = 0xedb88320;

  crc16 = 0; crc32 = ~0;

  while(!buf.eof())
    {
      unsigned char byte = buf.readByte();

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
}

/***** CInfoRecord *****/

CInfoRecord::CInfoRecord()
{
  type = SongInfo;
}

void CInfoRecord::read_own(binistream &in)
{
  title = in.readString();
  author = in.readString();
}

void CInfoRecord::write_own(binostream &out)
{
  out.writeString(title); out.writeByte('\0');
  out.writeString(author); out.writeByte('\0');
}

unsigned long CInfoRecord::get_size()
{
  return title.length() + author.length() + 2;
}

/***** CClockRecord *****/

CClockRecord::CClockRecord()
  : clock(0.0f)
{
  type = ClockSpeed;
}

void CClockRecord::read_own(binistream &in)
{
  clock = in.readFloat();
}

void CClockRecord::write_own(binostream &out)
{
  out.writeFloat(clock);
}

unsigned long CClockRecord::get_size()
{
  return sizeof(binio::Float);
}
