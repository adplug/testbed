#CXX = g++-3.2

testdb: testdb.o database.o binio.o binfile.o binwrap.o
	$(CXX) -o $@ $^

testdb.o: testdb.cpp database.h
database.o: database.cpp database.h
binio.o: binio.cpp binio.h
binwrap.o: binwrap.cpp binwrap.h
binfile.o: binfile.cpp binfile.h
