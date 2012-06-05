CPP     = g++
CPPOPTS = -g -O3 -Wall

all: driver

driver: util.o hash.o simhash.o driver.cpp
	$(CPP) $(CPPOPTS) -o driver driver.cpp simhash.o hash.o util.o -lJudy

util.o: util.h util.cpp
	$(CPP) $(CPPOPTS) -o util.o -c util.cpp

hash.o: hash.h hash.cpp
	$(CPP) $(CPPOPTS) -o hash.o -c hash.cpp

simhash.o: simhash.h simhash.cpp
	$(CPP) $(CPPOPTS) -o simhash.o -c simhash.cpp

clean:
	rm -rf driver *.o
