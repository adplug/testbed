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
 * maintdb.cpp - AdPlug database maintenance utility
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#include <fstream.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "database.h"

int main(int argc, char* argv[])
{
	printf("AdPlug database maintenance utility, (c) Riven the Mage <riven@ok.ru>\n\n");

	if (argc == 1)
	{
		printf("usage: testdb.exe <command> [file]\n");
		printf("\ncommands:\n");
		printf("    add         add file info to database\n");
		printf("    list        view database\n");
		printf("    resolve     try to resolve file info from database\n\n");
		return 1;
	}

	CAdPlugDatabase			mydb;
	CAdPlugDatabase::CRecord::Key	key;

	// Operate on a file
	if (argc > 2)
	{
		ifstream f(argv[2], ios::in | ios::binary);

		if (!f.is_open())
		{
			printf("error: can't open specified file\n");
			return 1;
		}

		// Generate key from file
		f.seekg(0,ios::end);
		long file_size = f.tellg();
		f.seekg(0);
		unsigned char *file = new unsigned char [file_size];
		f.read(file,file_size);
		f.close();
		CAdPlugDatabase::make_key(file,file_size,key);
		delete file;

		printf("key: 0x%04X:0x%08lX\n\n",*(unsigned short *)&key[4],*(unsigned long *)&key[0]);
	}

	mydb.load("adplug.db");

	if (!strcmp(argv[1],"add"))
	{
	  CAdPlugDatabase::CRecord::RecordType	type;
	  CAdPlugDatabase::CRecord		*record;

	  printf("type: "); scanf("%u",(unsigned int *)&type);

	  record = CAdPlugDatabase::CRecord::factory(type);
	  record->key = key;

	  /*	  printf("title: "); gets(record->title);
	  printf("author: "); gets(record->author);
	  printf("user data: "); gets((char *)record->data); */

	  if(!mydb.insert(record)) delete record;
	  mydb.save("adplug.db");
	}
	/*	else if (!strcmp(argv[1],"list"))
	{
		mydb.goto_begin();

		do
		{
			mydb.get_record(&record);

			printf("type: %i\n",record.type);
			printf("title: %s\n",record.title);
			printf("author: %s\n",record.author);
			printf("user data: %s\n",record.data);
			printf("\n");
		}
		while (mydb.go_forward());
	}
	else if (!strcmp(argv[1],"resolve"))
	{
		if (mydb.search(record.key))
		{
			mydb.get_record(&record);

			printf("type: %i\n",record.type);
			printf("title: %s\n",record.title);
			printf("author: %s\n",record.author);
			printf("user data: %s\n",record.data);
		}
		else
			printf("no info in database about this file.\n");
			} */
	else
		printf("error: unknown command.\n");
}
