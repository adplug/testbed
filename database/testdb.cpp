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

#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "binfile.h"
#include "database.h"

static void show_record(CAdPlugDatabase::CRecord *record)
{
  printf("type: %i\n", record->type);
  printf("key: 0x%X:0x%lX\n", record->key.crc16, record->key.crc32);
  printf("FileType: %u\n", record->filetype);

  CInfoRecord *inforec = (CInfoRecord *)record;
  CClockRecord *clockrec = (CClockRecord *)record;

  switch(record->type) {
  case CAdPlugDatabase::CRecord::Plain: break;
  case CAdPlugDatabase::CRecord::SongInfo:
    cout << "Title: " << inforec->title << endl;
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

static CAdPlugDatabase::CKey make_key_from_file(const char *filename)
{
  binifstream f(filename);

  if(!f.is_open()) {
    puts("error: can't open specified file");
    exit(EXIT_FAILURE);
  }

  CAdPlugDatabase::CKey key(f);
  printf("File \"%s\" key: 0x%X:0x%lX\n", filename, key.crc16, key.crc32);
  return key;
}

int main(int argc, char* argv[])
{
  CAdPlugDatabase	mydb;

  puts("AdPlug database maintenance utility");
  puts("Copyright (c) 2002 Riven the Mage <riven@ok.ru>");
  puts("Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>\n");

  if (argc == 1) {
    printf("usage: %s <command> [file]\n", argv[0]);
    printf("\ncommands:\n");
    printf("    add         add file info to database\n");
    printf("    list        view database\n");
    printf("    resolve     try to resolve file info from database\n\n");
    return 1;
  }

  mydb.load("adplug.db");

  if(argc > 2 && !strcmp(argv[1],"add")) {
    CAdPlugDatabase::CKey key = make_key_from_file(argv[2]);
    CAdPlugDatabase::CRecord::RecordType type;
    CAdPlugDatabase::CRecord *record;
    CInfoRecord *inforec;
    CClockRecord *clockrec;
    char tmpstr[256];

    if(mydb.lookup(key)) {
      puts("Error: File already in database!");
      exit(EXIT_FAILURE);
    }

    cout << "type: "; cin >> (unsigned int)type; cin.ignore();
    record = CAdPlugDatabase::CRecord::factory(type);
    record->key = key;

    switch(type) {
    case CAdPlugDatabase::CRecord::Plain: break;
    case CAdPlugDatabase::CRecord::SongInfo:
      inforec = (CInfoRecord *)record;
      cout << "Title: "; cin.getline(tmpstr, 256); inforec->title = tmpstr;
      cout << "Author: "; cin.getline(tmpstr, 256); inforec->author = tmpstr;
      break;
    case CAdPlugDatabase::CRecord::ClockSpeed:
      clockrec = (CClockRecord *)record;
      cout << "Clockspeed: "; cin >> clockrec->clock;
      break;
    default:
      puts("Error: Unknown database record!");
      break;
    }

    if(!mydb.insert(record)) delete record;
    mydb.save("adplug.db");
  } else
    if(!strcmp(argv[1],"list")) {
      mydb.goto_begin();

      do {
	CAdPlugDatabase::CRecord *record = mydb.get_record();
	show_record(record);
	printf("\n");
      } while(mydb.go_forward());
    } else
      if(argc > 2 && !strcmp(argv[1],"resolve")) {
	CAdPlugDatabase::CRecord *record = mydb.search(make_key_from_file(argv[2]));
	if(record)
	  show_record(record);
	else
	  puts("Error: No info in database about this file.");
      } else
	puts("Error: Unknown command or missing argument(s).");
}
