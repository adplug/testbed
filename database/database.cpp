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
 * database.cpp - AdPlug database class by Riven the Mage <riven@ok.ru>
 *
 */

#include <fstream.h>
#include <memory.h>

#include "database.h"

CAdPlugDatabase::CAdPlugDatabase()
{
	memset(db_linear,0,sizeof(db_bucket *) * HASH_RADIX);
	memset(db_hashed,0,sizeof(db_bucket *) * HASH_RADIX);

	linear_index = 0;
	linear_length = 0;
	linear_logic_length = 0;
}

CAdPlugDatabase::~CAdPlugDatabase()
{
	for (long i=0;i<linear_length;i++)
		delete db_linear[i];
}

bool CAdPlugDatabase::load(const char *db_name)
{
	ifstream f(db_name, ios::in | ios::binary);

	if (!f.is_open())
		return false;

	// read id
	char id[35];

	f.read(id,35);

	if (memcmp(id,"AdPlug Module Information Database\x10",35))
		return false;

	// read length
	long length;

	f.read((char *)&length,4);

	// read records
	for (long i=0;i<length;i++)
	{
		db_record record;

		f.read((char *)&record,sizeof(db_record));
		insert(&record);
	}

	f.close();

	return true;
}

bool CAdPlugDatabase::save(const char *db_name)
{
	ofstream f(db_name, ios::out | ios::trunc | ios::binary);

	if (!f.is_open())
		return false;

	// write id
	f.write("AdPlug Module Information Database\x10",35);

	// write length
	f.write((char *)&linear_logic_length,4);

	// write records
	for (long i=0;i<linear_length;i++)
		if (!db_linear[i]->deleted)
			f.write((char *)&db_linear[i]->record,sizeof(db_record));

	f.close();

	return true;
}

bool CAdPlugDatabase::search(unsigned char *key)
{
	long index = make_hash(key);

	if (!db_hashed[index])
		return false;

	// immediate hit ?
	db_bucket *bucket = db_hashed[index];

	if (!memcmp(bucket->record.key,key,6))
	{
		linear_index = bucket->index;
		return true;
	}

	// in-chain hit ?
	bucket = (db_bucket *)db_hashed[index]->chain;

	while (bucket)
	{
		if (!memcmp(bucket->record.key,key,6))
		{
			linear_index = bucket->index;
			return true;
		}

		bucket = (db_bucket *)bucket->chain;
	}

	return false;
}

bool CAdPlugDatabase::insert(db_record *record)
{
	if (linear_length == HASH_RADIX)
		return false;

	if (search(record->key))
		return false;

	// make bucket
	db_bucket *bucket = new db_bucket;

	if (!bucket)
		return false;

	memcpy(&bucket->record,record,sizeof(db_record));

	bucket->index   = linear_length;
	bucket->deleted = false;
	bucket->chain   = NULL;

	// add to linear list
	db_linear[linear_length] = bucket;

	linear_length++;
	linear_logic_length++;

	// add to hashed list
	long index = make_hash(record->key);

	if (!db_hashed[index])
		db_hashed[index] = bucket;
	else
	{
		db_bucket *chain = db_hashed[index];

		while (chain->chain)
			chain = (db_bucket *)chain->chain;

		chain->chain = bucket;
	}

	return true;
}

void CAdPlugDatabase::wipe()
{
	if (linear_length)
	{
		if (!db_linear[linear_index]->deleted)
			linear_logic_length--;

		db_linear[linear_index]->deleted = true;
	}
}

void CAdPlugDatabase::unwipe()
{
	if (linear_length)
	{
		if (db_linear[linear_index]->deleted)
			linear_logic_length++;

		db_linear[linear_index]->deleted = false;
	}
}

bool CAdPlugDatabase::get_record(db_record *record)
{
	if (linear_length)
	{
		memcpy(record,&db_linear[linear_index]->record,sizeof(db_record));
		return true;
	}

	return false;
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

void CAdPlugDatabase::make_key(unsigned char *buffer, long buffer_size, unsigned char *key)
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

/* -------- Private Methods ------------------------------- */

long CAdPlugDatabase::make_hash(unsigned char *key)
{
	return (((*(unsigned long *)&key[0]) + (*(unsigned short *)&key[4])) % HASH_RADIX);
}
