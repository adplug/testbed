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

CAdPlugDatabase	mydb;

bool make_key_from_file(const char *filename, CAdPlugDatabase::CRecord::Key key)
/* This should be placed into CAdPlugDatabase directly... */
{
  ifstream f(filename, ios::in | ios::binary);
  if (!f.is_open()) return false;

  // Generate key from file
  f.seekg(0,ios::end);
  long file_size = f.tellg();
  f.seekg(0);
  unsigned char *file = new unsigned char [file_size];
  f.read(file,file_size);
  CAdPlugDatabase::make_key(file,file_size,key);
  delete file;

  return true;
}

void show_record(CAdPlugDatabase::CRecord *record)
{
  printf("type: %i\n", record->type);
  printf("size: %lu\n", record->size);
  printf("key: ...\n");
  printf("FileType: %u\n", record->filetype);

  CInfoRecord *inforec = (CInfoRecord *)record;
  CClockRecord *clockrec = (CClockRecord *)record;

  switch(record->type) {
  case CAdPlugDatabase::CRecord::Plain: break;
  case CAdPlugDatabase::CRecord::SongInfo:
    cout << "title: " << inforec->title << endl;
    cout << "Author: " << inforec->author << endl;
    break;
  case CAdPlugDatabase::CRecord::ClockSpeed:
    printf("Clock speed: %.2f\n", clockrec->clock);
    break;
  default:
    puts("Error: Unknown database entry!");
    break;
  }
}

int main(int argc, char* argv[])
{
	puts("AdPlug database maintenance utility");
	puts("Copyright (c) 2002 Riven the Mage <riven@ok.ru>");
	puts("Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>\n");

	if (argc == 1)
	{
		printf("usage: %s <command> [file]\n", argv[0]);
		printf("\ncommands:\n");
		printf("    add         add file info to database\n");
		printf("    list        view database\n");
		printf("    resolve     try to resolve file info from database\n\n");
		return 1;
	}

	CAdPlugDatabase::CRecord::Key key;

	// Operate on a file
	if (argc > 2)
	  if(!make_key_from_file(argv[2], key))
	    printf("error: can't open specified file\n");

	mydb.load("adplug.db");

	if (!strcmp(argv[1],"add"))
	{
	  CAdPlugDatabase::CRecord::RecordType	type;
	  CAdPlugDatabase::CRecord		*record;
	  CInfoRecord *inforec;
	  CClockRecord *clockrec;

	  printf("type: "); scanf("%u",(unsigned int *)&type);

	  record = CAdPlugDatabase::CRecord::factory(type);
	  record->key = key;

	  switch(type) {
	  case CAdPlugDatabase::CRecord::Plain: break;
	  case CAdPlugDatabase::CRecord::SongInfo:
	    inforec = (CInfoRecord *)record;
	    cout << "Title: "; cin >> inforec->title;
	    cout << "Author: "; cin >> inforec->author;
	    break;
	  case CAdPlugDatabase::CRecord::ClockSpeed:
	    clockrec = (CClockRecord *)record;
	    cout << "Clockspeed: "; scanf("%f", &clockrec->clock);
	    break;
	  default:
	    puts("Error: Unknown database record!");
	    break;
	  }

	  if(!mydb.insert(record)) delete record;
	  mydb.save("adplug.db");
	}
	else if (!strcmp(argv[1],"list"))
	{
		mydb.goto_begin();

		do
		{
			CAdPlugDatabase::CRecord *record = mydb.get_record();
			show_record(record);
			printf("\n");
		}
		while (mydb.go_forward());
	}
	else if (!strcmp(argv[1],"resolve"))
	{
	  CAdPlugDatabase::CRecord *record = mydb.search(key);
	  if (record)
	    show_record(record);
	  else
	    printf("no info in database about this file.\n");
	}
	else
	  printf("error: unknown command.\n");
}
