/*
 * CAdPlugDatabase class tester
 */

#include <fstream.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "database.h"

int main(int argc, char* argv[])
{
	char temp[256];

	printf("AdPlug Database Console, (c) Riven the Mage <riven@ok.ru>\n\n");

	if (argc == 1)
	{
		printf("usage: testdb.exe <command> [file]\n");
		printf("\ncommands:\n");
		printf("    add         add file info to database\n");
		printf("    list        view database\n");
		printf("    resolve     try to resolve file info from database\n\n");
		return 1;
	}

	CAdPlugDatabase mydb;

	CAdPlugDatabase::db_record record;

	memset(&record,0,sizeof(CAdPlugDatabase::db_record));

	if (argc > 2)
	{
		ifstream f(argv[2], ios::in | ios::binary);
		if (!f.is_open())
		{
			printf("error: can't open specified file\n");
			return 1;
		}
		f.seekg(0,ios::end);
		long file_size = f.tellg();
		f.seekg(0);
		unsigned char *file = new unsigned char [file_size];
		f.read(file,file_size);
		f.close();

		CAdPlugDatabase::make_key(file,file_size,record.key);
		printf("key: 0x%04X:0x%08X\n\n",*(unsigned short *)&record.key[4],*(unsigned long  *)&record.key[0]);

		delete file;
	}

	mydb.load("adplug.db");

	if (!strcmp(argv[1],"add"))
	{
		printf("type: "); scanf("%i",&record.type); gets(temp);
		printf("title: "); gets(record.title);
		printf("author: "); gets(record.author);
		printf("user data: "); gets((char *)record.data);

		mydb.insert(&record);

		mydb.save("adplug.db");
	}
	else if (!strcmp(argv[1],"list"))
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
	}
	else
		printf("error: unknown command.\n");
}
