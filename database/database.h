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
 * database.h - AdPlug database class by Riven the Mage <riven@ok.ru>
 *
 */

#ifndef H_DATABASE
#define H_DATABASE

#define HASH_RADIX	0xfff1		// should be prime

class CAdPlugDatabase
{
public:
	CAdPlugDatabase();

	virtual ~CAdPlugDatabase();

#pragma pack(1)
	typedef struct
	{
		unsigned char	key[6];
		unsigned int	type;
		char            title[64];
		char            author[64];
		unsigned char	data[32];
	} db_record;
#pragma pack()

	bool	load(const char *);
	bool	save(const char *);

	bool    insert(db_record *);

	void	wipe();
	void	unwipe();

	bool	search(unsigned char *);

	bool	get_record(db_record *);

	bool	go_forward();
	bool	go_backward();

	void	goto_begin();
	void	goto_end();

	static	void	make_key(unsigned char *, long, unsigned char *);

private:
	typedef struct
	{
		unsigned long	index;
		bool            deleted;
		void            *chain;

		db_record       record;
	} db_bucket;

	db_bucket	*db_linear[HASH_RADIX];
	db_bucket	*db_hashed[HASH_RADIX];

	long		linear_index;
	long		linear_length;
	long		linear_logic_length;

	long		make_hash(unsigned char *);
};

#endif
