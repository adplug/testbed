#CXX = g++-3.2

testdb: testdb.o database.o
	$(CXX) -o $@ $^ -lbinio

testdb.o: testdb.cpp database.h
database.o: database.cpp database.h
